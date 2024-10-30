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
 * Date: 2024-10-26 18:02:10
 * Author: Eric Yonng
 * Description: 
*/


#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_KERNEL_FINALLY_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_KERNEL_FINALLY_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <exception>
#include <type_traits>

KERNEL_BEGIN
// 利用 FinalAction 析构来执行当前栈帧的清理工作
template<typename F>
class FinalAction 
{
public:
    FinalAction(F f) noexcept: f_(std::move(f)), invoke_(true) {}

    FinalAction(FinalAction &&other) noexcept: f_(std::move(other.f_)), invoke_(other.invoke_) 
    {
        other.invoke_ = false;
    }

    FinalAction(const FinalAction &) = delete;

    FinalAction &operator=(const FinalAction &) = delete;

    ~FinalAction() noexcept 
    {
        if (invoke_) f_();
    }

private:
    F f_;
    bool invoke_;
};

template<class F>
ALWAYS_INLINE FinalAction<F> _KernelFinally(const F &f) noexcept 
{
    return FinalAction<F>(f);
}

template<class F>
ALWAYS_INLINE FinalAction<F> _KernelFinally(F &&f) noexcept 
{
    return FinalAction<F>(std::forward<F>(f));
}

KERNEL_END

#define KERNEL_CONCAT1(a, b)       a ## b
#define KERNEL_CONCAT2(a, b)       KERNEL_CONCAT1(a, b)
#define _kernel_finally_object     concat2(_kernel_finally_object_, __COUNTER__)

// 实际使用 使用C++17 CTAD类模板自动推导, FinalAction可以不用传递类型, 由参数自动推导类模板参数
#define KERNEL_FINALLY             KERNEL_NS::FinalAction _kernel_finally_object = [&]()
#define KERNEL_FINALLY2(func)      KERNEL_NS::FinalAction _kernel_finally_object = KERNEL_NS::_KernelFinally(func)

#endif
