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
 * Date: 2024-10-21 01:31:10
 * Author: Eric Yonng
 * Description: 
*/


#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_COTASK_PARAM_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_COTASK_PARAM_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/LibTime.h>
#include <kernel/comp/SmartPtr.h>

KERNEL_BEGIN

struct  CoHandle;

class LibTimer;

// 只要在co_await 时候设置参数,参数都会生效
struct KERNEL_EXPORT CoTaskParam
{
    POOL_CREATE_OBJ_DEFAULT(CoTaskParam);

    ~CoTaskParam();
    
    void Release();

    // 错误码
    Int32 _errCode = 0;

    // 超时时长
    LibTime _endTime;

    // 超时定时器
    LibTimer *_timeout = NULL;

    // 协程句柄
    CoHandle *_handle = NULL;
};

struct KERNEL_EXPORT TaskParamRefWrapper
{
    POOL_CREATE_OBJ_DEFAULT(TaskParamRefWrapper);

    void Release();

    SmartPtr<CoTaskParam, AutoDelMethods::Release> _params;
};

KERNEL_END

#endif
