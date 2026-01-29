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
 * Date: 2025-11-12 15:13:00
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/thread/LibEventLoopThread.h>
#include <kernel/comp/thread/LibThread.h>

KERNEL_BEGIN

LibEventLoopThread::LibEventLoopThread(const LibString &threadName, IThreadStartUp *startUp)
 :_thread(new LibThread(startUp))
{
    if (!threadName.empty())
        _thread->SetThreadName(threadName);
}

LibEventLoopThread::~LibEventLoopThread()
{
 CRYSTAL_DELETE_SAFE(_thread);
}

void LibEventLoopThread::Release()
{
  delete this;
}

void LibEventLoopThread::Start()
{
    if(_thread->IsStart())
        return;

    CRYSTAL_TRACE("thread will start %s", _thread->ToString().c_str())

     _thread->AddTask2([](LibThread *thread,  KERNEL_NS::Variant *)
     {
        auto poller = KERNEL_NS::TlsUtil::GetPoller();
        if(!poller->PrepareLoop())
        {
            CRYSTAL_TRACE("thread1 prepare loop fail.")
            return;
        }

        poller->EventLoop();
        poller->OnLoopEnd();
     });

    _thread->Start();
}

void LibEventLoopThread::Close()
{
  if (!HalfClose())
   return;

  FinishClose();
}

bool LibEventLoopThread::HalfClose()
{
  return _thread->HalfClose();
}

void LibEventLoopThread::FinishClose()
{
    auto poller = _thread->GetPollerNoAsync();
    for (Int32 idx = 0; idx < 10; ++idx)
    {
        KERNEL_NS::SystemUtil::ThreadSleep(1000);
        poller = _thread->GetPollerNoAsync();
        if (g_Log && g_Log->IsEnable(LogLevel::Warn))
            g_Log->Warn(LOGFMT_OBJ_TAG("thread:%s, poller not ready"), _thread->ToString().c_str());

        if (poller)
            break;
    }

    // 退出事件循环
    if (poller)
        poller->QuitLoop();

    if (g_Log && g_Log->IsEnable(LogLevel::Info))
        g_Log->Info(LOGFMT_OBJ_TAG("poller will quit, thread:%s"), _thread->ToString().c_str());
    
    _thread->FinishClose();
}

#ifdef CRYSTAL_NET_CPP20
CoTask<const Poller *> LibEventLoopThread::GetPoller() const
{
  co_return co_await _thread->GetPoller();
}

CoTask<Poller *> LibEventLoopThread::GetPoller()
{
  co_return co_await _thread->GetPoller();
}
#endif

const Poller * LibEventLoopThread::GetPollerNoAsync() const
{
    return _thread->GetPollerNoAsync();
}

Poller *LibEventLoopThread::GetPollerNoAsync()
{
    return _thread->GetPollerNoAsync();
}

KERNEL_END