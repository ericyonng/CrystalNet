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
 * Author: Eric Yonng()
 * Date: 2021-03-13 16:51:11
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/MessageQueue/MessageQueue.h>
#include <kernel/comp/Utils/ContainerUtil.h>
#include <kernel/comp/MessageQueue/MessageBlock.h>
#include <kernel/comp/Lock/Impl/ConditionLocker.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(MessageQueue);

MessageQueue::MessageQueue()
{

}

MessageQueue::~MessageQueue()
{
    OnekeyClose();
}


Int32 MessageQueue::Start()
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

void MessageQueue::OnekeyClose()
{
    if(!HalfClose())
        return;

    FinishClose();
}

bool MessageQueue::HalfClose()
{
    if (!_isStart)
        return false;

    if (!_isWorking.exchange(false))
        return false;

    _consumer->Broadcast();

    return true;
}

void MessageQueue::FinishClose()
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

bool MessageQueue::Push(std::list<MessageBlock *> *&swapList)
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

Int32 MessageQueue::TimeWait(std::list<MessageBlock *> *&swapList, UInt64 timeoutMs)
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

Int32 MessageQueue::Wait(std::list<MessageBlock *> *&swapList)
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

void MessageQueue::Attach(ConditionLocker *consumer)
{
    if(UNLIKELY(_consumer && _isAttach))
        CRYSTAL_DELETE_SAFE(_consumer);

    _consumer = consumer;
    _isAttach = true;
}

void MessageQueue::Sinal()
{
    _consumer->Sinal();
}

KERNEL_END