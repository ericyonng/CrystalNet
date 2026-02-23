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

#include <pch.h>
#include <kernel/comp/Coroutines/CoHandle.h>
#include <kernel/comp/Log/log.h>
#include <kernel/comp/Coroutines/CoTools.h>
#include <kernel/comp/Utils/TlsUtil.h>

#ifdef CRYSTAL_NET_CPP20

KERNEL_BEGIN

KernelHandle::KernelHandle() noexcept
:_handleId(_GetMaxHandleId().fetch_add(1, std::memory_order_acq_rel) + 1)
{
    KERNEL_NS::TlsUtil::GetTlsCoDict()->AddCo(_handleId, this);
}

KernelHandle::~KernelHandle()
{
    KERNEL_NS::TlsUtil::GetTlsCoDict()->RemoveCo(_handleId);
}

const std::source_location& CoHandle::_GetFrameInfo() const 
{
    static const std::source_location frame_info = std::source_location::current();
    return frame_info;
}

void CoHandle::Schedule() 
{
    if (_state == KERNEL_NS::KernelHandle::UNSCHEDULED)
    {
        // TODO: 注册
        SetState(KernelHandle::SCHEDULED);

        auto handleId = GetHandleId();
        PostAsyncTask([handleId](){
            auto handle = KERNEL_NS::TlsUtil::GetTlsCoDict()->GetHandle(handleId);
            if(UNLIKELY(!handle))
                return;
            
            handle->Run(KernelHandle::UNSCHEDULED);
        });
    }
}

void CoHandle::Cancel() 
{
    if (_state != KERNEL_NS::KernelHandle::UNSCHEDULED)
    {
        // 即使此时已经在Poller的队列中, 在Run的时候也会判断状态
        SetState(KernelHandle::UNSCHEDULED);
    }
}

void CoHandle::DumpBacktrace(Int32 depth) const
{
    KERNEL_NS::LibString content;
    DumpBacktrace(depth, content);
}

void CoHandle::GetBacktrace(KERNEL_NS::LibString &content, Int32 depth) const
{
    content.AppendFormat("#%d CoHandleId:%llu, %s\n", depth, GetHandleId(), FrameName().c_str());
}


void CoHandle::DumpBacktrace(Int32 depth, KERNEL_NS::LibString &content) const
{
    content.AppendFormat("#%d CoHandleId:%llu, %s\n", depth, GetHandleId(), FrameName().c_str());
}

void CoHandle::DumpBacktraceFinish(const KERNEL_NS::LibString &content) const
{
    g_Log->Error(LOGFMT_OBJ_TAG("CoTask Backtrace:\n%s"), content.c_str());
}

KERNEL_END

#endif