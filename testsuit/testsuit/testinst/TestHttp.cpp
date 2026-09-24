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
 * Description: Http/HttpS组件使用示例与回环测试(服务端/客户端/GET/POST/404/异步/并发/HTTPS)
*/

#include <pch.h>
#include "TestHttp.h"

#include <kernel/comp/Http/Http.h>
#include <kernel/comp/Utils/SystemUtil.h>

#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

#include <cstdio>
#include <atomic>
#include <map>

namespace
{
    static Int32 s_failCount = 0;

    static void CheckTrue(bool cond, const char *name)
    {
        if (cond)
        {
            printf("[PASS] %s\n", name);
        }
        else
        {
            printf("[FAIL] %s\n", name);
            ++s_failCount;
        }
    }

    // 生成自签证书(仅测试用), 已存在则跳过
    static bool GenSelfSignedCert(const char *certFile, const char *keyFile)
    {
        FILE *fp = fopen(certFile, "rb");
        if (fp)
        {
            fclose(fp);
            return true;
        }

        // openssl 1.1兼容API生成RSA密钥对
        RSA *rsa = RSA_generate_key(2048, RSA_F4, NULL, NULL);
        if (!rsa)
            return false;

        EVP_PKEY *pkey = EVP_PKEY_new();
        if (!pkey || (EVP_PKEY_assign_RSA(pkey, rsa) != 1))
        {
            RSA_free(rsa);
            if (pkey)
                EVP_PKEY_free(pkey);
            return false;
        }

        X509 *x509 = X509_new();
        X509_set_version(x509, 2);
        ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);
        X509_gmtime_adj(X509_getm_notBefore(x509), 0);
        X509_gmtime_adj(X509_getm_notAfter(x509), 365L * 24 * 3600);
        X509_set_pubkey(x509, pkey);

        X509_NAME *name = X509_get_subject_name(x509);
        X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, reinterpret_cast<const unsigned char *>("localhost"), -1, -1, 0);
        X509_set_issuer_name(x509, name);
        X509_sign(x509, pkey, EVP_sha256());

        bool ok = false;
        FILE *cf = fopen(certFile, "wb");
        if (cf)
        {
            PEM_write_X509(cf, x509);
            fclose(cf);

            FILE *kf = fopen(keyFile, "wb");
            if (kf)
            {
                PEM_write_PrivateKey(kf, pkey, NULL, NULL, 0, NULL, NULL);
                fclose(kf);
                ok = true;
            }
        }

        X509_free(x509);
        EVP_PKEY_free(pkey);
        return ok;
    }

    // 等待server监听回执
    static bool WaitListenSuc(KERNEL_NS::HttpServer *server)
    {
        for (Int32 idx = 0; idx < 100; ++idx)
        {
            if (server->GetListenErrCode() != Status::Error)
                break;

            KERNEL_NS::SystemUtil::ThreadSleep(100);
        }

        return server->GetListenErrCode() == Status::Success;
    }
}

void TestHttp::Run()
{
    printf("========== http component test begin ==========\n");
    s_failCount = 0;

    // ============ http server ============
    auto *server = KERNEL_NS::HttpServerFactory::FactoryCreate()->Create()->CastTo<KERNEL_NS::HttpServer>();
    server->SetListen("127.0.0.1", 18080);
    server->RegisterHandler("GET", "/hello", [](KERNEL_NS::HttpRequest *req, KERNEL_NS::HttpResponse *res)
    {
        res->SetStatusCode(KERNEL_NS::HttpStatusCode::Ok);
        res->SetHeader("Content-Type", "text/plain; charset=utf-8");
        res->SetBody("hello world");
    });
    server->RegisterHandler("POST", "/echo", [](KERNEL_NS::HttpRequest *req, KERNEL_NS::HttpResponse *res)
    {
        res->SetStatusCode(KERNEL_NS::HttpStatusCode::Ok);
        res->SetHeader("Content-Type", "text/plain");
        res->SetBody(req->GetBody());
    });

    CheckTrue(server->OnCreated() == Status::Success, "server OnCreated");
    CheckTrue(server->Init() == Status::Success, "server Init");
    CheckTrue(server->Start() == Status::Success, "server Start");
    CheckTrue(WaitListenSuc(server), "server listen suc");

    // ============ http client ============
    auto *client = KERNEL_NS::HttpClientFactory::FactoryCreate()->Create()->CastTo<KERNEL_NS::HttpClient>();
    CheckTrue(client->OnCreated() == Status::Success, "client OnCreated");
    CheckTrue(client->Init() == Status::Success, "client Init");
    CheckTrue(client->Start() == Status::Success, "client Start");

    // ============ sync get ============
    {
        KERNEL_NS::HttpResponse res;
        const Int32 err = client->SyncGet("http://127.0.0.1:18080/hello", res);
        CheckTrue(err == Status::Success, "sync get request suc");
        CheckTrue(res.GetStatusCode() == KERNEL_NS::HttpStatusCode::Ok, "sync get status 200");
        CheckTrue(res.GetBody() == "hello world", "sync get body");
    }

    // ============ sync post ============
    {
        KERNEL_NS::HttpResponse res;
        const Int32 err = client->SyncPost("http://127.0.0.1:18080/echo", "post-body-12345", "text/plain", res);
        CheckTrue(err == Status::Success, "sync post request suc");
        CheckTrue(res.GetStatusCode() == KERNEL_NS::HttpStatusCode::Ok, "sync post status 200");
        CheckTrue(res.GetBody() == "post-body-12345", "sync post echo body");
    }

    // ============ 404 ============
    {
        KERNEL_NS::HttpResponse res;
        const Int32 err = client->SyncGet("http://127.0.0.1:18080/not_exist_path", res);
        CheckTrue(err == Status::Success, "sync get 404 request suc");
        CheckTrue(res.GetStatusCode() == KERNEL_NS::HttpStatusCode::NotFound, "sync get status 404");
    }

    // ============ async get ============
    {
        std::atomic<bool> done{false};
        Int32 asyncErr = Status::Error;
        Int32 asyncStatus = 0;
        KERNEL_NS::LibString asyncBody;
        const Int32 err = client->Get("http://127.0.0.1:18080/hello", [&](Int32 cbErr, KERNEL_NS::HttpResponse *res)
        {
            asyncErr = cbErr;
            if (res)
            {
                asyncStatus = res->GetStatusCode();
                asyncBody = res->GetBody();
            }
            done.store(true);
        });
        CheckTrue(err == Status::Success, "async get post suc");

        for (Int32 idx = 0; idx < 100 && !done.load(); ++idx)
            KERNEL_NS::SystemUtil::ThreadSleep(100);

        CheckTrue(done.load(), "async get callback");
        CheckTrue(asyncErr == Status::Success, "async get err suc");
        CheckTrue(asyncStatus == KERNEL_NS::HttpStatusCode::Ok, "async get status 200");
        CheckTrue(asyncBody == "hello world", "async get body");
    }

    // ============ 并发100 ============
    {
        const Int32 concurrent = 100;
        std::atomic<Int32> finished{0};
        std::atomic<Int32> succeed{0};
        KERNEL_NS::SpinLock failLck;
        std::map<Int32, Int32> failErrDict;
        std::atomic<Int32> badContent{0};
        for (Int32 idx = 0; idx < concurrent; ++idx)
        {
            client->Get("http://127.0.0.1:18080/hello", [&](Int32 cbErr, KERNEL_NS::HttpResponse *res)
            {
                if (cbErr == Status::Success && res && res->GetStatusCode() == KERNEL_NS::HttpStatusCode::Ok && res->GetBody() == "hello world")
                {
                    succeed.fetch_add(1);
                }
                else
                {
                    if (cbErr != Status::Success)
                    {
                        failLck.Lock();
                        ++failErrDict[cbErr];
                        failLck.Unlock();
                    }
                    else
                    {
                        badContent.fetch_add(1);
                    }
                }

                finished.fetch_add(1);
            });
        }

        for (Int32 idx = 0; idx < 300 && finished.load() < concurrent; ++idx)
            KERNEL_NS::SystemUtil::ThreadSleep(100);

        CheckTrue(finished.load() == concurrent, "concurrent 100 all finished");
        CheckTrue(succeed.load() == concurrent, "concurrent 100 all succeed");
        printf("concurrent detail: succeed:%d, badContent:%d, failErrs:", succeed.load(), badContent.load());
        for (auto &kv : failErrDict)
            printf("[%d]x%d ", kv.first, kv.second);
        printf("\n");
    }

    // ============ chunked整包发送 ============
    server->RegisterHandler("GET", "/chunked", [](KERNEL_NS::HttpRequest *req, KERNEL_NS::HttpResponse *res)
    {
        res->SetChunkedSend(true);
        res->SetBody("chunked-body-content");
    });
    {
        KERNEL_NS::HttpResponse res;
        const Int32 err = client->SyncGet("http://127.0.0.1:18080/chunked", res);
        CheckTrue(err == Status::Success, "chunked get request suc");
        CheckTrue(res.GetStatusCode() == KERNEL_NS::HttpStatusCode::Ok, "chunked get status 200");
        CheckTrue(res.GetBody() == "chunked-body-content", "chunked get body");
    }

    // ============ chunked流式发送(SendChunk) ============
    std::atomic<UInt64> streamSessionId{0};
    server->RegisterHandler("GET", "/stream", [&](KERNEL_NS::HttpRequest *req, KERNEL_NS::HttpResponse *res)
    {
        // 流式: 仅发送响应首部, chunk由业务后续追加
        res->SetChunkedSend(true, true);
        streamSessionId.store(req->GetSessionId());
    });
    {
        std::atomic<bool> streamDone{false};
        Int32 streamErr = Status::Error;
        KERNEL_NS::LibString streamBody;
        const Int32 err = client->Get("http://127.0.0.1:18080/stream", [&](Int32 cbErr, KERNEL_NS::HttpResponse *res)
        {
            streamErr = cbErr;
            if (res)
                streamBody = res->GetBody();

            streamDone.store(true);
        });
        CheckTrue(err == Status::Success, "stream get post suc");

        // 等待首部发出后流式追加chunk
        for (Int32 idx = 0; idx < 100 && (streamSessionId.load() == 0); ++idx)
            KERNEL_NS::SystemUtil::ThreadSleep(10);
        CheckTrue(streamSessionId.load() != 0, "stream session established");
        KERNEL_NS::SystemUtil::ThreadSleep(100);

        const UInt64 sid = streamSessionId.load();
        CheckTrue(server->SendChunk(sid, reinterpret_cast<const Byte8 *>("hello-"), 6, false) == Status::Success, "stream send chunk1");
        CheckTrue(server->SendChunk(sid, reinterpret_cast<const Byte8 *>("stream"), 6, false) == Status::Success, "stream send chunk2");
        CheckTrue(server->SendChunk(sid, NULL, 0, true) == Status::Success, "stream send last chunk");

        for (Int32 idx = 0; idx < 100 && !streamDone.load(); ++idx)
            KERNEL_NS::SystemUtil::ThreadSleep(100);

        CheckTrue(streamDone.load(), "stream callback");
        CheckTrue(streamErr == Status::Success, "stream err suc");
        CheckTrue(streamBody == "hello-stream", "stream body assembled");
    }

    // ============ 100-continue(原生socket模拟标准客户端) ============
    {
        KERNEL_NS::LibString errInfo;
        bool connOk = false;
        bool got100 = false;
        bool gotFinal = false;
        KERNEL_NS::LibString finalBody;

    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
        SOCKET sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock != INVALID_SOCKET)
        {
            sockaddr_in addr;
            ::memset(&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_port = ::htons(18080);
            addr.sin_addr.s_addr = ::inet_addr("127.0.0.1");

            DWORD recvTimeoutMs = 5000;
            ::setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char *>(&recvTimeoutMs), sizeof(recvTimeoutMs));

            connOk = (::connect(sock, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) == 0);
            if (connOk)
            {
                // 1.先发送带Expect: 100-continue的请求头(不发body)
                const Byte8 header[] = "POST /echo HTTP/1.1\r\nHost: 127.0.0.1:18080\r\nContent-Length: 11\r\nExpect: 100-continue\r\n\r\n";
                ::send(sock, header, static_cast<int>(sizeof(header) - 1), 0);

                // 2.读取100 Continue中间响应
                char buf[4096] = {0};
                Int32 total = 0;
                for (Int32 idx = 0; idx < 20; ++idx)
                {
                    const int n = ::recv(sock, buf + total, static_cast<int>(sizeof(buf) - 1 - total), 0);
                    if (n <= 0)
                        break;

                    total += n;
                    buf[total] = 0;
                    if (::strstr(buf, "100 Continue"))
                    {
                        got100 = true;
                        break;
                    }
                }

                // 3.发送body
                const Byte8 body[] = "hello-100-c";
                ::send(sock, body, static_cast<int>(sizeof(body) - 1), 0);

                // 4.读取最终响应
                total = 0;
                ::memset(buf, 0, sizeof(buf));
                KERNEL_NS::LibString all;
                for (Int32 idx = 0; idx < 50; ++idx)
                {
                    const int n = ::recv(sock, buf, static_cast<int>(sizeof(buf) - 1), 0);
                    if (n <= 0)
                        break;

                    all.append(buf, static_cast<UInt64>(n));
                    if (all.find("hello-100-c") != KERNEL_NS::LibString::npos)
                    {
                        gotFinal = (::strstr(all.c_str(), "200 OK") != NULL) || (all.find("200") != KERNEL_NS::LibString::npos);
                        finalBody = "hello-100-c";
                        break;
                    }
                }
            }

            ::closesocket(sock);
        }
    #endif

        CheckTrue(connOk, "100-continue raw socket connect");
        CheckTrue(got100, "100-continue server answered 100 Continue");
        CheckTrue(gotFinal, "100-continue final response 200 with echo body");
    }

    // ============ https server(自签证书) ============
    CheckTrue(GenSelfSignedCert("http_test_server.crt", "http_test_server.key"), "gen self signed cert");

    auto *tlsServer = KERNEL_NS::HttpServerFactory::FactoryCreate()->Create()->CastTo<KERNEL_NS::HttpServer>();
    tlsServer->SetListen("127.0.0.1", 18443);
    tlsServer->SetTlsCert("http_test_server.crt", "http_test_server.key");
    tlsServer->RegisterHandler("GET", "/hello", [](KERNEL_NS::HttpRequest *req, KERNEL_NS::HttpResponse *res)
    {
        res->SetStatusCode(KERNEL_NS::HttpStatusCode::Ok);
        res->SetBody("hello tls world");
    });
    tlsServer->RegisterHandler("POST", "/echo", [](KERNEL_NS::HttpRequest *req, KERNEL_NS::HttpResponse *res)
    {
        res->SetStatusCode(KERNEL_NS::HttpStatusCode::Ok);
        res->SetBody(req->GetBody());
    });

    CheckTrue(tlsServer->OnCreated() == Status::Success, "https server OnCreated");
    CheckTrue(tlsServer->Init() == Status::Success, "https server Init");
    CheckTrue(tlsServer->Start() == Status::Success, "https server Start");
    CheckTrue(WaitListenSuc(tlsServer), "https server listen suc");

    // https sync get(复用同一client, http/https会话独立tls)
    {
        KERNEL_NS::HttpResponse res;
        const Int32 err = client->SyncGet("https://127.0.0.1:18443/hello", res);
        CheckTrue(err == Status::Success, "https sync get request suc");
        CheckTrue(res.GetStatusCode() == KERNEL_NS::HttpStatusCode::Ok, "https sync get status 200");
        CheckTrue(res.GetBody() == "hello tls world", "https sync get body");
    }

    // https sync post
    {
        KERNEL_NS::HttpResponse res;
        const Int32 err = client->SyncPost("https://127.0.0.1:18443/echo", "tls-post-body-678", "text/plain", res);
        CheckTrue(err == Status::Success, "https sync post request suc");
        CheckTrue(res.GetStatusCode() == KERNEL_NS::HttpStatusCode::Ok, "https sync post status 200");
        CheckTrue(res.GetBody() == "tls-post-body-678", "https sync post echo body");
    }

    // https 并发50
    {
        const Int32 concurrent = 50;
        std::atomic<Int32> finished{0};
        std::atomic<Int32> succeed{0};
        for (Int32 idx = 0; idx < concurrent; ++idx)
        {
            client->Get("https://127.0.0.1:18443/hello", [&](Int32 cbErr, KERNEL_NS::HttpResponse *res)
            {
                if (cbErr == Status::Success && res && res->GetStatusCode() == KERNEL_NS::HttpStatusCode::Ok && res->GetBody() == "hello tls world")
                    succeed.fetch_add(1);

                finished.fetch_add(1);
            });
        }

        for (Int32 idx = 0; idx < 300 && finished.load() < concurrent; ++idx)
            KERNEL_NS::SystemUtil::ThreadSleep(100);

        CheckTrue(finished.load() == concurrent, "https concurrent 50 all finished");
        CheckTrue(succeed.load() == concurrent, "https concurrent 50 all succeed");
        printf("https concurrent detail: succeed:%d/%d\n", succeed.load(), concurrent);
    }

    // ============ destroy ============
    tlsServer->WillClose();
    tlsServer->Close();
    tlsServer->Release();

    client->WillClose();
    client->Close();
    client->Release();

    server->WillClose();
    server->Close();
    server->Release();

    printf("========== http component test finish, fail count:%d ==========\n", s_failCount);
}
