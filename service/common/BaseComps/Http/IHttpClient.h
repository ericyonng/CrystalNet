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
 * Date: 2026-09-24 10:00:00
 * Author: CrystalNet
 * Description: http客户端组件接口
*/

#ifndef __CRYSTAL_NET_SERVICE_COMMON_BASE_COMPS_HTTP_HTTP_CLIENT_H__
#define __CRYSTAL_NET_SERVICE_COMMON_BASE_COMPS_HTTP_HTTP_CLIENT_H__

#pragma once

#include "service/common/macro.h"
#include "service/common/BaseComps/GlobalSys/IGlobalSys.h"


SERVICE_BEGIN
   
class HttpResponse;

class KERNEL_EXPORT IHttpClient : public IGlobalSys
{
    POOL_CREATE_OBJ_DEFAULT_P1(IGlobalSys, IHttpClient);

public:
    IHttpClient(UInt64 objTypeId):IGlobalSys(objTypeId) {}
    virtual ~IHttpClient() override {}

    // 单条报文body上限(默认16MB)
    virtual void SetMaxBodyBytes(UInt64 maxBodyBytes) = 0;
    // 是否校验https对端证书(默认不校验), caFile为空则使用系统默认CA
    virtual void SetTlsVerifyPeer(bool verifyPeer, const LibString &caFile) = 0;

    // 异步请求(Start之后调用, 可在任意线程调用)
    // cb回调在客户端内部派发线程执行, res生命周期仅限回调内
    // 返回: Status::Success成功投递, 其他为错误码
    virtual Int32 SendRequest(Int32 method, const KERNEL_NS::LibString &url, const KERNEL_NS::LibString &body, const KERNEL_NS::LibString &contentType, KERNEL_NS::IDelegate<void, Int32, HttpResponse *> *cb) = 0;

    // 同步请求(Start之后调用, 不可在客户端派发线程内调用) 结果填充resOut
    // 返回: Status::Success成功, 其他为错误码
    virtual Int32 SyncRequest(Int32 method, const KERNEL_NS::LibString &url, const KERNEL_NS::LibString &body, const KERNEL_NS::LibString &contentType, HttpResponse &resOut) = 0;
};

SERVICE_END

#endif
