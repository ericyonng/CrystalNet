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
 * Date: 2024-08-12 09:55:00
 * Author: Eric Yonng
 * Description: poller内部类型, 外部以PollerEventInternalType::MAX_INTERNAL_TYPE开始
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_POLLER_POLLER_EVENT_INTERNAL_TYPE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_POLLER_POLLER_EVENT_INTERNAL_TYPE_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>

KERNEL_BEGIN

// 内部的PollerEvent类型
class KERNEL_EXPORT PollerEventInternalType
{
public:
    enum InternalType
    {
        UNKOWN_TYPE = 0,
        // 异步任务
        AsyncTaskType = 1,

        // 执行action事件
        ActionPollerEventType = 2,

        // 空事件
        EmptyPollerEventType = 3,

        MAX_INTERNAL_TYPE,
    };
};

KERNEL_END

#endif
