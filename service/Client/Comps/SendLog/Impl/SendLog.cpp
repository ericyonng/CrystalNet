#include <pch.h>
#include <service/Client/Comps/SendLog/Impl/SendLog.h>
#include <service/Client/Comps/SendLog/Impl/SendLogFactory.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(ISendLog);
POOL_CREATE_OBJ_DEFAULT_IMPL(SendLog);

SendLog::SendLog()
    :ISendLog(KERNEL_NS::RttiUtil::GetTypeId<SendLog>())
{
    
}

SendLog::~SendLog()
{
    
}

void SendLog::Release()
{
    SendLog::DeleteByAdapter_SendLog(SendLogFactory::_buildType.V, this);
}

void SendLog::SendData(SERVICE_NS::DataSourceInfo *data)
{
    // 1.直接广播给每个User,
    // 2.发起定时, 如果15秒内没收到返回, 则重传, 直到收到返回包为止(移除data)
    // 3. 断线重登后, 重新发没收到Response的数据, 所以要监听User登录完成事件
}



SERVICE_END