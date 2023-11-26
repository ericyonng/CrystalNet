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
,_isThreadLocalCreate(false)

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

void MemoryAlloctor::Free(MemoryBlock *block)
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
        return;

    // 判断是不是属于当前分配器
    if(UNLIKELY(!CheckOwner(block)))
    {
        auto buffer = block->_buffer;
        MemoryAlloctor *bufferAlloctor = buffer ? buffer->_alloctor : NULL;
        if(LIKELY(bufferAlloctor))
        {
            // 只有都是threadlocal且同一个线程下才能直接处理,其他情况都丢给中央收集器
            if(_isThreadLocalCreate && bufferAlloctor->_isThreadLocalCreate && (bufferAlloctor->_threadId == _threadId))
            {
                // 直接释放
                bufferAlloctor->_DoFreeBlock(block);
                return;
            }

            if(!bufferAlloctor->_isThreadLocalCreate)
                ++block->_ref;

            // 当前free操作调用线程独占所以之和中央收集器会有锁冲突,丢给中央收集器处理后续的操作
            _centerMemroyCollector->PushBlock(SystemUtil::GetCurrentThreadId(), block);
            return;
        }
        
        g_Log->Error(LOGFMT_OBJ_TAG("check owner fail not in any alloctor, free a error block:%p, not belong to this alloctor:%p, current alloctor info:\n %s,\n block owner buffer:%p, buffer owner alloctor:%p, buffer owner alloctor info:%s, \n backtrace:%s")
                                , block, this, ToString().c_str(), buffer, bufferAlloctor, bufferAlloctor? bufferAlloctor->ToString().c_str() : "", BackTraceUtil::CrystalCaptureStackBackTrace().c_str());
        
        throw std::logic_error("free block error not in any alloctor");
        return;
    }

    // #if _DEBUG
    // if(g_Log->IsEnable(LogLevel::Debug))
    //     g_Log->Debug(LOGFMT_OBJ_TAG("block free:%p, block size:%llu, buffer info:%s alloctor info:%s"), block, _blockSizeAfterAlign, block->_buffer ? block->_buffer->ToString().c_str() : "no buffer", ToString().c_str());
    // #endif

    _DoFreeBlock(block);
}

LibString MemoryAlloctor::UsingInfo() const
{
    // 总的使用大小,总的大小,使用的block块数,总块数,总的buffer数量,使用的buffer数量,block大小,创建来源,总的使用字节数,总的字节数,当前死亡buffer数量,死亡buffer限制数量,分配与释放操作次数,是否初始化创建buffer,半数空缓存限制,当前活跃buffer数量
    LibString str;

    str << "alloctor init thread id:" << _threadId << ";\n"
        << "alloctor address:" << this <<  ";\n"
        << "block size:" << _blockSizeAfterAlign << ", create source:" << _createSource << "create memory buffer num when init:" << _initMemoryBufferNum <<  ";\n"
        << "current alloctor buffer total bytes:" << _totalBytes << ", current all using bytes:" << _bytesInUsed << ";\n"
        << "total block amount:" << _totalBlockAmount << ", using block:" << _blockCountInUsed << ";\n"
        << "current block count per buffer for next time:" << _curBlockCntPerBuffer << ", max block count limit per buffer:" << _bufferBlockNumLimit << ";\n"
        << "active buffer num:" << _curActiveBufferNum << ";\n"
        << "trigger new buffer when alloc block num:" << _trigerNewBufferWhenAlloc << ";\n"
        << "need to merge buffer count:" << _bufferToMergeCount.load() << ";\n"
        << "is thread local create alloctor:" << _isThreadLocalCreate << ";\n"
        ;
        
    return str;
}

void MemoryAlloctor::Init(bool isThreadLocalCreate, UInt64 initBlockNumPerBuffer, const std::string &source)
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
    _isThreadLocalCreate = isThreadLocalCreate;

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
    
    if(_isThreadLocalCreate)
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

