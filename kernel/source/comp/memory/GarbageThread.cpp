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
 * Date: 2020-12-07 02:43:57
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/memory/GarbageThread.h>
#include <kernel/comp/thread/LibThread.h>
#include <kernel/comp/Lock/Impl/CoLocker.h>
#include <kernel/comp/thread/LibEventLoopThreadPool.h>

#include "kernel/comp/Coroutines/Runner.h"
#include <memory>

KERNEL_BEGIN

GarbageThread::GarbageThread(UInt64 gcIntervalMs)
    :_isStart{false}
    ,_garbagePurgeCallback(new std::map<void *, IDelegate<void> *>)
    ,_toRegisterPurgeCallback(new std::map<void *, IDelegate<void> *>)
    ,_swapRegisters(new std::map<void *, IDelegate<void> *>)
    ,_toPurge(new std::set<void *>)
    ,_purgeSwap(new std::set<void *>)
    ,_gcIntervalMs{0}
    ,_isQuit{false}
    ,_isWorking{false}
{
    _gcIntervalMs.store(gcIntervalMs, std::memory_order_release);
}

GarbageThread::~GarbageThread()
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

void GarbageThread::Start()
{
    if(_isStart.exchange(true, std::memory_order_acq_rel))
    {
        CRYSTAL_TRACE("garbage thread already start")
        return;
    }

    _lck = new KERNEL_NS::CoLocker();
    g_EventLoopEasyTaskThreadPool->Send([this]()
    {
        KERNEL_NS::PostCaller([this]()->KERNEL_NS::CoTask<>
        {
            CRYSTAL_TRACE("garbage task started.")
            auto poller = KERNEL_NS::TlsUtil::GetPoller();
            _isWorking.store(true, std::memory_order_release);
            for(;;)
            {
                if ( UNLIKELY(poller->IsQuit() || _isQuit.load(std::memory_order_acquire)) )
                {
                    _Work();
                    break;
                }
                
                _Work();

                // 休息一会儿
                co_await _lck->TimeWait(_gcIntervalMs.load(std::memory_order_acquire));
            }

            _isWorking.store(false, std::memory_order_release);

            CRYSTAL_TRACE("garbage task quit.")
        });
    });
}

void GarbageThread::Close()
{
    if(!_isStart.exchange(false, std::memory_order_acq_rel))
        return;

    // 退出
    _isQuit.store(true, std::memory_order_release);
    if(_lck)
        _lck->Broadcast();

    if(_isWorking.load(std::memory_order_acquire))
    {
        while (_isWorking.load(std::memory_order_acquire))
        {
            KERNEL_NS::SystemUtil::ThreadSleep(1000);
            CRYSTAL_TRACE("waiting garbage quit thread pool...")
        }
    }

    CRYSTAL_TRACE("waiting garbage quit thread pool completed.")

    // 清理资源
    _toRegisterLck.Lock();
    for(auto iterDeleg = _toRegisterPurgeCallback->begin(); iterDeleg != _toRegisterPurgeCallback->end();)
    {
        iterDeleg->second->Release();
        iterDeleg = _toRegisterPurgeCallback->erase(iterDeleg);
    }

    for(auto iterDeleg = _swapRegisters->begin(); iterDeleg != _swapRegisters->end();)
    {
        iterDeleg->second->Release();
        iterDeleg = _swapRegisters->erase(iterDeleg);
    }

    for (auto iterDeleg = _garbagePurgeCallback->begin(); iterDeleg != _garbagePurgeCallback->end(); )
    {
        CRYSTAL_RELEASE_SAFE(iterDeleg->second);
        iterDeleg = _garbagePurgeCallback->erase(iterDeleg);
    }

    _toRegisterLck.Unlock();

    CRYSTAL_DELETE_SAFE(_lck);
}

void GarbageThread::Release()
{
    CRYSTAL_DELETE(this);
}

GarbageThread *GarbageThread::GetInstence()
{
    // 战略性泄露, 不释放, 因为其他模块可能在程序结束时候调用UnRegister, 不确定时序先后所以不宜释放
    static GarbageThread *instance = new GarbageThread();
    return instance;
}

void GarbageThread::_Work()
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
