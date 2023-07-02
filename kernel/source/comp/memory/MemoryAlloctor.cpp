/*!
 *  MIT License
 *  
 *  Copyright (c) 2020 ericyonng<120453674@qq.com>
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE. 
 * 
 * Date: 2020-11-09 02:17:28
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include<kernel/comp/memory/MemoryAlloctor.h>
#include <kernel/comp/memory/EmptyBufferLimitType.h>
#include <kernel/comp/memory/MemoryBlock.h>
#include <kernel/comp/memory/MemoryDefs.h>
#include <kernel/comp/Utils/Utils.h>
#include <kernel/comp/memory/Defs/MemoryAlloctorConfig.h>
#include <kernel/comp/Log/log.h>
#include <kernel/comp/Utils/BackTraceUtil.h>
#include <kernel/comp/memory/CenterMemoryCollector.h>
#include <kernel/comp/Utils/TlsUtil.h>

KERNEL_BEGIN

MemoryAlloctor::MemoryAlloctor(const MemoryAlloctorConfig &config)
:_unitBytesToAlloc(config._unitBytes)
, _blockSizeAfterAlign( __MEMORY_ALIGN__(config._unitBytes + __MEMORY_ALIGN__(sizeof(MemoryBlock))) )
, _bufferBlockNumLimit(config._bufferBlockNumLimit)
, _memoryBlockHeadSize(__MEMORY_ALIGN__(sizeof(MemoryBlock)))
, _initMemoryBufferNum(config._initMemoryBufferNum)
, _initMinBlockCntPerBuffer(MEMORY_BUFFER_BLOCK_INIT)
, _curBlockCntPerBuffer(MEMORY_BUFFER_BLOCK_INIT)
,_blockCountInUsed(0)
,_bytesInUsed(0)
,_totalBytes(0)
,_totalBlockAmount(0)
,_trigerNewBufferWhenAlloc(0)
,_threadId(0)
,_isInit{false}

, _activeHead(NULL)
, _busyHead(0)
, _curActiveBufferNum(0)
,_needMerge{false}
,_bufferToMergeCount{0}
,_mergeBufferList(NULL)
,_mergeBufferListSwap(NULL)
,_centerMemroyCollector(NULL)
,_tlsDefaultObj(TlsUtil::GetDefTls())
{
    _centerMemroyCollector = CenterMemoryCollector::GetInstance();
}

MemoryAlloctor::~MemoryAlloctor()
{
    Destroy();
}

MemoryAlloctor *MemoryAlloctor::Free(MemoryBlock *block)
{
    // 引用计数 TODO:重复释放ref为小于0
    if(UNLIKELY(block->_ref == 0))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("block is repeated free please check block:%p, is in pool:%d, block size:%llu, buffer info:%s alloctor info:%s, stack trace:%s")
                    , block, block->_isInAlloctor, _blockSizeAfterAlign, block->_buffer ? block->_buffer->ToString().c_str() : "no buffer", ToString().c_str(), BackTraceUtil::CrystalCaptureStackBackTrace().c_str());
        throw std::logic_error("block is repeated free please check block");
    }

    // 引用计数
    if(UNLIKELY(--block->_ref > 0))
        return NULL;

    // 判断是不是属于当前分配器
    if(UNLIKELY(!CheckOwner(block)))
    {
        auto buffer = block->_buffer;
        MemoryAlloctor *bufferAlloctor = buffer ? buffer->_alloctor : NULL;
        if(LIKELY(bufferAlloctor))
        {
            // 同一个线程直接调用分配器释放
            if(bufferAlloctor->_threadId == _threadId)
            {
                // 由外部进行再次释放
                ++block->_ref;
                return bufferAlloctor;
            }
            else
            {// 不同线程需要丢给中央收集器 TODO
                _centerMemroyCollector->PushBlock(SystemUtil::GetCurrentThreadId(), block);
                return NULL;
            }
        }
        
        g_Log->Error(LOGFMT_OBJ_TAG("check owner fail not in any alloctor, free a error block:%p, not belong to this alloctor:%p, current alloctor info:\n %s,\n block owner buffer:%p, buffer owner alloctor:%p, buffer owner alloctor info:%s, \n backtrace:%s")
                                , block, this, ToString().c_str(), buffer, bufferAlloctor, bufferAlloctor? bufferAlloctor->ToString().c_str() : "", BackTraceUtil::CrystalCaptureStackBackTrace().c_str());
        
        throw std::logic_error("free block error not in any alloctor");
        return NULL;
    }

    auto buffer = block->_buffer;

    // 回收
    buffer->FreeBlock(block);
    
    --_blockCountInUsed;
    _bytesInUsed -= _blockSizeAfterAlign;

    // 判断可以回收就丢给gc, 没有判断是否切换忙或者活跃队列
    _JudgeBufferRecycle(buffer);

    // 合并内存
    _MergeBlocks();

    return NULL;
}

void MemoryAlloctor::Init(UInt64 initBlockNumPerBuffer, const std::string &source)
{
    if(_isInit.exchange(true))
        return;

    _lck.Lock();
    _createSource = source;
    _threadId = KernelGetCurrentThreadId();
    _curBlockCntPerBuffer = initBlockNumPerBuffer > _bufferBlockNumLimit ? _bufferBlockNumLimit : initBlockNumPerBuffer;
    if(UNLIKELY(!_curBlockCntPerBuffer))
        _curBlockCntPerBuffer = MEMORY_BUFFER_BLOCK_INIT;

    _initMinBlockCntPerBuffer = _curBlockCntPerBuffer;

    // 初始内存块数量
    for(UInt64 idx = 0; idx < _initMemoryBufferNum; ++idx)
    {
        MemoryBuffer *newBuffer = _NewBuffer();
        _AddToList(_activeHead, newBuffer);
    }

    _lck.Unlock();
}

void MemoryAlloctor::Destroy()
{
    if(!_isInit.exchange(false))
        return;
    
    _MergeBlocks();

    _lck.Lock();
    MemoryBuffer *head = _activeHead;
    while (_activeHead)
    {
        head = _activeHead->_next;
        CRYSTAL_DELETE_SAFE(_activeHead); 
        _activeHead = head;
    }

    head = _busyHead;
    while (_busyHead)
    {
        head = _busyHead->_next;
        CRYSTAL_DELETE_SAFE(_busyHead); 
        _busyHead = head;
    }

    _blockCountInUsed = 0;
    _bytesInUsed = 0;
    _curActiveBufferNum = 0;
    _lck.Unlock();

}

void MemoryAlloctor::_Recycle(UInt64 recycleNum, MergeMemoryBufferInfo *bufferInfoListHead, MergeMemoryBufferInfo *bufferInfoListTail)
{
    _centerMemroyCollector->Recycle(recycleNum, bufferInfoListHead, bufferInfoListTail);
}

KERNEL_END

