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
 * Description: http组件聚合入口头文件
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_HTTP_HTTP_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_HTTP_HTTP_H__

#pragma once

#include <kernel/comp/Http/HttpDefs.h>
#include <kernel/comp/Http/HttpMessage.h>
#include <kernel/comp/Http/HttpRequest.h>
#include <kernel/comp/Http/HttpResponse.h>
#include <kernel/comp/Http/HttpParser.h>
#include <kernel/comp/Http/HttpRawCoder.h>
#include <kernel/comp/Http/HttpTlsContext.h>
#include <kernel/comp/Http/HttpProtocolStack.h>
#include <kernel/comp/Http/HttpServiceProxy.h>
#include <kernel/comp/Http/IHttpServer.h>
#include <kernel/comp/Http/HttpServer.h>
#include <kernel/comp/Http/HttpServerFactory.h>
#include <kernel/comp/Http/IHttpClient.h>
#include <kernel/comp/Http/HttpClient.h>
#include <kernel/comp/Http/HttpClientFactory.h>

#endif
