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
 * Date: 2022-09-18 20:36:56
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <service/common/SessionType.h>
#include <service/common/PriorityLevelDefine.h>
#include <service/common/ServiceConfig.h>
#include <service/common/BaseComps/Event/Event.h>
#include <service/common/BaseComps/GlobalSys/GlobalSys.h>

SERVICE_BEGIN

class SessionMgr;
struct ServiceSessionInfo;

class ServiceSession
{
    POOL_CREATE_OBJ_DEFAULT(ServiceSession);

public:
    ServiceSession(SessionMgr *sessionMgr);
    virtual ~ServiceSession();

    virtual void Release();

    void SetSessionInfo(const ServiceSessionInfo &sessionInfo);
    UInt64 GetSessionId() const;
    const ServiceSessionInfo *GetSessionInfo() const;

    KERNEL_NS::LibString ToString() const;

private:
    void _Clear();

private:
    ServiceSessionInfo *_sessionInfo;
    SessionMgr *_sessionMgr;
};

ALWAYS_INLINE  const ServiceSessionInfo *ServiceSession::GetSessionInfo() const
{
    return _sessionInfo;
}

SERVICE_END
