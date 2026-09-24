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
 * Description: http服务端组件接口
*/

#ifndef __CRYSTAL_NET_SERVICE_COMMON_BASE_COMPS_HTTP_HTTP_SERVER_H__
#define __CRYSTAL_NET_SERVICE_COMMON_BASE_COMPS_HTTP_HTTP_SERVER_H__

#pragma once

#include <kernel/comp/Delegate/IDelegate.h>

#include "service/common/BaseComps/GlobalSys/IGlobalSys.h"

SERVICE_BEGIN

class HttpRequest;
class HttpResponse;
class IPollerMgr;

class IHttpServer : public SERVICE_NS::IGlobalSys
{
    POOL_CREATE_OBJ_DEFAULT_P1(IGlobalSys, IHttpServer);

public:
    IHttpServer(UInt64 objTypeId):IGlobalSys(objTypeId) {}
    virtual ~IHttpServer() override {}

    // 设置监听地址(Init之前调用)
    virtual void SetListen(const KERNEL_NS::LibString &ip, UInt16 port) = 0;
    // 设置tls证书与私钥(pem), 设置后启用https(Init之前调用)
    virtual void SetTlsCert(const KERNEL_NS::LibString &certFile, const KERNEL_NS::LibString &keyFile) = 0;
    virtual void SetPollerMgr(IPollerMgr *pollerMgr) = 0;
    // 服务id
    virtual void SetServiceId(UInt64 serviceId) = 0;
    // 单条报文body上限(默认16MB)
    virtual void SetMaxBodyBytes(UInt64 maxBodyBytes) = 0;

    // 注册路由: handler在服务端内部派发线程执行, req/res生命周期仅限handler内
    virtual void RegisterHandler(const KERNEL_NS::LibString &method, const KERNEL_NS::LibString &path, KERNEL_NS::IDelegate<void, HttpRequest *, HttpResponse *> *handler) = 0;
    // 未匹配路由时的处理(可选, 默认返回404)
    virtual void SetNotFoundHandler(KERNEL_NS::IDelegate<void, HttpRequest *, HttpResponse *> *handler) = 0;

    // 监听结果(Status::Success表示监听成功, Start之后有效)
    virtual Int32 GetListenErrCode() const = 0;

    // 流式chunked追加发送: handler中res->SetChunkedSend(true, true)仅发送响应首部后, 用本接口追加chunk块
    // isLast=true时发送结束块0\r\n\r\n(data可为NULL,len为0); 若请求为Connection: close则结束块后自动关闭会话
    // 返回: Status::Success成功投递, 其他为错误码
    virtual Int32 SendChunk(UInt64 sessionId, const Byte8 *data, UInt64 len, bool isLast) = 0;
};

SERVICE_END

#endif
