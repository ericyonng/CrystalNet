#include <pch.h>
#include <simple_api/simple_api.h>
#include <simple_api/Ini.h>

class LibSimpleApiLog : public KERNEL_NS::LibLog
{
public:
    LibSimpleApiLog() {}
    ~LibSimpleApiLog() {}
};

class LogFactory : public KERNEL_NS::ILogFactory
{
public:
    virtual KERNEL_NS::ILog *Create()
    {
        return new LibSimpleApiLog();
    }
};

// 事件
class EventType
{
public:
    enum ENUMS
    {
        ACTION = 1, // action事件
        PROFILE = 2,    // profile
    };
};

struct ProfileEvent : public KERNEL_NS::PollerEvent
{
    POOL_CREATE_OBJ_DEFAULT_P1(PollerEvent, ProfileEvent);

    ProfileEvent()
    :KERNEL_NS::PollerEvent(EventType::PROFILE)
    {

    }


    virtual void Release()
    {
        ProfileEvent::Delete_ProfileEvent(this);
    }
    
    Int64 _putDataTime = 0;
    Int32 _messageId = 0;
    Int32 _requestId = 0;
    Int64 _dispatchMs = 0;
    Int64 _gwRecvRequestTime = 0;
    Int64 _gwSendRequestTime = 0;
    Int64 _gwPrepareTurnRequestToRpcTime = 0;
    Int64 _gsRecvRequestTime = 0;
    Int64 _gsDispatchRequestTime = 0;
    Int64 _gsHandlerRequestTime = 0;
    Int64 _gsSendResponseTime = 0;
    Int64 _gwRecvResponseTime = 0;
};

POOL_CREATE_OBJ_DEFAULT_IMPL(ProfileEvent);

class SimpleApiHost : public KERNEL_NS::CompHostObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, SimpleApiHost);

public:
    
    SimpleApiHost()
    {

    }
    ~SimpleApiHost()
    {
        _Clear();
    }

    virtual void Release()
    {
        SimpleApiHost::Delete_SimpleApiHost(this);
    }

public:
    virtual void OnRegisterComps()
    {
        RegisterComp<KERNEL_NS::PollerFactory>();
    }

    void EventLoop()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("start event loop."));
        auto poller = GetComp<KERNEL_NS::Poller>();
        if(!poller->PrepareLoop())
        {
            g_Log->Error(LOGFMT_OBJ_TAG("prepare loop fail."));
            return;
        }

        poller->EventLoop();

        poller->OnLoopEnd();
    }

protected:
    virtual Int32 _OnHostInit() override
    {
        return Status::Success;
    }

    virtual Int32 _OnHostStart() override
    {
        return Status::Success;
    }

    virtual void _OnHostClose() override
    {
        _Clear();
    }

    virtual void Clear() override
    {
        _Clear();
        CompHostObject::Clear();
    }

private:
    void _Clear()
    {
        
    }

    virtual Int32 _OnCompsCreated() override
    {
        auto poller = GetComp<KERNEL_NS::Poller>();

        // poller 设置
        KERNEL_NS::TimeSlice span(0, 0, 8);
        poller->SetMaxPriorityLevel(2);
        poller->SetMaxPieceTime(span);
        poller->SetMaxSleepMilliseconds(20);
        poller->SetEventHandler(this, &SimpleApiHost::_OnMsg);

        auto defObj = KERNEL_NS::TlsUtil::GetDefTls();
        if(UNLIKELY(defObj->_poller))
            g_Log->Warn(LOGFMT_OBJ_TAG("poller already existes int current thread please check:%p, will assign new poller:%p, thread id:%llu")
            , defObj->_poller, poller, defObj->_threadId);

        defObj->_poller = poller;
        defObj->_pollerTimerMgr = poller->GetTimerMgr();

        g_Log->Info(LOGFMT_OBJ_TAG("comps created suc."));
        return Status::Success;
    }

    // 事件处理
    virtual void _OnMsg(KERNEL_NS::PollerEvent *ev)
    {
        if(ev->_type == EventType::ACTION)
        {
            auto actionEv = ev->CastTo<KERNEL_NS::ActionPollerEvent>();
            actionEv->_action->Invoke();
            return;
        }

        if(ev->_type == EventType::PROFILE)
        {
            _HandleProfile(ev->CastTo<ProfileEvent>());
            return;
        }
    }

    void _HandleProfile(ProfileEvent *ev)
    {
        // gw 收到request 到gw 发送response延迟
        const auto fromGwRecvRequestToGwSendResponse = ev->_putDataTime - ev->_gwRecvRequestTime;

        // rpc 之前收到消息到准备发送rpc的延迟
        const auto diffFromGwRecvRequestToGwSendRequest = ev->_gwSendRequestTime - ev->_gwRecvRequestTime;

        // gw发出request到gs 收到request延迟
        const auto fromGwSendRpcToGsRecvRequest = ev->_gsRecvRequestTime - ev->_gwPrepareTurnRequestToRpcTime;

        // gs 收到request 到gs dispatch request时间 这部分时间是在gs的队列中
        const auto fromGsRecvRequestToGsDispatch = ev->_gsDispatchRequestTime - ev->_gsRecvRequestTime;

        // gs handle 时间
        const auto gsHandlerTime = ev->_dispatchMs;

        // gs 发送response到gw收到response延迟
        const auto fromGsSendResponseToGwRecvResponse = ev->_gwRecvResponseTime - ev->_gsSendResponseTime;

        // gw 收到response到gw发送response延迟
        const auto fromGwRecvResponseToGwSendResponse = ev->_putDataTime - ev->_gwRecvResponseTime;

        const KERNEL_NS::LibTime gsDispatchTime = KERNEL_NS::LibTime::FromMilliSeconds(ev->_gsDispatchRequestTime);
        const KERNEL_NS::LibTime gsHandledTime = KERNEL_NS::LibTime::FromMilliSeconds(ev->_gsHandlerRequestTime);
        const KERNEL_NS::LibTime gsRecvRequestTime = KERNEL_NS::LibTime::FromMilliSeconds(ev->_gsRecvRequestTime);

        const auto timeMs = KERNEL_NS::LibTime::FromMilliSeconds(ev->_putDataTime);
        g_Log->Info(LOGFMT_OBJ_TAG("put profile time:%lld(ms timestamp) %s, profile message id:%d, requestId:%d, "
                                    " gw recv req => gw send response to client total cost:%lld(ms),"
                                    " gw recv req => gw prepare send req rpc to gs cost:%lld(ms),"
                                    " gw prepare send req to gs => gs recv req cost%lld(ms) recv request time date:%s, timestamp:%lld."
                                    "gs recv req => gs dispatch req(in gs queue time) cost:%lld(ms)."
                                    "gs start dispatch time time:%lld(ms) %s."
                                    "gs handled time:%lld(ms) %s."
                                    "gs handle req time:%lld(ms)."
                                    "gs send response => gw recv response cost:%lld(ms)."
                                    "gw recv response => gw send response(in gw response queue time) cost:%lld(ms).")
                    , ev->_putDataTime, timeMs.ToStringOfMillSecondPrecision().c_str(), ev->_messageId, ev->_requestId 
                    , fromGwRecvRequestToGwSendResponse
                    , diffFromGwRecvRequestToGwSendRequest
                    , fromGwSendRpcToGsRecvRequest
                    , gsRecvRequestTime.ToStringOfMillSecondPrecision().c_str()
                    ,ev->_gsRecvRequestTime
                    , fromGsRecvRequestToGsDispatch
                    , ev->_gsDispatchRequestTime
                    , gsDispatchTime.ToStringOfMillSecondPrecision().c_str()
                    , ev->_gsHandlerRequestTime
                    , gsHandledTime.ToStringOfMillSecondPrecision().c_str()
                    , gsHandlerTime
                    , fromGsSendResponseToGwRecvResponse
                    , fromGwRecvResponseToGwSendResponse
                    );

    }

private:
};

POOL_CREATE_OBJ_DEFAULT_IMPL(SimpleApiHost);

class SimpleApiInstance
{
public:
    
    SimpleApiInstance()
    {

    }
    ~SimpleApiInstance()
    {
        Destroy();
    }

    Int32 Init(bool needSignalHandle)
    {
        if(_isInit.exchange(true))
        {
            return Status::Repeat;
        }

        LogFactory logFactory;
        KERNEL_NS::LibString programPath = KERNEL_NS::SystemUtil::GetCurProgRootPath();
        KERNEL_NS::LibString logIniPath;
        logIniPath = programPath + "/ini/";
        KERNEL_NS::SystemUtil::GetProgramPath(true, programPath);
        Int32 err = KERNEL_NS::KernelUtil::Init(&logFactory, "LogCfg.ini", logIniPath.c_str(), s_logIniContent, s_consoleIniContent, needSignalHandle, -1, -1);
        if(err != Status::Success)
        {
            CRYSTAL_TRACE("kernel init fail err:%d", err);
            return err;
        }

        return Status::Success;
    }

    Int32 Start()
    {
        KERNEL_NS::KernelUtil::Start();
        _host = SimpleApiHost::New_SimpleApiHost();
        _thread = new KERNEL_NS::LibThread;

        _thread->AddTask(this, &SimpleApiInstance::_OnThread);

        Int32 err = _host->Init();
        if(err != Status::Success)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("init host fail err:%d"), err);
            return err;
        }

        err = _host->Start();
        if(err != Status::Success)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("host start fail err:%d"), err);
            return err;
        }

        _thread->Start();

        return Status::Success;
    }

    void Destroy()
    {
        if(!_isInit.exchange(false))
            return;

        if(_isDestroy.exchange(true))
            return;

        _host->GetComp<KERNEL_NS::Poller>()->QuitLoop();

        if(_thread->HalfClose())
            _thread->FinishClose();

        _host->WillClose();

        _host->Close();

        KERNEL_NS::KernelUtil::Destroy();
    }

    SimpleApiHost *GetHost()
    {
        return _host.AsSelf();
    }

    const SimpleApiHost *GetHost() const
    {
        return _host.AsSelf();
    }

    void _OnThread(KERNEL_NS::LibThread *t)
    {
        _host->EventLoop();
    }

private:
    std::atomic_bool _isDestroy{false};
    std::atomic_bool _isInit{false};

    KERNEL_NS::SmartPtr<SimpleApiHost, KERNEL_NS::AutoDelMethods::Release> _host;
    KERNEL_NS::SmartPtr<KERNEL_NS::LibThread, KERNEL_NS::AutoDelMethods::Release> _thread;
};

static KERNEL_NS::SmartPtr<SimpleApiInstance> s_SimpleApi = NULL;
static KERNEL_NS::Locker s_Lock;
static KERNEL_NS::Poller *s_poller = NULL;

int SimpleApiInit(bool needSignalHandle)
{
    s_Lock.Lock();
    if(s_SimpleApi)
    {
        s_Lock.Unlock();
        return Status::Success;
    }

    s_SimpleApi = new SimpleApiInstance;
    auto err = s_SimpleApi->Init(needSignalHandle);
    if(err != Status::Success)
    {
        s_Lock.Unlock();
        return err;
    }

    err = s_SimpleApi->Start();
    if(err != Status::Success)
    {
        s_Lock.Unlock();
        return err;
    }

    s_poller = s_SimpleApi->GetHost()->GetComp<KERNEL_NS::Poller>();
    s_Lock.Unlock();

    g_Log->Info(LOGFMT_NON_OBJ_TAG(SimpleApiInstance, "simple api init suc."));

    return Status::Success;
}

void SimpleApiDestroy()
{
    g_Log->Info(LOGFMT_NON_OBJ_TAG(SimpleApiInstance, "simple api will destroy suc."));
    s_Lock.Lock();
    do
    {
        if(!s_SimpleApi)
            break;

        s_SimpleApi->Destroy();   
    } while (false);

    s_Lock.Unlock();

}


void SimpleApiSetCppmonitor(void(*MonitorCb)(), int period)
{
    static KERNEL_NS::LibTimer *s_timer = NULL;
    auto &timer = s_timer;

    s_Lock.Lock();
    do
    {
        if(!s_SimpleApi)
            break;

        // 添加定时器事件  
        auto poller = s_SimpleApi->GetHost()->GetComp<KERNEL_NS::Poller>();
        poller->Push(0, EventType::ACTION, [&timer, MonitorCb, period, poller](){
            if(!timer)
            {
                timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
                timer->SetTimeOutHandler([MonitorCb, period](KERNEL_NS::LibTimer *timer){
                    (*MonitorCb)();
                    g_Log->Info(LOGFMT_NON_OBJ_TAG(SimpleApiInstance, "tick period:%d"), period);
                });
            }

            timer->Schedule(period);
        });

    } while (false);

    s_Lock.Unlock();
}

void SimpleApiLog(const char *content, Int32 contentSize)
{
    KERNEL_NS::LibString str;
    str.AppendData(content, contentSize);
    g_Log->Info(LOGFMT_NON_OBJ_TAG(SimpleApiInstance, "%s"), str.c_str());
}

void SimpleApiPushProfile(Int64 nowTime, int messageId, int requestId, Int64 dispatchMs
, Int64 gwRecvRequestTime, Int64 gwSendRequestTime, Int64 gwPrepareTurnRequestToRpcTime
, Int64 gsRecvRequestTime, Int64 gsDispatchRequestTime, Int64 gsHandlerRequestTime
, Int64 gsSendResponseTime, Int64 gwRecvResponseTime)
{
    auto ev = ProfileEvent::New_ProfileEvent();
    ev->_putDataTime = nowTime;
    ev->_messageId = messageId;
    ev->_requestId = requestId;
    ev->_dispatchMs = dispatchMs;
    ev->_gwRecvRequestTime = gwRecvRequestTime;
    ev->_gwSendRequestTime = gwSendRequestTime;
    ev->_gwPrepareTurnRequestToRpcTime = gwPrepareTurnRequestToRpcTime;
    ev->_gsRecvRequestTime = gsRecvRequestTime;
    ev->_gsDispatchRequestTime = gsDispatchRequestTime;
    ev->_gsHandlerRequestTime = gsHandlerRequestTime;
    ev->_gsSendResponseTime = gsSendResponseTime;
    ev->_gwRecvResponseTime = gwRecvResponseTime;
    s_poller->Push(0, ev);
}

void SimpleApiPushProfile2(Int64 nowTime, int messageId, int requestId, Int64 dispatchMs
, Int64 gwRecvMsgTime, Int64 gwPrepareRpcTime, Int64 gsRecvRpcTime
, Int64 gsDispatchMsgTime, Int64 gsHandledTime)
{
    s_poller->Push(0, EventType::ACTION, [nowTime, messageId, requestId, dispatchMs, gwRecvMsgTime, gwPrepareRpcTime, gsRecvRpcTime, gsDispatchMsgTime, gsHandledTime](){
        
        // gw 收到消息 到gw 发送response延迟
        const auto fromGwRecvMsgToGsHandledMsgTime = nowTime - gwRecvMsgTime;

        // gw 收到消息 到 gw 发送rpc时间
        const auto diffFromGwRecvMsgToGwSendRpc = gwPrepareRpcTime - gwRecvMsgTime;

        // gw发送rpc 到 gs收到rpc
        const auto fromGwSendRpcToGsRecvRpc = gsRecvRpcTime - gwPrepareRpcTime;

        // gs 收到rpc 到gs 开始处理消息
        const auto fromGsRecvRpcToGsDispatch = gsDispatchMsgTime - gsRecvRpcTime;

        // gs handle 时间
        const auto gsHandlerTime = gsHandledTime - gsDispatchMsgTime;

        const KERNEL_NS::LibTime gsRecvMsgTime = KERNEL_NS::LibTime::FromMilliSeconds(gsRecvRpcTime);
        const KERNEL_NS::LibTime gsDispatchTime = KERNEL_NS::LibTime::FromMilliSeconds(gsDispatchMsgTime);
        const KERNEL_NS::LibTime gsHandledMsgTime = KERNEL_NS::LibTime::FromMilliSeconds(gsHandledTime);
        const auto timeMs = KERNEL_NS::LibTime::FromMilliSeconds(nowTime);

        g_Log->Info(LOGFMT_NON_OBJ_TAG(SimpleApiInstance, "put profile 2 time:%lld(ms timestamp) %s, profile message id:%d, requestId:%d, "
                                    " gw recv msg => gs handled msg cost:%lld(ms),"
                                    " gw recv msg => gw prepare send rpc to gs cost:%lld(ms),"
                                    " gw prepare send msg to gs => gs recv msg cost%lld(ms), gs recv msg time date:%s, timestamp:%lld."
                                    "gs recv msg => gs dispatch msg(in gs queue time) cost:%lld(ms), gs dispatch msg time date:%s, timestamp:%lld."
                                    "gs handled time date:%s, timestamp:%lld."
                                    "gs handle msg cost time:%lld(ms).")
                    ,nowTime, timeMs.ToStringOfMillSecondPrecision().c_str(), messageId, requestId 
                    , fromGwRecvMsgToGsHandledMsgTime
                    , diffFromGwRecvMsgToGwSendRpc
                    , fromGwSendRpcToGsRecvRpc
                    , gsRecvMsgTime.ToStringOfMillSecondPrecision().c_str()
                    , gsRecvRpcTime
                    , fromGsRecvRpcToGsDispatch
                    , gsDispatchTime.ToStringOfMillSecondPrecision().c_str()
                    ,gsDispatchMsgTime
                    , gsHandledMsgTime.ToStringOfMillSecondPrecision().c_str()
                    , gsHandledTime
                    , gsHandlerTime
                    );
    });
}

void SimpleApiPushProfile3(Int64 nowTime, int messageId, int requestId
, Int64 gwRecvMsgTime, Int64 gwPrepareRpcTime, Int64 gsRecvRpcTime
, Int64 gsDispatchMsgTime, Int64 asynOverDispatchTime)
{
    s_poller->Push(0, EventType::ACTION, [nowTime, messageId, requestId, gwRecvMsgTime, gwPrepareRpcTime, gsRecvRpcTime, gsDispatchMsgTime, asynOverDispatchTime](){
        
        // gw 收到消息 到gw 发送response延迟
        const auto fromGwRecvMsgToGsHandledMsgTime = nowTime - gwRecvMsgTime;

        // gw 收到消息 到 gw 发送rpc时间
        const auto diffFromGwRecvMsgToGwSendRpc = gwPrepareRpcTime - gwRecvMsgTime;

        // gw发送rpc 到 gs收到rpc
        const auto fromGwSendRpcToGsRecvRpc = gsRecvRpcTime - gwPrepareRpcTime;

        // gs 收到rpc 到gs 开始处理消息
        const auto fromGsRecvRpcToGsDispatch = gsDispatchMsgTime - gsRecvRpcTime;

        // gs handle 时间
        const auto overDispatchTime = asynOverDispatchTime - gsDispatchMsgTime;

        const KERNEL_NS::LibTime gsRecvMsgTime = KERNEL_NS::LibTime::FromMilliSeconds(gsRecvRpcTime);
        const KERNEL_NS::LibTime gsDispatchTime = KERNEL_NS::LibTime::FromMilliSeconds(gsDispatchMsgTime);
        const KERNEL_NS::LibTime gsOverDispatch = KERNEL_NS::LibTime::FromMilliSeconds(asynOverDispatchTime);
        const auto timeMs = KERNEL_NS::LibTime::FromMilliSeconds(nowTime);

        g_Log->Info(LOGFMT_NON_OBJ_TAG(SimpleApiInstance, "put profile 3 time:%lld(ms timestamp) %s, profile message id:%d, requestId:%d, "
                                    " gw recv msg => gs async over dispatch cost:%lld(ms),"
                                    " gw recv msg => gw prepare send rpc to gs cost:%lld(ms),"
                                    " gw prepare send msg to gs => gs recv msg cost%lld(ms), gs recv msg time date:%s, timestamp:%lld."
                                    "gs recv msg => gs dispatch msg(in gs queue time) cost:%lld(ms), gs dispatch msg time date:%s, timestamp:%lld."
                                    "gs over dispatch time date:%s, timestamp:%lld."
                                    "gs from dispatch to over dispatch cost time:%lld(ms).")
                    ,nowTime, timeMs.ToStringOfMillSecondPrecision().c_str(), messageId, requestId 
                    , fromGwRecvMsgToGsHandledMsgTime
                    , diffFromGwRecvMsgToGwSendRpc
                    , fromGwSendRpcToGsRecvRpc
                    , gsRecvMsgTime.ToStringOfMillSecondPrecision().c_str()
                    , gsRecvRpcTime
                    , fromGsRecvRpcToGsDispatch
                    , gsDispatchTime.ToStringOfMillSecondPrecision().c_str()
                    ,gsDispatchMsgTime
                    , gsOverDispatch.ToStringOfMillSecondPrecision().c_str()
                    , asynOverDispatchTime
                    , overDispatchTime
                    );
    });
}