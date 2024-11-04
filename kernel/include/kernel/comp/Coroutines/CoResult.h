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
 * Date: 2024-10-26 20:27:13
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_CORESULT_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_CORESULT_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <kernel/comp/Coroutines/Exceptions.h>
#include <variant>
#include <optional>
#include <kernel/comp/Coroutines/CoTaskParam.h>

KERNEL_BEGIN

template<typename T>
struct CoResult 
{
    // 在获取结果时候需要判断HasValue
    constexpr bool HasValue() const noexcept 
    {
        return std::get_if<std::monostate>(&_result) == nullptr;
    }

    template<typename R>
    constexpr void SetValue(R&& value) noexcept 
    {
        #if CRYSTAL_TARGET_PLATFORM_WINDOWS
         _result.emplace<T>(std::forward<R>(value));
        #else
         _result.template emplace<T>(std::forward<R>(value));
        #endif
    }

    // promise_type 的接口
    template<typename R>
    constexpr void return_value(R&& value) noexcept 
    {
        SetValue(std::forward<T>(value));
    }

    // 有没有异常
    bool HasError() const
    {
        if(std::get_if<std::exception_ptr>(&_result))
            return true;

        return false;
    }

    void RethrowException()
    {
        if (auto exception = std::get_if<std::exception_ptr>(&_result)) 
        {
            std::rethrow_exception(*exception);
        }
    }
    
    std::exception_ptr GetException()
    {
        if (auto exception = std::get_if<std::exception_ptr>(&_result)) 
        {
            return *exception;
        }

        return std::exception_ptr();
    }

    // 外部需要考虑异常的情况
    constexpr T GetResult() & 
    {
        // 如果有异常, 则抛异常
        if (auto exception = std::get_if<std::exception_ptr>(&_result)) 
        {
            std::rethrow_exception(*exception);
        }
        // 如果有值
        if (auto res = std::get_if<T>(&_result)) 
        {
            return *res;
        }

        // 没有异常也没有值，只能抛异常了, TODO:也可以返回一个空的值
        throw NoResultError{};
    }

    // 外部需要考虑异常的情况
    constexpr T GetResult() && 
    {
        // 有异常
        if (auto exception = std::get_if<std::exception_ptr>(&_result)) 
        {
            std::rethrow_exception(*exception);
        }

        // 有结果
        if (auto res = std::get_if<T>(&_result)) 
        {
            return std::move(*res);
        }

        // 其他情况抛异常
        throw NoResultError{};
    }

    void set_exception(std::exception_ptr exception) noexcept { _result = exception; }

    virtual void unhandled_exception() { _result = std::current_exception(); }

    virtual SmartPtr<CoTaskParam, AutoDelMethods::CustomDelete> &GetParam() = 0;

protected:
    // variant有三个类型的可能值:monostate, T, std::exception_ptr, 使用std::get按照类型获取值, variant 内部实际上是union
    std::variant<std::monostate, T, std::exception_ptr> _result;
};

template<>
struct KERNEL_EXPORT CoResult<void> 
{
    constexpr bool HasValue() const noexcept 
    {
        return _result.has_value();
    }

    // for: promise_type
    void return_void() noexcept 
    {
        _result.emplace(nullptr);
    }

    void GetResult() 
    {
        // 有异常则抛异常
        if (_result.has_value() && *_result != nullptr)
            std::rethrow_exception(*_result);
    }

    // 有没有异常
    bool HasError() const
    {
        if (_result.has_value() && *_result != nullptr)
            return true;

        return false;
    }

    void RethrowException()
    {
        if (_result.has_value() && *_result != nullptr)
            std::rethrow_exception(*_result);
    }

    std::exception_ptr GetException()
    {
        if (_result.has_value() && *_result != nullptr)
            return *_result;

        return std::exception_ptr();
    }

    // for: promise_type
    void set_exception(std::exception_ptr exception) noexcept { _result = exception; }
    virtual void unhandled_exception() { _result = std::current_exception(); }
    virtual SmartPtr<CoTaskParam, AutoDelMethods::CustomDelete> &GetParam() = 0;

private:
    std::optional<std::exception_ptr> _result;
};

KERNEL_END

#endif
