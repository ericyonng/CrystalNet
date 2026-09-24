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
 * Description: http组件公共定义: 方法/状态码/opcode/协议栈模式等
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_HTTP_HTTP_DEFS_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_HTTP_HTTP_DEFS_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

// http方法
class KERNEL_EXPORT HttpMethodType
{
public:
    enum Type : Int32
    {
        Unknown = 0,
        Get,
        Post,
        Head,
        Put,
        Delete,
        Options,
        Patch,
    };

    static const Byte8 *ToString(Int32 method)
    {
        switch (method)
        {
        case Get: return "GET";
        case Post: return "POST";
        case Head: return "HEAD";
        case Put: return "PUT";
        case Delete: return "DELETE";
        case Options: return "OPTIONS";
        case Patch: return "PATCH";
        default:
            break;
        }

        return "UNKNOWN";
    }

    static Int32 FromString(const Byte8 *method, size_t len)
    {
        if(len == 3)
        {
            if(::strncmp(method, "GET", 3) == 0) return Get;
            if(::strncmp(method, "PUT", 3) == 0) return Put;
        }
        else if(len == 4)
        {
            if(::strncmp(method, "POST", 4) == 0) return Post;
            if(::strncmp(method, "HEAD", 4) == 0) return Head;
        }
        else if(len == 6)
        {
            if(::strncmp(method, "DELETE", 6) == 0) return Delete;
        }
        else if(len == 7)
        {
            if(::strncmp(method, "OPTIONS", 7) == 0) return Options;
        }
        else if(len == 5)
        {
            if(::strncmp(method, "PATCH", 5) == 0) return Patch;
        }

        return Unknown;
    }
};

// http状态码
class KERNEL_EXPORT HttpStatusCode
{
public:
    enum Type : Int32
    {
        Continue = 100,
        SwitchingProtocols = 101,

        Ok = 200,
        Created = 201,
        Accepted = 202,
        NoContent = 204,
        PartialContent = 206,

        MovedPermanently = 301,
        Found = 302,
        NotModified = 304,
        TemporaryRedirect = 307,

        BadRequest = 400,
        Unauthorized = 401,
        Forbidden = 403,
        NotFound = 404,
        MethodNotAllowed = 405,
        RequestTimeout = 408,
        LengthRequired = 411,
        PayloadTooLarge = 413,
        UriTooLong = 414,
        TooManyRequests = 429,

        InternalServerError = 500,
        NotImplemented = 501,
        BadGateway = 502,
        ServiceUnavailable = 503,
        GatewayTimeout = 504,
    };

    static const Byte8 *Reason(Int32 statusCode)
    {
        switch (statusCode)
        {
        case Continue: return "Continue";
        case SwitchingProtocols: return "Switching Protocols";
        case Ok: return "OK";
        case Created: return "Created";
        case Accepted: return "Accepted";
        case NoContent: return "No Content";
        case PartialContent: return "Partial Content";
        case MovedPermanently: return "Moved Permanently";
        case Found: return "Found";
        case NotModified: return "Not Modified";
        case TemporaryRedirect: return "Temporary Redirect";
        case BadRequest: return "Bad Request";
        case Unauthorized: return "Unauthorized";
        case Forbidden: return "Forbidden";
        case NotFound: return "Not Found";
        case MethodNotAllowed: return "Method Not Allowed";
        case RequestTimeout: return "Request Timeout";
        case LengthRequired: return "Length Required";
        case PayloadTooLarge: return "Payload Too Large";
        case UriTooLong: return "URI Too Long";
        case TooManyRequests: return "Too Many Requests";
        case InternalServerError: return "Internal Server Error";
        case NotImplemented: return "Not Implemented";
        case BadGateway: return "Bad Gateway";
        case ServiceUnavailable: return "Service Unavailable";
        case GatewayTimeout: return "Gateway Timeout";
        default:
            break;
        }

        return "Unknown";
    }
};

// http协议栈的packet opcode
class KERNEL_EXPORT HttpOpcode
{
public:
    enum Type : Int32
    {
        HttpMessage = 1,    // http请求/响应报文(coder: HttpRequest/HttpResponse)
        RawData = 2,        // 原始字节流(coder: HttpRawCoder), 用于tls记录等直通发送(不做tls加密)
        HttpLayerData = 3,  // http层原始字节(coder: HttpRawCoder), 用于100 Continue/chunk块等(tls会话需加密)
    };
};

// http协议栈工作模式
class KERNEL_EXPORT HttpStackMode
{
public:
    enum Type : Int32
    {
        Server = 1,     // 服务端模式: 解析请求, 发送响应
        Client = 2,     // 客户端模式: 解析响应, 发送请求
    };
};

// 协议栈类型标识(SessionOption._protocolStackType)
class KERNEL_EXPORT HttpProtocolStackType
{
public:
    enum Type : Int32
    {
        HTTP = 1001,
        HTTPS = 1002,
    };
};

// 默认限制
class KERNEL_EXPORT HttpDefaultLimit
{
public:
    enum : UInt64
    {
        MAX_HEADER_BYTES = 64 * 1024,           // 头部最大64KB
        MAX_BODY_BYTES = 16 * 1024 * 1024,      // body最大16MB
        MAX_REQUEST_LINE_BYTES = 8 * 1024,      // 请求行/状态行最大8KB
    };
};

KERNEL_END

#endif
