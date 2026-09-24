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
 * Description: http/https客户端组件实现
*/

#include <pch.h>
#include <kernel/comp/Http/HttpClient.h>
#include <kernel/comp/Http/HttpProtocolStack.h>
#include <kernel/comp/Http/HttpServiceProxy.h>
#include <kernel/comp/Http/HttpTlsContext.h>
#include <kernel/comp/NetEngine/Poller/impl/PollerMgrFactory.h>
#include <kernel/comp/NetEngine/Poller/impl/PollerMgr.h>
#include <kernel/comp/NetEngine/Poller/impl/Tcp/TcpPollerMgr.h>
#include <kernel/comp/NetEngine/Poller/Defs/PollerEvent.h>
#include <kernel/comp/NetEngine/Poller/Defs/PollerEventType.h>
#include <kernel/comp/NetEngine/Defs/LibConnectInfo.h>
#include <kernel/comp/NetEngine/Defs/ProtocolType.h>
#include <kernel/comp/NetEngine/LibPacket.h>
#include <kernel/comp/Poller/Poller.h>
#include <kernel/comp/thread/LibEventLoopThread.h>
#include <kernel/comp/Utils/RttiUtil.h>
#include <kernel/comp/Utils/SystemUtil.h>
#include <kernel/comp/Utils/SocketUtil.h>
#include <kernel/comp/Log/log.h>

KERNEL_BEGIN

namespace
{
    static std::atomic<UInt64> s_httpClientServiceId{200000};
}

HttpClient::HttpClient()
    :IHttpClient(KERNEL_NS::RttiUtil::GetTypeId<HttpClient>())
    , _maxBodyBytes(HttpDefaultLimit::MAX_BODY_BYTES)
    , _verifyPeer(false)
    , _stack(NULL)
    , _serviceProxy(NULL)
    , _tlsCtx(NULL)
    , _dispatchThread(NULL)
    , _dispatchPoller(NULL)
    , _tcpPollerMgr(NULL)
    , _serviceId(0)
    , _maxStub(0)
{
}

HttpClient::~HttpClient()
{
}

void HttpClient::Release()
{
    HttpClient::Delete_HttpClient(this);
}

void HttpClient::OnRegisterComps()
{
    RegisterComp<PollerMgrFactory>();
}

void HttpClient::SetNetConfig(const NetConfig &cfg)
{
    _netConfig = cfg;
}

void HttpClient::SetMaxBodyBytes(UInt64 maxBodyBytes)
{
    _maxBodyBytes = maxBodyBytes;
}

void HttpClient::SetTlsVerifyPeer(bool verifyPeer, const LibString &caFile)
{
    _verifyPeer = verifyPeer;
    _caFile = caFile;
}

Int32 HttpClient::Get(const LibString &url, RequestCallback *cb)
{
    return SendRequest(HttpMethodType::Get, url, "", "", cb);
}

Int32 HttpClient::Post(const LibString &url, const LibString &body, const LibString &contentType, RequestCallback *cb)
{
    return SendRequest(HttpMethodType::Post, url, body, contentType, cb);
}

Int32 HttpClient::SendRequest(Int32 method, const LibString &url, const LibString &body, const LibString &contentType, RequestCallback *cb)
{
    if (UNLIKELY(!cb))
        return Status::ParamError;

    if (UNLIKELY(!IsStarted()))
    {
        if (g_Log)
        {
            CLOG_ERROR("http client not started, url:%s", url.c_str());
        }

        cb->Release();
        return Status::NotStart;
    }

    HttpUrlInfo urlInfo;
    Int32 err = _ParseUrl(url, urlInfo);
    if (UNLIKELY(err != Status::Success))
    {
        if (g_Log)
        {
            CLOG_ERROR("parse url fail err:%d, url:%s", err, url.c_str());
        }

        cb->Release();
        return err;
    }

    if (UNLIKELY(urlInfo._isTls && !_tlsCtx))
    {
        if (g_Log)
        {
            CLOG_ERROR("tls ctx not inited, can not request https url:%s", url.c_str());
        }

        cb->Release();
        return Status::Http_TlsFail;
    }

    // 请求包
    auto *req = HttpRequest::New_HttpRequest();
    req->SetMethod(method);
    req->SetPath(urlInfo._path);
    req->SetQuery(urlInfo._query);
    req->SetVersion("HTTP/1.1");

    LibString host = urlInfo._host;
    if ((urlInfo._isTls && urlInfo._port != 443) || (!urlInfo._isTls && urlInfo._port != 80))
        host.AppendFormat(":%hu", urlInfo._port);
    req->SetHeader("Host", host);
    req->SetHeader("Connection", "close");

    if (!body.empty())
    {
        req->SetBody(body);
        if (!contentType.empty())
            req->SetHeader("Content-Type", contentType);
    }

    // 挂起请求
    auto *pending = PendingRequest::New_PendingRequest();
    pending->_stub = ++_maxStub;
    pending->_isTls = urlInfo._isTls;
    pending->_req = req;
    pending->_cb = cb;

    _pendingLck.Lock();
    _stubRefPending.insert(std::make_pair(pending->_stub, pending));
    _pendingLck.Unlock();

    // 投递异步连接(Windows下ConnectEx要求socket先bind, 绑定0.0.0.0:0由系统分配本地地址)
    auto *connectInfo = LibConnectInfo::New_LibConnectInfo();
    connectInfo->_localIp._ip = "0.0.0.0";
    connectInfo->_localPort = 0;
    connectInfo->_targetIp._ip = urlInfo._host;
    connectInfo->_targetIp._isHostName = urlInfo._isHostName;
    connectInfo->_targetPort = urlInfo._port;
    connectInfo->_family = AF_INET;
    connectInfo->_protocolType = ProtocolType::TCP;
    connectInfo->_stub = pending->_stub;
    connectInfo->_fromServiceId = _serviceId;
    connectInfo->_stack = _stack;
    connectInfo->_retryTimes = 0;
    connectInfo->_sessionOption._protocolStackType = urlInfo._isTls ? HttpProtocolStackType::HTTPS : HttpProtocolStackType::HTTP;
    connectInfo->_sessionOption._noDelay = true;
    connectInfo->_sessionOption._maxPacketSize = _maxBodyBytes + HttpDefaultLimit::MAX_HEADER_BYTES;

    _tcpPollerMgr->PostConnect(connectInfo);
    return Status::Success;
}

Int32 HttpClient::SyncGet(const LibString &url, HttpResponse &resOut)
{
    return SyncRequest(HttpMethodType::Get, url, "", "", resOut);
}

Int32 HttpClient::SyncPost(const LibString &url, const LibString &body, const LibString &contentType, HttpResponse &resOut)
{
    return SyncRequest(HttpMethodType::Post, url, body, contentType, resOut);
}

Int32 HttpClient::SyncRequest(Int32 method, const LibString &url, const LibString &body, const LibString &contentType, HttpResponse &resOut)
{
    std::mutex mtx;
    std::condition_variable cv;
    bool done = false;
    Int32 errCode = Status::Success;
    HttpResponse resCache;

    auto &&lamb = [&](Int32 err, HttpResponse *res)
    {
        {
            std::lock_guard<std::mutex> guard(mtx);
            errCode = err;
            if (res)
                resCache.CopyFrom(*res);

            done = true;
        }
        cv.notify_one();
    };

    const Int32 err = SendRequest(method, url, body, contentType, KERNEL_CREATE_CLOSURE_DELEGATE(lamb, void, Int32, HttpResponse *));
    if (err != Status::Success)
        return err;

    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&done] { return done; });
    }

    if (errCode == Status::Success)
        resOut.CopyFrom(resCache);

    return errCode;
}

Int32 HttpClient::_OnHostInit()
{
    // 初始化socket环境(幂等)
    const Int32 sockEnvErr = SocketUtil::InitSocketEnv();
    if (UNLIKELY(sockEnvErr != Status::Success))
    {
        if (g_Log)
        {
            CLOG_ERROR("init socket env fail err:%d", sockEnvErr);
        }

        return sockEnvErr;
    }

    _serviceId = ++s_httpClientServiceId;

    // 协议栈
    _stack = new HttpProtocolStack(HttpStackMode::Client);
    _stack->SetMaxBodyBytes(_maxBodyBytes);

    auto &&rawSendLamb = [this](UInt64 pollerId, UInt64 sessionId, LibPacket *packet)
    {
        if (LIKELY(_tcpPollerMgr))
            _tcpPollerMgr->PostSend(pollerId, sessionId, packet);
        else
            packet->Release();
    };
    _stack->SetRawSendHandler(KERNEL_CREATE_CLOSURE_DELEGATE(rawSendLamb, void, UInt64, UInt64, LibPacket *));

    auto &&tlsHandshakeLamb = [this](UInt64 sessionId, Int32 errCode)
    {
        _OnTlsHandshake(sessionId, errCode);
    };
    _stack->SetTlsHandshakeHandler(KERNEL_CREATE_CLOSURE_DELEGATE(tlsHandshakeLamb, void, UInt64, Int32));

    // 客户端tls上下文(创建失败不影响http使用, 仅https请求会报错)
    _tlsCtx = HttpTlsContext::New_HttpTlsContext();
    Int32 err = _tlsCtx->InitClient(_verifyPeer, _caFile);
    if (err != Status::Success)
    {
        if (g_Log)
        {
            CLOG_WARN("init client tls ctx fail err:%d, https request will not work.", err);
        }

        HttpTlsContext::Delete_HttpTlsContext(_tlsCtx);
        _tlsCtx = NULL;
    }
    else
    {
        _stack->SetTlsContext(_tlsCtx);
    }

    // 服务代理
    _serviceProxy = HttpServiceProxy::New_HttpServiceProxy();
    _serviceProxy->SetProtocolStack(_stack);

    if (g_Log)
    {
        CLOG_INFO("http client host init finish, service id:%llu", _serviceId);
    }

    return Status::Success;
}

Int32 HttpClient::_OnPriorityLevelCompsCreated()
{
    // 高优先级组件(PollerMgr)创建完成, 配置网络引擎
    auto pollerMgr = GetComp<IPollerMgr>();
    pollerMgr->SetConfig(_netConfig);
    pollerMgr->SetServiceProxy(_serviceProxy);

    return Status::Success;
}

Int32 HttpClient::_OnCompsCreated()
{
    return Status::Success;
}

Int32 HttpClient::_OnHostStart()
{
    // 所有组件已启动, 获取tcp poller管理器
    _tcpPollerMgr = GetComp<IPollerMgr>()->GetComp<TcpPollerMgr>();
    if (UNLIKELY(!_tcpPollerMgr))
    {
        if (g_Log)
        {
            CLOG_ERROR("get tcp poller mgr fail.");
        }

        return Status::MissComp;
    }

    // 启动业务派发线程
    _dispatchThread = new LibEventLoopThread([this]() {}, LibString().AppendFormat("HttpClientDispatch%llu", _serviceId));
    _dispatchThread->Start();

    // 等待poller ready
    Int32 waitCount = 0;
    while (true)
    {
        _dispatchPoller = _dispatchThread->GetPollerNoAsync();
        if (LIKELY(_dispatchPoller))
            break;

        if (UNLIKELY(++waitCount > 10000))
        {
            if (g_Log)
            {
                CLOG_ERROR("wait dispatch poller ready timeout.");
            }

            return Status::PollerFail;
        }

        SystemUtil::ThreadSleep(1);
    }

    // 减小事件轮询间隔, 提升请求处理吞吐(默认粒度会成为高并发瓶颈)
    _dispatchPoller->SetMaxSleepMilliseconds(1);

    _dispatchPoller->Subscribe(PollerEventType::SessionCreated, this, &HttpClient::_OnSessionCreated);
    _dispatchPoller->Subscribe(PollerEventType::AsynConnectRes, this, &HttpClient::_OnAsynConnectRes);
    _dispatchPoller->Subscribe(PollerEventType::SessionDestroy, this, &HttpClient::_OnSessionDestroy);
    _dispatchPoller->Subscribe(PollerEventType::RecvMsg, this, &HttpClient::_OnRecvMsg);

    _serviceProxy->SetEventPoller(_dispatchPoller);

    MaskReady(true);

    if (g_Log)
    {
        CLOG_INFO("http client start, service id:%llu", _serviceId);
    }

    return Status::Success;
}

void HttpClient::_OnHostBeforeCompsWillClose()
{
    // 先停业务派发线程, 避免资源竞争
    if (_dispatchThread)
        _dispatchThread->Close();
}

void HttpClient::_OnHostWillClose()
{
    if (g_Log)
    {
        CLOG_INFO("http client will close, service id:%llu", _serviceId);
    }
}

void HttpClient::_OnHostClose()
{
    // 未完成请求回调错误
    _pendingLck.Lock();
    auto pendingCopy = _sessionIdRefPending;
    auto stubPendingCopy = _stubRefPending;
    _sessionIdRefPending.clear();
    _stubRefPending.clear();
    _pendingLck.Unlock();

    for (auto &iter : stubPendingCopy)
    {
        auto *pending = iter.second;
        if (pending->_sessionId != 0 && pendingCopy.find(pending->_sessionId) != pendingCopy.end())
            pendingCopy.erase(pending->_sessionId);

        _FinishPendingWithoutErase(pending, Status::Http_SessionClosed, NULL);
    }

    for (auto &iter : pendingCopy)
        _FinishPendingWithoutErase(iter.second, Status::Http_SessionClosed, NULL);

    if (_serviceProxy)
    {
        _serviceProxy->SetEventPoller(NULL);
        _serviceProxy->SetProtocolStack(NULL);
        HttpServiceProxy::Delete_HttpServiceProxy(_serviceProxy);
        _serviceProxy = NULL;
    }

    CRYSTAL_DELETE_SAFE(_dispatchThread);
    CRYSTAL_DELETE_SAFE(_stack);

    if (_tlsCtx)
    {
        HttpTlsContext::Delete_HttpTlsContext(_tlsCtx);
        _tlsCtx = NULL;
    }
}

Int32 HttpClient::_ParseUrl(const LibString &url, HttpUrlInfo &info)
{
    if (UNLIKELY(url.empty()))
        return Status::Http_BadUrl;

    LibString rest = url;

    // scheme
    const auto schemePos = url.find("://");
    if (schemePos != LibString::npos)
    {
        LibString scheme = url.substr(0, schemePos);
        for (auto &ch : scheme)
        {
            if (ch >= 'A' && ch <= 'Z')
                ch = static_cast<Byte8>(ch - 'A' + 'a');
        }

        if (scheme == "https")
            info._isTls = true;
        else if (scheme == "http")
            info._isTls = false;
        else
            return Status::Http_BadUrl;

        rest = url.substr(schemePos + 3);
    }

    // host[:port] 与 path?query
    const auto slashPos = rest.find('/');
    const LibString hostPort = (slashPos == LibString::npos) ? rest : rest.substr(0, slashPos);
    LibString pathQuery = (slashPos == LibString::npos) ? "" : rest.substr(slashPos);

    const auto questionPos = pathQuery.find('?');
    if (questionPos != LibString::npos)
    {
        info._path = pathQuery.substr(0, questionPos);
        info._query = pathQuery.substr(questionPos + 1);
    }
    else
    {
        info._path = pathQuery;
    }

    if (info._path.empty())
        info._path = "/";

    const auto colonPos = hostPort.rfind(':');
    if (colonPos != LibString::npos)
    {
        info._host = hostPort.substr(0, colonPos);
        const LibString portStr = hostPort.substr(colonPos + 1);
        UInt32 port = 0;
        for (auto ch : portStr)
        {
            if (UNLIKELY(ch < '0' || ch > '9'))
                return Status::Http_BadUrl;

            port = port * 10 + static_cast<UInt32>(ch - '0');
        }

        if (UNLIKELY(port == 0 || port > 65535))
            return Status::Http_BadUrl;

        info._port = static_cast<UInt16>(port);
    }
    else
    {
        info._host = hostPort;
        info._port = info._isTls ? 443 : 80;
    }

    if (UNLIKELY(info._host.empty()))
        return Status::Http_BadUrl;

    // 是否域名(存在非数字非点的字符则为域名)
    info._isHostName = false;
    for (auto ch : info._host)
    {
        if ((ch < '0' || ch > '9') && ch != '.')
        {
            info._isHostName = true;
            break;
        }
    }

    return Status::Success;
}

void HttpClient::_OnSessionCreated(PollerEvent *ev)
{
    auto *created = static_cast<SessionCreatedEvent *>(ev);
    if (!created->_isFromConnect)
        return;

    auto *pending = _GetPendingByStub(created->_stub);
    if (!pending)
        return;

    pending->_sessionId = created->_sessionId;
    pending->_pollerId = created->_sessionPollerId;

    _pendingLck.Lock();
    _sessionIdRefPending[created->_sessionId] = pending;
    _pendingLck.Unlock();
}

void HttpClient::_OnAsynConnectRes(PollerEvent *ev)
{
    auto *connectRes = static_cast<AsynConnectResEvent *>(ev);
    auto *pending = _GetPendingByStub(connectRes->_stub);
    if (!pending)
        return;

    if (connectRes->_errCode != Status::Success)
    {
        if (g_Log)
        {
            CLOG_ERROR("connect fail err:%d, target:%s", connectRes->_errCode, connectRes->_targetAddr.ToString().c_str());
        }

        _FinishPending(pending, Status::Http_ConnectFail, NULL);
        return;
    }

    pending->_sessionId = connectRes->_sessionId;
    pending->_pollerId = connectRes->_sessionPollerId;

    if (pending->_isTls)
    {
        // 发起tls握手, 握手完成后由_OnTlsHandshake发送请求
        const Int32 err = _stack->StartTlsClientHandshake(connectRes->_sessionId, connectRes->_sessionPollerId);
        if (err != Status::Success && err != Status::Repeat)
            _FinishPending(pending, err, NULL);
    }
    else
    {
        _DoSendRequest(pending);
    }
}

void HttpClient::_OnSessionDestroy(PollerEvent *ev)
{
    auto *destroy = static_cast<SessionDestroyEvent *>(ev);

    auto *pending = _GetPendingBySession(destroy->_sessionId);
    if (pending)
        _FinishPending(pending, Status::Http_SessionClosed, NULL);

    _stack->OnSessionDestroy(destroy->_sessionId);
}

void HttpClient::_OnRecvMsg(PollerEvent *ev)
{
    auto *recvEv = static_cast<RecvMsgEvent *>(ev);
    if (!recvEv->_packets)
        return;

    for (auto *node = recvEv->_packets->Begin(); node; node = node->_next)
    {
        auto *packet = node->_data;
        if (!packet || (packet->GetOpcode() != HttpOpcode::HttpMessage))
            continue;

        auto *res = packet->GetCoder<HttpResponse>();
        if (UNLIKELY(!res))
            continue;

        // 跳过1xx中间响应(如100 Continue), 继续等待最终响应
        const Int32 statusCode = res->GetStatusCode();
        if ((statusCode >= 100) && (statusCode < 200))
            continue;

        auto *pending = _GetPendingBySession(recvEv->_sessionId);
        if (!pending)
            continue;

        const UInt64 pollerId = pending->_pollerId;
        const UInt64 sessionId = recvEv->_sessionId;

        _FinishPending(pending, Status::Success, res);

        // 短连接: 响应完成后关闭会话
        _tcpPollerMgr->PostCloseSession(pollerId, _serviceId, sessionId, 0, false, false);
    }
}

void HttpClient::_OnTlsHandshake(UInt64 sessionId, Int32 errCode)
{
    auto *pending = _GetPendingBySession(sessionId);
    if (!pending)
        return;

    if (errCode != Status::Success)
    {
        _FinishPending(pending, errCode, NULL);
        return;
    }

    _DoSendRequest(pending);
}

void HttpClient::_DoSendRequest(PendingRequest *pending)
{
    auto *packet = LibPacket::New_LibPacket();
    packet->SetSessionId(pending->_sessionId);
    packet->SetOpcode(HttpOpcode::HttpMessage);
    packet->SetCoder(pending->_req);
    pending->_req = NULL;

    _tcpPollerMgr->PostSend(pending->_pollerId, pending->_sessionId, packet);
}

void HttpClient::_FinishPending(PendingRequest *pending, Int32 errCode, HttpResponse *res)
{
    _ErasePending(pending);
    _FinishPendingWithoutErase(pending, errCode, res);
}

void HttpClient::_FinishPendingWithoutErase(PendingRequest *pending, Int32 errCode, HttpResponse *res)
{
    if (pending->_cb)
    {
        pending->_cb->Invoke(errCode, res);
        pending->_cb->Release();
        pending->_cb = NULL;
    }

    if (pending->_req)
    {
        pending->_req->Release();
        pending->_req = NULL;
    }

    PendingRequest::Delete_PendingRequest(pending);
}

HttpClient::PendingRequest *HttpClient::_GetPendingByStub(UInt64 stub)
{
    _pendingLck.Lock();
    auto iter = _stubRefPending.find(stub);
    auto *pending = iter == _stubRefPending.end() ? NULL : iter->second;
    _pendingLck.Unlock();
    return pending;
}

HttpClient::PendingRequest *HttpClient::_GetPendingBySession(UInt64 sessionId)
{
    _pendingLck.Lock();
    auto iter = _sessionIdRefPending.find(sessionId);
    auto *pending = iter == _sessionIdRefPending.end() ? NULL : iter->second;
    _pendingLck.Unlock();
    return pending;
}

void HttpClient::_ErasePending(PendingRequest *pending)
{
    _pendingLck.Lock();
    _stubRefPending.erase(pending->_stub);
    if (pending->_sessionId != 0)
        _sessionIdRefPending.erase(pending->_sessionId);
    _pendingLck.Unlock();
}

KERNEL_END
