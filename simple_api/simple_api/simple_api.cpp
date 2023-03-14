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

        // 异常关闭app
    //     const auto currentTid = KERNEL_NS::SystemUtil::GetCurrentThreadId();
    //     auto signalCloseLambda = [currentTid]()->void{
    //         auto threadId = KERNEL_NS::SystemUtil::GetCurrentThreadId();
    //         g_Log->Info(LOGFMT_OBJ_TAG("signal catched, application will close threadId:%llu, application thread id:%llu..."), threadId, currentTid);
    
    // #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    //         if (threadId == currentTid)
    //         {
    //             KERNEL_NS::KernelUtil::OnSignalClose();

    //             printf("\napplication quit finish.\n");
    //         }
    //         else
    //         {
    //             KERNEL_NS::KernelUtil::OnAbnormalClose();
    //             printf("\napplication quit finish.\n");

    //         }
    // #else

    //         g_Log->Info(LOGFMT_OBJ_TAG("application close finished."));

    //         KERNEL_NS::KernelUtil::OnSignalClose();

    //         printf("\napplication quit finish.\n");
    //         // while(true)
    //         // {
    //         //     auto v = getchar();
    //         //     if(v == 'q')
    //         //         break;
    //         // }
    // #endif
    //     };

    //     auto closeDelg = KERNEL_CREATE_CLOSURE_DELEGATE(signalCloseLambda, void);
    //     KERNEL_NS::KernelUtil::InstallSignalCloseHandler(closeDelg);

        return Status::Success;
    }

    Int32 Start()
    {
        KERNEL_NS::KernelUtil::Start();
    }

    void Destroy()
    {
        if(!_isInit.exchange(false))
            return;

        if(_isDestroy.exchange(true))
            return;

        KERNEL_NS::KernelUtil::Destroy();
    }

private:
    std::atomic_bool _isDestroy{false};
    std::atomic_bool _isInit{false};
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

    s_Lock.Unlock();
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

class MonitorTask : public KERNEL_NS::ITask
{
public:
    POOL_CREATE_OBJ_DEFAULT_P1(ITask, MonitorTask);

    MonitorTask(int period)
    {

    }

    ~MonitorTask()
    {
        
    }

    void Run()
    {
        
    }

    void Release()
    {
        MonitorTask::Delete_MonitorTask(this);
    }

    int _period;
    KERNEL_NS::Poller *_poller;
};

POOL_CREATE_OBJ_DEFAULT_IMPL(MonitorTask);

void SetCppmonitor(void(*MonitorCb)(), int period)
{

}


