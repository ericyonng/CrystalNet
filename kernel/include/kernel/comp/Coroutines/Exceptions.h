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
 * Date: 2024-10-26 17:23:10
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_EXCEPTIONS_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_EXCEPTIONS_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <exception>

KERNEL_BEGIN

struct KERNEL_EXPORT TimeoutError : std::exception 
{
    // 超时异常
    [[nodiscard]] const char* what() const noexcept override 
    {
        return "TimeoutError";
    }
};

struct KERNEL_EXPORT NoResultError : std::exception 
{
    // 结果还没设置
    [[nodiscard]] const char* what() const noexcept override 
    {
        return "result is unset";
    }
};

struct KERNEL_EXPORT InvalidFuture : std::exception 
{
    // future是无效的
    [[nodiscard]] const char* what() const noexcept override 
    {
        return "future is invalid";
    }
};

KERNEL_END

#endif
