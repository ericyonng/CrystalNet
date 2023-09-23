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
 * Date: 2021-03-21 17:50:15
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_EVENT_EVENT_MANAGER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_EVENT_EVENT_MANAGER_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/Delegate/Delegate.h>
#include <kernel/comp/Event/Defs.h>
#include <kernel/comp/memory/memory.h>

KERNEL_BEGIN


class LibEvent;

class KERNEL_EXPORT EventManager
{
    POOL_CREATE_OBJ_DEFAULT(EventManager);
    
public:
    EventManager();
    virtual ~EventManager();

public:
    /**
     * Add event listener.
     * @param[in] id         - event Id.
     * @param[in] listener   - event listener.
     * @param[in] bindedStub - the binded stub, if not specified, will auto gen stub. 指定的存根若存在则销毁当前的delegate
     * @return ListenerStub - return INVALID_LISTENER_STUB if failed, otherwise return validate stub.
     * delegate由内部释放，外部不释放
     */
    virtual ListenerStub AddListener(int id,
                                          IDelegate<void, LibEvent *> *listener,
                                          const ListenerStub &bindedStub = INVALID_LISTENER_STUB);

    template <typename ObjectType>
    ListenerStub AddListener(int id,
                                  ObjectType *obj,
                                  void (ObjectType::*listener)(LibEvent *),
                                  const ListenerStub &bindedStub = INVALID_LISTENER_STUB);

    /**
     * Remove event listener.
     * @param[in] id - event Id.
     * @return int - success if return StatusDefs::success, otherwise return Error.
     *               specially, if return Error,  and fetch the last error is pending,
     *               it means operation will success on later, but pending at now.
     */
    virtual int RemoveListener(int id);

    /**
     * Remove event listener using listener stub.
     * @param[in] stub - event listener stub.
     * @return int - success if return success, otherwise return Error,
     *               specially, if return Error, and fetch the last error is pending,
     *               it means operation will success on later, but pending at now.
     */
    virtual int RemoveListener(const ListenerStub &stub);

    /**
     * Remove event listener using listener stub and clear the listener stub.
     * @param[in] stub - event listener stub.
     * @return int - success if return success, otherwise return Error,
     *               specially, if return Error, and fetch the last error is pending,
     *               it means operation will success on later, but pending at now.
     */
    virtual int RemoveListenerX(ListenerStub &stub);

public:
    /**
     * Fire the event. 处于isfiring中进行监听的事件，
     * FireEvent若在IsFiring状况下addlisten并在统一帧下Fire的话事件将在之后的某个合适时机触发
     * 若立即FireEvent会丢失事件,因为IsFiring状态下的AddListen在异步队列中
     * @param[in] event - event object.
     * @return(Int32) FireEvResult, 为了降低复杂度,请保证统一征内事件不同时Add与Fire(IsFiring状态下)
     */
    virtual Int32 FireEvent(LibEvent *event);

    /**
     * Check event manager is firing or not.
     * @return bool - firing flag.
     */
    virtual bool IsFiring() const;

    // 正在监听某个时间 O(n) ObjType:是监听者, 可以是类对象也可以是函数地址
    template<typename ObjType>
    bool IsListening(int id, ObjType *obj) const;

protected:
    /**
     * \brief Wrap the event listener.
     */
    struct _Listener
    {
        ListenerStub _stub;

        int _evId;
        IDelegate<void, LibEvent *> *_listenCallBack;

        _Listener();
    };

    struct _FireInfo
    {
        LibEvent *_ev;
        _FireInfo();
    };

    /**
     * \brief Wrap the event operation information.
     */
    enum OpType
    {
        ADD = 0,
        REMOVE = 1,
        FIRE = 2,
    };
    struct _Op
    {
        Int32 _op;
        _Listener _listener;
        _FireInfo _fireInfo;
    };

protected:
    /**
     * Search given listen stub in the event manager.
     */
    bool _SearchStub(const ListenerStub &stub) const;

    /**
     * Process event operation.
     */
    int ProcessEventOperation(_Op &op);

    /**
     * Before fire event method.
     */
    void BeforeFireEvent();

    /**
     * After fire event method.
     */
    void AfterFireEvent();

protected:
    int _firing;
    Int32 _delaying;
    ListenerStub _maxListenerStub;

    // 延迟到afterevent执行相关事件
    typedef std::list<_Op> _DelayedOps;
    _DelayedOps _delayedOps;
    std::map<Int32, Int32> _delayAddOpRefCount;

    // 事件对应的监听回调
    typedef std::vector<_Listener> _Listeners;
    typedef std::map<int, _Listeners> _ListenersMap;
    _ListenersMap _listeners;

    // 存根对应的监听回调
    typedef std::map<ListenerStub, _Listener> _StubIndexedListeners;
    _StubIndexedListeners _stubListeners;

    // 针对某个对象上监听的
    std::map<Int32, std::set<void *>> _eventIdRefListenerOwners;
};


inline EventManager::_Listener::_Listener()
    : _stub(INVALID_LISTENER_STUB)
    , _evId(-1)
    , _listenCallBack(NULL)
{
}

inline EventManager::_FireInfo::_FireInfo()
    : _ev(NULL)
{
}


inline EventManager::EventManager()
: _firing(0)
, _delaying(0)
,_maxListenerStub(0)
{

}

template <typename ObjectType>
inline ListenerStub EventManager::AddListener(int id,
                                    ObjectType *obj,
                                    void (ObjectType::*listener)(LibEvent *),
                                    const ListenerStub &bindedStub)
{
    if(!obj || !listener)
        return INVALID_LISTENER_STUB;

    auto listenerDelegate = DelegateFactory::Create(obj, listener);
    return AddListener(id, listenerDelegate, bindedStub);
}

inline int EventManager::RemoveListenerX(ListenerStub &stub)
{
    if(RemoveListener(stub) != Status::Success)
    {
        stub = INVALID_LISTENER_STUB;
        return Status::Error;
    }

    stub = INVALID_LISTENER_STUB;
    return Status::Success;
}

inline bool EventManager::IsFiring() const
{
    return _firing > 0;
}

inline void EventManager::BeforeFireEvent()
{
    ++_firing;
}

template<typename ObjType>
ALWAYS_INLINE bool EventManager::IsListening(int id, ObjType *obj) const
{
    bool isListening = false;
    {
        auto iter = _eventIdRefListenerOwners.find(id);
        if(iter != _eventIdRefListenerOwners.end())
            isListening = iter->second.find(obj) != iter->second.end();
    }

    // 在延迟队列中
    {
        for(auto &op : _delayedOps)
        {
            if(op._op == EventManager::REMOVE)
            {
                if(op._listener._evId == id)
                {
                    isListening = false;
                    continue;
                }

                if(op._listener._stub)
                {
                    auto iterStub = _stubListeners.find(op._listener._stub);
                    if(iterStub != _stubListeners.end())
                    {
                        auto &listenInfo = iterStub->second;
                        if(listenInfo._listenCallBack && listenInfo._listenCallBack->IsBelongTo(obj))
                            isListening = false;
                    }
                }
            }
            else if(op._op == EventManager::ADD)
            {
                if(op._listener._evId != id)
                    continue;

                if(op._listener._listenCallBack)
                {
                    if(op._listener._listenCallBack->IsBelongTo(obj))
                        isListening = true;
                }
            }
        }
    }

    return isListening;
}

KERNEL_END

#endif

