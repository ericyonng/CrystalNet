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

KERNEL_BEGIN

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
        get_event_loop().call_soon(*this);
    }
}

void CoHandle::Cancel() 
{
    if (_state != KERNEL_NS::KernelHandle::UNSCHEDULED)
    {
        // TODO:取消调度
        get_event_loop().cancel_handle(*this);
    }
}

KERNEL_END
