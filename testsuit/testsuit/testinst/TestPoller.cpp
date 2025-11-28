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
 * Date: 2022-09-12 18:36:48
 * Author: Eric Yonng
 * Description: 
 * linux下 生产者 4, 消费者1情况下测得poller qps:380w/s
 * windows下 生产者1, 消费者1情况下测得poller qps:500w/s
*/
#include <pch.h>
#include <testsuit/testinst/TestPoller.h>

class HostObj : public KERNEL_NS::CompHostObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, HostObj);
public:
    HostObj()
    :KERNEL_NS::CompHostObject(KERNEL_NS::RttiUtil::GetTypeId<HostObj>())
    {

    }

    ~HostObj()
    {

    }

    void Release() override
    {
        HostObj::Delete_HostObj(this);
    }

    virtual void DefaultMaskReady(bool isReady) override{}

    virtual void OnRegisterComps() override
    {
        // RegisterComp<KERNEL_NS::PollerFactory>();
    }

    void OnWork()
    {
        auto poller = KERNEL_NS::TlsUtil::GetDefTls()->_tlsComps->GetPoller();
        if(!poller->PrepareLoop())
        {
            SetErrCode(poller, Status::PreparePollerFail);
            g_Log->Error(LOGFMT_OBJ_TAG("prepare loop fail."));
            return;
        }

        MaskReady(true);

        // auto timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
        // auto timeoutHandler = [poller, this](KERNEL_NS::LibTimer *tm)
        // {
        //     auto drityHelper = poller->GetDirtyHelper();
        //     g_Log->Info(LOGFMT_OBJ_TAG("on work frame poller loaded:%llu"), poller->CalcLoadScore());
        //     drityHelper->MaskDirty(NULL, 0, true)->BecomeStr() = "hello dirty!";
        // };

        // timer->SetTimeOutHandler(KERNEL_CREATE_CLOSURE_DELEGATE(timeoutHandler, void, KERNEL_NS::LibTimer *));
        // timer->Schedule(500);

        g_Log->Info(LOGFMT_OBJ_TAG("event loop..."));
        // poller->OnEventLoopFrame();
        g_Log->Info(LOGFMT_OBJ_TAG("finish loop..."));

        g_Log->Info(LOGFMT_OBJ_TAG("on working end..."));
    }

    // 组件接口资源
protected:

    // 在组件初始化前 必须重写
    virtual Int32 _OnHostInit() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on host init."), ToString().c_str());
        return Status::Success;
    }

    virtual Int32 _OnPriorityLevelCompsCreated() override
    {
        auto poller = GetComp<KERNEL_NS::Poller>();
        poller->SetPepareEventWorkerHandler(this, &HostObj::_OnPollerPrepare);
        poller->SetEventWorkerCloseHandler(this, &HostObj::_OnPollerWillDestroy);
        poller->Subscribe(1, this, &HostObj::_OnHelloWorldEv);

        auto dirtyHelper = poller->GetDirtyHelper();
        dirtyHelper->Init(1);
        auto delg = KERNEL_NS::DelegateFactory::Create(this, &HostObj::_OnDirty);
        dirtyHelper->SetHandler(0, delg);

        return Status::Success;
    }

    virtual Int32 _OnCompsCreated() override
    {
        return Status::Success;
    }

    // 组件启动之后 此时可以启动线程 必须重写
    virtual Int32 _OnHostStart() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on host start."), ToString().c_str());
        return Status::Success;
    }

    virtual void _OnHostBeforeCompsWillClose() override
    {
        auto poller = GetComp<KERNEL_NS::Poller>();
        poller->QuitLoop();
        g_Log->Info(LOGFMT_OBJ_TAG("host before comp will close."));
    }

    // 在组件Close之后
    virtual void _OnHostClose() override
    {
        // 等待ready
        KERNEL_NS::CompObject *notReady = NULL;
        while(!IsAllCompsDown(notReady))
        {
            KERNEL_NS::SystemUtil::ThreadSleep(1000);
            g_Log->Warn(LOGFMT_OBJ_TAG("%s not down."), notReady->GetObjName().c_str());
        }

        _Clear();

        g_Log->Info(LOGFMT_OBJ_TAG("%s on host close."), ToString().c_str());
    }

    bool _OnPollerPrepare(KERNEL_NS::Poller *poller)
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s poller prepare."), ToString().c_str());
        return true;
    }

    void _OnPollerWillDestroy(KERNEL_NS::Poller *poller)
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s poller will destroy."), ToString().c_str());
    }

    void _OnHelloWorldEv(KERNEL_NS::PollerEvent *ev)
    {
        g_Log->Info(LOGFMT_OBJ_TAG("event coming:%s"), ev->ToString().c_str());
    }

    void _OnWork(KERNEL_NS::LibThread *t)
    {
        g_Log->Info(LOGFMT_OBJ_TAG("on working..."));
        
        // 等待ready
        KERNEL_NS::CompObject *notReady = NULL;
        while(!IsAllCompsReady(notReady))
        {
            KERNEL_NS::SystemUtil::ThreadSleep(1000);
            g_Log->Warn(LOGFMT_OBJ_TAG("%s not ready."), notReady->GetObjName().c_str());
        }

        auto poller = KERNEL_NS::TlsUtil::GetDefTls()->_tlsComps->GetPoller();
        if(!poller->PrepareLoop())
        {
            SetErrCode(poller, Status::PreparePollerFail);
            g_Log->Error(LOGFMT_OBJ_TAG("prepare loop fail."));
            return;
        }

        MaskReady(true);

        auto timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
        timer->GetMgr()->TakeOverLifeTime(timer, [](KERNEL_NS::LibTimer *t){
            KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
        });
        auto timeoutHandler = [poller](KERNEL_NS::LibTimer *tm)
        {
            auto drityHelper = poller->GetDirtyHelper();
            g_Log->Info(LOGFMT_NON_OBJ_TAG(HostObj, "on work frame poller loaded:%llu"), poller->CalcLoadScore());
            drityHelper->MaskDirty(NULL, 0, true)->BecomeStr() = "hello dirty!";
        };

        timer->SetTimeOutHandler(KERNEL_CREATE_CLOSURE_DELEGATE(timeoutHandler, void, KERNEL_NS::LibTimer *));
        timer->Schedule(500);

        g_Log->Info(LOGFMT_OBJ_TAG("event loop..."));
        poller->EventLoop();
        g_Log->Info(LOGFMT_OBJ_TAG("finish loop..."));
        poller->OnLoopEnd();

        g_Log->Info(LOGFMT_OBJ_TAG("on working end..."));
        MaskReady(false);
    }

    void _OnDirty(KERNEL_NS::LibDirtyHelper<void *, UInt32> *dirtyHelper, void *&ptr, KERNEL_NS::Variant *params)
    {
        dirtyHelper->Clear(NULL);
        g_Log->Info(LOGFMT_OBJ_TAG("dirty %p, param:%s"), ptr, params->AsStr().c_str());
    }

private:
    void _Clear()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s _Clear"), ToString().c_str());
    }

private:
};

POOL_CREATE_OBJ_DEFAULT_IMPL(HostObj);

class HostObjFactory : public KERNEL_NS::CompFactory
{
public:
    static constexpr KERNEL_NS::_Build::MT _buildType{};
    static KERNEL_NS::CompFactory *FactoryCreate()
    {
       return KERNEL_NS::ObjPoolWrap<HostObjFactory>::NewByAdapter(_buildType.V);
    }
    void Release() override
    {
        KERNEL_NS::ObjPoolWrap<HostObjFactory>::DeleteByAdapter(_buildType.V, this);
    }
    KERNEL_NS::CompObject *Create() const override
    {
        return HostObj::New_HostObj();
    }


};

struct HelloWorldRes
{
    POOL_CREATE_OBJ_DEFAULT(HelloWorldRes);

    void Release()
    {
        HelloWorldRes::Delete_HelloWorldRes(this);
    }

    virtual KERNEL_NS::LibString ToString() const
    {
        return "HelloWorldRes";
    }
};
POOL_CREATE_OBJ_DEFAULT_IMPL(HelloWorldRes);

struct  HelloWorldReq : public KERNEL_NS::PollerEvent
{
    POOL_CREATE_OBJ_DEFAULT_P1(PollerEvent, HelloWorldReq);

    HelloWorldReq()
    :KERNEL_NS::PollerEvent(0)
    {

    }

    ~HelloWorldReq()
    {

    }

    void Release() override
    {
        HelloWorldReq::Delete_HelloWorldReq(this);
    }

    virtual KERNEL_NS::LibString ToString() const override
    {
        return KERNEL_NS::PollerEvent::ToString().AppendFormat(", hello world request.");
    }
};

POOL_CREATE_OBJ_DEFAULT_IMPL(HelloWorldReq);


// static void HelloWorldEventWork(KERNEL_NS::LibThread *t, KERNEL_NS::Variant *var)
// {
//     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestPoller, "startint hello world request..."));
//     auto hostObj = var->AsPtr<HostObj>();
//     auto poller = hostObj->GetComp<KERNEL_NS::Poller>();

//     while(!t->IsDestroy())
//     {
//         auto req = HelloWorldReq::New_HelloWorldReq();
//         poller->Push(1, req);
//         KERNEL_NS::SystemUtil::ThreadSleep(1000);
//         hostObj->OnWork();
//     }

//     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestPoller, "quit hello world request..."));
// }

// void TestPoller::Run()
// {
//     HostObjFactory hostObjFactory;

//     auto& libListNodePool = KERNEL_NS::ListNode<KERNEL_NS::PollerEvent *>::GetAlloctor__ListNodeobjAlloctor();
//     libListNodePool._againstLazy = 100;
//     auto& tlsLibListNodePool = KERNEL_NS::ListNode<KERNEL_NS::PollerEvent *>::GetThreadLocalAlloctor__ListNodeobjAlloctor();
//     tlsLibListNodePool._againstLazy = 100;
    
//     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestPoller, "libListNodePool:%p, againstLazy:%d, tlsLibListNodePool:%p, againstLazy:%d")
//                         , &libListNodePool, libListNodePool._againstLazy, &tlsLibListNodePool, tlsLibListNodePool._againstLazy);

//     auto hostObj = hostObjFactory.Create();
//     auto newThread = new KERNEL_NS::LibThread;

//     auto newVar = KERNEL_NS::Variant::New_Variant();
//     newVar->BecomePtr() = hostObj;
//     auto delg = KERNEL_NS::DelegateFactory::Create(&HelloWorldEventWork);
//     newThread->AddTask2(delg, newVar);

//     hostObj->Init();
//     hostObj->Start();

//     newThread->Start();

//     getchar();

//     newThread->HalfClose();
//     newThread->FinishClose();
    
//     hostObj->WillClose();
//     hostObj->Close();
// }

static KERNEL_NS::SmartPtr<KERNEL_NS::Poller, KERNEL_NS::AutoDelMethods::Release> s_Poller;
static KERNEL_NS::ConcurrentPriorityQueue<KERNEL_NS::PollerEvent *> *g_concurrentQueue = NULL;

// 测试性能
static std::atomic<Int64> g_genNum{0};
static std::atomic<Int64> g_consumNum{0};

static const Int32 g_maxConcurrentLevel = 1;

static void _OnPollerEvent(KERNEL_NS::PollerEvent *ev)
{
    // g_Log->Debug(LOGFMT_NON_OBJ_TAG(KERNEL_NS::Poller, "recv event:%s"), ev->ToString().c_str());
    // g_Log->Info(LOGFMT_NON_OBJ_TAG(TestPoller, "test crash..........."));
    // KERNEL_NS::SystemUtil::ThreadSleep(5000);

    ++g_consumNum;
    // int b = 0;
    // int a = 1 / b;
}

struct AcEvent : public KERNEL_NS::PollerEvent
{
    POOL_CREATE_OBJ_P1(1, 1024, PollerEvent, AcEvent);
    
    AcEvent()
    :KERNEL_NS::PollerEvent(1)
    {

    }

    virtual void Release() override
    {
        // delete this;
        AcEvent::DeleteThreadLocal_AcEvent(this);
        // AcEvent::Delete_AcEvent(this);
    }

};

POOL_CREATE_OBJ_IMPL(1, 1024, AcEvent);

static void _OnPoller(KERNEL_NS::LibThread *t)
{
    // std::vector<KERNEL_NS::LibList<KERNEL_NS::PollerEvent *, KERNEL_NS::_Build::MT> *> priorityEvents;
    // const Int64 priorityQueueSize = static_cast<Int64>(g_concurrentQueue->GetMaxLevel() + 1);
    // for(Int64 idx = 0; idx < priorityQueueSize; ++idx)
    //     priorityEvents.push_back(KERNEL_NS::LibList<KERNEL_NS::PollerEvent *, KERNEL_NS::_Build::MT>::New_LibList());
    // TODO:
    if(!s_Poller->PrepareLoop())
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(TestPoller, "prepare loop fail."));
        return;
    }

    // s_Poller->EventLoop();
    s_Poller->EventLoop();
    s_Poller->OnLoopEnd();

    // while(!t->IsDestroy())
    // {
    //     g_concurrentQueue->MergeTailAllTo(priorityEvents);
    //     for(auto list:priorityEvents)
    //     {
    //         if(!list || list->IsEmpty())
    //             continue;

    //         for(auto node = list->Begin(); node;)
    //         {
    //             auto data = node->_data;
    //             ++g_consumNum;

    //             data->Release();
    //             node = list->Erase(node);
    //         }
    //     }
    // }
}

static void _OnTask(KERNEL_NS::LibThreadPool *t, KERNEL_NS::Variant *param)
{
    Int32 idx = param->AsInt32();
    KERNEL_NS::Poller *poller = s_Poller.AsSelf();

    // 10秒清理一次
    auto timerMgr = KERNEL_NS::TlsUtil::GetPoller()->GetTimerMgr();
    auto tlsOwner = KERNEL_NS::TlsUtil::GetTlsCompsOwner();
    auto memoryCleaner = tlsOwner->GetComp<KERNEL_NS::TlsMemoryCleanerComp>();
    memoryCleaner->SetIntervalMs(10 * 1000);

    while (!t->IsDestroy())
    {
        auto ev = AcEvent::NewThreadLocal_AcEvent();
        ++g_genNum;
        poller->Push(ev);

        timerMgr->Drive();

        // g_concurrentQueue->PushQueue(idx, &((new KERNEL_NS::LibString())->AppendFormat("hello idx:%d", idx)));
        // g_concurrentQueue->PushQueue(idx, new AcEvent());
        // ++g_genNum;
        // KERNEL_NS::SystemUtil::ThreadSleep(5000);
    }

    memoryCleaner->WillClose();
    memoryCleaner->Close();
} 

static void _OnMonitor(KERNEL_NS::LibThreadPool *t, KERNEL_NS::Variant *param)
{
    KERNEL_NS::Poller *poller = s_Poller.AsSelf();

    KERNEL_NS::LibString err, threadName;
    KERNEL_NS::SystemUtil::GetCurrentThreadName(threadName, err);

    while (!t->IsDestroy())
    {
        KERNEL_NS::SystemUtil::ThreadSleep(1000);
        const Int64 genNum = g_genNum;
        const Int64 comsumNum = g_consumNum;
        const Int64 backlogNum = static_cast<Int64>(poller->GetEventAmount());
        g_genNum -= genNum;
        g_consumNum -= comsumNum;

        g_Log->Monitor("thread name:%s,Monitor:[gen:%lld, consum:%lld, backlog:%lld]", threadName.c_str(), genNum, comsumNum, backlogNum);
    }
}

class HelloCrossPollerRequest
{
    POOL_CREATE_OBJ_DEFAULT(HelloCrossPollerRequest);

public:
    HelloCrossPollerRequest()
    {
        
    }
    ~HelloCrossPollerRequest()
    {
        
    }

    void Release()
    {
        HelloCrossPollerRequest::Delete_HelloCrossPollerRequest(this);
    }
    KERNEL_NS::LibString ToString() const
    {
        return KERNEL_NS::LibString().AppendFormat("hello cross poller:%s", _info.c_str());
    }
    KERNEL_NS::LibString _info;
};

class HelloCrossPollerResponse
{
    POOL_CREATE_OBJ_DEFAULT(HelloCrossPollerResponse);

public:
    HelloCrossPollerResponse()
    {
        
    }
    ~HelloCrossPollerResponse()
    {
        
    }

    void Release()
    {
        HelloCrossPollerResponse::Delete_HelloCrossPollerResponse(this);
    }
    KERNEL_NS::LibString ToString() const
    {
        return KERNEL_NS::LibString().AppendFormat("hello cross poller response:%s", _info.c_str());
    }
    KERNEL_NS::LibString _info;
};

POOL_CREATE_OBJ_DEFAULT_IMPL(HelloCrossPollerRequest);
POOL_CREATE_OBJ_DEFAULT_IMPL(HelloCrossPollerResponse);

class ThreadStartup : public KERNEL_NS::IThreadStartUp
{
    POOL_CREATE_OBJ_DEFAULT_P1(IThreadStartUp, ThreadStartup);

public:
    ThreadStartup(std::atomic<CRYSTAL_NET::kernel::Poller *> &otherPoller)
        : _otherPoller(otherPoller)
    {
        
    }

private:
    std::atomic<CRYSTAL_NET::kernel::Poller *> &_otherPoller;
    
public:
    virtual void Run() override
    {
        KERNEL_NS::RunRightNow([this]() mutable -> KERNEL_NS::CoTask<> 
        {
            auto poller = KERNEL_NS::TlsUtil::GetPoller();

            // 订阅 HelloCrossPollerRequest 消息
            poller->SubscribeObjectEvent<HelloCrossPollerRequest>([this](KERNEL_NS::StubPollerEvent *ev) mutable 
            {
                while (_otherPoller.load() == NULL)
                {
                    KERNEL_NS::SystemUtil::ThreadSleep(10);
                }
        
                auto stub = ev->_stub;
                auto request = ev->CastTo<KERNEL_NS::ObjectPollerEvent<HelloCrossPollerRequest>>();
                g_Log->Info(LOGFMT_NON_OBJ_TAG(TestPoller, "HelloCrossPollerRequest :%s"), request->ToString().c_str());

                // 返回包
                auto res = KERNEL_NS::ObjectPollerEvent<HelloCrossPollerResponse>::New_ObjectPollerEvent(stub
                    , true, KERNEL_NS::TlsUtil::GetPoller(), ev->_srcChannel);
                res->_obj = HelloCrossPollerResponse::New_HelloCrossPollerResponse();
                res->_obj->_info = request->_obj->_info;
                _otherPoller.load()->Push(res);
            });
        
            co_return;
        });
    }
    
    virtual void Release() override
    {
        ThreadStartup::Delete_ThreadStartup(this);
    }
};

class ThreadStartup2 : public KERNEL_NS::IThreadStartUp
{
    POOL_CREATE_OBJ_DEFAULT_P1(IThreadStartUp, ThreadStartup2);

public:
    ThreadStartup2(std::atomic<CRYSTAL_NET::kernel::Poller *> &otherPoller)
    : _otherPoller(otherPoller)
    {
        
    }

    virtual void Release() override
    {
        ThreadStartup2::Delete_ThreadStartup2(this);
    }
    virtual void Run() override
    {
        KERNEL_NS::PostCaller([this]() mutable  ->KERNEL_NS::CoTask<>
        {
            while (_otherPoller.load() == NULL)
            {
                co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromMilliSeconds(10));
            }
                
            // 发送消息
            auto req = HelloCrossPollerRequest::New_HelloCrossPollerRequest();
            req->_info = "hello cross-poller req";
                
            // 跨线程发送异步消息
            auto res = co_await KERNEL_NS::TlsUtil::GetPoller()->
            SendToAsync<HelloCrossPollerResponse, HelloCrossPollerRequest>(*_otherPoller.load(), req);
                    
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestPoller, "res :%s"), res->ToString().c_str());
        });
    }

private:
    std::atomic<CRYSTAL_NET::kernel::Poller *> &_otherPoller;
};

POOL_CREATE_OBJ_DEFAULT_IMPL(ThreadStartup);

class TestTimeoutStartup : public KERNEL_NS::IThreadStartUp
{
    POOL_CREATE_OBJ_DEFAULT_P1(IThreadStartUp, TestTimeoutStartup);

public:
    TestTimeoutStartup(KERNEL_NS::LibEventLoopThread * target)
    : _target(target)
    {
        
    }

    virtual void Run() override
    {
        KERNEL_NS::PostCaller([this]() mutable  -> KERNEL_NS::CoTask<>
        {
            auto targetPoller = co_await _target->GetPoller();

            auto req = HelloWorldReq::New_HelloWorldReq();
            auto res = co_await targetPoller->SendAsync<HelloWorldRes, HelloWorldReq>(req).SetTimeout(KERNEL_NS::TimeSlice::FromSeconds(5));

            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestTimeoutStartup, "res return"));
        });
    }
    
    virtual void Release() override
    {
        TestTimeoutStartup::Delete_TestTimeoutStartup(this);
    }

    KERNEL_NS::LibEventLoopThread * _target;
};


class TestTimeoutStartup2 : public KERNEL_NS::IThreadStartUp
{
    POOL_CREATE_OBJ_DEFAULT_P1(IThreadStartUp, TestTimeoutStartup2);

public:
    TestTimeoutStartup2()
    {
        
    }

    virtual void Run() override
    {
        auto poller = KERNEL_NS::TlsUtil::GetPoller();
        poller->SubscribeObjectEvent<HelloWorldReq>([](KERNEL_NS::StubPollerEvent *req)
        {
            auto objEvent = req->CastTo<KERNEL_NS::ObjectPollerEvent<HelloWorldReq>>();
        });
    }
    
    virtual void Release() override
    {
        TestTimeoutStartup2::Delete_TestTimeoutStartup2(this);
    }
};

static void TestCrossPoller()
{
    std::atomic<CRYSTAL_NET::kernel::Poller *> _poller1 = {NULL};
    std::atomic<CRYSTAL_NET::kernel::Poller *> _poller2 = {NULL};

    KERNEL_NS::SmartPtr<KERNEL_NS::LibThread> thread1 = new KERNEL_NS::LibThread(ThreadStartup::New_ThreadStartup(_poller2));
    KERNEL_NS::SmartPtr<KERNEL_NS::LibThread> thread2 = new KERNEL_NS::LibThread(ThreadStartup2::New_ThreadStartup2(_poller1));
    KERNEL_NS::SmartPtr<KERNEL_NS::LibEventLoopThread> thread3 = new KERNEL_NS::LibEventLoopThread("host", TestTimeoutStartup2::New_TestTimeoutStartup2());
    KERNEL_NS::SmartPtr<KERNEL_NS::LibEventLoopThread> thread4 = new KERNEL_NS::LibEventLoopThread("test timeout", TestTimeoutStartup::New_TestTimeoutStartup(thread3));

    thread3->Start();
    thread4->Start();
    //
    // thread1->AddTask2([&_poller1, &_poller2](KERNEL_NS::LibThread *t,  KERNEL_NS::Variant *) mutable  
    // {
    //     auto poller = KERNEL_NS::TlsUtil::GetPoller();
    //     if(!poller->PrepareLoop())
    //     {
    //         g_Log->Error(LOGFMT_NON_OBJ_TAG(TestPoller, "thread1 prepare loop fail."));
    //         return;
    //     }
    //
    //     _poller1 = poller;
    //     
    //     // s_Poller->EventLoop();
    //     poller->EventLoop();
    //     poller->OnLoopEnd();
    // });
    //
    // thread2->AddTask2([&_poller1, &_poller2](KERNEL_NS::LibThread *t,  KERNEL_NS::Variant *) mutable  
    // {
    //     auto poller = KERNEL_NS::TlsUtil::GetPoller();
    //     if(!poller->PrepareLoop())
    //     {
    //         g_Log->Error(LOGFMT_NON_OBJ_TAG(TestPoller, "thread1 prepare loop fail."));
    //         return;
    //     }
    //     _poller2 = poller;
    //     poller->EventLoop();
    //     poller->OnLoopEnd();
    // });
    //
    // thread1->Start();
    // thread2->Start();
    
    getchar();

    thread3->HalfClose();
    thread4->HalfClose();
    
    // thread1->HalfClose();
    // thread2->HalfClose();
    //
    // thread1->FinishClose();
    // thread2->FinishClose();

    thread3->FinishClose();
    thread4->FinishClose();
}

void TestPoller::Run()
{
    TestCrossPoller();
}


// void TestPoller::Run()
// {
//     g_concurrentQueue = KERNEL_NS::ConcurrentPriorityQueue<KERNEL_NS::PollerEvent *>::New_ConcurrentPriorityQueue();
//     s_Poller = reinterpret_cast<KERNEL_NS::Poller *>(KERNEL_NS::PollerFactory::FactoryCreate()->Create());
//     s_Poller->SetMaxPriorityLevel(g_maxConcurrentLevel);
//     s_Poller->Subscribe(1, &_OnPollerEvent);
//     g_concurrentQueue->SetMaxLevel(g_maxConcurrentLevel);
//     g_concurrentQueue->Init();
//
//     auto err = s_Poller->Init();
//     if(err != Status::Success)
//     {
//         g_Log->Error(LOGFMT_NON_OBJ_TAG(TestPoller, "init poller fail."));
//         return;
//     }
//
//     err = s_Poller->Start();
//     if(err != Status::Success)
//     {
//         g_Log->Error(LOGFMT_NON_OBJ_TAG(TestPoller, "start poller fail."));
//         return;
//     }
//
//     KERNEL_NS::LibThread *pollerThread = new KERNEL_NS::LibThread;
//     pollerThread->AddTask(&_OnPoller);
//     pollerThread->Start();
//
//     KERNEL_NS::SmartPtr<KERNEL_NS::LibThreadPool, KERNEL_NS::AutoDelMethods::Release> pool = new KERNEL_NS::LibThreadPool;
//     pool->Init(0, g_maxConcurrentLevel + 2);
//
//     pool->SetPoolName(KERNEL_NS::LibString().AppendFormat("TestPoller"));
//
//     for(Int32 idx=1; idx <=g_maxConcurrentLevel; ++idx)
//     {
//         KERNEL_NS::Variant *var=KERNEL_NS::Variant::New_Variant();
//         *var = idx;
//         pool->AddTask2(&_OnTask, var, false, 0);
//     }
//
//     pool->AddTask2(&_OnMonitor, NULL, false, 0);
//
//     pool->Start(true, g_maxConcurrentLevel + 1);
//
//     getchar();
//
//     pool->HalfClose();
//
//     s_Poller->QuitLoop();
//     pollerThread->HalfClose();
//
//     pool->FinishClose();
//     pollerThread->FinishClose();
//
//     s_Poller.Release();
//     g_concurrentQueue->Destroy();
//     KERNEL_NS::ConcurrentPriorityQueue<KERNEL_NS::PollerEvent *>::Delete_ConcurrentPriorityQueue(g_concurrentQueue);
//
//     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestPoller, "test poller finished."));
//
//     // 测试poller性能
// }
