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
 * Date: 2023-08-12 17:57:38
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <ServiceCompHeader.h>
#include <protocols/protocols.h>

SERVICE_BEGIN

class IClientUser;

class IClientUserMgr : public IGlobalSys
{
    POOL_CREATE_OBJ_DEFAULT_P1(IGlobalSys, IClientUserMgr);

public:
    IClientUserMgr(UInt64 objTypeId) : IGlobalSys(objTypeId){}
    virtual Int32 Login(const LoginInfo &loginInfo, Int32 stackType = SERVICE_COMMON_NS::CrystalProtocolStackType::CRYSTAL_PROTOCOL) = 0;

    virtual void AddUserBySessionId(UInt64 sessionId, IClientUser *user) = 0;
    virtual void RemoveUserBySessionId(UInt64 sessionId) = 0;
};

SERVICE_END
