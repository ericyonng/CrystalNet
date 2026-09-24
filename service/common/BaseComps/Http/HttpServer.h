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
 * Description: http/https服务端组件: 基于网络引擎(Windows IOCP/Linux epoll)的高并发http服务端,
 *              支持GET/POST等方法路由注册, 业务handler在独立派发线程执行,
 *              配置证书后自动启用https
 *
 * 用法示例:
 *      auto *server = KERNEL_NS::HttpServerFactory::FactoryCreate()->Create()->CastTo<KERNEL_NS::HttpServer>();
 *      server->SetListen("0.0.0.0", 8080);
 *      // server->SetTlsCert("server.crt", "server.key");   // 可选, 设置后启用https
 *      server->RegisterHandler("GET", "/hello", [](KERNEL_NS::HttpRequest *req, KERNEL_NS::HttpResponse *res){
 *          res->SetStatusCode(KERNEL_NS::HttpStatusCode::Ok);
 *          res->SetBody("hello world");
 *      });
 *      server->OnCreated();
 *      server->Init();
 *      server->Start();
 *      // ... 退出时:
 *      server->WillClose();
 *      server->Close();
 *      server->Release();
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_HTTP_HTTP_SERVER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_HTTP_HTTP_SERVER_H__

#pragma once

#include <kernel/comp/Http/IHttpServer.h>
#include <kernel/comp/Config/KernelConfig.h>
#include <kernel/comp/Http/HttpDefs.h>
#include <kernel/comp/Http/HttpRequest.h>
#include <kernel/comp/Http/HttpResponse.h>
#include <kernel/comp/Lock/Impl/SpinLock.h>
#include <kernel/comp/Delegate/IDelegate.h>

#include <unordered_map>
#include <unordered_set>
#include <map>

KERNEL_BEGIN
    
class IPollerMgr;

class Poller;
class LibEventLoopThread;
class HttpProtocolStack;
class HttpServiceProxy;
class HttpTlsContext;
class TcpPollerMgr;
struct PollerEvent;

class HttpServer : public IHttpServer
{
    POOL_CREATE_OBJ_DEFAULT_P1(IHttpServer, HttpServer);

public:
    HttpServer();
    virtual ~HttpServer();
    virtual void Release() override;
    virtual void OnRegisterComps() override;

    // 以下接口请在OnCreated/Init之前调用

    // 设置监听地址
    virtual void SetListen(const LibString &ip, UInt16 port) override;
    // 设置tls证书与私钥(pem), 设置后启用https
    virtual void SetTlsCert(const LibString &certFile, const LibString &keyFile) override;
    virtual void SetPollerMgr(IPollerMgr *pollerMgr) override;
    // 服务id
    virtual void SetServiceId(UInt64 serviceId) override;
    // 单条报文body上限(默认16MB)
    virtual void SetMaxBodyBytes(UInt64 maxBodyBytes) override;

    // 注册路由: method支持GET/POST等(见HttpMethodType), path精确匹配如"/hello"
    // handler在服务端内部派发线程执行, req/res生命周期仅限handler内, res由handler填充
    virtual void RegisterHandler(const LibString &method, const LibString &path, IDelegate<void, HttpRequest *, HttpResponse *> *handler) override;
    template<typename LambType>
    void RegisterHandler(const LibString &method, const LibString &path, LambType &&lamb)
    {
        RegisterHandler(method, path, KERNEL_CREATE_CLOSURE_DELEGATE(lamb, void, HttpRequest *, HttpResponse *));
    }

    // 未匹配路由时的处理(可选, 默认返回404)
    virtual void SetNotFoundHandler(IDelegate<void, HttpRequest *, HttpResponse *> *handler) override;
    template<typename LambType>
    void SetNotFoundHandler(LambType &&lamb)
    {
        SetNotFoundHandler(KERNEL_CREATE_CLOSURE_DELEGATE(lamb, void, HttpRequest *, HttpResponse *));
    }

    // 监听结果(Status::Success表示监听成功, Start之后有效)
    virtual Int32 GetListenErrCode() const override;

    // 流式chunked追加发送(handler中res->SetChunkedSend(true, true)仅发送首部后调用)
    virtual Int32 SendChunk(UInt64 sessionId, const Byte8 *data, UInt64 len, bool isLast) override;

    // 内部有线程, ready在_OnHostStart中由组件自行标记
    virtual void DefaultMaskReady(bool isReady) override {}

protected:
    virtual Int32 _OnHostInit() override;
    virtual Int32 _OnPriorityLevelCompsCreated() override;
    virtual Int32 _OnCompsCreated() override;
    virtual Int32 _OnHostStart() override;
    virtual void _OnHostBeforeCompsWillClose() override;
    virtual void _OnHostWillClose() override;
    virtual void _OnHostClose() override;

private:
    void _OnSessionCreated(PollerEvent *ev);
    void _OnSessionDestroy(PollerEvent *ev);
    void _OnRecvMsg(PollerEvent *ev);
    void _OnAddListenRes(PollerEvent *ev);

    void _HandleRequest(UInt64 sessionId, HttpRequest *req);
    void _SendResponse(UInt64 sessionId, HttpResponse *res, bool keepAlive);
    UInt64 _GetSessionPollerId(UInt64 sessionId) const;

    static LibString _MakeRouteKey(const LibString &method, const LibString &path);
    static void _DefaultNotFoundHandler(HttpRequest *req, HttpResponse *res);

private:
    LibString _listenIp;
    UInt16 _listenPort;
    LibString _certFile;
    LibString _keyFile;
    UInt64 _maxBodyBytes;

    HttpProtocolStack *_stack;
    HttpServiceProxy *_serviceProxy;
    HttpTlsContext *_tlsCtx;
    LibEventLoopThread *_dispatchThread;
    Poller *_dispatchPoller;
    TcpPollerMgr *_tcpPollerMgr;
    IPollerMgr *_pollerMgr;

    UInt64 _serviceId;
    std::atomic<Int32> _listenErrCode;

    mutable SpinLock _sessionLck;
    std::unordered_map<UInt64, UInt64> _sessionIdRefPollerId;
    std::unordered_set<UInt64> _streamCloseSessionIds;  // 流式chunked且请求为close的会话(结束块后需关闭)

    std::map<LibString, IDelegate<void, HttpRequest *, HttpResponse *> *> _routes;
    IDelegate<void, HttpRequest *, HttpResponse *> *_notFoundHandler;
};

ALWAYS_INLINE Int32 HttpServer::GetListenErrCode() const
{
    return _listenErrCode.load(std::memory_order_acquire);
}

KERNEL_END

#endif
