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
 * Date: 2025-11-12 18:04:29
 * Author: Eric Yonng
 * Description: 
*/

#include "pch.h"
#include "TestEventLoopThread.h"

struct  TestEventLoopDataReq
{
    void Release()
    {
        delete this;
    }

    KERNEL_NS::LibString ToString() const
    {
        return info;
    }
    
    KERNEL_NS::LibString info = "hello test event loop";
};

struct  TestEventLoopDataPush
{
    void Release()
    {
        delete this;
    }

    KERNEL_NS::LibString ToString() const
    {
        return info;
    }
    
    KERNEL_NS::LibString info = "hello i'm push";
};


struct TestEventLoopDataRes
{
    void Release()
    {
        delete this;
    }

    KERNEL_NS::LibString ToString() const
    {
        return info;
    }
    
    KERNEL_NS::LibString info = "TestEventLoopDataRes back";
};

class EventLoopThreadStartup : public KERNEL_NS::IThreadStartUp
{
    POOL_CREATE_OBJ_DEFAULT_P1(IThreadStartUp, EventLoopThreadStartup);

    virtual void Run() override
    {
        // 先订阅消息
        auto poller = KERNEL_NS::TlsUtil::GetPoller();
        poller->SubscribeObjectEvent<TestEventLoopDataReq>([](KERNEL_NS::StubPollerEvent *ev) mutable 
        {
            TestEventLoopDataReq *req = ev->CastTo<KERNEL_NS::ObjectPollerEvent<TestEventLoopDataReq>>()->_obj;
            KERNEL_NS::LibString name;
            KERNEL_NS::LibString err;
            KERNEL_NS::SystemUtil::GetCurrentThreadName(name, err);
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestEventLoopThread, "req:%s, thread:%s"), req->ToString().c_str(), name.c_str());

            // 返回消息
            auto res = new TestEventLoopDataRes();
            if(LIKELY(ev->_srcChannel))
            {
                ev->_srcChannel->SendResponse(ev->_stub, KERNEL_NS::TlsUtil::GetPoller(), res);
            }
            else
            {
                ev->_srcPoller->SendResponse(ev->_stub, res);
            }
        });

        poller->SubscribeObjectEvent<TestEventLoopDataPush>([](KERNEL_NS::StubPollerEvent *ev) mutable 
        {
            TestEventLoopDataPush *push = ev->CastTo<KERNEL_NS::ObjectPollerEvent<TestEventLoopDataPush>>()->_obj;
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestEventLoopThread, "push:%s"), push->ToString().c_str());
        });
    }
    virtual void Release() override
    {
        EventLoopThreadStartup::Delete_EventLoopThreadStartup(this);
    }
};

POOL_CREATE_OBJ_DEFAULT_IMPL(EventLoopThreadStartup);

void TestEventLoopThread::Run()
{
  auto eventLoopThread = new KERNEL_NS::LibEventLoopThread("TestEventLoopThread", EventLoopThreadStartup::New_EventLoopThreadStartup());
  eventLoopThread->Start();

    auto poller = KERNEL_NS::TlsUtil::GetPoller();

    poller->PrepareLoop();

#ifdef CRYSTAL_NET_CPP20
    // 控制event loop退出
    KERNEL_NS::PostCaller([eventLoopThread]() mutable ->KERNEL_NS::CoTask<>
    {
        co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromSeconds(1));

        // 发送请求消息
        auto req = new TestEventLoopDataReq();
        auto res = co_await eventLoopThread->SendAsync<TestEventLoopDataRes, TestEventLoopDataReq>(req);

        auto push = new TestEventLoopDataPush();
        co_await eventLoopThread->SendAsync2<TestEventLoopDataPush>(push);

        // g_Log->Info(LOGFMT_NON_OBJ_TAG(TestEventLoopThread, "waiting quit loop res:%s..."), res->ToString().c_str());

        getchar();
        
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestEventLoopThread, "quit loop."));
        KERNEL_NS::TlsUtil::GetPoller()->QuitLoop();
    });
#endif

    poller->EventLoop();
    poller->OnLoopEnd();

    eventLoopThread->Close();
}
