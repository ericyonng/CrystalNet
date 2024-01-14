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
 * Author: Eric Yonng
 * Date: 2021-02-08 10:03:33
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MESSAGE_QUEUE_MESSAGE_QUEUE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MESSAGE_QUEUE_MESSAGE_QUEUE_H__

#pragma once

#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/Lock/Impl/SpinLock.h>
#include <kernel/comp/LibList.h>
#include <kernel/comp/Utils/ContainerUtil.h>

KERNEL_BEGIN

// 生产者持有,消费者只引用
template<typename Elem, typename BuildType = _Build::MT, typename LockType = SpinLock>
class MessageQueue
{
    POOL_CREATE_TEMPLATE_OBJ_DEFAULT(MessageQueue, Elem, BuildType, LockType);

public:
    MessageQueue();
    virtual ~MessageQueue();

public:
    void Clear();
    template<typename DeleElemMethod>
    void Clear(DeleElemMethod &&cb)
    {
        _lck.Lock();
        ContainerUtil::DelContainer(*_queue, std::forward<DeleElemMethod>(cb));
        _lck.Unlock();
    }

    // 60ns左右
    void PushBack(Elem e);
    void MergeTail(LibList<Elem, BuildType> *elems);
    bool PopFront(Elem &e);
    template<typename T>
    bool PopFront(T &e)
    {
        if(UNLIKELY(_queue->IsEmpty()))
            return false;
        
        _lck.Lock();
        e = T(_queue->Begin()->_data);
        _queue->PopFront();
        _lck.Unlock();

        return true;
    }

    // 35ns左右
    void SwapQueue(LibList<Elem, BuildType> *&elems);
    void PopTo(LibList<Elem, BuildType> *&elems);
    UInt64 GetAmount() const;

private:
    LockType _lck;
    std::atomic<UInt64> _queueCount;
    LibList<Elem, BuildType> *_queue;
};

template<typename Elem, typename BuildType, typename LockType>
POOL_CREATE_TEMPLATE_OBJ_DEFAULT_IMPL(MessageQueue, Elem, BuildType, LockType);

template<typename Elem, typename BuildType, typename LockType>
POOL_CREATE_TEMPLATE_OBJ_DEFAULT_TL_IMPL(MessageQueue, Elem, BuildType, LockType);

template<typename Elem, typename BuildType, typename LockType>
ALWAYS_INLINE MessageQueue<Elem, BuildType, LockType>::MessageQueue()
:_queueCount{0}
,_queue(LibList<Elem, BuildType>::NewByAdapter_LibList(BuildType::V))
{

}

template<typename Elem, typename BuildType, typename LockType>
ALWAYS_INLINE MessageQueue<Elem, BuildType, LockType>::~MessageQueue()
{
    _lck.Lock();
    if(LIKELY(_queue))
    {
        LibList<Elem, BuildType>::DeleteByAdapter_LibList(BuildType::V, _queue);
        _queue = NULL;
    }
    _queueCount.store(0, , std::memory_order_release);
    _lck.Unlock();
}

template<typename Elem, typename BuildType, typename LockType>
ALWAYS_INLINE void MessageQueue<Elem, BuildType, LockType>::Clear()
{
    _lck.Lock();
    _queue->Clear();
    _queueCount.store(0, , std::memory_order_release);
    _lck.Unlock();
}

template<typename Elem, typename BuildType, typename LockType>
ALWAYS_INLINE void MessageQueue<Elem, BuildType, LockType>::PushBack(Elem e)
{
    _lck.Lock();
    _queue->PushBack(e);
    _queueCount.fetch_add(1, std::memory_order_release);
    _lck.Unlock();
}

template<typename Elem, typename BuildType, typename LockType>
ALWAYS_INLINE void MessageQueue<Elem, BuildType, LockType>::MergeTail(LibList<Elem, BuildType> *elems)
{
    _lck.Lock();
    _queueCount.fetch_add(elems->GetAmount(), std::memory_order_release);
    _queue->MergeTail(elems);
    _lck.Unlock();
}

template<typename Elem, typename BuildType, typename LockType>
ALWAYS_INLINE bool MessageQueue<Elem, BuildType, LockType>::PopFront(Elem &e)
{
    if(UNLIKELY(_queue->IsEmpty()))
        return false;
    
    _lck.Lock();
    e = _queue->Begin()->_data;
    _queue->PopFront();
    _queueCount.fetch_sub(1, std::memory_order_release);
    _lck.Unlock();

    return true;
}


template<typename Elem, typename BuildType, typename LockType>
ALWAYS_INLINE void MessageQueue<Elem, BuildType, LockType>::SwapQueue(LibList<Elem, BuildType> *&elems)
{
    _lck.Lock();
    auto temp = _queue;
    _queue = elems;
    elems = temp;
    _queueCount.exchange(_queue->GetAmount(), std::memory_order_release);
    _lck.Unlock();
}

template<typename Elem, typename BuildType, typename LockType>
ALWAYS_INLINE void MessageQueue<Elem, BuildType, LockType>::PopTo(LibList<Elem, BuildType> *&elems)
{
    _lck.Lock();
    elems->MergeTail(_queue);
    _queueCount.store(0, , std::memory_order_release);
    _lck.Unlock();
}

template<typename Elem, typename BuildType, typename LockType>
ALWAYS_INLINE UInt64 MessageQueue<Elem, BuildType, LockType>::GetAmount() const
{
    return _queueCount.load(std::memory_order_acquire);
}

KERNEL_END

#endif