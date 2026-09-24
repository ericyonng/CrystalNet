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
 * Description: tls上下文(openssl SSL_CTX封装): 服务端证书/私钥, 客户端校验配置
*/

#ifndef __CRYSTAL_NET_SERVICE_COMMON_BASE_COMPS_HTTP_HTTP_TLS_CONTEXT_H__
#define __CRYSTAL_NET_SERVICE_COMMON_BASE_COMPS_HTTP_HTTP_TLS_CONTEXT_H__

#pragma once
#include "kernel/comp/memory/ObjPoolMacro.h"
#include "service/common/macro.h"

struct ssl_ctx_st;
typedef struct ssl_ctx_st SSL_CTX;

SERVICE_BEGIN

class HttpTlsContext
{
    POOL_CREATE_OBJ_DEFAULT(HttpTlsContext);

public:
    HttpTlsContext();
    ~HttpTlsContext();
    void Release();

    // 服务端模式: 加载证书与私钥(pem格式文件) 返回: Status::Success成功, 其他为错误码
    Int32 InitServer(const KERNEL_NS::LibString &certFile, const KERNEL_NS::LibString &keyFile);

    // 客户端模式: verifyPeer为true时校验对端证书(caFile为空则使用系统默认CA) 返回: Status::Success成功, 其他为错误码
    Int32 InitClient(bool verifyPeer = false, const KERNEL_NS::LibString &caFile = "");

    bool IsInited() const;
    bool IsServer() const;
    SSL_CTX *GetCtx();
    const SSL_CTX *GetCtx() const;

    KERNEL_NS::LibString ToString() const;

private:
    SSL_CTX *_ctx;
    bool _isServer;
};

ALWAYS_INLINE bool HttpTlsContext::IsInited() const
{
    return _ctx != NULL;
}

ALWAYS_INLINE bool HttpTlsContext::IsServer() const
{
    return _isServer;
}

ALWAYS_INLINE SSL_CTX *HttpTlsContext::GetCtx()
{
    return _ctx;
}

ALWAYS_INLINE const SSL_CTX *HttpTlsContext::GetCtx() const
{
    return _ctx;
}

SERVICE_END

#endif
