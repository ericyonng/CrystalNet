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

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_COHANDLE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_COHANDLE_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <cstdint>
#include <source_location>
#include <atomic>
#include <kernel/comp/LibString.h>
#include <map>
#include <coroutine>

KERNEL_BEGIN

using HandleId = UInt64;

struct KERNEL_EXPORT KernelHandle
{
    enum State : U8 
    {
        UNSCHEDULED,
        SUSPEND,
        SCHEDULED,
    };

    KernelHandle() noexcept;

    // 获取堆栈
    virtual void GetBacktrace(KERNEL_NS::LibString &content, Int32 depth = 0) const = 0;
    virtual void Run(KernelHandle::State changeState) = 0;
    virtual void ForceAwake() = 0;

    void SetState(State state) { _state = state; }

    // co id
    HandleId GetHandleId() const { return _handleId; }

    virtual ~KernelHandle();

    bool CanSelfDestroy() const { return _canSelfDestroy; }
    void SetSelfDestory(bool canSelfDestroy)
    {
        _canSelfDestroy = canSelfDestroy;
    }

private:
    static std::atomic<HandleId> &_GetMaxHandleId()
    {
        static std::atomic<HandleId> s_MaxId {0};
        return s_MaxId;
    }

    HandleId _handleId;

protected:
    State _state {KernelHandle::UNSCHEDULED};
    bool _canSelfDestroy = false;
};

// handle maybe destroyed, using the increasing id to track the lifetime of handle.
// don't directly using a raw pointer to track coroutine lifetime,
// because a destroyed coroutine may has the same address as a new ready coroutine has created.
struct KERNEL_EXPORT KernelHandleInfo 
{
    HandleId _id { };
    KernelHandle *_handle { };
};

struct KERNEL_EXPORT CoHandle : KernelHandle 
{
    KERNEL_NS::LibString FrameName() const 
    {
        const auto& frame_info = _GetFrameInfo();
        return KERNEL_NS::LibString().AppendFormat("%s at %s:%d", frame_info.function_name(), frame_info.file_name(), frame_info.line());
    }

    virtual void GetBacktrace(KERNEL_NS::LibString &content, Int32 depth = 0) const;
    virtual void DumpBacktrace(Int32 depth = 0) const;
    virtual void DumpBacktrace(Int32 depth, KERNEL_NS::LibString &content) const;
    virtual void DumpBacktraceFinish(const KERNEL_NS::LibString &content) const;

    void Schedule();

    void Cancel();

    virtual void DestroyHandle() {}

    virtual CoHandle *GetContinuation() { return NULL; }

    virtual void ThrowErrorIfExists() {}

    virtual std::coroutine_handle<> GetCoHandle() = 0;

protected:
    virtual const std::source_location& _GetFrameInfo() const;
    
};

KERNEL_END

#endif
