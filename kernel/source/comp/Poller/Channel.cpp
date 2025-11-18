// MIT License
// 
// Copyright (c) 2020 ericyonng<120453674@qq.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// Date: 2025-11-19 00:11:30
// Author: Eric Yonng
// Description:
#include <pch.h>
#include <kernel/comp/Poller/Channel.h>
#include <kernel/comp/Poller/Poller.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(Channel);

Channel::Channel(UInt64 channelId, Poller *target, SPSCQueue<PollerEvent *> *queue)
:_channelId(channelId)
,_events(queue)
,_wakeupTarget(& target->GetConditionLocker())
,_target(target)
{
    
}

UInt64 Channel::GenChannelId()
{
    static std::atomic<UInt64> s_channelId = {0};
    return s_channelId.fetch_add(1, std::memory_order_release) + 1;
}



KERNEL_END

