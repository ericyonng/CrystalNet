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
 * Date: 2021-01-17 22:07:38
 * Author: Eric Yonng
 * Description: 提供运行时类型识别缓冲,线程id
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TLS_TLS_DEFAULT_OBJ_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TLS_TLS_DEFAULT_OBJ_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/Tls/ITlsObj.h>
#include <kernel/comp/Tls/Defs.h>

KERNEL_BEGIN

class LibThread;
class LibThreadPool;
class TimerMgr;
class Poller;

class KERNEL_EXPORT TlsDefaultObj : public ITlsObj
{
public:
    TlsDefaultObj();
    ~TlsDefaultObj(){}

public:
    virtual const char *GetObjTypeName(){ return _objTypeName.c_str(); }

public:
    const std::string _objTypeName;

    // TODO:... 添加线程域内全局变量

    // 类型识别缓冲
    char rtti[TlsDefs::LIB_RTTI_BUF_SIZE];
    // 线程id
    UInt64 _threadId;
    // 线程
    LibThread *_thread;
    // 线程池
    LibThreadPool *_threadPool;
    // 定时器管理
    TimerMgr *_pollerTimerMgr;
    // poller
    Poller *_poller;
    // 雪花算法信息 TODO:tlsutil给一个接口方便获取uid
    // SnowflakeInfo _snowFlakeInfo;
};

KERNEL_END
#endif
