#pragma once

#include <service/Client/Comps/SendLog/Interface/ISendLog.h>

SERVICE_BEGIN

class SendLog : public ISendLog
{
    POOL_CREATE_OBJ_DEFAULT_P1(ISendLog, SendLog);
public:
    SendLog();
    ~SendLog() override;
    virtual void Release() override;

    virtual void SendData(SERVICE_NS::DataSourceInfo *data) override;


private:
};


SERVICE_END