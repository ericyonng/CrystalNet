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
// MPMCQueue 单生产者单消费者情况下 400w qps（windows下）
// SPSCQueue 单生产者单消费者 >= 500w qps (windows)
// poller MPMCQueue/SPSCQueue优化后, 如果使用Send qps:300w qps(单生产者单消费者情况), 2个生产者1个消费者情况下 200w qps
// 如果使用channel进一步优化：单生产者单消费者下 290wqps, 2个个生产者单消费者情况下238wqps,4生产者1个消费者情况下200w qps

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
// static KERNEL_NS::MPMCQueue<TestGenReq *> *g_ReqList = NULL;
static KERNEL_NS::SPSCQueue<TestGenReq *> *g_ReqList = NULL;
static std::atomic<Int64> g_Version = {0};

// 生产者
class ThreadGeneratorStartup : public KERNEL_NS::IThreadStartUp
{
public:
    ThreadGeneratorStartup(KERNEL_NS::LibEventLoopThread *consumer, int id)
        :_consumer(consumer)
    ,_id(id)
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
                // g_ReqList->Push(req);
                
                while (!g_ReqList->TryPush( req))
                {
                    ;
                }
                

                // 失败重试
                // while (!g_ReqList->TryPush(req))
                //     ;
            }
        });
    }
    virtual void Release() override
    {
        delete this;
    }

private:
    KERNEL_NS::LibEventLoopThread *_consumer;
    int _id;
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
               // co_await KERNEL_NS::Waiting();
               
               data = NULL;
               if (!g_ReqList->TryPop(data))
                   continue;

               // 失败重试
               // if (!g_ReqList->TryPop(data) || data == NULL || data == (void *)(0xcdcdcdcdcdcdcdcd))
               //     continue;

               // g_Log->Info(LOGFMT_NON_OBJ_TAG(ThreadConsumerStartup, "data, ver:%lld"), data->_version);
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

class ThreadControlStartup : public KERNEL_NS::IThreadStartUp
{
public:
    ThreadControlStartup(KERNEL_NS::Poller *control)
        :_control(control)
    {
        
    }

    virtual void Run() override
    {
        KERNEL_NS::PostCaller([this]()->KERNEL_NS::CoTask<void>
        {
            co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromSeconds(1));

            getchar();

            g_Log->Info(LOGFMT_OBJ_TAG("will quit controler: %s."), _control->ToString().c_str());

            _control->QuitLoop();
        });
    }
    virtual void Release() override
    {
        delete this;
    }

private:
    KERNEL_NS::Poller *_control;
};


#pragma region // test poller events with >= 200w qps


// 消费者
class ThreadPollerConsumerStartup : public KERNEL_NS::IThreadStartUp
{
public:
    ThreadPollerConsumerStartup()
    {
        
    }
    virtual void Run() override
    {
        auto poller = KERNEL_NS::TlsUtil::GetPoller();
        poller->SubscribeObjectEvent<TestGenReq>([](KERNEL_NS::StubPollerEvent *req)
        {
            g_ConsumeNum.fetch_add(1, std::memory_order_release);
        });
    }
    virtual void Release() override
    {
        delete this;
    }
};


// 生产者
class ThreadPollerGeneratorStartup : public KERNEL_NS::IThreadStartUp
{
public:
    ThreadPollerGeneratorStartup(KERNEL_NS::LibEventLoopThread *consumer, int id)
        :_consumer(consumer)
    ,_id(id)
    {
        
    }
    virtual void Run() override
    {
        // 投递到事件循环中
        KERNEL_NS::PostCaller([this]()->KERNEL_NS::CoTask<>
        {
            auto poller = KERNEL_NS::TlsUtil::GetPoller();

            auto consumerPoller = co_await _consumer->GetPoller();
            auto channel = co_await consumerPoller->ApplyChannel();
            auto toSelfChannel = co_await poller->ApplyChannel();
            co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromMilliSeconds(1));

            while (!poller->IsQuit())
            {
                //co_await KERNEL_NS::Waiting();
                
                auto req = TestGenReq::New_TestGenReq();
                req->_version = g_Version.fetch_add(1, std::memory_order_release) + 1;
                g_GenNum.fetch_add(1, std::memory_order_release);
                channel->Send(poller, req);
                
                // consumerPoller->Send(req);
                // g_ReqList->Push(req);
                
                // while (!g_ReqList->TryPush( req))
                // {
                //     ;
                // }
                

                // 失败重试
                // while (!g_ReqList->TryPush(req))
                //     ;
            }
        });
    }
    virtual void Release() override
    {
        delete this;
    }

private:
    KERNEL_NS::LibEventLoopThread *_consumer;
    int _id;
};

#pragma endregion
void TestLockFree::Run()
{
    // g_ReqList = KERNEL_NS::MPMCQueue<TestGenReq *>::New_MPMCQueue();
    g_ReqList = KERNEL_NS::SPSCQueue<TestGenReq *>::New_SPSCQueue();
    
    // auto consumer = new KERNEL_NS::LibEventLoopThread("consumer1", new ThreadConsumerStartup());
    auto consumer = new KERNEL_NS::LibEventLoopThread("poller_consumer1", new ThreadPollerConsumerStartup());
    auto consumer2 = new KERNEL_NS::LibEventLoopThread("consumer2", new ThreadConsumerStartup());

    // 1. 构建生产者消费者模型
    // auto genThread = new KERNEL_NS::LibEventLoopThread("gen", new ThreadGeneratorStartup(consumer, 1));
    auto genThread = new KERNEL_NS::LibEventLoopThread("poller_gen", new ThreadPollerGeneratorStartup(consumer, 1));
    auto genThread2 = new KERNEL_NS::LibEventLoopThread("poller_gen", new ThreadPollerGeneratorStartup(consumer, 2));
    auto genThread3 = new KERNEL_NS::LibEventLoopThread("poller_gen", new ThreadPollerGeneratorStartup(consumer, 3));
    auto genThread4 = new KERNEL_NS::LibEventLoopThread("poller_gen", new ThreadPollerGeneratorStartup(consumer, 4));
    // auto genThread2 = new KERNEL_NS::LibEventLoopThread("gen", new ThreadGeneratorStartup(consumer, 2));
    // auto genThread3 = new KERNEL_NS::LibEventLoopThread("gen", new ThreadGeneratorStartup(consumer, 3));
    // auto genThread4 = new KERNEL_NS::LibEventLoopThread("gen", new ThreadGeneratorStartup(consumer, 4));

    auto controlMgrThread = new KERNEL_NS::LibEventLoopThread("control", new ThreadControlStartup(KERNEL_NS::TlsUtil::GetPoller()));

    controlMgrThread->Start();
    
    consumer->Start();
    // consumer2->Start();
    genThread->Start();
    genThread2->Start();
    // genThread3->Start();
    // genThread4->Start();

    auto poller = KERNEL_NS::TlsUtil::GetPoller();
    KERNEL_NS::SmartPtr<KERNEL_NS::LibTimer, KERNEL_NS::AutoDelMethods::CustomDelete> timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
    timer.SetClosureDelegate([](void *p)
    {
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(KERNEL_NS::KernelCastTo<KERNEL_NS::LibTimer>(p));
    });
    timer->SetTimeOutHandler([](KERNEL_NS::LibTimer *timer)
    {
        auto genNum = g_GenNum.exchange(0, std::memory_order_acq_rel);
        auto consume = g_ConsumeNum.exchange(0, std::memory_order_acq_rel);
        g_Log->Custom("genNum:%lld, consume:%lld", genNum, consume);
    });
    timer->Schedule(KERNEL_NS::TimeSlice::FromSeconds(1));

    auto worker = g_MemoryMonitor->MakeWorkTask();
    auto workerTimer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
    workerTimer->SetTimeOutHandler([worker](KERNEL_NS::LibTimer *timer)
    {
        worker->Invoke();
    });
    workerTimer->Schedule(KERNEL_NS::TimeSlice::FromMinutes(1));
    
    poller->PrepareLoop();
    poller->EventLoop();
    poller->OnLoopEnd();

    genThread->HalfClose();
    genThread2->HalfClose();
    consumer->HalfClose();
    controlMgrThread->HalfClose();

    genThread->FinishClose();
    genThread2->FinishClose();
    consumer->FinishClose();
    controlMgrThread->FinishClose();

    KERNEL_NS::SPSCQueue<TestGenReq *>::Delete_SPSCQueue(g_ReqList);
    g_ReqList = NULL;
}

