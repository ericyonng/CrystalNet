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
#include <kernel/comp/thread/thread.h>
#include <kernel/comp/memory/MemoryDefs.h>

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


void GarbageThread::Start()
{
    if(_isStart.exchange(true))
    {
        CRYSTAL_TRACE("garbage thread already start");
        return;
    }

    // TODO:启动线程
    auto deleg = DelegateFactory::Create(this, &GarbageThread::_Work);
    auto newTask = GarbageThreadTask::New_GarbageThreadTask(_thread, deleg, _gcIntervalMs, _gcTaskLck);
    _thread->AddTask(newTask);

    _thread->Start();
}

void GarbageThread::Close()
{
    if(!_isStart.exchange(false))
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

    CRYSTAL_DELETE_SAFE(_garbagePurgeCallback);
    CRYSTAL_DELETE_SAFE(_toRegisterPurgeCallback);
    CRYSTAL_DELETE_SAFE(_swapRegisters);

    _toRegisterLck.Unlock();

    _lckPurge.Lock();
    CRYSTAL_DELETE_SAFE(_toPurge);
    CRYSTAL_DELETE_SAFE(_purgeSwap);
    _lckPurge.Unlock();

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

KERNEL_END
