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
// Date: 2026-06-02 23:06:15
// Author: Eric Yonng
// Description:

#include <pch.h>
#include <google/protobuf/stubs/status.h>
#include <OptionComp/Command/Impl/CommandMgr.h>
#include <OptionComp/Command/Impl/CommandMgrFactory.h>
#include <kernel/comp/Utils/RttiUtil.h>
#include <kernel/comp/Log/log.h>
#include <kernel/common/status.h>
#include <kernel/comp/Utils/ContainerUtil.h>
#include <kernel/comp/thread/LibEventLoopThread.h>
#include <kernel/comp/Utils/TlsUtil.h>
#include <kernel/comp/Coroutines/CoTask.h>
#include <kernel/comp/Utils/SystemUtil.h>

KERNEL_BEGIN

CommandMgr::CommandMgr()
    :ICommandMgr(KERNEL_NS::RttiUtil::GetTypeId<CommandMgr>())
    ,_eventLoopThread(NULL)
    ,_isWorking{false}
{
    
}

CommandMgr::~CommandMgr()
{
    _Clear();
}

void CommandMgr::Release()
{
    CommandMgr::DeleteByAdapter_CommandMgr(CommandMgrFactory::_buildType.V, this);
}

void CommandMgr::DefaultMaskReady(bool isReady)
{
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    MaskReady(isReady);
#endif
}

void CommandMgr::AddCommand(const LibString &cmd, IDelegate<void> *callback)
{
    _lck.Lock();
    auto iter = _cmdRefCallback.find(cmd);
    if(iter != _cmdRefCallback.end())
    {
        auto oldCb = iter->second;
        CLOG_INFO("will remove old cmd:%s, callback:%p, %s", cmd.c_str(), oldCb, oldCb->GetCallbackRtti().c_str());

        oldCb->Release();
        _cmdRefCallback.erase(iter);
    }

    CLOG_INFO("will add cmd:%s, callback:%p, %s", cmd.c_str(), callback, callback->GetCallbackRtti().c_str());

    _cmdRefCallback.insert(std::make_pair(cmd, callback));
    _lck.Unlock();
}


Int32 CommandMgr::_OnHostInit()
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS

    _eventLoopThread = new LibEventLoopThread([this]()
    {
        // 只有windows下才有接收command, linux下是脱离终端的后台进程, 关闭了终端输入输出
        KERNEL_NS::PostCaller([this]()->CoTask<>
        {
            CLOG_INFO("command monitor thread started");

            _isWorking.store(true, std::memory_order_release);
            MaskReady(true);
            auto poller = KERNEL_NS::TlsUtil::GetPoller();
            while (!poller->IsQuit())
            {
                KERNEL_NS::LibString input;
                std::getline(std::cin, input);

                _lck.Lock();
                auto allCallback = _cmdRefCallback;
                _lck.Unlock();
                auto iter = allCallback.find(input);
                if(iter == allCallback.end())
                {
                    CLOG_DEBUG("unknown command:%s", input.c_str());
                    continue;
                }

                CLOG_INFO("invoke cmd:%s...");
                auto cb = iter->second;
                cb->Invoke();
                CLOG_INFO("invoke cmd:%s finished.");
            }

            _isWorking.store(false, std::memory_order_release);
            CLOG_INFO("Comand thread quit.");

            co_return;
        });        
    }, "CommandMonitor");
#endif

    CLOG_INFO("Comand init success");
    return Status::Success;
}

Int32 CommandMgr::_OnCompsCreated()
{
    return Status::Success;
}

Int32 CommandMgr::_OnHostWillStart()
{
    CLOG_INFO("Comand will start...");
    return Status::Success;
}

Int32 CommandMgr::_OnHostStart()
{
    if(!_eventLoopThread)
        return Status::Success;
    
    _eventLoopThread->Start();
    auto poller = _eventLoopThread->GetPollerNoAsync();
    while (!poller)
    {
        KERNEL_NS::SystemUtil::ThreadSleep(1000);
        poller = _eventLoopThread->GetPollerNoAsync();

        CLOG_INFO("waiting command thread started...");
    }
    
    CLOG_INFO("Comand started.");
    return Status::Success;
}

void CommandMgr::_OnHostBeforeCompsWillClose()
{
    if(!_eventLoopThread)
    {
        return;
    }
    
    auto poller = _eventLoopThread->GetPollerNoAsync();
    for(Int32 idx = 0; idx < 10; ++idx)
    {
        if(poller)
            break;
        
        KERNEL_NS::SystemUtil::ThreadSleep(1000);
        poller = _eventLoopThread->GetPollerNoAsync();
        CLOG_INFO("waiting command thread poller...");
        if(poller)
            break;
    }

    if(poller)
    {
        poller->QuitLoop();

        while (_isWorking.load(std::memory_order_acquire))
        {
            std::cin.setstate(std::ios::badbit);
            CLOG_INFO("Waiting command thread quit...");
            SystemUtil::ThreadSleep(1000);
        }
    }
    _eventLoopThread->Close();
    MaskReady(false);
    CLOG_INFO("Comand thread closed.");
}

void CommandMgr::_OnHostClose()
{
    _Clear();
    CLOG_INFO("Comand closed.");
}

void CommandMgr::_Clear()
{
    ContainerUtil::DelContainer2(_cmdRefCallback);
    CRYSTAL_RELEASE_SAFE(_eventLoopThread);
}

KERNEL_END
