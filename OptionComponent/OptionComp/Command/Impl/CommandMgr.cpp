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
#include <kernel/comp/Timer/LibTimer.h>
#include <kernel/comp/Utils/StringUtil.h>

KERNEL_BEGIN

CommandThreadContainer::~CommandThreadContainer()
{
    if(!IsNoRelease)
    {
        KERNEL_NS::ContainerUtil::DelContainer2(_cmdRefCallback);
        KERNEL_NS::ContainerUtil::DelContainer2(_regularKeywordRefCallback);
    }
}

CommandMgr::CommandMgr()
    :ICommandMgr(KERNEL_NS::RttiUtil::GetTypeId<CommandMgr>())
    ,_eventLoopThread(NULL)
    ,_isWorking{false}
    ,_inWaiting{false}
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
    auto &&toLower = cmd.tolower();
    auto threadId = KERNEL_NS::SystemUtil::GetCurrentThreadId();
    _lck.Lock();
    auto iterThread = _threadIdRefCmdContainer.find(threadId);
    if(iterThread == _threadIdRefCmdContainer.end())
    {
        iterThread = _threadIdRefCmdContainer.insert(std::make_pair(threadId, CommandThreadContainer())).first;
    }

    auto &cmdContainer = iterThread->second;
    auto iter = cmdContainer._cmdRefCallback.find(toLower);
    if(iter != cmdContainer._cmdRefCallback.end())
    {
        auto oldCb = iter->second;
        CLOG_INFO("will remove old cmd:%s, callback:%p, %s", toLower.c_str(), oldCb, oldCb->GetCallbackRtti().c_str());

        oldCb->Release();
        cmdContainer._cmdRefCallback.erase(iter);
    }

    CLOG_INFO("will add cmd:%s, callback:%p, %s", toLower.c_str(), callback, callback->GetCallbackRtti().c_str());

    cmdContainer._cmdRefCallback.insert(std::make_pair(toLower, callback));
    _lck.Unlock();
}

void CommandMgr::AddRegularCommand(const LibString &cmd, IDelegate<void, const KERNEL_NS::LibString &> *callback)
{
    auto &&toLower = cmd.tolower();
    auto threadId = KERNEL_NS::SystemUtil::GetCurrentThreadId();

    _lck.Lock();
    auto iterThread = _threadIdRefCmdContainer.find(threadId);
    if(iterThread == _threadIdRefCmdContainer.end())
    {
        iterThread = _threadIdRefCmdContainer.insert(std::make_pair(threadId, CommandThreadContainer())).first;
    }
    auto &cmdContainer = iterThread->second;
    auto iter = cmdContainer._regularKeywordRefCallback.find(toLower);
    if(iter != cmdContainer._regularKeywordRefCallback.end())
    {
        auto oldCb = iter->second;
        CLOG_INFO("will remove old cmd:%s, callback:%p, %s", toLower.c_str(), oldCb, oldCb->GetCallbackRtti().c_str());

        oldCb->Release();
        cmdContainer._regularKeywordRefCallback.erase(iter);
    }

    CLOG_INFO("will add cmd:%s, callback:%p, %s", toLower.c_str(), callback, callback->GetCallbackRtti().c_str());

    cmdContainer._regularKeywordRefCallback.insert(std::make_pair(toLower, callback));
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
                _isWorking.store(true, std::memory_order_release);

                KERNEL_NS::LibString input;
                _inWaiting.store(true, std::memory_order_release);
                std::getline(std::cin, input);
                _inWaiting.store(false, std::memory_order_release);
                input = input.tolower();

                CLOG_DEBUG("getline waking up input:%s...", input.c_str());

                // 拷贝出来的cmd不释放
                _lck.Lock();
                auto cmdContainer = _threadIdRefCmdContainer;
                for(auto &iter : cmdContainer)
                    iter.second.IsNoRelease = true;
                _lck.Unlock();

                // 优先执行正则匹配
                bool isRunRegular = false;
                {
                    if(!input.empty())
                    {
                        for(auto &iter : cmdContainer)
                        {
                            auto threadId = iter.first;
                            auto &containerInfo = iter.second;
                            for(auto &iter : containerInfo._regularKeywordRefCallback)
                            {
                                auto &first = iter.first;
                                auto cb = iter.second;
                                if(KERNEL_NS::StringUtil::IsMatch(input, first))
                                {
                                    isRunRegular = true;
                                    CLOG_INFO("invoke command:%s, input:%s", first.c_str(), input.c_str());
                                    cb->Invoke(input);
                                    CLOG_INFO("invoke command:%s finished", first.c_str());
                                    break;
                                }
                            }
                        }
                    }
                }

                if(!isRunRegular)
                {
                    for(auto &iterContainer : cmdContainer)
                    {
                        auto threadId = iterContainer.first;
                        auto &containerInfo = iterContainer.second;
                        auto iter = containerInfo._cmdRefCallback.find(input);
                        if(iter == containerInfo._cmdRefCallback.end())
                        {
                            CLOG_DEBUG("unknown command:%s", input.c_str());
                            continue;
                        }

                        CLOG_INFO("invoke cmd:%s...");
                        auto cb = iter->second;
                        cb->Invoke();
                        CLOG_INFO("invoke cmd:%s finished.");
                    }
                }

                // 让event loop处理其他事情避免导致其他事情无法执行 TODO:跳到Poller，此后poller退出, 无法再往回了 让出了控制权, 为了避免退出时无法退出需要设置false
                _isWorking.store(false, std::memory_order_release);
                co_await CoDelay(KERNEL_NS::TimeSlice::FromSeconds(5));
            }

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

    // 定时10秒唤醒std::getline
    KERNEL_NS::PostCaller([this]()->KERNEL_NS::CoTask<>
    {
        auto timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();

        timer->SetTimeOutHandler([this](KERNEL_NS::LibTimer *t)
        {
            t->Cancel();
            
            KERNEL_NS::PostCaller([this, t]()->CoTask<>
            {
                CLOG_INFO_GLOBAL(CommandMgr, "wake up getline to let poller event handle other things...");
                // 唤醒
                _WakeupCin();

                // 等待唤醒结束
                co_await CoDelay(KERNEL_NS::TimeSlice::FromSeconds(1));

                CLOG_INFO_GLOBAL(CommandMgr, "wait getline turn blocking...");

                Int32 waitCount = 0;
                while (!_inWaiting.load(std::memory_order_acquire))
                {
                    co_await CoDelay(KERNEL_NS::TimeSlice::FromSeconds(1));
                    ++waitCount;

                    if ((waitCount % 10) == 0)
                    {
                        CLOG_INFO_GLOBAL(CommandMgr, "wait getline turn blocking over: %d...", waitCount);
                    }
                }

                CLOG_INFO_GLOBAL(CommandMgr, "getline turn blocking, you can input commands.");

                // 重新定时
                t->Schedule(KERNEL_NS::TimeSlice::FromSeconds(30));
            });
        });

        timer->GetMgr()->TakeOverLifeTime(timer, [](KERNEL_NS::LibTimer *t)
        {
            KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
        });

        // 30秒唤醒一次getline让eventloop去干其他活儿
        timer->Schedule(KERNEL_NS::TimeSlice::FromSeconds(30));

        co_return;
    });
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
            _WakeupCin();
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
    _threadIdRefCmdContainer.clear();
    CRYSTAL_RELEASE_SAFE(_eventLoopThread);
}

#if CRYSTAL_TARGET_PLATFORM_WINDOWS

// 向控制台输入缓冲区注入一行文本（字符串 + 回车）
static  bool InjectConsoleInput(const std::wstring& text) {
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn == INVALID_HANDLE_VALUE || hIn == nullptr) {
        return false;
    }

    std::wstring full = text;
    full.push_back(L'\r'); // 回车，触发 getline 返回一行

    for (wchar_t ch : full) {
        INPUT_RECORD records[2] = {};

        // 按下
        records[0].EventType = KEY_EVENT;
        records[0].Event.KeyEvent.bKeyDown = TRUE;
        records[0].Event.KeyEvent.wRepeatCount = 1;
        records[0].Event.KeyEvent.uChar.UnicodeChar = ch;
        // 抬起
        records[1] = records[0];
        records[1].Event.KeyEvent.bKeyDown = FALSE;

        DWORD written = 0;
        if (!WriteConsoleInputW(hIn, records, 2, &written) || written != 2) {
            auto err = KERNEL_NS::SystemUtil::GetErrNo();
            CLOG_WARN_GLOBAL(CommandMgr, "WriteConsoleInputW Err:%d, %s", err, KERNEL_NS::SystemUtil::GetErrString(err).c_str());
            return false;
        }
    }
    return true;
}
#endif

void CommandMgr::_WakeupCin() const
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    InjectConsoleInput(L"wakeup");
#endif
}


KERNEL_END
