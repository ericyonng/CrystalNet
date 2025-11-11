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
#include <kernel/comp/memory/GarbageThreadTask.h>
#include <kernel/comp/thread/LibThread.h>

KERNEL_BEGIN

GarbageThread::GarbageThread(UInt64 gcIntervalMs)
    :_isStart{false}
    ,_garbagePurgeCallback(new std::map<void *, IDelegate<void> *>)
    ,_toRegisterPurgeCallback(new std::map<void *, IDelegate<void> *>)
    ,_swapRegisters(new std::map<void *, IDelegate<void> *>)
    ,_toPurge(new std::set<void *>)
    ,_purgeSwap(new std::set<void *>)
    ,_thread(new LibThread)
    ,_gcIntervalMs(gcIntervalMs)
{

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
        CRYSTAL_TRACE("garbage thread already start");
        return;
    }

    // TODO:启动线程
    auto deleg = DelegateFactory::Create(this, &GarbageThread::_Work);
    auto newTask = GarbageThreadTask::New_GarbageThreadTask(_thread, deleg, _gcIntervalMs, _gcTaskLck);
    _thread->AddTask(newTask);
    _thread->SetThreadName("GarbageThread");

    _thread->Start();
}

void GarbageThread::Close()
{
    if(!_isStart.exchange(false, std::memory_order_acq_rel))
        return;

    // TODO:关闭线程
    _thread->HalfClose();
    _gcTaskLck.Sinal();
    _thread->FinishClose();

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

    CRYSTAL_DELETE_SAFE(_thread);
}

void GarbageThread::Release()
{
    CRYSTAL_DELETE(this);
}

GarbageThread *GarbageThread::GetInstence()
{
    static GarbageThread *instence = new GarbageThread();
    return instence;
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
