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
#include "kernel/comp/FileMonitor/FileChangeManagerFactory.h"
#include <kernel/common/statics.h>
#include <kernel/comp/thread/LibEventLoopThreadPool.h>

#include "kernel/comp/KernelFinally.h"
#include "kernel/comp/Coroutines/Runner.h"
#include "kernel/comp/KernelFinally/KernelFinally.h"
#include <kernel/comp/Utils/SystemUtil.h>

#include "kernel/comp/Coroutines/CoDelay.h"
#include "kernel/comp/Utils/ContainerUtil.h"

KERNEL_NS::FileChangeManager *g_FileChangeManager = NULL;

KERNEL_BEGIN

FileChangeManager::FileChangeManager()
    :CompObject(KERNEL_NS::RttiUtil::GetTypeId<FileChangeManager>())
,_isQuit{false}
,_isWorking{false}
,_workerPoller{NULL}
{
    
}

FileChangeManager::~FileChangeManager()
{
    KERNEL_NS::ContainerUtil::DelContainer2(_filePathRefFileObj);
}

void FileChangeManager::Release()
{
    FileChangeManager::DeleteByAdapter_FileChangeManager(FileChangeManagerFactory::_buildType.V, this);
}

void FileChangeManager::_InitWorker()
{
    g_LibEventLoopThreadPool->Send([this]()
    {
       KERNEL_NS::PostCaller([this]()->KERNEL_NS::CoTask<>
       {
           auto poller = KERNEL_NS::TlsUtil::GetPoller();

           _workerPoller.exchange(poller);

           _isWorking.exchange(true, std::memory_order_release);

           // 启动后继续
           while (!_isStart.load(std::memory_order_acquire))
           {
               co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromSeconds(1));

                CRYSTAL_TRACE("FileChangeManager waiting worker init...")

           }

           g_Log->Info(LOGFMT_OBJ_TAG("file change manage working"));

            // 阻塞等待
            while (!poller->IsQuit() && !_isQuit.load(std::memory_order_acquire))
            {
                // 唤醒者在当前poller执行唤醒时, 一定处于挂起状态, 即使挂起点在Waiting之后, 只要params一样, 那么一定可以使用同一个param唤醒, 如果不想要那么
                co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromSeconds(5));

                // 扫描文件看是否文件变化
                for(auto iter : _filePathRefFileObj)
                {
                    auto monitorInfo = iter.second;
                    void *fromMemory = NULL;
                    if(!monitorInfo->_checkChange->Invoke(fromMemory))
                        continue;

                    auto newObj = monitorInfo->_loadNewObj->Invoke(fromMemory);
                    if(!newObj)
                    {
                        if (g_Log)
                            g_Log->Error(LOGFMT_OBJ_TAG("file: %s, load file fail"), monitorInfo->_path.c_str());

                        continue;
                    }

                    if (g_Log)
                        g_Log->Info(LOGFMT_OBJ_TAG("file: %s, changed, and load new one"), monitorInfo->_path.c_str());

                    {
                        if(monitorInfo->_sourceObj)
                            monitorInfo->_releaseObj->Invoke(monitorInfo->_sourceObj);

                        monitorInfo->_sourceObj = newObj;
                    }
                    
                    for(auto iterHandle : monitorInfo->_keyRefFileChangeHandle)
                    {
                        auto handle = iterHandle.second;
                        if(handle->_notListen.load(std::memory_order_acquire))
                            continue;

                        // 反序列化新数据
                        auto newData = handle->_deserialize->Invoke(newObj);
                        if(!newData)
                        {
                            if (g_Log)
                                g_Log->Warn(LOGFMT_OBJ_TAG("file:%s deserialize from file fail dataName:%s, ")
                                , monitorInfo->_path.c_str(), handle->_dataName.c_str());
                            continue;
                        }

                        // 切换成新数据,移除旧的数据
                        if(auto oldData = handle->_data.exchange(newData))
                        {
                            handle->_release->Invoke(oldData);

                            if (g_Log)
                                g_Log->Info(LOGFMT_OBJ_TAG("new data:%s updated"), handle->_dataName.c_str());
                        }
                    }
                }
            }

           _isWorking.exchange(false, std::memory_order_release);
       }); 
    });
}

Int32 FileChangeManager::_OnInit()
{
    _InitWorker();
    return Status::Success;
}

// start 可以启动线程，再此之前都不可以启动线程
Int32 FileChangeManager::_OnStart()
{
    _isStart.store(true, std::memory_order_release);
    return Status::Success;
}

void FileChangeManager::_OnClose()
{
    _isQuit.store(true, std::memory_order_release);
    while (_isWorking.load(std::memory_order_acquire))
    {
        KERNEL_NS::SystemUtil::ThreadSleep(1000);
        CRYSTAL_TRACE("waiting worker quit...")
    }

    KERNEL_NS::ContainerUtil::DelContainer2(_filePathRefFileObj);

    CRYSTAL_TRACE("file change manager close.")
}

KERNEL_END