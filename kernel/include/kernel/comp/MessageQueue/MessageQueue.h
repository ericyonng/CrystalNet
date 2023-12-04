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

#include <atomic>
#include <list>

KERNEL_BEGIN

struct MessageBlock;
class ConditionLocker;

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

ALWAYS_INLINE bool MessageQueue::IsWorking() const
{
    return _isWorking.load();
}

ALWAYS_INLINE bool MessageQueue::Push(MessageBlock *msg)
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

ALWAYS_INLINE void MessageQueue::PopImmediately(std::list<MessageBlock *> *&swapList)
{
    _lck.Lock();
    _PopImmediately(swapList);
    _lck.Unlock();
}

ALWAYS_INLINE bool MessageQueue::HasMsg()
{
    return _hasMsg.load();
}

ALWAYS_INLINE UInt64 MessageQueue::GetMsgCount()
{
    _lck.Lock();
    auto count = _msgQueue->size();
    _lck.Unlock();

    return count;
}

ALWAYS_INLINE void MessageQueue::_PopImmediately(std::list<MessageBlock *> *&swapList)
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