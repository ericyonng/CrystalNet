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
 * Date: 2021-09-25 22:44:52
 * Author: Eric Yonng
 * Description: 
 * 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_SERVICE_ISERVICE_PROXY_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_SERVICE_ISERVICE_PROXY_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>

#include <kernel/comp/CompObject/CompObject.h>
#include <vector>

KERNEL_BEGIN

class IProtocolStack;
struct PollerEvent;
class LibSession;

class KERNEL_EXPORT IServiceProxy : public CompObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, IServiceProxy);

public:
    IServiceProxy(UInt64 objTypeId):CompObject(objTypeId) {}
    ~IServiceProxy() {}
    
    virtual void PostMsg(UInt64 serviceId, PollerEvent *msg, Int64 packetsCount = 0) = 0;
    virtual void PostQuitService() = 0;
    virtual IProtocolStack *GetProtocolStack(LibSession *session) = 0;

    virtual bool IsServiceReady(const KERNEL_NS::LibString &serviceName) const = 0;
};

KERNEL_END

#endif
