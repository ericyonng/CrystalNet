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
 * Date: 2023-08-12 22:20:38
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <Comps/User/interface/IClientSys.h>
#include <Comps/User/interface/IClientUser.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(IClientSys);

IClientSys::IClientSys(UInt64 objTypeId)
:ILogicSys(objTypeId)
,_userOwner(NULL)
,_userMgr(NULL)
{
    _SetType(ServiceCompType::USER_SYS_COMP);
    AddFlag(LogicSysFlagsType::DISABLE_FOCUS_BY_SERVICE_FLAG);
}

IClientSys::~IClientSys()
{

}

Int64 IClientSys::Send(KERNEL_NS::LibPacket *packet) const
{
    return _userOwner->Send(packet);
}

void IClientSys::Send(const std::list<KERNEL_NS::LibPacket *> &packets) const
{
    _userOwner->Send(packets);
}

Int64 IClientSys::Send(Int32 opcode, const KERNEL_NS::ICoder &coder, Int64 packetId) const
{
    return _userOwner->Send(opcode, coder, packetId);
}

void IClientSys::OnLogin()
{

}

void IClientSys::OnLoginFinish()
{

}

void IClientSys::OnLogout()
{

}

void IClientSys::OnUserCreated()
{

}

Int32 IClientSys::_OnSysInit()
{
    _userOwner = GetOwner()->CastTo<IClientUser>();
    _userMgr = _userOwner->GetUserMgr();

    // 使用User的EventMgr
    SetEventMgr(_userOwner->GetEventMgr());

    auto err = _OnUserSysInit();
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("_OnUserSysInit fail err:%d, user:%s"), err, _userOwner->ToString().c_str());
        return err;
    }

    return Status::Success;
}
SERVICE_END
