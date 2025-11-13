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
// Date: 2025-11-13 00:11:39
// Author: Eric Yonng
// Description:
// 改之前：190w qps

#include <pch.h>
#include "TestLockFree.h"

struct TestGenReq
{
    POOL_CREATE_OBJ_DEFAULT(TestGenReq);
    
public:
    void Release()
    {
        TestGenReq::Delete_TestGenReq(this);
    }

    KERNEL_NS::LibString ToString() const
    {
        return TestStr;
    }
    
    char TestStr[256] = {'h','e','l','l','o','!'};

    Int64 _version = 0;
};

POOL_CREATE_OBJ_DEFAULT_IMPL(TestGenReq);


struct TestGenRes
{
    POOL_CREATE_OBJ_DEFAULT(TestGenRes);
    
public:
    void Release()
    {
        TestGenRes::Delete_TestGenRes(this);
    }

    KERNEL_NS::LibString ToString() const
    {
        return TestStr;
    }
    
    char TestStr[256] = {'h','e','l','l','o','!', 'r','e','s','.'};
};
POOL_CREATE_OBJ_DEFAULT_IMPL(TestGenRes);

static std::atomic<Int64> g_GenNum = 0;
static std::atomic<Int64> g_ConsumeNum = 0;
static KERNEL_NS::RingBuffer<TestGenReq *> *g_ReqList = NULL;
static std::atomic<Int64> g_Version = {0};

// 生产者
class ThreadGeneratorStartup : public KERNEL_NS::IThreadStartUp
{
public:
    ThreadGeneratorStartup(KERNEL_NS::LibEventLoopThread *consumer)
        :_consumer(consumer)
    {
        
    }
    virtual void Run() override
    {
        // 投递到事件循环中
        KERNEL_NS::PostCaller([this]()->KERNEL_NS::CoTask<>
        {
            co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromMilliSeconds(1));

            auto poller = KERNEL_NS::TlsUtil::GetPoller();
            while (!poller->IsQuit())
            {
                auto req = TestGenReq::New_TestGenReq();
                req->_version = g_Version.fetch_add(1, std::memory_order_release) + 1;
                g_GenNum.fetch_add(1, std::memory_order_release);
                g_ReqList->Push( req);
            }
        });
    }
    virtual void Release() override
    {
        delete this;
    }

private:
    KERNEL_NS::LibEventLoopThread *_consumer;
};

// 消费者
class ThreadConsumerStartup : public KERNEL_NS::IThreadStartUp
{
public:
    ThreadConsumerStartup()
    {
        
    }
    virtual void Run() override
    {
        auto poller = KERNEL_NS::TlsUtil::GetPoller();
        KERNEL_NS::PostCaller([]()->KERNEL_NS::CoTask<void>
        {
           co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromMilliSeconds(1));
            auto poller = KERNEL_NS::TlsUtil::GetPoller();

            TestGenReq *data = NULL;
           while (!poller->IsQuit())
           {
               data = NULL;
               if (!g_ReqList->Pop(data))
                   continue;

               data->Release();
               g_ConsumeNum.fetch_add(1, std::memory_order_release);

               // while (head)
               // {
               //     auto tmp = head;
               //     // g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLockFree, "LockFree head:%p, version:%lld"), tmp, tmp->_data->_version);
               //     head = head->_next;
               //     tmp->_data->Release();
               //     KERNEL_NS::LockFreeNode<TestGenReq *>::Delete_LockFreeNode(tmp);
               // }
           }
            
        });
        // poller->SubscribeObjectEvent<TestGenReq>([](KERNEL_NS::StubPollerEvent *ev)
        // {
        //     g_ConsumeNum.fetch_add(1, std::memory_order_release);
        // });
    }
    virtual void Release() override
    {
        delete this;
    }
};


void TestLockFree::Run()
{
    g_ReqList = KERNEL_NS::RingBuffer<TestGenReq *>::New_RingBuffer();
    
    auto consumer = new KERNEL_NS::LibEventLoopThread("consumer1", new ThreadConsumerStartup());
    auto consumer2 = new KERNEL_NS::LibEventLoopThread("consumer2", new ThreadConsumerStartup());

    // 1. 构建生产者消费者模型
    auto genThread = new KERNEL_NS::LibEventLoopThread("gen", new ThreadGeneratorStartup(consumer));
    auto genThread2 = new KERNEL_NS::LibEventLoopThread("gen", new ThreadGeneratorStartup(consumer));
    auto genThread3 = new KERNEL_NS::LibEventLoopThread("gen", new ThreadGeneratorStartup(consumer));
    auto genThread4 = new KERNEL_NS::LibEventLoopThread("gen", new ThreadGeneratorStartup(consumer));

    consumer->Start();
    consumer2->Start();
    genThread->Start();
    // genThread2->Start();
    // genThread3->Start();
    // genThread4->Start();

    auto poller = KERNEL_NS::TlsUtil::GetPoller();
    auto timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
    timer->SetTimeOutHandler([](KERNEL_NS::LibTimer *timer)
    {
        auto genNum = g_GenNum.exchange(0, std::memory_order_acq_rel);
        auto consume = g_ConsumeNum.exchange(0, std::memory_order_acq_rel);
        g_Log->Custom("genNum:%lld, consume:%lld", genNum, consume);
    });
    timer->Schedule(KERNEL_NS::TimeSlice::FromSeconds(1));
    
    poller->PrepareLoop();
    poller->EventLoop();
    poller->OnLoopEnd();
}

