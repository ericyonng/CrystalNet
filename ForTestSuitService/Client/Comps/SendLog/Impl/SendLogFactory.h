#pragma once


#include <service/Client/ServiceCompFactoryHeader.h>

SERVICE_BEGIN

class SendLogFactory : public KERNEL_NS::CompFactory
{
public:
    static constexpr KERNEL_NS::_Build::TL _buildType{};

    static KERNEL_NS::CompFactory *FactoryCreate();

    virtual void Release() override;
    
public:
    virtual KERNEL_NS::CompObject *Create() const override;
};

SERVICE_END