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
 * Date: 2023-09-17 16:28:11
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/kernel.h>
#include <service_common/ServiceCommon.h>
#include <service/common/common.h>
#include <service/TestService/Common/ServiceCommon.h>

#include <Comps/InviteCode/impl/InviteCodeGlobal.h>
#include <Comps/InviteCode/impl/InviteCodeGlobalFactory.h>
#include <Comps/InviteCode/impl/InviteCodeGlobalStorageFactory.h>
#include <Comps/config/config.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(IInviteCodeGlobal);
POOL_CREATE_OBJ_DEFAULT_IMPL(InviteCodeGlobal);

InviteCodeGlobal::InviteCodeGlobal()
{

}

InviteCodeGlobal::~InviteCodeGlobal()
{

}

void InviteCodeGlobal::Release()
{
    InviteCodeGlobal::DeleteByAdapter_InviteCodeGlobal(InviteCodeGlobalFactory::_buildType.V, this);
}

void InviteCodeGlobal::OnRegisterComps()
{
    RegisterComp<InviteCodeGlobalStorageFactory>();
}

Int32 InviteCodeGlobal::OnLoaded(UInt64 key, const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db)
{
    KERNEL_NS::LibString inviteCode;
    const auto len = db.GetReadableSize();
    inviteCode.AppendData(db.GetReadBegin(), len);

    _AddInviteCode(key, inviteCode);

    return Status::Success;  
}

Int32 InviteCodeGlobal::OnSave(UInt64 key, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) const
{
    auto iter = _idRefInviteCode.find(key);
    if(iter == _idRefInviteCode.end())
    {
        g_Log->Error(LOGFMT_OBJ_TAG("cant find key:%llu"), key);
        return Status::Failed;
    }

    db.Write(iter->second.data(), static_cast<Int64>(iter->second.length()));
    return Status::Success;
}

bool InviteCodeGlobal::IsUsed(const KERNEL_NS::LibString &inviteCode) const
{
    return _usedInviteCodes.find(inviteCode) != _usedInviteCodes.end();
}

bool InviteCodeGlobal::IsValidCode(const KERNEL_NS::LibString &inviteCode) const
{
    auto inviteCodeConfigMgr = GetGlobalSys<ConfigLoader>()->GetComp<InviteCodeConfigMgr>();
    return inviteCodeConfigMgr->GetConfigByInviteCode(inviteCode) != NULL;
}

void InviteCodeGlobal::AddUsedInviteCode(const KERNEL_NS::LibString &inviteCode)
{
    auto id = GetGlobalSys<IGlobalUidMgr>()->NewGuid();
    _AddInviteCode(id, inviteCode);
    MaskNumberKeyAddDirty(id);
}

void InviteCodeGlobal::_AddInviteCode(UInt64 id, const KERNEL_NS::LibString &inviteCode)
{
    if(_usedInviteCodes.find(inviteCode) != _usedInviteCodes.end())
        return;

    _usedInviteCodes.insert(inviteCode);
    _idRefInviteCode.insert(std::make_pair(id, inviteCode));
}

SERVICE_END
