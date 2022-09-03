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

#include <kernel/kernel_inc.h>
#include <kernel/comp/Lock/Lock.h>
#include <kernel/comp/Utils/ContainerUtil.h>
#include <kernel/comp/MessageQueue/MessageBlock.h>

KERNEL_BEGIN

struct MessageBlock;

// 生产者持有,消费者只引用
class KERNEL_EXPORT MessageQueue
{
    POOL_CREATE_OBJ_DEFAULT(MessageQueue);

public:
    MessageQueue();
    virtual ~MessageQueue();

public:
    Int32 Start();

    void OnekeyClose();
    bool HalfClose();
    void FinishClose();
    bool IsWorking() const;

public:
    void Attach(ConditionLocker *consumer);
    bool Push(std::list<MessageBlock *> *&swapList);
    bool Push(MessageBlock *msg);
    Int32 TimeWait(std::list<MessageBlock *> *&swapList, UInt64 timeoutMs);
    Int32 Wait(std::list<MessageBlock *> *&swapList);
    void PopImmediately(std::list<MessageBlock *> *&swapList);
    void Sinal();
    bool HasMsg();
    UInt64 GetMsgCount();

private:
    void _PopImmediately(std::list<MessageBlock *> *&swapList);
    
private:
    std::atomic_bool _isAttach = {false};
    ConditionLocker *_consumer = NULL;
    SpinLock _lck;
    std::atomic_bool _hasMsg = {false};
    std::list<MessageBlock *> *_msgQueue = CRYSTAL_NEW(std::list<MessageBlock *>);

    /* 系统参数 */
    std::atomic_bool _isWorking = {false};
    std::atomic_bool _isStart = {false};
};

inline Int32 MessageQueue::Start()
{
    if(_isStart.exchange(true))
        return Status::Success;

    _isWorking.store(true);
    if(!_consumer)
    {
        _isAttach = false;
        _consumer = CRYSTAL_NEW(ConditionLocker);
    }

    return Status::Success;
}

inline void MessageQueue::OnekeyClose()
{
    if(!HalfClose())
        return;

    FinishClose();
}

inline bool MessageQueue::HalfClose()
{
    if (!_isStart)
        return false;

    if (!_isWorking.exchange(false))
        return false;

    _consumer->Broadcast();

    return true;
}

inline void MessageQueue::FinishClose()
{
    if(!_isStart.exchange(false))
        return;

    UInt64 unhandleAmount = _msgQueue->size();
    ContainerUtil::DelContainer<MessageBlock *, AutoDelMethods::Release>(*_msgQueue);
    CRYSTAL_TRACE("message queue has %llu msg unhandlerd", unhandleAmount);

    CRYSTAL_DELETE_SAFE(_msgQueue);
    if(_isAttach)
        CRYSTAL_DELETE_SAFE(_consumer);
}

inline bool MessageQueue::IsWorking() const
{
    return _isWorking.load();
}

inline void MessageQueue::Attach(ConditionLocker *consumer)
{
    if(UNLIKELY(_consumer && _isAttach))
        CRYSTAL_DELETE_SAFE(_consumer);

    _consumer = consumer;
    _isAttach = true;
}

inline bool MessageQueue::Push(std::list<MessageBlock *> *&swapList)
{
    if(UNLIKELY(!_isWorking))
        return false;

    _lck.Lock();

    if(_hasMsg)
    {// 追加
        for(auto iterMsg = swapList->begin(); iterMsg != swapList->end();)
        {
            _msgQueue->push_back(*iterMsg);
            iterMsg = swapList->erase(iterMsg);
        }
    }
    else
    {// 交换
        std::list<MessageBlock *> *temp = NULL;
        temp = swapList;
        swapList = _msgQueue;
        _msgQueue = temp;
        _hasMsg = true;
    }

    _lck.Unlock();

    Sinal();

    return true;
}

inline bool MessageQueue::Push(MessageBlock *msg)
{
    if(UNLIKELY(!_isWorking))
        return false;

    _lck.Lock();
    _hasMsg = true;
    _msgQueue->push_back(msg);
    _lck.Unlock();

    Sinal();

    return true;
}

inline Int32 MessageQueue::TimeWait(std::list<MessageBlock *> *&swapList, UInt64 timeoutMs)
{
    _consumer->Lock();
    Int32 ret = Status::Success;
    if(!_hasMsg)
        ret = _consumer->TimeWait(timeoutMs);
    _consumer->Unlock();

    _lck.Lock();
    _PopImmediately(swapList);
    _lck.Unlock();

    return ret;
}

inline Int32 MessageQueue::Wait(std::list<MessageBlock *> *&swapList)
{
    _consumer->Lock();
    Int32 ret = Status::Success;
    if(!_hasMsg)
        ret = _consumer->Wait();
    _consumer->Unlock();

    _lck.Lock();
    _PopImmediately(swapList);
    _lck.Unlock();
    return ret;
}

inline void MessageQueue::PopImmediately(std::list<MessageBlock *> *&swapList)
{
    _lck.Lock();
    _PopImmediately(swapList);
    _lck.Unlock();
}

inline void MessageQueue::Sinal()
{
    _consumer->Sinal();
}

inline bool MessageQueue::HasMsg()
{
    return _hasMsg.load();
}

inline UInt64 MessageQueue::GetMsgCount()
{
    _lck.Lock();
    auto count = _msgQueue->size();
    _lck.Unlock();

    return count;
}

inline void MessageQueue::_PopImmediately(std::list<MessageBlock *> *&swapList)
{
    if(!_hasMsg.exchange(false))
        return;

    std::list<MessageBlock *> *temp = NULL;
    temp = swapList;
    swapList = _msgQueue;
    _msgQueue = temp;
}

KERNEL_END

#endif