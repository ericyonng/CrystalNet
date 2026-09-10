#pragma once

#include <service/Client/Comps/SendLog/Interface/ISendLog.h>
#include <kernel/comp/TimeSlice.h>
#include <protocols/protocols.h>

KERNEL_BEGIN
class LibTimer;
KERNEL_END

SERVICE_BEGIN

class DataSourceInfoCompare
{
public:
    bool operator()(const DataSourceInfo * left, const DataSourceInfo * right) const;
};

class SendLog : public ISendLog
{
    POOL_CREATE_OBJ_DEFAULT_P1(ISendLog, SendLog);
public:
    SendLog();
    ~SendLog() override;
    virtual void Release() override;

    virtual void SendData(SERVICE_NS::DataSourceInfo *data) override;


private:
    Int32 _OnGlobalSysInit() override;
    void _OnGlobalSysClose() override;

    void _OnSendDataResponse(KERNEL_NS::LibPacket *&packet);
    void _OnResendTimer(KERNEL_NS::LibTimer *timer);

    void _Clear();


    std::set<SERVICE_NS::DataSourceInfo *, DataSourceInfoCompare> _dataSortedByTime;

    // 待确认的包, 确认后list的数据会被清空, 并发送下一批数据, 
    SendDataRequest _requestWaitConfirm;
    Int64 _waitConfirmId;// 用requestId当做确认id

    // 超时重传
    KERNEL_NS::LibTimer *_reSendTimer;
    KERNEL_NS::TimeSlice _reSendInterval;
};


SERVICE_END