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

KERNEL_BEGIN

MemoryAlloctor::MemoryAlloctor(const MemoryAlloctorConfig &config)
:_unitBytesToAlloc(config._unitBytes)
, _blockSizeAfterAlign( __MEMORY_ALIGN__(config._unitBytes + __MEMORY_ALIGN__(sizeof(MemoryBlock))) )
, _bufferBlockNumLimit(config._bufferBlockNumLimit)
, _memoryBlockHeadSize(__MEMORY_ALIGN__(sizeof(MemoryBlock)))
, _initMinBlockCntPerBuffer(MEMORY_BUFFER_BLOCK_INIT)
, _curBlockCntPerBuffer(MEMORY_BUFFER_BLOCK_INIT)
,_blockCountInUsed(0)
,_bytesInUsed(0)
,_totalBytes(0)
,_totalBlockAmount(0)
,_threadId(0)
,_isInit{false}

, _activeBuffersHead(NULL)
, _curActiveBufferNum(0)
, _deadBlockStackHead(NULL)

, _deadBufferHead(NULL)
,_deadBufferTail(NULL)
, _gcBeginNode(NULL)
, _deadBufferLimit(config._deadBufferLimit)
,_halfDeadBufferLimit(config._deadBufferLimit >> 1)
,_isCreateBufferWhenInit(config._isCreateBufferWhenInit)
,_deadBufferCount(0)
,_trigerGcCond(MEMORY_GC_COND_PARAM)
,_curOpCount(0)
,_isDuringGc(false)
{
}

MemoryAlloctor::~MemoryAlloctor()
{
    Destroy();
}

void *MemoryAlloctor::Alloc(UInt64 objBytes)
{
    // 先判断死亡区内存
    MemoryBlock *newBlock = NULL;
    MemoryBuffer *buffer = NULL;
    if(_deadBlockStackHead)
    {
        // 从栈中取内存块
        newBlock = _deadBlockStackHead->pop();
        buffer = newBlock->_buffer;

        // 从列表中移除
        if (UNLIKELY(_deadBlockStackHead->empty()))
        {
            _deadBlockStackHead->_isInStackList = false;
            _deadBlockStackHead = _deadBlockStackHead->_next;
            if (LIKELY(_deadBlockStackHead))
                _deadBlockStackHead->_pre = NULL;

            // 没有可用内存则扔到gc区
            if (UNLIKELY(_activeBuffersHead && !_activeBuffersHead->_head))
            {
                MemoryBuffer *transfer = _activeBuffersHead;
                _RemoveFromActive(_activeBuffersHead);

                if (!transfer->_usedBlockCnt)
                {
                   _AddToDeadBufferList(transfer);
                    ++_deadBufferCount;
                    if(_deadBufferCount > _halfDeadBufferLimit)
                    {
                        // 前面缓冲多了要往前移
                        if(LIKELY(_gcBeginNode))
                        {
                            _totalBytes -= _gcBeginNode->_bufferSize;
                            _totalBlockAmount -= _gcBeginNode->_blockCnt;

                            // gc一次降低一倍
                            _curBlockCntPerBuffer >>= 1;
                            _curBlockCntPerBuffer = (_curBlockCntPerBuffer < _initMinBlockCntPerBuffer) ?_initMinBlockCntPerBuffer:_curBlockCntPerBuffer;

                            _gcBeginNode = _gcBeginNode->_pre;
                        }
                    }

                    _UpdateGCStatus();
                }
            }
        }
    }
    else
    {
        buffer = _activeBuffersHead;
        if (LIKELY(buffer))
            newBlock = buffer->AllocNewBlock();

        // 活跃区没有内存要向死亡区取
        if (UNLIKELY(!newBlock))
        {
            // 没可用内存且没占用的 buffer从活跃区中移除 并转移到gc区
//             if (UNLIKELY(buffer && !buffer->_usedBlockCnt))
//             {
//                 _RemoveFromActive(buffer);
//                 _AddToDeadBufferList(buffer);
//             }

            buffer = _deadBufferHead;
            if (LIKELY(!buffer))    // 死亡区没内存 新建一块内存
            {
                // TODO:有性能消耗（里面有for循环）已采用惰性初始化解决
                buffer = _NewBuffer();
                
                // 加入到活跃区
                 _AddToActiveHead(buffer);

                // 取内存
                newBlock = buffer->AllocNewBlock();
            }
                
            else  // 死亡区有内存调到deadBlockStack区
            {
                _deadBufferHead = _deadBufferHead->_next;
                if (!_deadBufferHead)
                    _deadBufferTail = NULL;
                else
                    _deadBufferHead->_pre = NULL;

                --_deadBufferCount;

                // 低于gc条件则不需要gc
                if(_deadBufferCount <= _halfDeadBufferLimit)
                {
                    if (LIKELY(_gcBeginNode))
                    {
                        _totalBytes += _gcBeginNode->_bufferSize;
                        _totalBlockAmount += _gcBeginNode->_blockCnt;

                        // 还原一倍
                        _curBlockCntPerBuffer <<= 1;
                        _curBlockCntPerBuffer = _curBlockCntPerBuffer>_bufferBlockNumLimit?_bufferBlockNumLimit:_curBlockCntPerBuffer;

                    }

                    _gcBeginNode = NULL;
                }

                if(LIKELY(_gcBeginNode))
                {
                    _totalBytes += _gcBeginNode->_bufferSize;
                    _totalBlockAmount += _gcBeginNode->_blockCnt;

                    // 还原一倍
                    _curBlockCntPerBuffer <<= 1;
                    _curBlockCntPerBuffer = _curBlockCntPerBuffer>_bufferBlockNumLimit?_bufferBlockNumLimit:_curBlockCntPerBuffer;
                    
                    _gcBeginNode = _gcBeginNode->_next;
                }

                //_ReduceDeadEmptyBufferCount();
                _UpdateGCStatus();

                // 加入到deadBlockStack区
                auto bufferDeadArea = buffer->_deadArea;
                bufferDeadArea->_isInStackList = true;
                bufferDeadArea->_next = _deadBlockStackHead;
                if(LIKELY(_deadBlockStackHead))
                    _deadBlockStackHead->_pre = bufferDeadArea;

                newBlock = bufferDeadArea->pop();
            }
        }
    }

//     if (!newBlock)
//         return NULL;

    // 给内存
    buffer->_idleBlockCount -= 1;
    buffer->_usedBlockCnt += 1;
    buffer->_historyOpNum += 1;

    ASSERT(newBlock->_ref == 0);
    newBlock->_ref += 1;
    newBlock->_objBytes = objBytes;
    newBlock->_next = NULL;
    ++_blockCountInUsed;
    _bytesInUsed += _blockSizeAfterAlign;
    ++_curOpCount;
    
    // gc
    _UpdateGCStatus();
    _GC();

    return ((Byte8 *)newBlock) + _memoryBlockHeadSize;
}

void MemoryAlloctor::Free(void *ptr)
{
    // 内存块头
    Byte8 *toFree = reinterpret_cast<Byte8 *>(ptr);
    MemoryBlock *block = reinterpret_cast<MemoryBlock *>(reinterpret_cast<Byte8 *>(toFree - _memoryBlockHeadSize));

    // 判断是不是属于当前分配器
    if(UNLIKELY(!CheckOwner(block)))
    {
        auto buffer = block->_buffer;
        MemoryAlloctor *bufferAlloctor = buffer ? buffer->_alloctor : NULL;
        
        g_Log->Error(LOGFMT_OBJ_TAG("check owner fail, free a error block:%p, not belong to this alloctor:%p, %s, block owner buffer:%p, buffer owner alloctor:%p, buffer owner alloctor info:%s, \n backtrace:%s")
                                , block, this, ToString().c_str(), buffer, bufferAlloctor, bufferAlloctor? bufferAlloctor->ToString().c_str() : "", BackTraceUtil::CrystalCaptureStackBackTrace().c_str());

        // CRYSTAL_TRACE("free error block =[%p], not belong to this alloc [%p], buffer owner = [%p], buffer alloc =[%p], alloctor info:%s"
        // , block, this, buffer, buffer ? buffer->_alloctor:NULL, bufferAlloctor ? bufferAlloctor->ToString().c_str():"");
        // ASSERT(!"memory alloctor fatal error:free other allocter memory block!");
        return;
    }

    // 引用计数 TODO:重复释放ref为小于0
    ASSERT(block->_ref != 0);
    --block->_ref;
    if(UNLIKELY(block->_ref != 0))
        return;

    // 更新buffer
    MemoryBuffer *buffer = block->_buffer;
    --buffer->_usedBlockCnt;
    ++buffer->_idleBlockCount;
    buffer->_historyOpNum += 1;

    ++_curOpCount;

    // 被释放的节点插到free链表头
    LibStack<MemoryBlock *, MemoryBuffer> *deadArea = buffer->_deadArea;
    deadArea->push(block);

    // 在死亡队列中先从队列中移除
    if(LIKELY(deadArea->_isInStackList))
    {
        // 断开
        _RemoveFromDeadBlockStackList(deadArea);
    }

    // 有可用内存且释放了一个block放到deadStack队列,或者没有可用内存但是从gc区调整到deadStack区的被占用的需要加入到队列
    if (buffer->_head || buffer->_usedBlockCnt)
    {
        // 放到队列头
        deadArea->_isInStackList = true;
        deadArea->_next = _deadBlockStackHead;
        if(LIKELY(_deadBlockStackHead))
           _deadBlockStackHead->_pre = deadArea;
        _deadBlockStackHead = deadArea;
    }

    // 没可用内存放到gc区
    else
    {
        // 将buffer弹出活跃队列
        if(LIKELY(buffer->_isInList))
            _RemoveFromActive(buffer);

        // 放到队列头便于即时被使用
        buffer->_next = _deadBufferHead;
        if (LIKELY(_deadBufferHead))
        {
            _deadBufferHead->_pre = buffer;
        }
        else
        {
            _deadBufferTail = buffer;
        }

        _deadBufferHead = buffer;

        ++_deadBufferCount;
        if(_deadBufferCount >= _halfDeadBufferLimit)
        {
            // 前面缓冲多了要往前移
            if(LIKELY(_gcBeginNode))
            {
                _totalBytes -= _gcBeginNode->_bufferSize;
                _totalBlockAmount -= _gcBeginNode->_blockCnt;

                // gc一次降低一倍
                _curBlockCntPerBuffer >>= 1;
                _curBlockCntPerBuffer = _curBlockCntPerBuffer<_initMinBlockCntPerBuffer?_initMinBlockCntPerBuffer:_curBlockCntPerBuffer;
                
                _gcBeginNode = _gcBeginNode->_pre;
            }
        }

        // _AddDeadEmptyBufferCount();
        _UpdateGCStatus();
    }

    --_blockCountInUsed;
    _bytesInUsed -= _blockSizeAfterAlign;

    // gc
    _GC();
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
    // _curBlockCntPerBuffer = initBlockNumPerBuffer;

    // if(UNLIKELY(_isCreateBufferWhenInit))
    // {
    //     MemoryBuffer *newBuffer = _NewBuffer();
    //     _AddToActiveHead(newBuffer);
    // }
    _lck.Unlock();
}

void MemoryAlloctor::Destroy()
{
    if(!_isInit.exchange(false))
        return;
    
    _lck.Lock();
    MemoryBuffer *head = _activeBuffersHead;
    while (_activeBuffersHead)
    {
        head = _activeBuffersHead->_next;
        CRYSTAL_DELETE_SAFE(_activeBuffersHead); 
        _activeBuffersHead = head;
    }

    head = _deadBufferHead;
    while (_deadBufferHead)
    {
        head = _deadBufferHead->_next;
        CRYSTAL_DELETE_SAFE(_deadBufferHead); 
        _deadBufferHead = head;
    }

    _blockCountInUsed = 0;
    _bytesInUsed = 0;
    _curActiveBufferNum = 0;
    _deadBlockStackHead = NULL;
    _deadBufferTail = NULL;
    _deadBufferCount = 0;
    _isDuringGc = false;
    _lck.Unlock();
}

KERNEL_END

