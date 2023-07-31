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
 * Date: 2023-07-31 23:47:39
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <Comps/User/interface/IUserSys.h>
#include <Comps/User/interface/IUser.h>
#include <Comps/User/interface/IUserMgr.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(IUserSys);

IUserSys::IUserSys(IUser *owner)
:_owner(owner)
,_userMgr(owner->GetUserMgr())
,_userEventMgr(owner->GetEventMgr())
{

}

IUserSys::~IUserSys()
{

}

Int64 IUserSys::Send(KERNEL_NS::LibPacket *packet) const
{
    return _owner->Send(packet);
}

void IUserSys::Send(const std::list<KERNEL_NS::LibPacket *> &packets) const
{
    _owner->Send(packets);
}

Int64 IUserSys::Send(Int32 opcode, const KERNEL_NS::ICoder &coder, Int64 packetId) const
{
    return _owner->Send(opcode, coder, packetId);
}

SERVICE_END
