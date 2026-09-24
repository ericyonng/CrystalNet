# Http 组件（HTTP/HTTPS 客户端与服务端）

基于 NetEngine（Windows IOCP / Linux epoll 高并发网络引擎）+ OpenSSL 的 HTTP/1.1 组件，提供路由注册式服务端与同步/异步客户端。

- 聚合入口：`#include <kernel/comp/Http/Http.h>`（框架总入口 `comp/comp.h` 已包含）
- 支持方法：GET/POST/PUT/DELETE/HEAD/OPTIONS/PATCH/CONNECT/TRACE（见 `HttpMethodType`）
- body 传输：`Content-Length` 与 `Transfer-Encoding: chunked`（收方向自动解码组包；发方向支持整包 chunked 与 `HttpServer::SendChunk` 流式发送）
- `Expect: 100-continue`：服务端自动应答 `100 Continue`，客户端自动跳过 1xx 中间响应
- 跨平台：Windows / Linux，平台差异由 NetEngine 底层屏蔽
- 错误一律走 `Status::` 错误码（Http 段 6300-6399），不抛异常

## 组件构成

| 类 | 说明 | 头文件 |
|----|------|--------|
| `IHttpServer` / `HttpServer` / `HttpServerFactory` | 服务端组件（CompHostObject） | `HttpServer.h` |
| `IHttpClient` / `HttpClient` / `HttpClientFactory` | 客户端组件（CompHostObject） | `HttpClient.h` |
| `HttpRequest` / `HttpResponse` / `HttpMessage` | 报文对象（ICoder，对象池） | `HttpRequest.h` / `HttpResponse.h` |
| `HttpParser` | 报文边界切割（粘包/半包/chunked） | `HttpParser.h` |
| `HttpProtocolStack` | 协议栈（IProtocolStack 实现，含 TLS 状态机） | `HttpProtocolStack.h` |
| `HttpTlsContext` | OpenSSL 封装（证书/CA/会话复用） | `HttpTlsContext.h` |

## 前置：kernel 初始化

```cpp
class LogFactory : public KERNEL_NS::ILogFactory
{
public:
    virtual KERNEL_NS::ILog *Create() override { return new KERNEL_NS::LibLog(); }
};

LogFactory logFactory;
KERNEL_NS::KernelUtil::Init(&logFactory, "Log.yaml", "./ini/", NULL);
KERNEL_NS::KernelUtil::Start();
// 进程退出前: KERNEL_NS::KernelUtil::Destroy();
```

socket 环境（WSAStartup/IOCP）由组件内部自动初始化，无需调用方处理。

## HttpServer 使用

```cpp
// 1. 创建与配置(全部在Init之前)
auto *server = KERNEL_NS::HttpServerFactory::FactoryCreate()->Create()->CastTo<KERNEL_NS::HttpServer>();
server->SetListen("0.0.0.0", 8080);
// server->SetTlsCert("server.crt", "server.key");   // 可选: 启用https(pem证书+私钥)
// server->SetNetConfig(cfg);                        // 可选: 网络引擎调优(poller数量等)
// server->SetMaxBodyBytes(32 * 1024 * 1024);        // 可选: body上限(默认16MB)

// 2. 注册路由(method/path精确匹配, lambda或IDelegate*)
server->RegisterHandler("GET", "/hello", [](KERNEL_NS::HttpRequest *req, KERNEL_NS::HttpResponse *res)
{
    res->SetStatusCode(KERNEL_NS::HttpStatusCode::Ok);
    res->SetHeader("Content-Type", "application/json");
    res->SetBody("{\"msg\":\"hello\"}");
});
server->RegisterHandler("POST", "/echo", [](KERNEL_NS::HttpRequest *req, KERNEL_NS::HttpResponse *res)
{
    res->SetBody(req->GetBody());
});
// server->SetNotFoundHandler([](req, res){ ... }); // 可选: 默认404

// chunked整包发送(自动Transfer-Encoding: chunked编码)
server->RegisterHandler("GET", "/download", [](req, res)
{
    res->SetChunkedSend(true);
    res->SetBody(largeContent);
});

// chunked流式发送(首部发出后按需追加chunk, 适合SSE/流式下载)
server->RegisterHandler("GET", "/stream", [server](req, res)
{
    res->SetChunkedSend(true, true);            // 本次仅发送响应首部
    const UInt64 sid = req->GetSessionId();
    // 后续在合适的时机(如定时器/事件回调)调用:
    // server->SendChunk(sid, data, len, false);   // 追加chunk
    // server->SendChunk(sid, NULL, 0, true);      // 结束块(请求为close时自动关闭会话)
});

// 3. 生命周期
server->OnCreated();
server->Init();
server->Start();
// server->GetListenErrCode() == Status::Success 表示监听成功(Start后异步回执)

// 4. 关闭
server->WillClose();
server->Close();
server->Release();
```

请求对象常用接口：

```cpp
req->GetMethod();                 // HttpMethodType
req->GetRawMethod();              // "GET"/"POST"...
req->GetPath();                   // "/hello"
req->GetQuery();                  // "a=1&b=2"
req->GetHeader("content-type");   // header名大小写不敏感(内部小写存储)
req->GetBody();                   // body(chunked自动组包)
req->IsKeepAlive();
```

要点：

- handler 在 server 内部**独立业务派发线程**执行（不阻塞网络 IO 线程），`req`/`res` 生命周期仅限回调内，禁止持有指针
- 响应自动补 `Content-Length`；`Connection` 跟随请求（HTTP/1.1 默认 keep-alive，请求带 `Connection: close` 时应答后自动关闭会话）
- 会话上下文可经 `req->GetSessionId()` 获取

## HttpClient 使用

```cpp
auto *client = KERNEL_NS::HttpClientFactory::FactoryCreate()->Create()->CastTo<KERNEL_NS::HttpClient>();
// client->SetTlsVerifyPeer(true, "ca.pem");  // 可选: 校验https对端证书(默认不校验)
client->OnCreated();
client->Init();
client->Start();

// ---- 同步接口(阻塞当前线程, 禁止在client派发线程内调用) ----
KERNEL_NS::HttpResponse res;
Int32 err = client->SyncGet("http://127.0.0.1:8080/hello", res);
err = client->SyncPost("https://example.com/echo", "{\"a\":1}", "application/json", res);
// res.GetStatusCode() / res.GetReason() / res.GetBody() / res.GetHeader(...)

// ---- 异步接口(任意线程可调用, 回调在client内部派发线程执行) ----
client->Get("https://example.com/api", [](Int32 errCode, KERNEL_NS::HttpResponse *res)
{
    if (errCode == Status::Success && res->GetStatusCode() == KERNEL_NS::HttpStatusCode::Ok)
    {
        // res仅回调内有效, 需要留存请 res->CopyFrom 或响应.CopyFrom(*res)
    }
});
client->Post(url, body, "text/plain", cb);
client->SendRequest(KERNEL_NS::HttpMethodType::Put, url, body, contentType, cb);

client->WillClose();
client->Close();
client->Release();
```

要点：

- 同一 client 实例可同时请求 `http://` 与 `https://`（TLS 按会话自动启用），URL 支持域名（自动 DNS 解析）与端口省略（http:80/https:443）
- 当前为**短连接模型**：每请求一个连接（`Connection: close`），连接复用（连接池）后续版本提供
- 异步回调中跨线程留存响应用 `HttpResponse::CopyFrom`（深拷贝首行/header/body）

## 宿主组件中使用（推荐业务层方式）

```cpp
void MyHost::OnRegisterComps()
{
    RegisterComp<HttpServerFactory>();   // 按接口获取, 依赖接口而非实现
}

Int32 MyHost::_OnHostInit()
{
    auto *http = GetComp<IHttpServer>();
    http->SetListen("0.0.0.0", 8080);
    http->RegisterHandler("GET", "/health", [](req, res){ res->SetBody("ok"); });
    return Status::Success;
}
```

## 错误码（status.h Http 段 [6300, 6399]）

| 错误码 | 含义 |
|--------|------|
| `Http_HeaderTooLarge` (6300) | 头部超过 64KB |
| `Http_BodyTooLarge` (6301) | body 超过上限(默认16MB, 可调) |
| `Http_BadMessage` (6302) | 报文格式错误 |
| `Http_UnsupportedBody` (6303) | 响应无 Content-Length 且非 chunked |
| `Http_TlsFail` (6304) | TLS 握手/加解密失败 |
| `Http_BadUrl` (6305) | URL 格式错误 |
| `Http_ConnectFail` (6306) | 连接失败 |
| `Http_SessionClosed` (6307) | 请求未完成时会话已关闭 |

## 限制与注意事项

- 协议：HTTP/1.1（不支持 HTTP/2、WebSocket）
- 服务端路由为精确匹配（无路径参数/通配符），复杂路由在 handler 内自行分发
- keep-alive 仅在客户端显式 `Connection: keep-alive` 时保持（当前客户端固定短连接）；服务端遵循请求头
- `Expect: 100-continue`：客户端发送带该 header 的请求时服务端自动应答 `100 Continue`；客户端收到 1xx 中间响应自动跳过
- chunked：收方向自动解码组包；发方向整包用 `res->SetChunkedSend(true)`，流式用 `SetChunkedSend(true, true)` + `HttpServer::SendChunk`（注意 chunk 必须在响应首部发出后追加）
- 日志一律 `CLOG_XXX` 宏；组件内部线程已重写 `DefaultMaskReady`，ready 在 Start 完成后标记
- 可运行示例见 `build/http_test/main.cpp` 与 `testsuit/testsuit/testinst/TestHttp.cpp`（回环测试：GET/POST/404/异步/并发/HTTPS/chunked/100-continue 全链路）
