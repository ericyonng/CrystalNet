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
 * Date: 2026-01-16 01:15:47
 * Author: Eric Yonng
 * Description: 
*/

#include "pch.h"
#include "client_api.h"
#include <client/Entry.h>
#include <kernel/kernel.h>

#include "service/common/macro.h"
#include <protocols/protocols.h>
#include <Comps/SendLog/SendLog.h>
#include <Common/TestAccount.h>


extern "C"
{
    int ClientInit()
    {
        Entry::Run();

        CLOG_INFO_GLOBAL(Entry, "Entry inited.");

        return 0;
    }

    void ClientSet(char *ip, int ipLen, int port, char *account, int accountLen, char *pwd, int pwdLen)
    {
        AccountData accountInfo;
        accountInfo.ip.AppendData(ip, ipLen);
        accountInfo.port = port;
        accountInfo.Account.AppendData(account, accountLen);
        accountInfo.Pwd.AppendData(pwd, pwdLen);
        TestAccount::AccountInfo = accountInfo;
        
        CLOG_INFO_GLOBAL(Entry, "ClientSet accountInfo set: ip:%s, port:%d, account:%s, pwd:%s", accountInfo.ip.c_str(), accountInfo.port, accountInfo.Account.c_str(), accountInfo.Pwd.c_str());
    }


    void ClientStart()
    {
        if (UNLIKELY(!Entry::EntryThread))
        {
            CLOG_ERROR_GLOBAL(Entry, "Entry Thread not init...");
            return;
        }
        Entry::EntryThread->Start();

        // 等待app启动
        while (Entry::Application.load(std::memory_order_acquire) == NULL)
        {
            CLOG_INFO_GLOBAL(Entry, "waiting for app started...");
            KERNEL_NS::SystemUtil::ThreadSleep(KERNEL_NS::TimeSlice::FromSeconds(1).GetTotalMilliSeconds());
        }

        // 等待服务启动
        auto app = Entry::Application.load(std::memory_order_acquire);

        auto serviceProxy = app->GetComp<SERVICE_COMMON_NS::ServiceProxy>();
        while (!serviceProxy->IsServiceReady("Client"))
        {
            CLOG_INFO_GLOBAL(Entry, "waiting for service started...");
            KERNEL_NS::SystemUtil::ThreadSleep(KERNEL_NS::TimeSlice::FromSeconds(1).GetTotalMilliSeconds());
        }

        auto services = serviceProxy->GetServices("Client");
        Entry::Service.exchange(services[0]);

        CLOG_INFO_GLOBAL(Entry, "service started:%s...", Entry::Service.load(std::memory_order_acquire)->ToString().c_str());
    }

    void ClientClose()
    {
        CLOG_INFO_GLOBAL(Entry, "Entry will close...");

        Entry::Service.exchange(NULL);
        KERNEL_NS::SystemUtil::ThreadSleep(KERNEL_NS::TimeSlice::FromSeconds(1).GetTotalMilliSeconds());

        if (Entry::Application.load(std::memory_order_acquire))
        {
            Entry::Application.load(std::memory_order_acquire)->SinalFinish();
        }
        if (LIKELY(Entry::EntryThread))
        {
            Entry::EntryThread->Close();
        }

        CLOG_INFO_GLOBAL(Entry, "Entry closed");

        // KERNEL_NS::KernelUtil::Destroy();
    }

    void SendLog(char *buffer, int bufferSize, long long uid)
    {
        KERNEL_NS::LibString data;
        data.AppendData(buffer, bufferSize);
        auto nowTime = KERNEL_NS::LibTime::Now().GetMilliTimestamp();

        KERNEL_NS::SmartPtr<SERVICE_NS::DataSourceInfo> dataPb = new SERVICE_NS::DataSourceInfo();
        dataPb->set_data(data.GetRaw());
        dataPb->set_requestid(uid);
        dataPb->set_mstime(nowTime);

        // 结束了
        auto service = Entry::Service.load(std::memory_order_acquire);
        if (!service)
        {
            CLOG_WARN_GLOBAL(Entry, "SendLog have no service uid:%lld, bufferSize:%d", uid, bufferSize);
            return;
        }

        auto lamb = [service, pb = dataPb.pop()]()
        {
            auto sendLog = service->GetComp<CRYSTAL_NET::service::ISendLog>();
            sendLog->SendData(pb);
        };
        auto deleg = KERNEL_CREATE_CLOSURE_DELEGATE(lamb, void);
        service->GetPoller()->Push(deleg);

        CLOG_DEBUG_GLOBAL(Entry, "SendLog uid:%lld, bufferSize:%d", uid, bufferSize);
    }

}

