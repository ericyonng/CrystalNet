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
 * Date: 2021-03-21 17:50:30
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Event/EventManager.h>
#include <kernel/comp/Event/LibEvent.h>
#include <kernel/comp/Log/log.h>
#include <kernel/comp/Utils/BackTraceUtil.h>
#include <kernel/comp/Utils/RttiUtil.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(EventManager);


EventManager::~EventManager()
{
    _eventIdRefListenerOwners.clear();
    if(!_listeners.empty())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("have some listener not remove listen count:%d"), static_cast<Int32>(_listeners.size()));
    }
    
    for(_DelayedOps::iterator it = _delayedOps.begin();
        it != _delayedOps.end();
        it++)
    {
        _Op &op = *it;
        CRYSTAL_RELEASE_SAFE(op._listener._listenCallBack);
    }

    for(_ListenersMap::iterator mIt = _listeners.begin();
        mIt != _listeners.end();
        mIt++)
    {
        _Listeners listeners = mIt->second;
        for(_Listeners::iterator lIt = listeners.begin();
            lIt != listeners.end();
            lIt++)
        {
            _Listener &listener = *lIt;
            CRYSTAL_RELEASE_SAFE(listener._listenCallBack);
        }
    }

    _stubListeners.clear();
}

ListenerStub EventManager::AddListener(int id, IDelegate<void, LibEvent *> *listener, const ListenerStub &bindedStub /*= INVALID_LISTENER_STUB*/)
{
    if(id < 0 || listener == NULL)
    {
        CRYSTAL_RELEASE_SAFE(listener);
        return INVALID_LISTENER_STUB;
    }

    auto owner = listener->GetOwner();
    if(UNLIKELY(owner && IsListening(id, owner)))
    {// 重复监听就告警下
        const LibString ownerRtti = listener->GetOwnerRtti();
        g_Log->Warn(LOGFMT_OBJ_TAG("repeat add listen for owner, ev id:%d, obj:%p, %s stack back trace:\n%s"), id, owner, ownerRtti.c_str(), BackTraceUtil::CrystalCaptureStackBackTrace().c_str());
    }

    ListenerStub stub = INVALID_LISTENER_STUB;
    if(bindedStub != INVALID_LISTENER_STUB)
    {
        if(_SearchStub(bindedStub))
        {
            CRYSTAL_RELEASE_SAFE(listener);
            return INVALID_LISTENER_STUB;
        }

        stub = bindedStub;
    }
    else
        stub = ++_maxListenerStub;

    _Op op;
    op._op = EventManager::ADD;
    op._listener._evId = id;
    op._listener._listenCallBack = listener;
    op._listener._stub = stub;

    // 若监听时候正在fire状态则放入延迟队列（延迟到after fire）,且当前的监听会失效
    if(IsFiring())
    {
        _delayedOps.push_back(op);
        auto iterCount = _delayAddOpRefCount.find(id);
        if (iterCount == _delayAddOpRefCount.end())
            iterCount = _delayAddOpRefCount.insert(std::make_pair(id, 0)).first;

        ++(iterCount->second);
        return op._listener._stub;
    }

    if(ProcessEventOperation(op) != Status::Success)
    {
        CRYSTAL_RELEASE_SAFE(listener);
        return INVALID_LISTENER_STUB;
    }

    return op._listener._stub;
}

int EventManager::RemoveListener(int id)
{
    if(id <= 0)
        return Status::Error;

    _Op op;
    op._op = EventManager::REMOVE;
    op._listener._evId = id;

    // 正在fire则放入延迟队列，延迟执行
    if(IsFiring())
    {
        _delayedOps.push_back(op);
        return Status::Error;
    }

    return ProcessEventOperation(op);
}

int EventManager::RemoveListener(const ListenerStub &stub)
{
    _Op op;
    op._op = EventManager::REMOVE;
    op._listener._stub = stub;

    // 正在fire则放入延迟队列，延迟执行
    if(IsFiring())
    {
        _delayedOps.push_back(op);
        return Status::Error;
    }

    return ProcessEventOperation(op);
}

Int32 EventManager::FireEvent(LibEvent *event)
{
    BeforeFireEvent();

    Int32 res = FireEvResult::Fail;
    const Int32 evId = event->GetId();

    // 监听所有事件的回调
    auto iterAllEvs = _listeners.find(0);
    if(iterAllEvs != _listeners.end())
    {
        _Listeners &listeners = iterAllEvs->second;
        for(_Listeners::iterator lIt = listeners.begin();
            lIt != listeners.end();
            ++lIt)
        {
            _Listener &listener = *lIt;
            if(listener._listenCallBack)
                listener._listenCallBack->Invoke(event);
        }

        res = FireEvResult::Success;
    }
    else if(_delayAddOpRefCount.find(0) != _delayAddOpRefCount.end())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("attention: fire event[%d] with delay listen all events please check if it is a terrible design, it cant fire event when listen is not finish!"), evId);
    }

    res = FireEvResult::Fail;
    _ListenersMap::iterator mIt = _listeners.find(evId);
    if(mIt != _listeners.end())
    {
        _Listeners &listeners = mIt->second;
        for(_Listeners::iterator lIt = listeners.begin();
            lIt != listeners.end();
            ++lIt)
        {
            _Listener &listener = *lIt;
            if(listener._listenCallBack)
                listener._listenCallBack->Invoke(event);
        }

        res = FireEvResult::Success;
    }
    else if(_delayAddOpRefCount.find(evId) != _delayAddOpRefCount.end())
    {
        // 延迟监听,延迟fire TODO:
        _Op op;
        op._op = EventManager::FIRE;
        op._fireInfo._ev = event;
        _delayedOps.push_back(op);
        --_firing;

        g_Log->Warn(LOGFMT_OBJ_TAG("attention: fire event[%d] with delay listen please check if it is a terrible design, it cant fire event when listen is not finish!"), evId);
        return FireEvResult::Asyn;
    }

    AfterFireEvent();

    if(!event->IsDontDelAfterFire())
        event->Release();

    return res;
}

bool EventManager::_SearchStub(const ListenerStub &stub) const
{
    return _stubListeners.find(stub) != _stubListeners.end();
}

int EventManager::ProcessEventOperation(EventManager::_Op &op)
{
    _Listener &listener = op._listener;
    if(op._op == EventManager::ADD)
    {// 加入事件操作
        _ListenersMap::iterator mIt = _listeners.find(listener._evId);
        if(mIt == _listeners.end())
            mIt = _listeners.insert(std::make_pair(listener._evId, std::vector<_Listener>())).first;

        _Listeners &listeners = mIt->second;
        listeners.push_back(listener);

        _stubListeners.insert(std::make_pair(listener._stub, listener));

        if(listener._listenCallBack)
        {
            auto owner = listener._listenCallBack->GetOwner();
            if(LIKELY(owner))
            {
                auto iter = _eventIdRefListenerOwners.find(listener._evId);
                if(iter == _eventIdRefListenerOwners.end())
                    iter = _eventIdRefListenerOwners.insert(std::make_pair(listener._evId, std::set<void *>())).first;

                iter->second.insert(owner);
            }
        }

        // 扣除异步次数
        auto iterCount = _delayAddOpRefCount.find(listener._evId);
        if (iterCount != _delayAddOpRefCount.end())
        {
            --(iterCount->second);
            if (iterCount->second <= 0)
                _delayAddOpRefCount.erase(iterCount);
        }
    }
    else if(op._op == EventManager::REMOVE)
    {// 移除事件
        if(listener._evId >= 0)
        {
            _delayAddOpRefCount.erase(listener._evId);
            _ListenersMap::iterator mIt = _listeners.find(listener._evId);
            if(mIt == _listeners.end())
                return Status::Error;

            // 移除
            _Listeners &listeners = mIt->second;
            for(_Listeners::iterator lIt = listeners.begin();
                lIt != listeners.end();
                ++lIt)
            {
                _Listener &l = *lIt;

                auto iterOwners = _eventIdRefListenerOwners.find(l._evId);
                if((iterOwners != _eventIdRefListenerOwners.end()) && l._listenCallBack)
                {
                    auto owner = l._listenCallBack->GetOwner();
                    if(owner)
                        iterOwners->second.erase(owner);

                    if(iterOwners->second.empty())
                    {
                        _eventIdRefListenerOwners.erase(iterOwners);
                    }
                }
                _stubListeners.erase(l._stub);
                CRYSTAL_RELEASE_SAFE(l._listenCallBack);
            }

            _listeners.erase(mIt);
        }
        else
        {
            _StubIndexedListeners::iterator stubIt = _stubListeners.find(listener._stub);
            if(stubIt == _stubListeners.end())
                return Status::Error;

            const int evId = stubIt->second._evId;
            _ListenersMap::iterator mIt = _listeners.find(evId);
            _delayAddOpRefCount.erase(evId);

            _Listeners &listeners = mIt->second;
            for(_Listeners::iterator lIt = listeners.begin();
                lIt != listeners.end();
                ++lIt)
            {
                _Listener &l = *lIt;
                if(l._stub == listener._stub)
                {
                    auto iterOwners = _eventIdRefListenerOwners.find(l._evId);
                    if((iterOwners != _eventIdRefListenerOwners.end()) && l._listenCallBack)
                    {
                        auto owner = l._listenCallBack->GetOwner();
                        if(owner)
                            iterOwners->second.erase(owner);

                        if(iterOwners->second.empty())
                        {
                            _eventIdRefListenerOwners.erase(iterOwners);
                        }
                    }

                    CRYSTAL_RELEASE_SAFE(l._listenCallBack);
                    listeners.erase(lIt);
                    break;
                }
            }

            if(listeners.empty())
                _listeners.erase(mIt);

            _stubListeners.erase(stubIt);
        }
    }
    else if(EventManager::FIRE == op._op)
    {// 延迟fire
        auto delayEv = op._fireInfo._ev;
        if (LIKELY(delayEv))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("attention: there is a event[%d] in delay firing"), delayEv->GetId());
            FireEvent(delayEv);
        }
        else
            g_Log->Error(LOGFMT_OBJ_TAG("delay fire lose event info"));
    }

    return Status::Success;
}

void EventManager::AfterFireEvent()
{
    // after fire 执行延迟执行事件
    if(--_firing == 0)
    {
        if (_delaying > 0)
            return;

        ++_delaying;
        for (auto iter = _delayedOps.begin(); iter != _delayedOps.end();)
        {
            ProcessEventOperation(*iter);
            iter = _delayedOps.erase(iter);
        }
        --_delaying;
    }
}

KERNEL_END