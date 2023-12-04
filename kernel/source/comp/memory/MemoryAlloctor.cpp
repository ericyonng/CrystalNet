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
#include <kernel/comp/memory/MemoryBuffer.h>
#include <kernel/comp/memory/MemoryDefs.h>
#include <kernel/comp/memory/GarbageCollector.h>
#include <kernel/comp/memory/MergeMemoryBufferInfo.h>
#include <kernel/comp/Tls/TlsDefaultObj.h>

#include<kernel/comp/memory/MemoryAlloctor.h>
#include <kernel/comp/memory/EmptyBufferLimitType.h>
#include <kernel/comp/memory/MemoryBlock.h>
#include <kernel/comp/memory/Defs/MemoryAlloctorConfig.h>
#include <kernel/comp/Log/log.h>
#include <kernel/comp/memory/CenterMemoryCollector.h>

#include <kernel/comp/Utils/TlsUtil.h>
#include <kernel/comp/Utils/BackTraceUtil.h>
#include <kernel/comp/Utils/SystemUtil.h>

KERNEL_BEGIN

ALWAYS_INLINE bool MemoryAlloctor::CheckOwner(const MemoryBlock *block) const
{
    return block->_buffer && (block->_buffer->_alloctor == this);
}

ALWAYS_INLINE MemoryBuffer *MemoryAlloctor::_NewBuffer()
{
    MemoryBuffer *new_buffer = new MemoryBuffer(this->_blockSizeAfterAlign, this->_curBlockCntPerBuffer, this->_unitBytesToAlloc, this);
    new_buffer->Init();

    // 下一次block cnt是*2
    UInt64 blockNum = _curBlockCntPerBuffer << 1;
    _curBlockCntPerBuffer = blockNum > _bufferBlockNumLimit ? _bufferBlockNumLimit : blockNum;
    _totalBytes += new_buffer->_bufferSize;
    _totalBlockAmount += new_buffer->_blockCnt;

    // 占用总大小
    if(_isThreadLocalCreate)
        _tlsDefaultObj->_alloctorTotalBytes += new_buffer->_bufferSize;

    ++_curActiveBufferNum;
    return new_buffer;
}

ALWAYS_INLINE void MemoryAlloctor::_GcBuffer(MemoryBuffer *buffer)
{
    // 降低内存分配占用, 这个时候说明内存不那么紧张了 TODO:测试
    _curBlockCntPerBuffer >>= 1;
    _curBlockCntPerBuffer = _curBlockCntPerBuffer >= _initMinBlockCntPerBuffer ? _curBlockCntPerBuffer : _initMinBlockCntPerBuffer;

    // 数据更新
    _totalBytes -= buffer->_bufferSize;
    _totalBlockAmount -= buffer->_blockCnt;

    if(_isThreadLocalCreate)
        _tlsDefaultObj->_alloctorTotalBytes -= buffer->_bufferSize;
    
    --_curActiveBufferNum;

    _gc->push(buffer);
}

ALWAYS_INLINE void MemoryAlloctor::_AddToList(MemoryBuffer *&listHead, MemoryBuffer *buffer)
{
    buffer->_next = listHead;
    
    if (LIKELY(listHead))
        listHead->_pre = buffer;

    listHead = buffer;
}

ALWAYS_INLINE void MemoryAlloctor::_RemoveFromList(MemoryBuffer *&listHead, MemoryBuffer *buffer)
{
    MemoryBuffer *preBuffer = buffer->_pre;
    MemoryBuffer *nextBuffer = buffer->_next;

    if(LIKELY(preBuffer))
        preBuffer->_next = nextBuffer;
    if(LIKELY(nextBuffer))
        nextBuffer->_pre = preBuffer;

    if (UNLIKELY(listHead == buffer))
        listHead = nextBuffer;

    buffer->_pre = NULL;
    buffer->_next = NULL;
}

ALWAYS_INLINE void MemoryAlloctor::_JudgeBufferRecycle(MemoryBuffer *buffer)
{
    // 空闲buffer 且_notEnableGcFlag为0 或者强制释放buffer时候gc掉
    bool checkGc = (buffer->_usedBlockCnt == 0) && ((buffer->_notEnableGcFlag == 0) || (_isThreadLocalCreate && _tlsDefaultObj->_isForceFreeIdleBuffer));
    if(UNLIKELY(checkGc))
    {
        if(UNLIKELY(buffer->_isInBusy))
            _RemoveFromList(_busyHead, buffer);
        else
            _RemoveFromList(_activeHead, buffer);

        _GcBuffer(buffer);
        return;
    }

    // 若在busy中放回active队列
    if(UNLIKELY(buffer->_isInBusy))
    {
        _RemoveFromList(_busyHead, buffer);
        _AddToList(_activeHead, buffer);
        buffer->_isInBusy = false;
    }
}

ALWAYS_INLINE void MemoryAlloctor::_DoFreeBlock(MemoryBlock *block)
{
    // 回收
    auto buffer = block->_buffer;
    buffer->FreeBlock(block);
    
    --_blockCountInUsed;
    _bytesInUsed -= _blockSizeAfterAlign;

    // 判断可以回收就丢给gc, 没有判断是否切换忙或者活跃队列
    _JudgeBufferRecycle(buffer);

    // 合并内存
    if(LIKELY(_isThreadLocalCreate))
        _MergeBlocks();
}

ALWAYS_INLINE void MemoryAlloctor::_MergeBlocks()
{
    if(LIKELY(_needMerge.load() == false))
        return;

    // 先进行交换
    _toMergeLck.Lock();
    auto toSwap = _mergeBufferList;
    _mergeBufferList = _mergeBufferListSwap;
    _mergeBufferListSwap = toSwap;
    _needMerge = false;
    _bufferToMergeCount = 0;
    _toMergeLck.Unlock();

    // 遍历并吧内存归并 为了避免频繁归并内存, 跨线程的时候请使用跨线程版本的分配内存, 生命周期只在单线程的, 使用线程本地版本的分配内存
    MergeMemoryBufferInfo *bufferList = _mergeBufferList;
    MergeMemoryBufferInfo *bufferListTail = NULL;
    UInt64 count = 0;
    for(;_mergeBufferList;)
    {
        _MergeBlocks(_mergeBufferList);

        bufferListTail = _mergeBufferList;
        _mergeBufferList = _mergeBufferList->_next;
        ++count;
    }

    _Recycle(count, bufferList, bufferListTail);

    // 没有需要合并的就把自己移除
    if(LIKELY(!_needMerge))
        _RemoveSelfFromDurtyList();
}

ALWAYS_INLINE void MemoryAlloctor::_MergeBlocks(MergeMemoryBufferInfo *memoryBufferInfo)
{
    auto buffer = memoryBufferInfo->_buffer;
    auto mergeBlockCount = memoryBufferInfo->_count;
    auto mergeBlockHead = memoryBufferInfo->_head;
    auto mergeBlockEnd= memoryBufferInfo->_tail;

    // 回收
    buffer->FreeBlockList(mergeBlockHead, mergeBlockEnd, mergeBlockCount);
    
    _blockCountInUsed -= mergeBlockCount;
    _bytesInUsed -= (_blockSizeAfterAlign * mergeBlockCount);

    _JudgeBufferRecycle(buffer);
}

ALWAYS_INLINE void MemoryAlloctor::_Recycle(UInt64 recycleNum, MergeMemoryBufferInfo *bufferInfoListHead, MergeMemoryBufferInfo *bufferInfoListTail)
{
    _centerMemroyCollector->Recycle(recycleNum, bufferInfoListHead, bufferInfoListTail);
}

ALWAYS_INLINE void MemoryAlloctor::_RemoveSelfFromDurtyList()
{
    _tlsDefaultObj->_lck->Lock();
    _tlsDefaultObj->_durtyList->erase(this);
    _tlsDefaultObj->_lck->Unlock();
}

ALWAYS_INLINE void MemoryAlloctor::_AddSelfToDurtyListIfNotIn()
{
    // 如果不在队列中就加入
    _tlsDefaultObj->_lck->Lock();
    _tlsDefaultObj->_durtyList->insert(this);
    _tlsDefaultObj->_lck->Unlock();
}

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
,_gc(new GarbageCollector())
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

    CRYSTAL_DELETE_SAFE(_gc);
}

void *MemoryAlloctor::Alloc(UInt64 objBytes)
{
    // 从活跃列表中取
    auto memoryBuffer = _activeHead;
    if(UNLIKELY(!memoryBuffer))
    {
        memoryBuffer = _NewBuffer();
        _trigerNewBufferWhenAlloc += 1;
        _AddToList(_activeHead, memoryBuffer);
    }

    // block 参数设置
    auto block = memoryBuffer->AllocNewBlock();
    block->_ref = 1;
    block->_realUseBytes = objBytes;
    ++_blockCountInUsed;
    _bytesInUsed += _blockSizeAfterAlign;

    // 一般情况不会相等
    if(LIKELY(memoryBuffer->_blockCnt ^ memoryBuffer->_usedBlockCnt))
        return ((Byte8 *)block) + _memoryBlockHeadSize;

    // memory buffer 被分配光放到busy链表
    // 从active队列移除
    _RemoveFromList(_activeHead, memoryBuffer);
    
    // 加入busy队列
    _AddToList(_busyHead, memoryBuffer);
    memoryBuffer->_isInBusy = true;

    // 归并内存
    if(LIKELY(_isThreadLocalCreate))
        _MergeBlocks();
    
    return ((Byte8 *)block) + _memoryBlockHeadSize;
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

void MemoryAlloctor::PushMergeList(UInt64 memoryBuffMergeNum, MergeMemoryBufferInfo *head, MergeMemoryBufferInfo *tail)
{
    _toMergeLck.Lock();
    _needMerge = true;
    _bufferToMergeCount += memoryBuffMergeNum;

    // 合并链表
    tail->_next = _mergeBufferListSwap;
    _mergeBufferListSwap = head;
    _toMergeLck.Unlock();

    // 不在脏队列中则加入脏队列 放在tls中提供上层去处理合并事情
    if(LIKELY(_needMerge))
        _AddSelfToDurtyListIfNotIn();
}

KERNEL_END

