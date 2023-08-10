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
 * Date: 2023-08-08 13:37:36
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <ServiceCompHeader.h>

SERVICE_BEGIN

class LoginPendingStatus
{
public:
    enum ENUMS
    {
        UN_LOGIN = 0,       // 未登录
        LOGINED,            // 已登录
    };
};

struct LoginPendingInfo
{
    POOL_CREATE_OBJ_DEFAULT(LoginPendingInfo);

    LoginPendingInfo(UInt64 sessionId);
    ~LoginPendingInfo();

    UInt64 _sessionId;
    Int32 _status;
    Int64 _expiredTime;
    KERNEL_NS::LibTimer *_timer;
};

ALWAYS_INLINE LoginPendingInfo::LoginPendingInfo(UInt64 sessionId)
:_sessionId(sessionId)
,_status(LoginPendingStatus::UN_LOGIN)
,_expiredTime(0)
,_timer(KERNEL_NS::LibTimer::NewThreadLocal_LibTimer())
{

}

ALWAYS_INLINE LoginPendingInfo::~LoginPendingInfo()
{
    if(LIKELY(_timer))
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(_timer);

    _timer = NULL;
}

SERVICE_END
