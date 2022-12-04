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
 * Date: 2020-12-06 19:55:32
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_GARBAGE_THREAD_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_GARBAGE_THREAD_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/Lock/Lock.h>
#include <kernel/comp/Delegate/Delegate.h>
#include <kernel/comp/memory/MemoryDefs.h>

KERNEL_BEGIN

class LibThread;

class KERNEL_EXPORT GarbageThread
{
public:
    GarbageThread(UInt64 gcIntervalMs = DEF_GC_INTERVAL);
    virtual ~GarbageThread();
    virtual void Release();

public:
    static GarbageThread *GetInstence();

    // 注册与标记
    template<typename ObjType>
    void RegisterPurge(ObjType *gc, void(ObjType::*callback)(void));
    void UnRegisterPurge(void *gc);
    void MaskPurge(void *gc);

    // 启动线程
    void Start();
    // 关闭线程
    void Close();

private:
    void _Work();

private:
    std::atomic_bool _isStart;
    
    // 全部回调
    std::map<void *, IDelegate<void> *> *_garbagePurgeCallback;

    // 注册回调增量区
    SpinLock _toRegisterLck;
    std::map<void *, IDelegate<void> *> *_toRegisterPurgeCallback;
    std::map<void *, IDelegate<void> *> *_swapRegisters;

    // 反注册
    SpinLock _unRegisterLck;
    std::set<void *> _unRegisters;
    
    // 当前要清理的垃圾收集器列表
    SpinLock _lckPurge;
    std::set<void *> *_toPurge;
    std::set<void *> *_purgeSwap;

    // 线程对象
    LibThread *_thread;
    UInt64 _gcIntervalMs;
    ConditionLocker _gcTaskLck;
};

inline GarbageThread::~GarbageThread()
{
    Close();

    _toRegisterLck.Lock();
    CRYSTAL_DELETE_SAFE(_garbagePurgeCallback);
    CRYSTAL_DELETE_SAFE(_toRegisterPurgeCallback);
    CRYSTAL_DELETE_SAFE(_swapRegisters);
    _toRegisterLck.Unlock();
    
    _lckPurge.Lock();
    CRYSTAL_DELETE_SAFE(_toPurge);
    CRYSTAL_DELETE_SAFE(_purgeSwap);
    _lckPurge.Unlock();
}

template<typename ObjType>
inline void GarbageThread::RegisterPurge(ObjType *gc, void(ObjType::*callback)(void))
{
    _toRegisterLck.Lock();
    auto deleg = DelegateFactory::Create(gc, callback);
    _toRegisterPurgeCallback->insert(std::make_pair(gc, deleg));
    _toRegisterLck.Unlock();
}

inline void GarbageThread::UnRegisterPurge(void *gc)
{
    _unRegisterLck.Lock();
    _unRegisters.insert(gc);
    _unRegisterLck.Unlock();
}

inline void GarbageThread::MaskPurge(void *gc)
{
    _lckPurge.Lock();
    _toPurge->insert(gc);
    _lckPurge.Unlock();
}

inline void GarbageThread::_Work()
{  
    // 交换注册
    std::map<void *, IDelegate<void> *> *tmpRegister;
    _toRegisterLck.Lock();
    tmpRegister = _toRegisterPurgeCallback;
    _toRegisterPurgeCallback = _swapRegisters;
    _swapRegisters = tmpRegister;
    _toRegisterLck.Unlock();

    // 交换垃圾
    std::set<void *> *tmp;
    _lckPurge.Lock();
    tmp = _toPurge;
    _toPurge = _purgeSwap;
    _purgeSwap = tmp;
    _lckPurge.Unlock();

    // 回调交换
    for(auto iterSwap = _swapRegisters->begin(); iterSwap != _swapRegisters->end(); )
    {
        _garbagePurgeCallback->insert(std::make_pair(iterSwap->first, iterSwap->second));
        iterSwap = _swapRegisters->erase(iterSwap);
    }

    // 当前需要执行的垃圾回收交换
    for(auto iterSwap = _purgeSwap->begin(); iterSwap != _purgeSwap->end(); )
    {
        auto obj = *iterSwap;
        _unRegisterLck.Lock();
        auto iterCallback = _garbagePurgeCallback->find(obj);
        if(_unRegisters.find(obj) == _unRegisters.end())
        {
            iterCallback->second->Invoke();
        }
        else
        {// 被反注册了
            iterCallback->second->Release();
            _garbagePurgeCallback->erase(iterCallback);
        }
        _unRegisterLck.Unlock();

        iterSwap = _purgeSwap->erase(iterSwap);
    }

    // 移除剩余反注册
    for(auto iter = _garbagePurgeCallback->begin(); iter != _garbagePurgeCallback->end();)
    {
        _unRegisterLck.Lock();
        if(_unRegisters.find(iter->first) != _unRegisters.end())
        {
            _unRegisters.erase(iter->first);
            iter->second->Release();
            iter = _garbagePurgeCallback->erase(iter);
            _unRegisterLck.Unlock();
            continue;
        }

        ++iter;
        _unRegisterLck.Unlock();
    }
}

KERNEL_END

#endif
