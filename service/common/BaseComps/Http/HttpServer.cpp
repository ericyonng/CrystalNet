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
 * Description: http/https服务端组件实现
*/

#include <pch.h>
#include <kernel/comp/Http/HttpServer.h>
#include <kernel/comp/Http/HttpProtocolStack.h>
#include <kernel/comp/Http/HttpServiceProxy.h>
#include <kernel/comp/Http/HttpTlsContext.h>
#include <kernel/comp/Http/HttpRawCoder.h>
#include <kernel/comp/NetEngine/Poller/interface/IPollerMgr.h>
#include <kernel/comp/NetEngine/Poller/impl/Tcp/TcpPollerMgr.h>
#include <kernel/comp/NetEngine/Poller/Defs/PollerEvent.h>
#include <kernel/comp/NetEngine/Poller/Defs/PollerEventType.h>
#include <kernel/comp/NetEngine/Defs/LibListenInfo.h>
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
    static std::atomic<UInt64> s_httpServerServiceId{100000};

    // 路由key统一大写method
    static LibString s_ToUpper(const LibString &str)
    {
        LibString ret(str);
        const UInt64 len = static_cast<UInt64>(ret.size());
        for (UInt64 idx = 0; idx < len; ++idx)
        {
            if (ret[idx] >= 'a' && ret[idx] <= 'z')
                ret[idx] = static_cast<Byte8>(ret[idx] - 'a' + 'A');
        }

        return ret;
    }
}

HttpServer::HttpServer()
    :IHttpServer(KERNEL_NS::RttiUtil::GetTypeId<HttpServer>())
    , _listenPort(0)
    , _maxBodyBytes(HttpDefaultLimit::MAX_BODY_BYTES)
    , _stack(NULL)
    , _serviceProxy(NULL)
    , _tlsCtx(NULL)
    , _dispatchThread(NULL)
    , _dispatchPoller(NULL)
    , _tcpPollerMgr(NULL)
    ,_pollerMgr(NULL)
    , _serviceId(0)
    , _listenErrCode(Status::Error)
    , _notFoundHandler(NULL)
{
}

HttpServer::~HttpServer()
{
}

void HttpServer::Release()
{
    HttpServer::Delete_HttpServer(this);
}

void HttpServer::OnRegisterComps()
{
}

void HttpServer::SetListen(const LibString &ip, UInt16 port)
{
    _listenIp = ip;
    _listenPort = port;
}

void HttpServer::SetTlsCert(const LibString &certFile, const LibString &keyFile)
{
    _certFile = certFile;
    _keyFile = keyFile;
}

void HttpServer::SetPollerMgr(IPollerMgr *pollerMgr)
{
    _pollerMgr = pollerMgr;
}

void HttpServer::SetServiceId(UInt64 serviceId)
{
    _serviceId = serviceId;
}


void HttpServer::SetMaxBodyBytes(UInt64 maxBodyBytes)
{
    _maxBodyBytes = maxBodyBytes;
}

void HttpServer::RegisterHandler(const LibString &method, const LibString &path, IDelegate<void, HttpRequest *, HttpResponse *> *handler)
{
    if (UNLIKELY(!handler))
        return;

    const LibString key = _MakeRouteKey(method, path);
    auto iter = _routes.find(key);
    if (iter != _routes.end())
    {
        iter->second->Release();
        iter->second = handler;
        return;
    }

    _routes.insert(std::make_pair(key, handler));
}

void HttpServer::SetNotFoundHandler(IDelegate<void, HttpRequest *, HttpResponse *> *handler)
{
    CRYSTAL_RELEASE_SAFE(_notFoundHandler);
    _notFoundHandler = handler;
}

Int32 HttpServer::_OnHostInit()
{
    if (_pollerMgr == NULL)
    {
        CLOG_ERROR("poller mgr is null");
        return Status::Failed;
    }
    
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

    // 协议栈
    _stack = new HttpProtocolStack(HttpStackMode::Server);
    _stack->SetMaxBodyBytes(_maxBodyBytes);

    auto &&rawSendLamb = [this](UInt64 pollerId, UInt64 sessionId, LibPacket *packet)
    {
        if (LIKELY(_tcpPollerMgr))
            _tcpPollerMgr->PostSend(pollerId, sessionId, packet);
        else
            packet->Release();
    };
    _stack->SetRawSendHandler(KERNEL_CREATE_CLOSURE_DELEGATE(rawSendLamb, void, UInt64, UInt64, LibPacket *));

    // tls
    if (!_certFile.empty())
    {
        _tlsCtx = HttpTlsContext::New_HttpTlsContext();
        const Int32 err = _tlsCtx->InitServer(_certFile, _keyFile);
        if (err != Status::Success)
        {
            if (g_Log)
            {
                CLOG_ERROR("init tls ctx fail err:%d, cert file:%s, key file:%s", err, _certFile.c_str(), _keyFile.c_str());
            }

            return err;
        }

        _stack->SetTlsContext(_tlsCtx);
    }

    // 服务代理
    _serviceProxy = HttpServiceProxy::New_HttpServiceProxy();
    _serviceProxy->SetProtocolStack(_stack);

    if (g_Log)
    {
        CLOG_INFO("http server host init finish, listen:%s:%hu, tls:%d", _listenIp.c_str(), _listenPort, _tlsCtx ? 1 : 0);
    }

    return Status::Success;
}

Int32 HttpServer::_OnPriorityLevelCompsCreated()
{
    // 高优先级组件(PollerMgr)创建完成, 配置网络引擎
    auto pollerMgr = GetComp<IPollerMgr>();
    pollerMgr->SetConfig(_netConfig);
    pollerMgr->SetServiceProxy(_serviceProxy);

    return Status::Success;
}

Int32 HttpServer::_OnCompsCreated()
{
    return Status::Success;
}

Int32 HttpServer::_OnHostStart()
{
    // 所有组件已启动, 获取tcp poller管理器
    _tcpPollerMgr = _pollerMgr->GetComp<TcpPollerMgr>();
    if (UNLIKELY(!_tcpPollerMgr))
    {
        if (g_Log)
        {
            CLOG_ERROR("get tcp poller mgr fail.");
        }

        return Status::MissComp;
    }

    // 启动业务派发线程
    _dispatchThread = new LibEventLoopThread([this]() {}, LibString().AppendFormat("HttpServerDispatch%llu", _serviceId));
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

    _dispatchPoller->Subscribe(PollerEventType::SessionCreated, this, &HttpServer::_OnSessionCreated);
    _dispatchPoller->Subscribe(PollerEventType::SessionDestroy, this, &HttpServer::_OnSessionDestroy);
    _dispatchPoller->Subscribe(PollerEventType::RecvMsg, this, &HttpServer::_OnRecvMsg);
    _dispatchPoller->Subscribe(PollerEventType::AddListenRes, this, &HttpServer::_OnAddListenRes);

    _serviceProxy->SetEventPoller(_dispatchPoller);

    // 投递监听
    auto *listenInfo = LibListenInfo::New_LibListenInfo();
    listenInfo->_ip = _listenIp;
    listenInfo->_port = _listenPort;
    listenInfo->_family = AF_INET;
    listenInfo->_serviceId = _serviceId;
    listenInfo->_stub = _serviceId;     // stub非0才会有监听回执
    listenInfo->_protocolType = ProtocolType::TCP;
    listenInfo->_sessionCount = 1;
    listenInfo->_sessionOption._protocolStackType = _tlsCtx ? HttpProtocolStackType::HTTPS : HttpProtocolStackType::HTTP;
    listenInfo->_sessionOption._noDelay = true;
    listenInfo->_sessionOption._maxPacketSize = _maxBodyBytes + HttpDefaultLimit::MAX_HEADER_BYTES;

    _tcpPollerMgr->PostAddlisten(listenInfo);

    MaskReady(true);

    if (g_Log)
    {
        CLOG_INFO("http server start, listen:%s:%hu, tls:%d, service id:%llu"
            , _listenIp.c_str(), _listenPort, _tlsCtx ? 1 : 0, _serviceId);
    }

    return Status::Success;
}

void HttpServer::_OnHostBeforeCompsWillClose()
{
    // 先停业务派发线程, 避免资源竞争
    if (_dispatchThread)
        _dispatchThread->Close();
}

void HttpServer::_OnHostWillClose()
{
    if (g_Log)
    {
        CLOG_INFO("http server will close, service id:%llu", _serviceId);
    }
}

void HttpServer::_OnHostClose()
{
    // 路由
    for (auto &iter : _routes)
        iter.second->Release();
    _routes.clear();

    CRYSTAL_RELEASE_SAFE(_notFoundHandler);

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

    _sessionIdRefPollerId.clear();
}

void HttpServer::_OnSessionCreated(PollerEvent *ev)
{
    auto *created = static_cast<SessionCreatedEvent *>(ev);
    if (created->_isLinker || created->_isFromConnect)
        return;

    _sessionLck.Lock();
    _sessionIdRefPollerId[created->_sessionId] = created->_sessionPollerId;
    _sessionLck.Unlock();
}

void HttpServer::_OnSessionDestroy(PollerEvent *ev)
{
    auto *destroy = static_cast<SessionDestroyEvent *>(ev);

    _sessionLck.Lock();
    _sessionIdRefPollerId.erase(destroy->_sessionId);
    _streamCloseSessionIds.erase(destroy->_sessionId);
    _sessionLck.Unlock();

    _stack->OnSessionDestroy(destroy->_sessionId);
}

void HttpServer::_OnRecvMsg(PollerEvent *ev)
{
    auto *recvEv = static_cast<RecvMsgEvent *>(ev);
    if (!recvEv->_packets)
        return;

    for (auto *node = recvEv->_packets->Begin(); node; node = node->_next)
    {
        auto *packet = node->_data;
        if (!packet || (packet->GetOpcode() != HttpOpcode::HttpMessage))
            continue;

        auto *req = packet->GetCoder<HttpRequest>();
        if (UNLIKELY(!req))
            continue;

        _HandleRequest(recvEv->_sessionId, req);
    }
}

void HttpServer::_OnAddListenRes(PollerEvent *ev)
{
    auto *listenRes = static_cast<AddListenResEvent *>(ev);
    _listenErrCode.store(listenRes->_errCode, std::memory_order_release);

    if (g_Log)
    {
        if (listenRes->_errCode == Status::Success)
        {
            CLOG_INFO("http server listen suc, listen:%s:%hu, tls:%d", _listenIp.c_str(), _listenPort, _tlsCtx ? 1 : 0);
        }
        else
        {
            CLOG_ERROR("http server listen fail err:%d, listen:%s:%hu", listenRes->_errCode, _listenIp.c_str(), _listenPort);
        }
    }
}

void HttpServer::_HandleRequest(UInt64 sessionId, HttpRequest *req)
{
    auto *res = HttpResponse::New_HttpResponse();
    res->SetVersion("HTTP/1.1");

    const LibString key = _MakeRouteKey(req->GetRawMethod(), req->GetPath());
    auto iter = _routes.find(key);
    if (iter != _routes.end())
    {
        iter->second->Invoke(req, res);
    }
    else if (_notFoundHandler)
    {
        _notFoundHandler->Invoke(req, res);
    }
    else
    {
        _DefaultNotFoundHandler(req, res);
    }

    _SendResponse(sessionId, res, req->IsKeepAlive());
}

void HttpServer::_SendResponse(UInt64 sessionId, HttpResponse *res, bool keepAlive)
{
    const UInt64 pollerId = _GetSessionPollerId(sessionId);
    if (UNLIKELY(pollerId == 0))
    {
        if (g_Log)
        {
            CLOG_ERROR("session not exist, response will drop, session id:%llu", sessionId);
        }

        res->Release();
        return;
    }

    res->SetKeepAlive(keepAlive);

    auto *packet = LibPacket::New_LibPacket();
    packet->SetSessionId(sessionId);
    packet->SetOpcode(HttpOpcode::HttpMessage);
    packet->SetCoder(res);

    _tcpPollerMgr->PostSend(pollerId, sessionId, packet);

    if (!keepAlive)
    {
        if (res->IsChunkedStream())
        {
            // 流式chunked: 延迟到结束块后关闭
            _sessionLck.Lock();
            _streamCloseSessionIds.insert(sessionId);
            _sessionLck.Unlock();
        }
        else
        {
            _tcpPollerMgr->PostCloseSession(pollerId, _serviceId, sessionId, 1000, false, true);
        }
    }
}

Int32 HttpServer::SendChunk(UInt64 sessionId, const Byte8 *data, UInt64 len, bool isLast)
{
    const UInt64 pollerId = _GetSessionPollerId(sessionId);
    if (UNLIKELY(pollerId == 0))
    {
        if (g_Log)
        {
            CLOG_ERROR("session not exist, chunk will drop, session id:%llu", sessionId);
        }

        return Status::Http_SessionClosed;
    }

    // chunk编码: <hex len>\r\n<data>\r\n, 结束块: 0\r\n\r\n
    LibString chunk;
    if (isLast)
    {
        chunk = "0\r\n\r\n";
    }
    else
    {
        if (UNLIKELY(!data || (len == 0)))
            return Status::ParamError;

        chunk.AppendFormat("%llx", len);
        chunk += "\r\n";
        chunk.append(data, len);
        chunk += "\r\n";
    }

    auto *packet = LibPacket::New_LibPacket();
    packet->SetSessionId(sessionId);
    packet->SetOpcode(HttpOpcode::HttpLayerData);

    auto *coder = HttpRawCoder::New_HttpRawCoder();
    coder->SetData(chunk.data(), static_cast<UInt64>(chunk.size()));
    packet->SetCoder(coder);

    _tcpPollerMgr->PostSend(pollerId, sessionId, packet);

    if (isLast)
    {
        // 请求为close的流式会话, 结束块后关闭
        bool needClose = false;
        _sessionLck.Lock();
        auto iter = _streamCloseSessionIds.find(sessionId);
        if (iter != _streamCloseSessionIds.end())
        {
            _streamCloseSessionIds.erase(iter);
            needClose = true;
        }
        _sessionLck.Unlock();

        if (needClose)
            _tcpPollerMgr->PostCloseSession(pollerId, _serviceId, sessionId, 1000, false, true);
    }

    return Status::Success;
}

UInt64 HttpServer::_GetSessionPollerId(UInt64 sessionId) const
{
    _sessionLck.Lock();
    auto iter = _sessionIdRefPollerId.find(sessionId);
    const UInt64 pollerId = iter == _sessionIdRefPollerId.end() ? 0 : iter->second;
    _sessionLck.Unlock();
    return pollerId;
}

LibString HttpServer::_MakeRouteKey(const LibString &method, const LibString &path)
{
    LibString key = s_ToUpper(method);
    key += ' ';
    key += path;
    return key;
}

void HttpServer::_DefaultNotFoundHandler(HttpRequest *req, HttpResponse *res)
{
    res->SetStatusCode(HttpStatusCode::NotFound);
    res->SetReason(HttpStatusCode::Reason(HttpStatusCode::NotFound));
    res->SetHeader("Content-Type", "text/plain; charset=utf-8");

    LibString body;
    body.AppendFormat("404 Not Found: %s %s", req->GetRawMethod().c_str(), req->GetPath().c_str());
    res->SetBody(std::move(body));
}

KERNEL_END
