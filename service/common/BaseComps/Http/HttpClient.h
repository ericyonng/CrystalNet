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
 * Description: http/https客户端组件: 基于网络引擎(Windows IOCP/Linux epoll)的异步http客户端,
 *              支持GET/POST等方法, url以https://开头自动启用tls, 同时提供同步等待接口,
 *              响应回调在客户端内部派发线程执行
 *
 * 用法示例:
 *      auto *client = KERNEL_NS::HttpClientFactory::FactoryCreate()->Create()->CastTo<KERNEL_NS::HttpClient>();
 *      client->OnCreated();
 *      client->Init();
 *      client->Start();
 *      // 异步
 *      client->Get("http://127.0.0.1:8080/hello", [](Int32 err, KERNEL_NS::HttpResponse *res){
 *          // res生命周期仅限回调内
 *      });
 *      // 同步
 *      KERNEL_NS::HttpResponse res;
 *      Int32 err = client->SyncGet("https://www.example.com/", res);
 *      // ... 退出时:
 *      client->WillClose();
 *      client->Close();
 *      client->Release();
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_HTTP_HTTP_CLIENT_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_HTTP_HTTP_CLIENT_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/comp/Http/IHttpClient.h>
#include <kernel/comp/Config/KernelConfig.h>
#include <kernel/comp/Http/HttpDefs.h>
#include <kernel/comp/Http/HttpRequest.h>
#include <kernel/comp/Http/HttpResponse.h>
#include <kernel/comp/Lock/Impl/SpinLock.h>
#include <kernel/comp/Delegate/IDelegate.h>

#include <unordered_map>
#include <mutex>
#include <condition_variable>

KERNEL_BEGIN

class Poller;
class LibEventLoopThread;
class HttpProtocolStack;
class HttpServiceProxy;
class HttpTlsContext;
class TcpPollerMgr;
struct PollerEvent;

class KERNEL_EXPORT HttpClient : public IHttpClient
{
    POOL_CREATE_OBJ_DEFAULT_P1(IHttpClient, HttpClient);

    // 请求回调: errCode见status.h, res生命周期仅限回调内(errCode != Status::Success时res为NULL)
    using RequestCallback = IDelegate<void, Int32, HttpResponse *>;

public:
    HttpClient();
    virtual ~HttpClient();
    virtual void Release() override;
    virtual void OnRegisterComps() override;

    // 以下配置接口请在OnCreated/Init之前调用

    // 网络引擎配置(可选, 有默认值)
    virtual void SetNetConfig(const NetConfig &cfg) override;
    // 单条报文body上限(默认16MB)
    virtual void SetMaxBodyBytes(UInt64 maxBodyBytes) override;
    // 是否校验https对端证书(默认不校验), caFile为空则使用系统默认CA
    virtual void SetTlsVerifyPeer(bool verifyPeer, const LibString &caFile = "") override;

    // 异步请求(Start之后调用, 可在任意线程调用)
    // 返回: Status::Success成功投递, 其他为错误码
    Int32 Get(const LibString &url, RequestCallback *cb);
    Int32 Post(const LibString &url, const LibString &body, const LibString &contentType, RequestCallback *cb);
    virtual Int32 SendRequest(Int32 method, const LibString &url, const LibString &body, const LibString &contentType, RequestCallback *cb) override;

    template<typename LambType>
    Int32 Get(const LibString &url, LambType &&lamb)
    {
        return Get(url, KERNEL_CREATE_CLOSURE_DELEGATE(lamb, void, Int32, HttpResponse *));
    }

    template<typename LambType>
    Int32 Post(const LibString &url, const LibString &body, const LibString &contentType, LambType &&lamb)
    {
        return Post(url, body, contentType, KERNEL_CREATE_CLOSURE_DELEGATE(lamb, void, Int32, HttpResponse *));
    }

    // 同步请求(Start之后调用, 不可在客户端派发线程内调用) 结果填充resOut
    // 返回: Status::Success成功, 其他为错误码
    Int32 SyncGet(const LibString &url, HttpResponse &resOut);
    Int32 SyncPost(const LibString &url, const LibString &body, const LibString &contentType, HttpResponse &resOut);
    virtual Int32 SyncRequest(Int32 method, const LibString &url, const LibString &body, const LibString &contentType, HttpResponse &resOut) override;

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
    struct HttpUrlInfo
    {
        bool _isTls = false;
        LibString _host;
        UInt16 _port = 0;
        LibString _path;
        LibString _query;
        bool _isHostName = false;
    };

    struct PendingRequest
    {
        POOL_CREATE_OBJ_DEFAULT(PendingRequest);

        UInt64 _stub = 0;               // 连接存根
        UInt64 _sessionId = 0;          // 会话id(连接建立后回填)
        UInt64 _pollerId = 0;           // 会话所在poller
        bool _isTls = false;
        HttpRequest *_req = NULL;       // 请求(发出后由网络层管理)
        RequestCallback *_cb = NULL;    // 响应回调
    };

private:
    // 解析url 返回: Status::Success成功, 其他为错误码
    static Int32 _ParseUrl(const LibString &url, HttpUrlInfo &info);

    void _OnSessionCreated(PollerEvent *ev);
    void _OnAsynConnectRes(PollerEvent *ev);
    void _OnSessionDestroy(PollerEvent *ev);
    void _OnRecvMsg(PollerEvent *ev);
    void _OnTlsHandshake(UInt64 sessionId, Int32 errCode);

    // 在会话上发送请求
    void _DoSendRequest(PendingRequest *pending);
    // 完成请求并回调(回调后释放pending)
    void _FinishPending(PendingRequest *pending, Int32 errCode, HttpResponse *res);
    // 不从map中移除直接完成(close时统一清理场景)
    void _FinishPendingWithoutErase(PendingRequest *pending, Int32 errCode, HttpResponse *res);

    PendingRequest *_GetPendingByStub(UInt64 stub);
    PendingRequest *_GetPendingBySession(UInt64 sessionId);
    void _ErasePending(PendingRequest *pending);

private:
    NetConfig _netConfig;
    UInt64 _maxBodyBytes;
    bool _verifyPeer;
    LibString _caFile;

    HttpProtocolStack *_stack;
    HttpServiceProxy *_serviceProxy;
    HttpTlsContext *_tlsCtx;
    LibEventLoopThread *_dispatchThread;
    Poller *_dispatchPoller;
    TcpPollerMgr *_tcpPollerMgr;

    UInt64 _serviceId;
    std::atomic<UInt64> _maxStub;

    SpinLock _pendingLck;
    std::unordered_map<UInt64, PendingRequest *> _stubRefPending;
    std::unordered_map<UInt64, PendingRequest *> _sessionIdRefPending;
};

KERNEL_END

#endif
