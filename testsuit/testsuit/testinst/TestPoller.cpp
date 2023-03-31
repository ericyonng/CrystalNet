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
*/
#include <pch.h>
#include <testsuit/testinst/TestPoller.h>

class HostObj : public KERNEL_NS::CompHostObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, HostObj);
public:
    HostObj()
    {

    }

    ~HostObj()
    {

    }

    void Release()
    {
        HostObj::Delete_HostObj(this);
    }

    virtual void DefaultMaskReady(bool isReady){}

    virtual void OnRegisterComps()
    {
        RegisterComp<KERNEL_NS::PollerFactory>();
    }

    void OnWork()
    {
        auto poller = GetComp<KERNEL_NS::Poller>();
        if(!poller->PrepareLoop())
        {
            SetErrCode(poller, Status::PreparePollerFail);
            g_Log->Error(LOGFMT_OBJ_TAG("prepare loop fail."));
            return;
        }

        auto defObj = KERNEL_NS::TlsUtil::GetDefTls();
        if(UNLIKELY(defObj->_poller))
            g_Log->Warn(LOGFMT_OBJ_TAG("poller already existes int current thread please check:%p, will assign new poller:%p, thread id:%llu")
            , defObj->_poller, poller, defObj->_threadId);

        defObj->_poller = poller;
        defObj->_pollerTimerMgr = poller->GetTimerMgr();

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

    virtual Int32 _OnCompsCreated() override
    {
        auto poller = GetComp<KERNEL_NS::Poller>();

        poller->SetMaxPriorityLevel(8);
        poller->SetPepareEventWorkerHandler(this, &HostObj::_OnPollerPrepare);
        poller->SetEventWorkerCloseHandler(this, &HostObj::_OnPollerWillDestroy);
        poller->SetEventHandler(this, &HostObj::_OnPollerEvent);

        auto dirtyHelper = poller->GetDirtyHelper();
        dirtyHelper->Init(1);
        auto delg = KERNEL_NS::DelegateFactory::Create(this, &HostObj::_OnDirty);
        dirtyHelper->SetHandler(0, delg);

        return Status::Success;
    }

    // 组件启动之后 此时可以启动线程 必须重写
    virtual Int32 _OnHostStart() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on host start."), ToString().c_str());
        return Status::Success;
    }

    virtual void _OnHostBeforeCompsWillClose() 
    {
        auto poller = GetComp<KERNEL_NS::Poller>();
        poller->QuitLoop();
        g_Log->Info(LOGFMT_OBJ_TAG("host before comp will close."));
    }

    // 在组件Close之后
    virtual void _OnHostClose()
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

    void _OnPollerEvent(KERNEL_NS::PollerEvent *ev)
    {
        if(static_cast<Int32>(sizeof(_eventHandlerArray)) <= ev->_type)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("bad event type, event:%s"), ev->ToString().c_str());
            return;
        }

        auto handler = _eventHandlerArray[ev->_type];
        if(!handler)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("event type have no handler, event:%s"), ev->ToString().c_str());
            return;
        }

        (this->*handler)(ev);
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

        auto poller = GetComp<KERNEL_NS::Poller>();
        if(!poller->PrepareLoop())
        {
            SetErrCode(poller, Status::PreparePollerFail);
            g_Log->Error(LOGFMT_OBJ_TAG("prepare loop fail."));
            return;
        }

        auto defObj = KERNEL_NS::TlsUtil::GetDefTls();
        defObj->_poller = poller;
        defObj->_pollerTimerMgr = poller->GetTimerMgr();

        MaskReady(true);

        auto timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
        auto timeoutHandler = [poller, this](KERNEL_NS::LibTimer *tm)
        {
            auto drityHelper = poller->GetDirtyHelper();
            g_Log->Info(LOGFMT_OBJ_TAG("on work frame poller loaded:%llu"), poller->CalcLoadScore());
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

    void _OnDirty(KERNEL_NS::LibDirtyHelper<void *, UInt32> *dirtyHelper, void *ptr, KERNEL_NS::Variant *params)
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
    typedef void (HostObj::*PollerEventHandler)(KERNEL_NS::PollerEvent *);
    static PollerEventHandler _eventHandlerArray[1];
};

POOL_CREATE_OBJ_DEFAULT_IMPL(HostObj);

HostObj::PollerEventHandler HostObj::_eventHandlerArray[1] = {
    &HostObj::_OnHelloWorldEv,
};

class HostObjFactory : public KERNEL_NS::CompFactory
{
public:
    static constexpr KERNEL_NS::_Build::MT _buildType{};
    static KERNEL_NS::CompFactory *FactoryCreate()
    {
       return KERNEL_NS::ObjPoolWrap<HostObjFactory>::NewByAdapter(_buildType.V);
    }
    void Release()
    {
        KERNEL_NS::ObjPoolWrap<HostObjFactory>::DeleteByAdapter(_buildType.V, this);
    }
    KERNEL_NS::CompObject *Create() const override
    {
        return HostObj::New_HostObj();
    }
};

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

    virtual KERNEL_NS::LibString ToString() const
    {
        return KERNEL_NS::PollerEvent::ToString().AppendFormat(", hello world request.");
    }
};

POOL_CREATE_OBJ_DEFAULT_IMPL(HelloWorldReq);


static void HelloWorldEventWork(KERNEL_NS::LibThread *t, KERNEL_NS::Variant *var)
{
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestPoller, "startint hello world request..."));
    auto hostObj = var->AsPtr<HostObj>();
    auto poller = hostObj->GetComp<KERNEL_NS::Poller>();

    while(!t->IsDestroy())
    {
        auto req = HelloWorldReq::New_HelloWorldReq();
        poller->Push(1, req);
        KERNEL_NS::SystemUtil::ThreadSleep(1000);
        hostObj->OnWork();
    }

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestPoller, "quit hello world request..."));
}

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

// 测试性能
static std::atomic<Int64> g_genNum{0};
static std::atomic<Int64> g_consumNum{0};

static const Int32 g_maxConcurrentLevel = 4;

static void _OnPollerEvent(KERNEL_NS::PollerEvent *ev)
{
    // g_Log->Debug(LOGFMT_NON_OBJ_TAG(KERNEL_NS::Poller, "recv event:%s"), ev->ToString().c_str());
    ++g_consumNum;
}

struct AcEvent : public KERNEL_NS::PollerEvent
{
    POOL_CREATE_OBJ_DEFAULT_P1(PollerEvent, AcEvent);

    AcEvent()
    :KERNEL_NS::PollerEvent(1)
    {

    }

    virtual void Release()
    {
        AcEvent::Delete_AcEvent(this);
    }

};

POOL_CREATE_OBJ_DEFAULT_IMPL(AcEvent);

static void _OnPoller(KERNEL_NS::LibThread *t)
{
    auto defObj = KERNEL_NS::TlsUtil::GetDefTls();
    defObj->_poller = s_Poller;
    defObj->_pollerTimerMgr = s_Poller->GetTimerMgr();

    if(!s_Poller->PrepareLoop())
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(TestPoller, "prepare loop fail."));
        return;
    }

    s_Poller->EventLoop();
    s_Poller->OnLoopEnd();

}

static void _OnTask(KERNEL_NS::LibThreadPool *t, KERNEL_NS::Variant *param)
{
    Int32 idx = param->AsInt32();
    KERNEL_NS::Poller *poller = s_Poller.AsSelf();
    while (!t->IsDestroy())
    {
        auto ev = AcEvent::New_AcEvent();
        ++g_genNum;
        poller->Push(idx, ev);
    }
} 

static void _OnMonitor(KERNEL_NS::LibThreadPool *t, KERNEL_NS::Variant *param)
{
    while (!t->IsDestroy())
    {
        KERNEL_NS::SystemUtil::ThreadSleep(1000);
        const Int64 genNum = g_genNum;
        const Int64 comsumNum = g_consumNum;
        const Int64 backlogNum = s_Poller->GetEventAmount();
        g_genNum -= genNum;
        g_consumNum -= comsumNum;

        g_Log->Custom("Monitor:[gen:%lld, consum:%lld, backlog:%lld]", genNum, comsumNum, backlogNum);
    }
}

void TestPoller::Run()
{
    s_Poller = reinterpret_cast<KERNEL_NS::Poller *>(KERNEL_NS::PollerFactory::FactoryCreate()->Create());
    s_Poller->SetMaxPriorityLevel(g_maxConcurrentLevel);
    s_Poller->SetEventHandler(KERNEL_NS::DelegateFactory::Create(&_OnPollerEvent));

    auto err = s_Poller->Init();
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(TestPoller, "init poller fail."));
        return;
    }

    err = s_Poller->Start();
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(TestPoller, "start poller fail."));
        return;
    }

    KERNEL_NS::LibThread *pollerThread = new KERNEL_NS::LibThread;
    pollerThread->AddTask(&_OnPoller);
    pollerThread->Start();

    KERNEL_NS::SmartPtr<KERNEL_NS::LibThreadPool, KERNEL_NS::AutoDelMethods::Release> pool = new KERNEL_NS::LibThreadPool;
    pool->Init(0, g_maxConcurrentLevel + 2);

    for(Int32 idx=1; idx <=g_maxConcurrentLevel; ++idx)
    {
        KERNEL_NS::Variant *var=KERNEL_NS::Variant::New_Variant();
        *var = idx;
        pool->AddTask2(&_OnTask, var, false, 0);
    }

    pool->AddTask2(&_OnMonitor, NULL, false, 0);

    pool->Start(true, g_maxConcurrentLevel + 1);

    getchar();

    pool->HalfClose();

    s_Poller->QuitLoop();
    pollerThread->HalfClose();

    pool->FinishClose();
    pollerThread->FinishClose();

    // 测试poller性能
}
