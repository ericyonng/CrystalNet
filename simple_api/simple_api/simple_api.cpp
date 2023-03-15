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

class EventType
{
public:
    enum ENUMS
    {
        ACTION = 1, // action事件
    };
};

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
        poller->SetMaxSleepMilliseconds(1);
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

    virtual void _OnMsg(KERNEL_NS::PollerEvent *ev)
    {
        if(ev->_type == EventType::ACTION)
        {
            auto actionEv = ev->CastTo<KERNEL_NS::ActionPollerEvent>();
            actionEv->_action->Invoke();
            return;
        }
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

    Int32 Init()
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
        Int32 err = KERNEL_NS::KernelUtil::Init(&logFactory, "LogCfg.ini", logIniPath.c_str(), s_logIniContent, s_consoleIniContent);
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

int Init()
{
    s_Lock.Lock();
    if(s_SimpleApi)
    {
        s_Lock.Unlock();
        return Status::Success;
    }

    s_SimpleApi = new SimpleApiInstance;
    auto err = s_SimpleApi->Init();
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

    s_Lock.Unlock();

    return Status::Success;
}

void Destroy()
{
    s_Lock.Lock();
    do
    {
        if(!s_SimpleApi)
            break;

        s_SimpleApi->Destroy();   
    } while (false);

    s_Lock.Unlock();
}


void SetCppmonitor(void(*MonitorCb)(), int period)
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

void Log(const char *content, Int32 contentSize)
{
    KERNEL_NS::LibString str;
    str.AppendData(content, contentSize);
    g_Log->Info(LOGFMT_NON_OBJ_TAG(SimpleApiInstance, "%s"), str.c_str());
}


