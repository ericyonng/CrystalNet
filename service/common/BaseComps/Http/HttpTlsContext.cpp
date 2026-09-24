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
 * Description: tls上下文实现(openssl)
*/

#include <pch.h>
#include <service/common/BaseComps/Http/HttpTlsContext.h>
#include <kernel/comp/Log/log.h>

#include <openssl/ssl.h>
#include <openssl/err.h>


namespace
{
    static KERNEL_NS::LibString s_GetOpenSslErr()
    {
        KERNEL_NS::LibString errInfo;
        unsigned long err = 0;
        while ((err = ::ERR_get_error()) != 0)
        {
            char buf[256] = {0};
            ::ERR_error_string_n(err, buf, sizeof(buf));
            errInfo.AppendFormat("%s;", buf);
        }

        return errInfo;
    }
}


SERVICE_BEGIN



HttpTlsContext::HttpTlsContext()
    :_ctx(NULL)
    , _isServer(false)
{
}

HttpTlsContext::~HttpTlsContext()
{
    if (_ctx)
    {
        ::SSL_CTX_free(_ctx);
        _ctx = NULL;
    }
}

void HttpTlsContext::Release()
{
    HttpTlsContext::Delete_HttpTlsContext(this);
}

Int32 HttpTlsContext::InitServer(const KERNEL_NS::LibString &certFile, const KERNEL_NS::LibString &keyFile)
{
    if (UNLIKELY(_ctx))
    {
        if (g_Log)
        {
            CLOG_ERROR("tls ctx repeat init.");
        }

        return Status::Repeat;
    }

    if (UNLIKELY(certFile.empty() || keyFile.empty()))
    {
        if (g_Log)
        {
            CLOG_ERROR("cert file or key file is empty.");
        }

        return Status::ParamError;
    }

    _ctx = ::SSL_CTX_new(::TLS_server_method());
    if (UNLIKELY(!_ctx))
    {
        if (g_Log)
        {
            CLOG_ERROR("create ssl ctx fail, openssl err:%s", s_GetOpenSslErr().c_str());
        }

        return Status::Http_TlsFail;
    }

    ::SSL_CTX_set_min_proto_version(_ctx, TLS1_2_VERSION);

    if (::SSL_CTX_use_certificate_chain_file(_ctx, certFile.c_str()) != 1)
    {
        if (g_Log)
        {
            CLOG_ERROR("load cert file fail:%s, openssl err:%s", certFile.c_str(), s_GetOpenSslErr().c_str());
        }

        return Status::Http_TlsFail;
    }

    if (::SSL_CTX_use_PrivateKey_file(_ctx, keyFile.c_str(), SSL_FILETYPE_PEM) != 1)
    {
        if (g_Log)
        {
            CLOG_ERROR("load key file fail:%s, openssl err:%s", keyFile.c_str(), s_GetOpenSslErr().c_str());
        }

        return Status::Http_TlsFail;
    }

    if (::SSL_CTX_check_private_key(_ctx) != 1)
    {
        if (g_Log)
        {
            CLOG_ERROR("check private key fail, openssl err:%s", s_GetOpenSslErr().c_str());
        }

        return Status::Http_TlsFail;
    }

    _isServer = true;
    return Status::Success;
}

Int32 HttpTlsContext::InitClient(bool verifyPeer, const KERNEL_NS::LibString &caFile)
{
    if (UNLIKELY(_ctx))
    {
        if (g_Log)
        {
            CLOG_ERROR("tls ctx repeat init.");
        }

        return Status::Repeat;
    }

    _ctx = ::SSL_CTX_new(::TLS_client_method());
    if (UNLIKELY(!_ctx))
    {
        if (g_Log)
        {
            CLOG_ERROR("create ssl ctx fail, openssl err:%s", s_GetOpenSslErr().c_str());
        }

        return Status::Http_TlsFail;
    }

    ::SSL_CTX_set_min_proto_version(_ctx, TLS1_2_VERSION);

    if (verifyPeer)
    {
        ::SSL_CTX_set_verify(_ctx, SSL_VERIFY_PEER, NULL);
        if (!caFile.empty())
        {
            if (::SSL_CTX_load_verify_locations(_ctx, caFile.c_str(), NULL) != 1)
            {
                if (g_Log)
                {
                    CLOG_ERROR("load ca file fail:%s, openssl err:%s", caFile.c_str(), s_GetOpenSslErr().c_str());
                }

                return Status::Http_TlsFail;
            }
        }
        else
        {
            ::SSL_CTX_set_default_verify_paths(_ctx);
        }
    }
    else
    {
        ::SSL_CTX_set_verify(_ctx, SSL_VERIFY_NONE, NULL);
    }

    _isServer = false;
    return Status::Success;
}

KERNEL_NS::LibString HttpTlsContext::ToString() const
{
    KERNEL_NS::LibString info;
    info.AppendFormat("tls ctx:%p, is server:%d", _ctx, _isServer ? 1 : 0);
    return info;
}

SERVICE_END