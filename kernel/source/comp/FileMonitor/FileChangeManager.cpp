// MIT License
// 
// Copyright (c) 2020 ericyonng<120453674@qq.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// Date: 2026-01-21 23:01:11
// Author: Eric Yonng
// Description:


#include <pch.h>
#include <kernel/comp/FileMonitor/FileChangeManager.h>
#include <kernel/comp/Utils/RttiUtil.h>
#include <kernel/common/statics.h>
#include <kernel/comp/thread/LibEventLoopThreadPool.h>

#include "kernel/comp/KernelFinally.h"
#include "kernel/comp/Coroutines/Runner.h"
#include "kernel/comp/KernelFinally/KernelFinally.h"
#include <kernel/comp/Utils/SystemUtil.h>

#include "kernel/comp/Coroutines/CoDelay.h"
#include "kernel/comp/Utils/ContainerUtil.h"
#include <kernel/comp/Timer/LibTimer.h>
#include <kernel/comp/Lock/Lock.h>
#include <kernel/comp/FileMonitor/FileChangeImpl.h>
#include <kernel/comp/Coroutines/CoDelay.h>

KERNEL_NS::FileChangeManager *g_FileChangeManager = NULL;

KERNEL_BEGIN

FileChangeManager::FileChangeManager()
    :CompObject(KERNEL_NS::RttiUtil::GetTypeId<FileChangeManager>())
,_isStart{false}
,_isQuit{false}
,_isWorking{false}
,_workerPoller{NULL}
,_locker(new KERNEL_NS::CoLocker)
{
    
}

FileChangeManager::~FileChangeManager()
{
    // 战术性泄露
    // KERNEL_NS::ContainerUtil::DelContainer2(_filePathRefFileObj);

    CRYSTAL_DELETE_SAFE(_locker);
}

void FileChangeManager::Release()
{
    delete this;
}
//
// bool FileChangeManager::_InitWorker()
// {
//     g_EventLoopHeavyTaskThreadPool->Send([this]()
//     {
//         KERNEL_NS::PostCaller([this]()->KERNEL_NS::CoTask<>
//         {
//             auto poller = KERNEL_NS::TlsUtil::GetPoller();
//             _workerPoller.store(poller, std::memory_order_release);
//
//             _isWorking.exchange(true, std::memory_order_release);
//             
//             // 等待start完成
//             while (!_isStart.load(std::memory_order_acquire))
//             {
//                 co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromSeconds(1));
//                 CRYSTAL_TRACE("FileChangeManager waiting started...")
//             }
//             
//             CRYSTAL_TRACE("file change manage working")
//
//              // 阻塞等待
//              while (!poller->IsQuit() && !_isQuit.load(std::memory_order_acquire))
//              {
//                  // 唤醒者在当前poller执行唤醒时, 一定处于挂起状态, 即使挂起点在Waiting之后, 只要params一样, 那么一定可以使用同一个param唤醒, 如果不想要那么
//                  co_await _locker->TimeWait(KERNEL_NS::TimeSlice::FromSeconds(5));
//
//                  _DoWork();
//              }
//
//             _isWorking.exchange(false, std::memory_order_release);
//         });
//     });
//
//     // 等待初始化完成
//     while (_workerPoller.load(std::memory_order_acquire) == NULL)
//     {
//         CRYSTAL_TRACE("FileChangeManager worker waiting init worker poller...")
//     }
//
//     CRYSTAL_TRACE("FileChangeManager init worker poller completed.")
//     
//     return true;
// }

bool FileChangeManager::_InitWorker2()
{
    g_EventLoopHeavyTaskThreadPool->Send([this]()
     {
         KERNEL_NS::PostCaller([this]()->KERNEL_NS::CoTask<>
         {
             auto poller = KERNEL_NS::TlsUtil::GetPoller();
             _workerPoller.store(poller, std::memory_order_release);

             _isWorking.exchange(true, std::memory_order_release);
                
             // 等待start完成
             while (!_isStart.load(std::memory_order_acquire))
             {
                 co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromSeconds(1));
                 CRYSTAL_TRACE("FileChangeManager waiting started...")
             }
                
             CRYSTAL_TRACE("file change manage working")

              // 阻塞等待
              while (!poller->IsQuit() && !_isQuit.load(std::memory_order_acquire))
              {
                  // 唤醒者在当前poller执行唤醒时, 一定处于挂起状态, 即使挂起点在Waiting之后, 只要params一样, 那么一定可以使用同一个param唤醒, 如果不想要那么
                  co_await _locker->TimeWait(KERNEL_NS::TimeSlice::FromSeconds(5));

                  co_await _DoWork2();
              }

             _isWorking.exchange(false, std::memory_order_release);
         });
     });

    // 等待初始化完成
    while (_workerPoller.load(std::memory_order_acquire) == NULL)
    {
        CRYSTAL_TRACE("FileChangeManager worker waiting init worker poller...")
    }

    CRYSTAL_TRACE("FileChangeManager init worker poller completed.")
    
    return true;
}


Int32 FileChangeManager::_OnInit()
{
    if(!_InitWorker2())
    {
        return Status::Failed;
    }
    
    return Status::Success;
}

// start 可以启动线程，再此之前都不可以启动线程
Int32 FileChangeManager::_OnStart()
{
    _isStart.store(true, std::memory_order_release);
    return Status::Success;
}

void FileChangeManager::_OnWillClose()
{
    _isQuit.store(true, std::memory_order_release);

    // 唤醒
    _locker->Broadcast();

    if(_isWorking.load(std::memory_order_acquire))
    {
        while (_isWorking.load(std::memory_order_acquire))
        {
            KERNEL_NS::SystemUtil::ThreadSleep(1000);
            CRYSTAL_TRACE("waiting FileChangeManager worker quit...")
        } 
    }

    CRYSTAL_TRACE("FileChangeManager worker quit complete.")
}

void FileChangeManager::_OnClose()
{
    // 战术性泄露
    // KERNEL_NS::ContainerUtil::DelContainer2(_filePathRefFileObj);

    CRYSTAL_TRACE("file change manager close.")
}
//
// void FileChangeManager::_DoWork()
// {
//     // 扫描文件看是否文件变化
//     for(auto iter : _filePathRefFileObj)
//     {
//         auto monitorInfo = iter.second;
//         void *fromMemory = NULL;
//         if(!monitorInfo->_checkChange->Invoke(fromMemory))
//             continue;
//
//         auto newObj = monitorInfo->_loadNewObj->Invoke(fromMemory);
//         if(!newObj)
//         {
//             if (g_Log)
//                 CLOG_ERROR("file: %s, load file fail", monitorInfo->_path.c_str());
//
//             continue;
//         }
//
//         if (g_Log)
//             CLOG_INFO("file: %s, changed, and load new one", monitorInfo->_path.c_str());
//
//         {
//             if(monitorInfo->_sourceObj)
//                 monitorInfo->_releaseObj->Invoke(monitorInfo->_sourceObj);
//
//             monitorInfo->_sourceObj = newObj;
//         }
//         
//         for(auto iterHandle : monitorInfo->_keyRefFileChangeHandle)
//         {
//             auto handle = iterHandle.second;
//             if(handle->_notListen.load(std::memory_order_acquire))
//                 continue;
//
//             // 反序列化新数据
//             auto newData = handle->_deserialize->Invoke(newObj);
//             if(!newData)
//             {
//                 if (g_Log)
//                     CLOG_WARN("file:%s deserialize from file fail dataName:%s, "
//                     , monitorInfo->_path.c_str(), handle->_dataName.c_str());
//                 continue;
//             }
//
//             // 切换成新数据,移除旧的数据
//             if(auto oldData = handle->_data.exchange(newData))
//             {
//                 handle->_release->Invoke(oldData);
//
//                 if (g_Log)
//                     CLOG_INFO("new data:%s updated", handle->_dataName.c_str());
//             }
//         }
//     }
// }

KERNEL_NS::CoTask<> FileChangeManager::_DoWork2()
{
    // 先取所有的key
    std::vector<UInt64> keys;
    keys.reserve(_handleRefFileChangeImpl.size());
    for (auto iter : _handleRefFileChangeImpl)
    {
        keys.push_back(iter.first);
    }
    
    for (auto key : keys)
    {
        auto iter = _handleRefFileChangeImpl.find(key);
        if (iter == _handleRefFileChangeImpl.end())
            continue;
        
        iter->second->Run();
        
        co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromMilliSeconds(5));
    }
}


UInt64 FileChangeManager::GenId()
{
    static std::atomic<UInt64> s_id{0};
    return s_id.fetch_add(1, std::memory_order_release) + 1;
}

UInt64 FileChangeManager::GenVersion()
{
    static std::atomic<UInt64> s_id{0};
    return s_id.fetch_add(1, std::memory_order_release) + 1;
}


bool FileChangeManager::Register(FileChangeImpl *impl)
{
    auto poller = _workerPoller.load(std::memory_order_acquire);
    if (UNLIKELY(!poller))
    {
        CRYSTAL_TRACE("register poller is not ready... impl:%llu, %p", impl->GetHandle(), impl);
        return false;
    }
    
    std::atomic_bool isFinish = {false};
    
    // 注册
    auto handleRegisterLamb = [this, &isFinish, impl]()
    {
        auto handle = impl->GetHandle();
        _handleRefFileChangeImpl.insert(std::make_pair(handle, impl));
        impl->FirstRun();
        
        isFinish.store(true, std::memory_order_release);
        
        CRYSTAL_TRACE("Register FileChangeImpl handle:%llu", handle);
    };
    
    if (poller->GetWorkerThreadId() != KERNEL_NS::SystemUtil::GetCurrentThreadId())
    {
        poller->Push(handleRegisterLamb);

        // 等待完成(如果是在FileChangeManager同一个线程就G了)
        while (!isFinish.load(std::memory_order_acquire))
        {
            KERNEL_NS::SystemUtil::ThreadSleep(2);
            CRYSTAL_TRACE("register waiting... impl:%llu, %p", impl->GetHandle(), impl);
        }
    }
    else
    {
        // 在Poller线程中, 直接执行, 避免后面阻塞
        handleRegisterLamb();
    }
    
    return true;
}

void FileChangeManager::UnRegister(UInt64 handle)
{
    auto poller = _workerPoller.load(std::memory_order_acquire);
    if (UNLIKELY(!poller))
    {
        CRYSTAL_TRACE("register poller is not ready... impl:%llu", handle);
        return;
    }
    
    std::atomic_bool isFinish = {false};
    
    // 取消注册
    auto handleUnRegisterLamb = [this, &isFinish, handle]()
    {
        CRYSTAL_TRACE("UnRegister FileChangeImpl handle:%llu", handle);
        
        _handleRefFileChangeImpl.erase(handle);
        isFinish.store(true, std::memory_order_release);
    };
    
    if (poller->GetWorkerThreadId() != KERNEL_NS::SystemUtil::GetCurrentThreadId())
    {
        poller->Push(handleUnRegisterLamb);

        // 等待完成
        while (!isFinish.load(std::memory_order_acquire) && (!poller->IsQuit()))
        {
            KERNEL_NS::SystemUtil::ThreadSleep(2);
            CRYSTAL_TRACE("UnRegister waiting... impl:%llu", handle);
        }
    }
    else
    {
        // 在Poller线程中, 直接执行, 避免后面阻塞
        handleUnRegisterLamb();
    }
}



KERNEL_END