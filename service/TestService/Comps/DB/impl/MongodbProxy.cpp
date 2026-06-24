// MIT License
// 
// Copyright (c) 2020 ericyonng<120453674@qq.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// Date: 2026-06-24 22:06:52
// Author: Eric Yonng
// Description:

#include <pch.h>

#include <Comps/DB/impl/MongodbProxy.h>
#include <Comps/DB/impl/MongodbProxyFactory.h>
#include <kernel/comp/Utils/RttiUtil.h>

SERVICE_BEGIN

MongodbProxy::MongodbProxy()
    :IMongodbProxy(KERNEL_NS::RttiUtil::GetTypeId<MongodbProxy>())
    ,_closeServiceStub(INVALID_LISTENER_STUB)
    ,_purgeTimer(NULL)
    ,_options(NULL)
{
    
}

MongodbProxy::~MongodbProxy()
{
    
}

void MongodbProxy::Release()
{
    MongodbProxy::DeleteByAdapter_MongodbProxy(MongodbProxyFactory::_buildType.V, this);
}

void MongodbProxy::OnRegisterComps()
{
    
}

void MongodbProxy::RegisterDependence(ILogicSys *obj)
{
    
}
void MongodbProxy::UnRegisterDependence(const ILogicSys *obj)
{
    
}

// 标脏
void MongodbProxy::MaskLogicNumberKeyAddDirty(const ILogicSys *logic, UInt64 key)
{
    
}
void MongodbProxy::MaskLogicNumberKeyModifyDirty(const ILogicSys *logic, UInt64 key)
{
    
}

void MongodbProxy::MaskLogicNumberKeyDeleteDirty(const ILogicSys *logic, UInt64 key)
{
    
}
    
void MongodbProxy::MaskLogicStringKeyAddDirty(const ILogicSys *logic, const KERNEL_NS::LibString &key)
{
    
}

void MongodbProxy::MaskLogicStringKeyModifyDirty(const ILogicSys *logic, const KERNEL_NS::LibString &key)
{
    
}

void MongodbProxy::MaskLogicStringKeyDeleteDirty(const ILogicSys *logic, const KERNEL_NS::LibString &key)
{
    
}

// 等待落库完成
KERNEL_NS::CoTask<> MongodbProxy::Purge()
{
    co_return;
}

// 等待logic落库完成
KERNEL_NS::CoTask<> MongodbProxy::Purge(const ILogicSys *logic)
{
    co_return;
}

Int32 MongodbProxy::_OnGlobalSysInit()
{
    return Status::Success;
}
Int32 MongodbProxy::_OnGlobalSysCompsCreated()
{
    return Status::Success;
}

Int32 MongodbProxy::_OnHostStart()
{
    return Status::Success;
}

void MongodbProxy::_OnGlobalSysClose()
{
    _Clear();
}

void MongodbProxy::_CloseServiceEvent(KERNEL_NS::LibEvent *ev)
{
    
}

void MongodbProxy::_Clear()
{
    
}


SERVICE_END

