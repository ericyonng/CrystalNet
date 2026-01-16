
#pragma once

#include <service/Client/ServiceCompHeader.h>

SERVICE_BEGIN

class DataSourceInfo;

class ISendLog : public IGlobalSys
{
    POOL_CREATE_OBJ_DEFAULT_P1(IGlobalSys, ISendLog);
    
public:
    ISendLog(UInt64 objTypeId) : IGlobalSys(objTypeId) {}

    virtual void SendData(SERVICE_NS::DataSourceInfo *data) = 0;
};

SERVICE_END