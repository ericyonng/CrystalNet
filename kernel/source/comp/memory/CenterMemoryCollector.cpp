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
 * Date: 2023-07-01 23:57:23
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/memory/CenterMemoryCollector.h>
#include <kernel/comp/memory/CenterMemoryThreadInfo.h>
#include <kernel/comp/thread/thread.h>
#include <kernel/comp/Utils/ContainerUtil.h>
#include <kernel/comp/Utils/SystemUtil.h>
#include <kernel/comp/Log/log.h>

KERNEL_BEGIN

CenterMemoryCollector::CenterMemoryCollector()
:_isDestroy{false}
,_isWorking{false}
,_willQuitThreadCount{0}
,_totalRegisterThreadCount{0}
,_worker(NULL)
,_currentPendingBlockTotalNum{0}
,_historyPendingBlockTotalNum{0}
,_blockNumForPurgeLimit(128 * 1024)
,_workIntervalMs(100)
,_recycleMemoryBufferInfoCount{0}
,_historyRecycleMemoryBufferInfoCount{0}
,_recycleForPurgeLimit(128 * 1024)
,_head(NULL)
,_headToSwap(NULL)
,_allThreadMemoryAllocUpperLimit(4LLU * 1024LLU * 1024LLU * 1024LLU)    // 默认4GB
,_lastScanTotalAllocBytes(0)
,_threadAllocTopn(new BinaryArray<SmartPtr<CenterMemoryTopnThreadInfo>, CenterMemoryTopnThreadInfoComp>)
,_threadAllocTopnSwap(new BinaryArray<SmartPtr<CenterMemoryTopnThreadInfo>, CenterMemoryTopnThreadInfoComp>)
{
}

CenterMemoryCollector::~CenterMemoryCollector()
{
    if(_worker)
        CRYSTAL_RELEASE_SAFE(_worker);

    ContainerUtil::DelContainer(_threadIdRefThreadInfo);

    if(_threadAllocTopn)
        CRYSTAL_DELETE_SAFE(_threadAllocTopn);
    if(_threadAllocTopnSwap)
        CRYSTAL_DELETE_SAFE(_threadAllocTopnSwap);
}

CenterMemoryCollector *CenterMemoryCollector::GetInstance()
{
    static CenterMemoryCollector *g_collector = new CenterMemoryCollector();
    return g_collector;
}

void CenterMemoryCollector::Start()
{
    _isWorking = true;
    _worker = new LibThread();
    _worker->SetThreadName(KERNEL_NS::LibString().AppendFormat("CenterCollector"));
    _worker->AddTask(this, &CenterMemoryCollector::_OnWorker);
    _worker->Start();
}

void CenterMemoryCollector::WillClose()
{
    if(_isDestroy.exchange(true))
        return;

    if(_worker && _worker->HalfClose())
        _worker->FinishClose();
}

void CenterMemoryCollector::Close()
{

}

void CenterMemoryCollector::WaitClose()
{
    const auto threadId = SystemUtil::GetCurrentThreadId();
    if(_worker && threadId == _worker->GetTheadId())
        return;

    // 超过数量则自动关闭
    ++_willQuitThreadCount;
    if(_willQuitThreadCount >= _totalRegisterThreadCount)
        WillClose();

    while(IsWorking())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("waiting for center memory close current thread id:%llu..."), threadId);
        SystemUtil::ThreadSleep(1000);
    }

    g_Log->Info(LOGFMT_OBJ_TAG("center memory is closed current thread id:%llu..."), threadId);
}

void CenterMemoryCollector::OnThreadQuit(UInt64 threadId)
{
    auto threadInfo = GetThreadInfo(threadId);
    if(LIKELY(threadInfo))
        threadInfo->OnThreadWillQuit();
}

void CenterMemoryCollector::RegisterThreadInfo(UInt64 threadId, TlsStack<TlsStackSize::SIZE_1MB> *tlsStack)
{
    if(UNLIKELY(_isDestroy))
        return;

    _registerGuard.Lock();
    auto iter = _threadIdRefThreadInfo.find(threadId);
    if(iter == _threadIdRefThreadInfo.end())
    {
        auto newInfo = new CenterMemoryThreadInfo(threadId, this);
        newInfo->SetTlsStack(tlsStack);

        _threadIdRefThreadInfo.insert(std::make_pair(threadId, newInfo));

        if((_worker == NULL) || threadId != _worker->GetTheadId())
            ++_totalRegisterThreadCount;
    }
    _registerGuard.Unlock();
}

void CenterMemoryCollector::PushBlock(UInt64 freeThreadId, MemoryBlock *block)
{
    if(UNLIKELY(_isDestroy))
        return;

    auto iter = _threadIdRefThreadInfo.find(freeThreadId);
    if(UNLIKELY(iter == _threadIdRefThreadInfo.end()))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("thread id :%llu, not register to center memory collector please check."), freeThreadId);
        return;
    }

    iter->second->PushBlock(block);
    ++_currentPendingBlockTotalNum;
    ++_historyPendingBlockTotalNum;

    if(UNLIKELY(_currentPendingBlockTotalNum >= _blockNumForPurgeLimit))
        _guard.Sinal();
}

void CenterMemoryCollector::Recycle(UInt64 recycleNum, MergeMemoryBufferInfo *bufferInfoListHead, MergeMemoryBufferInfo *bufferInfoListTail)
{
    _recycleGuard.Lock();
    bufferInfoListTail->_next = _headToSwap;
    _headToSwap = bufferInfoListHead;
    _recycleGuard.Unlock();

    _recycleMemoryBufferInfoCount += recycleNum;
    _historyRecycleMemoryBufferInfoCount += recycleNum;

    if(UNLIKELY(_recycleMemoryBufferInfoCount >= _recycleForPurgeLimit))
        _guard.Sinal();
}

LibString CenterMemoryCollector::ToString() const
{
    if(_isDestroy)
        return "";

    const UInt64 pendingBlock = _currentPendingBlockTotalNum.load();
    const UInt64 historyBlock = _historyPendingBlockTotalNum.load();
    const UInt64 recycleBufferInfo = _recycleMemoryBufferInfoCount.load();
    const UInt64 historyRecycleBufferInfo = _historyRecycleMemoryBufferInfoCount.load();

    // 把排行榜换出来
    SmartPtr<BinaryArray<SmartPtr<CenterMemoryTopnThreadInfo>, CenterMemoryTopnThreadInfoComp>> topn = new BinaryArray<SmartPtr<CenterMemoryTopnThreadInfo>, CenterMemoryTopnThreadInfoComp>;
    _topnGuard.Lock();
    auto *swapTopn = topn.pop();
    topn = _threadAllocTopnSwap;
    _threadAllocTopnSwap = swapTopn;
    _topnGuard.Unlock();
    
    // 数据信息
    LibString info;
    info.AppendFormat("[CENTER MEMORY COLLECTOR BEGIN]\n");
    info.AppendFormat("center memory collector info: current block num:%llu, history num:%llu, recycle memory buffer info num:%llu, history num:%llu, all thread alloctor bytes limit:%llu, last alloc mem total bytes:%llu \n"
    , pendingBlock, historyBlock, recycleBufferInfo, historyRecycleBufferInfo, _allThreadMemoryAllocUpperLimit, _lastScanTotalAllocBytes);

    for(auto iter : _threadIdRefThreadInfo)
        info.AppendFormat("threadId:%llu, center memory collector info:%s\n", iter.first, iter.second->ToString().c_str());

    // 每个线程内存分配排行榜信息
    const Int32 count = static_cast<Int32>(topn->size());
    info.AppendFormat("thread alloc topn count:[%d]:\n", count);
    for(Int32 idx = 0; idx < count; ++idx)
    {
        auto elem = (*topn)[idx].AsSelf();
        info.AppendFormat("[%d] threadId:%llu, alloc bytes:%llu\n", idx + 1, elem->_threadId, elem->_totalAllocBytes);
    }

    info.AppendFormat("[CENTER MEMORY COLLECTOR END]\n");

    // 排行榜如果没更新再把数据放回去
    _topnGuard.Lock();
    if(_threadAllocTopnSwap->empty() && !topn->empty())
    {
        swapTopn = topn.pop();
        topn = _threadAllocTopnSwap;
        _threadAllocTopnSwap = swapTopn;
    }
    _topnGuard.Unlock();

    return info;
}

void CenterMemoryCollector::_OnWorker(LibThread *thread)
{
    _isWorking = true;
    while(!thread->IsDestroy())
    {
        _guard.Lock();
        _guard.TimeWait(_workIntervalMs);
        _guard.Unlock();

        _DoWorker();
    }

    _DoWorker();

    _OnThreadDown();

    _isWorking = false;
}

void CenterMemoryCollector::_DoWorker()
{
    _recycleGuard.Lock();
    _recycleMemoryBufferInfoCount = 0;
    auto toSwap = _head;
    _head = _headToSwap;
    _headToSwap = toSwap;
    _recycleGuard.Unlock();

    for(;_head;)
    {
        auto node = _head;
        _head = _head->_next;
        CRYSTAL_DELETE_SAFE(node);
    }

    _lastScanTotalAllocBytes = 0;

    _threadAllocTopn->clear();
    for(auto &iter : _threadIdRefThreadInfo)
    {
        auto info = iter.second;

        // 先合并
        auto mergedCount = info->MergeBlocks();

        // 再将每个MemoryAlloctor 回收的block push到各个alloctor 
        info->MergeToAlloctor();

        _lastScanTotalAllocBytes += info->GetAllocBytes();

        SmartPtr<CenterMemoryTopnThreadInfo> newInfo = new CenterMemoryTopnThreadInfo;
        newInfo->_threadId = iter.first;
        newInfo->_totalAllocBytes = info->GetAllocBytes();
        _threadAllocTopn->insert(newInfo);

        _currentPendingBlockTotalNum -= mergedCount;
    }

    // 更新排行榜
    _topnGuard.Lock();
    auto newTopn = _threadAllocTopn;
    _threadAllocTopn = _threadAllocTopnSwap;
    _threadAllocTopnSwap = newTopn; 
    _topnGuard.Unlock();

    // 设置是否强制释放空闲内存
    bool isForceFree = _lastScanTotalAllocBytes >= _allThreadMemoryAllocUpperLimit;
    for(auto &iter : _threadIdRefThreadInfo)
    {
        auto info = iter.second;
        info->SetForceFreeIdleBuffer(isForceFree);
    }
}

void CenterMemoryCollector::_OnThreadDown()
{
    for(auto iter : _threadIdRefThreadInfo)
    {
        // 主线程不清理
        if(iter.first == SystemUtil::GetCurProcessMainThreadId())
            continue;

        iter.second->OnCollectorThreadDown();
    }
}

UInt64 CenterMemoryCollector::GetWorkerThreadId() const
{
    return _worker ? _worker->GetTheadId() : 0;
}
KERNEL_END
