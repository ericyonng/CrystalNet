#include "pch.h"
#include "client_api.h"
#include <client/Entry.h>
#include <kernel/kernel.h>


extern "C"
{
    int ClientInit()
    {
        Entry::Run();

        g_Log->Info(LOGFMT_NON_OBJ_TAG(Entry, "Entry inited."));

        return 0;
    }

    void ClientSet(char *ip, int ipLen, int port, char *account, int accountLen, char *pwd, int pwdLen)
    {
        AccountInfo accountInfo;
        accountInfo.ip.AppendData(ip, ipLen);
        accountInfo.port = port;
        accountInfo.Account.AppendData(account, accountLen);
        accountInfo.Pwd.AppendData(pwd, pwdLen);
        Entry::AccountInfo = accountInfo;
        
        g_Log->Info(LOGFMT_NON_OBJ_TAG(Entry, "ClientSet accountInfo set: ip:%s, port:%d, account:%s, pwd:%s"), accountInfo.ip.c_str(), accountInfo.port, accountInfo.Account.c_str(), accountInfo.Pwd.c_str());
    }


    void ClientStart()
    {
        if (UNLIKELY(!Entry::EntryThread))
        {
            g_Log->Error(LOGFMT_NON_OBJ_TAG(Entry, "Entry Thread not init..."));
            return;
        }
        Entry::EntryThread->Start();

        // 等待app启动
        while (Entry::Application.load(std::memory_order_acquire) == NULL)
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(Entry, "waiting for app started..."));
            KERNEL_NS::SystemUtil::ThreadSleep(KERNEL_NS::TimeSlice::FromSeconds(1).GetTotalMilliSeconds());
        }

        // 等待服务启动
        auto app = Entry::Application.load(std::memory_order_acquire);

        auto serviceProxy = app->GetComp<SERVICE_COMMON_NS::ServiceProxy>();
        while (!serviceProxy->IsServiceReady("Client"))
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(Entry, "waiting for service started..."));
            KERNEL_NS::SystemUtil::ThreadSleep(KERNEL_NS::TimeSlice::FromSeconds(1).GetTotalMilliSeconds());
        }

        auto services = serviceProxy->GetServices("Client");
        Entry::Service.exchange(services[0]);

        g_Log->Info(LOGFMT_NON_OBJ_TAG(Entry, "service started:%s..."), Entry::Service.load(std::memory_order_acquire)->ToString().c_str());
    }

    void ClientClose()
    {
        g_Log->Info(LOGFMT_NON_OBJ_TAG(Entry, "Entry will close..."));

        if (Entry::Application.load(std::memory_order_acquire))
        {
            Entry::Application.load(std::memory_order_acquire)->SinalFinish();
        }
        if (LIKELY(Entry::EntryThread))
        {
            Entry::EntryThread->Close();
        }

        g_Log->Info(LOGFMT_NON_OBJ_TAG(Entry, "Entry closed"));

        // KERNEL_NS::KernelUtil::Destroy();
    }

    void SendLog(char *buffer, int bufferSize)
    {
        KERNEL_NS::LibString data;
        data.AppendData(buffer, bufferSize);
        g_Log->Info(LOGFMT_NON_OBJ_TAG(Entry, "buffer:%p, bufferSize:%d, str:%s, service:%s"), buffer, bufferSize, data.c_str(), Entry::Service.load(std::memory_order_acquire)->ToString().c_str());
    }

}

