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
 * Date: 2021-12-21 01:02:58
 * Author: Eric Yonng
 * Description: 并发优先级队列,分优先级处理各个队列,便于快速处理某些队列,避免因为其他队列过长而造成时效性问题
 *              优先级的某一等级的队列若是空的则会再swap出来以便连续不断的处理
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_CONCURRENT_PRIORITY_QUEUE_CONCURRENT_PRIORITY_QUEUE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_CONCURRENT_PRIORITY_QUEUE_CONCURRENT_PRIORITY_QUEUE_H__

#pragma once

#include <kernel/comp/LibList.h>
#include <kernel/comp/Lock/Impl/SpinLock.h>

KERNEL_BEGIN

template<typename Elem, typename BuildType = _Build::MT, typename LockType = SpinLock>
class ConcurrentPriorityQueue
{
    POOL_CREATE_TEMPLATE_OBJ_DEFAULT(ConcurrentPriorityQueue, Elem, BuildType, LockType);
    
public:
    ConcurrentPriorityQueue();
    ~ConcurrentPriorityQueue();

    // 线程不安全
public:
    bool SetMaxLevel(Int32 level);
    void Init()
    {
        if(_isInit.exchange(true))
            return;
            
        _elemAmount = 0;
        _guards.resize(_maxLevel + 1);
        for(Int32 idx = 0; idx <= _maxLevel; ++idx)
            _guards[idx] = CRYSTAL_NEW(LockType);

        _levelQueue.resize(_maxLevel + 1);
        for(Int32 idx = 0; idx <= _maxLevel; ++idx)
            _levelQueue[idx] = LibList<Elem, BuildType>::NewByAdapter_LibList(BuildType::V);
    }

    void Destroy()
    {
        if(!_isInit.exchange(false))
            return;

        const Int32 guardSize = static_cast<Int32>(_guards.size());
        for(Int32 idx = guardSize - 1; idx >= 0; --idx)
        {
            CRYSTAL_DELETE_SAFE(_guards[idx]);
            _guards.erase(_guards.begin() + idx);
        }

        const Int32 queueSize = static_cast<Int32>(_levelQueue.size());
        for(Int32 idx = queueSize - 1; idx >= 0; --idx)
        {
            if(_levelQueue[idx]->GetAmount() != 0)
            {
                CRYSTAL_TRACE("ConcurrentPriorityQueue has data left: level:%d, data amount:%llu", idx, _levelQueue[idx]->GetAmount());
            }

            LibList<Elem, BuildType>::DeleteByAdapter_LibList(BuildType::V, _levelQueue[idx]);
            _levelQueue.erase(_levelQueue.begin() + idx);
        }

        _elemAmount = 0;
    }

    // 线程安全接口
public:
    // queueOut 外部必须要提前创建好,内部只采用交换,提高性能 且BuildType请保持与list一致
    void SwapQueue(Int32 level, LibList<Elem, BuildType> *&queueOut);
    // push 时请保证,外部的BuildType和优先级队列的buildType一致
    void PushQueue(Int32 level, LibList<Elem, BuildType> *queue);
    void PushQueue(Int32 level, Elem e);
    // queuesOut 外部必须要提前创建好,内部只采用交换,提高性能 且BuildType请保持与list一致
    void SwapAll(LibList<LibList<Elem, BuildType> *, BuildType> *&queuesOut)
    {
        if(UNLIKELY(static_cast<Int32>(queuesOut->GetAmount()) != (_maxLevel + 1)))
        {
            ASSERT(false);
            return;
        }

        auto curNode = queuesOut->Begin();
        for(Int32 idx = 0; idx <= _maxLevel; ++idx)
        {
            _guards[idx]->Lock();
            auto queue = _levelQueue[idx];
            _elemAmount -= queue->GetAmount();
            _elemAmount += curNode->_data->GetAmount();
            _levelQueue[idx] = curNode->_data;
            curNode->_data = queue;
            _guards[idx]->Unlock();
            curNode = (curNode->_next != NULL) ? curNode->_next : queuesOut->Begin();
        }
    }

    // queuesOut 外部必须要提前创建好,内部只采用交换,提高性能 且BuildType请保持与list一致
    void SwapAll(std::vector<LibList<Elem, BuildType> *> &queuesOut)
    {
        if(UNLIKELY(static_cast<Int32>(queuesOut.size()) != _maxLevel + 1))
        {
            ASSERT(false);
            return;
        }

        for(Int32 idx = 0; idx <= _maxLevel; ++idx)
        {
            _guards[idx]->Lock();
            auto queue = _levelQueue[idx];
            _elemAmount -= queue->GetAmount();
            _elemAmount += queuesOut[idx]->GetAmount();
            _levelQueue[idx] = queuesOut[idx];
            queuesOut[idx] = queue;
            _guards[idx]->Unlock();
        }
    }

    void SwapAllOutIfEmpty(std::vector<LibList<Elem, BuildType> *> &queuesOut)
    {
        if(UNLIKELY(static_cast<Int32>(queuesOut.size()) != _maxLevel + 1))
        {
            ASSERT(false);
            return;
        }

        for(Int32 idx = 0; idx <= _maxLevel; ++idx)
        {
            if(UNLIKELY(!queuesOut[idx]->IsEmpty()))
                continue;

            _guards[idx]->Lock();
            auto queue = _levelQueue[idx];
            _elemAmount -= queue->GetAmount();
            _levelQueue[idx] = queuesOut[idx];
            queuesOut[idx] = queue;
            _guards[idx]->Unlock();
        }
    }

    // queuesOut 外部必须要提前创建好,内部只采用交换,提高性能 且BuildType请保持与list一致
    UInt64 MergeTailAllTo(LibList<LibList<Elem, BuildType> *, BuildType> *&queuesOut)
    {
        if(UNLIKELY(static_cast<Int32>(queuesOut->GetAmount()) != (_maxLevel + 1)))
        {
            ASSERT(false);
            return 0;
        }

        ListNode<LibList<Elem, BuildType> *> *curNode = queuesOut->Begin();
        UInt64 mergeCount = 0;
        for(Int32 idx = 0; idx <= _maxLevel; ++idx)
        {
            _guards[idx]->Lock();
            const auto queuAmount = _levelQueue[idx]->GetAmount();
            _elemAmount -= queuAmount;
            mergeCount += queuAmount;
            curNode->_data->MergeTail(_levelQueue[idx]);
            _guards[idx]->Unlock();
            curNode = (curNode->_next != NULL) ? curNode->_next : queuesOut->Begin();
        }

        return mergeCount;
    }
    
    // queuesOut 外部必须要提前创建好,内部只采用交换,提高性能 且BuildType请保持与list一致
    UInt64 MergeTailAllTo(std::vector<LibList<Elem, BuildType> *> &queuesOut)
    {
        if(UNLIKELY(static_cast<Int32>(queuesOut.size()) != _maxLevel + 1))
        {
            ASSERT(false);
            return 0;
        }

        UInt64 mergeCount = 0;
        for(Int32 idx = 0; idx <= _maxLevel; ++idx)
        {
            _guards[idx]->Lock();
            const auto queuAmount = _levelQueue[idx]->GetAmount();
            _elemAmount -= queuAmount;
            mergeCount += queuAmount;
            queuesOut[idx]->MergeTail(_levelQueue[idx]);
            _guards[idx]->Unlock();
        }

        return mergeCount;
    }
    
    UInt64 MergeTailTo(ConcurrentPriorityQueue<Elem, BuildType, LockType> &other);

    Int32 GetMaxLevel() const;
    UInt64 GetAmount() const;
    bool IsEmpty() const;
    std::vector<LibList<Elem, BuildType> *> &GetQueue();
    const std::vector<LibList<Elem, BuildType> *> &GetQueue() const;
private:
    Int32 _maxLevel;
    std::vector<LockType *> _guards;
    std::atomic<UInt64> _elemAmount;
    std::vector<LibList<Elem, BuildType> *> _levelQueue;
    std::atomic_bool _isInit;
};

template<typename Elem, typename BuildType, typename LockType>
POOL_CREATE_TEMPLATE_OBJ_DEFAULT_IMPL(ConcurrentPriorityQueue, Elem, BuildType, LockType);


template<typename Elem, typename BuildType, typename LockType>
ALWAYS_INLINE ConcurrentPriorityQueue<Elem, BuildType, LockType>::ConcurrentPriorityQueue()
:_maxLevel(0)
, _elemAmount{0}
,_isInit{false}
{

}

template<typename Elem, typename BuildType, typename LockType>
ALWAYS_INLINE ConcurrentPriorityQueue<Elem, BuildType, LockType>::~ConcurrentPriorityQueue()
{
    Destroy();
}

template<typename Elem, typename BuildType, typename LockType>
ALWAYS_INLINE bool ConcurrentPriorityQueue<Elem, BuildType, LockType>::SetMaxLevel(Int32 level)
{
    auto oldLevel = _maxLevel;
    _maxLevel = level;

    if(LIKELY(oldLevel != _maxLevel))
    {
        _elemAmount = 0;
        _guards.resize(_maxLevel + 1);
        for(Int32 idx = 0; idx <= _maxLevel; ++idx)
        {
            if(_guards[idx])
                continue;

            _guards[idx] = CRYSTAL_NEW(LockType);
        }

        _levelQueue.resize(_maxLevel + 1);
        for(Int32 idx = 0; idx <= _maxLevel; ++idx)
        {
            if(_levelQueue[idx])
                continue;

            _levelQueue[idx] = LibList<Elem, BuildType>::NewByAdapter_LibList(BuildType::V);
        }
    }

    return true;
}

template<typename Elem, typename BuildType, typename LockType>
ALWAYS_INLINE void ConcurrentPriorityQueue<Elem, BuildType, LockType>::SwapQueue(Int32 level, LibList<Elem, BuildType> *&queueOut)
{
    _guards[level]->Lock();
    _elemAmount -= _levelQueue[level]->GetAmount();
    LibList<Elem, BuildType> *other = queueOut;
    _elemAmount += other->GetAmount();
    queueOut = _levelQueue[level];
    _levelQueue[level] = other;
    _guards[level]->Unlock();
}

template<typename Elem, typename BuildType, typename LockType>
ALWAYS_INLINE void ConcurrentPriorityQueue<Elem, BuildType, LockType>::PushQueue(Int32 level, LibList<Elem, BuildType> *queue)
{
    _guards[level]->Lock();
    _elemAmount += queue->GetAmount();
    _levelQueue[level]->MergeTail(queue);
    _guards[level]->Unlock();
}

template<typename Elem, typename BuildType, typename LockType>
ALWAYS_INLINE void ConcurrentPriorityQueue<Elem, BuildType, LockType>::PushQueue(Int32 level, Elem e)
{
    _guards[level]->Lock();
    _elemAmount += 1;
    _levelQueue[level]->PushBack(e);
    _guards[level]->Unlock();
}


template<typename Elem, typename BuildType, typename LockType>
ALWAYS_INLINE UInt64 ConcurrentPriorityQueue<Elem, BuildType, LockType>::MergeTailTo(ConcurrentPriorityQueue<Elem, BuildType, LockType> &other)
{
    return MergeTailAllTo(other._levelQueue);
}

template<typename Elem, typename BuildType, typename LockType>
ALWAYS_INLINE Int32 ConcurrentPriorityQueue<Elem, BuildType, LockType>::GetMaxLevel() const
{
    return _maxLevel;
}

template<typename Elem, typename BuildType, typename LockType>
ALWAYS_INLINE UInt64 ConcurrentPriorityQueue<Elem, BuildType, LockType>::GetAmount() const
{
    return _elemAmount;
}

template<typename Elem, typename BuildType, typename LockType>
ALWAYS_INLINE bool ConcurrentPriorityQueue<Elem, BuildType, LockType>::IsEmpty() const
{
    return _elemAmount == 0;  
}

template<typename Elem, typename BuildType, typename LockType>
ALWAYS_INLINE std::vector<LibList<Elem, BuildType> *> &ConcurrentPriorityQueue<Elem, BuildType, LockType>::GetQueue()
{
    return _levelQueue;
}

template<typename Elem, typename BuildType, typename LockType>
ALWAYS_INLINE const std::vector<LibList<Elem, BuildType> *> &ConcurrentPriorityQueue<Elem, BuildType, LockType>::GetQueue() const
{
    return _levelQueue;
}

KERNEL_END

#endif
