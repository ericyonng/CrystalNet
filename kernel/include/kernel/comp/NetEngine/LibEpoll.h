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
 * 
 * Author: Eric Yonng
 * Date: 2021-03-22 17:00:09
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_LIB_EPOLL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_LIB_EPOLL_H__

#pragma once

#include <kernel/kernel_inc.h>

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS

#include <kernel/comp/NetEngine/Defs/EpollDef.h>

struct epoll_event;

KERNEL_BEGIN

class KERNEL_EXPORT LibEpoll
{
public:
    LibEpoll();
    virtual ~LibEpoll();

public:
    Int32 Create();
    Int32 Destroy();
    bool IsDestroy() const;
    // opType:EPOLL_CTL_ADD, EPOLL_CTL_MOD, EPOLL_CTL_DEL
    Int32 Ctl(Int32 opType, Int32 fd, epoll_event *ev);
    // Int32 AddEvent(Int32 newFd, UInt32 eventFlags = __ADD_LIB_EPOLL_EVENT_FLAGS_DEF__);
    Int32 AddEvent(Int32 newFd, UInt64 sessionId, UInt32 eventFlags);
    Int32 AddEvent(Int32 newFd, UInt32 eventFlags);
    Int32 ModifyEvent(Int32 modFd, UInt64 sessionId, UInt32 eventFlags);
    Int32 DelEvent(Int32 delFd, UInt64 sessionId, UInt32 eventFlagsToDel = __DEL_LIB_EPOLL_EVENT_FLAGS_DEF__);
    // 0??????????????????,-1??????????????????,>0?????????????????? Wait ????????????epoll_wait ?????????????????????status???????????????
    Int32 Wait(Int32 millisecondsToWait = EpollWaitTimeOutType::BlockNoTimeOut);
    epoll_event *GetEvs();
    Int32 GetEvFd()const;

private:
    // epoll_event ???data????????????????????????
    epoll_event *_evPtr;
    epoll_event _ev[__LIB_MAX_EVENT_NUM_TO_HANDLE_ONCE_DEF__];      // epoll????????????????????????    ????????????????????????
    Int32 _evFd;                                                    // epoll???????????????
};


inline bool LibEpoll::IsDestroy() const
{
    return (Int32)(_evFd) == EpollDefs::InvalidFd;
}

inline epoll_event *LibEpoll::GetEvs()
{
    return _evPtr;
}

inline Int32 LibEpoll::GetEvFd()const
{
    return _evFd;
}

KERNEL_END

#endif

#endif