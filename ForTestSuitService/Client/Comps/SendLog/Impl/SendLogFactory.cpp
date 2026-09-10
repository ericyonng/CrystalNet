#include <pch.h>
#include <service/Client/Comps/SendLog/Impl/SendLogFactory.h>
#include <service/Client/Comps/SendLog/Impl/SendLog.h>

SERVICE_BEGIN

KERNEL_NS::CompFactory *SendLogFactory::FactoryCreate()
{
    return KERNEL_NS::ObjPoolWrap<SendLogFactory>::NewByAdapter(_buildType.V);
}

void SendLogFactory::Release()
{
    KERNEL_NS::ObjPoolWrap<SendLogFactory>::DeleteByAdapter(_buildType.V, this);
}

KERNEL_NS::CompObject *SendLogFactory::Create() const
{
    CREATE_CRYSTAL_COMP(comp, SendLog);
    return comp;
}

SERVICE_END