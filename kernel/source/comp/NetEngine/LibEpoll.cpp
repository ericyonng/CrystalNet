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
 * Author: Eric Yonng
 * Date: 2021-03-23 11:02:46
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/NetEngine/LibEpoll.h>

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS

#include <kernel/comp/Log/log.h>

KERNEL_BEGIN

LibEpoll::LibEpoll()
{
    ::memset(&_ev, static_cast<UInt32>(EpollDefs::InvalidFd), sizeof(_ev));
    _evPtr = KernelCastTo<epoll_event>(_ev);
    _evFd = EpollDefs::InvalidFd;
}

LibEpoll::~LibEpoll()
{
    if(!IsDestroy())
        Destroy();
}

Int32 LibEpoll::Create()
{
    _evFd = ::epoll_create(__LIB_MAX_EPOLL_EVENT_NUM_LIMIT__);
    if(_evFd == -1)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("%d = epoll_create(%d) fail"), _evFd, __LIB_MAX_EPOLL_EVENT_NUM_LIMIT__);
        return Status::InvalidFd;
    }

    return Status::Success;
}

Int32 LibEpoll::Destroy()
{
    Int32 ret = ::close(_evFd);
    if(ret != 0)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("close evfd[%d] fail ret[%d]"), _evFd, ret);
        return Status::Failed;
    }

    _evFd = EpollDefs::InvalidFd;
    return Status::Success;
}

Int32 LibEpoll::Ctl(Int32 opType, Int32 fd, epoll_event *ev)
{
    // epoll_ctl 是线程安全的
    if(::epoll_ctl(_evFd, opType, fd, ev) == EpollDefs::EpollError)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("::epoll_ctl fail evFd[%d] optye[%d] fd[%d] ev[%p] eventFlags[%x]")
                           , _evFd, opType, fd, ev, ev->events);
        return Status::Failed;
    }
    
    return Status::Success;
}

Int32 LibEpoll::AddEvent(Int32 newFd, UInt64 sessionId, UInt32 eventFlags)
{
    // 关联的事件类型,sessionId
    epoll_event ev;
    ev.events = eventFlags;
    ev.data.u64 = sessionId;

    if(::epoll_ctl(_evFd, EPOLL_CTL_ADD, newFd, &ev) == EpollDefs::EpollError)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("::epoll_ctl(evFd[%d], add, newFd[%d], ev(%p) eventFlags[%x]) fail")
                           , _evFd, newFd, &ev, eventFlags);
        return Status::Failed;
    }

    return Status::Success;
}

Int32 LibEpoll::AddEvent(Int32 newFd, UInt32 eventFlags)
{
    // 关联的事件类型,sessionId
    epoll_event ev;
    ev.events = eventFlags;
    ev.data.fd = newFd;

    if(::epoll_ctl(_evFd, EPOLL_CTL_ADD, newFd, &ev) == EpollDefs::EpollError)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("::epoll_ctl(evFd[%d], add, newFd[%d], ev(%p) eventFlags[%x]) fail")
                           , _evFd, newFd, &ev, eventFlags);
        return Status::Failed;
    }

    return Status::Success;
}

Int32 LibEpoll::ModifyEvent(Int32 modFd, UInt64 sessionId, UInt32 eventFlags)
{
    // 关联的事件类型,sessionId
    epoll_event ev;
    ev.events = eventFlags;
    ev.data.fd = modFd;

    if(::epoll_ctl(_evFd, EPOLL_CTL_MOD, modFd, &ev) == EpollDefs::EpollError)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("::epoll_ctl(evFd[%d], mod, modFd[%d], ev(%p) eventFlags[%x]) fail")
                           , _evFd, modFd, &ev, eventFlags);
        return Status::Failed;
    }

    return Status::Success;
}

Int32 LibEpoll::DelEvent(Int32 delFd, UInt64 sessionId, UInt32 eventFlagsToDel)
{
    epoll_event ev;
    ev.events = eventFlagsToDel;
    ev.data.u64 = sessionId;
    if(::epoll_ctl(_evFd, EPOLL_CTL_DEL, delFd, &ev) == EpollDefs::EpollError)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("::epoll_ctl(evFd[%d], mod, delFd[%d], ev(%p) eventFlags[%x]) fail")
                           , _evFd, delFd, &ev, __DEL_LIB_EPOLL_EVENT_FLAGS_DEF__);
        return Status::Failed;
    }

    return Status::Success;
}

Int32 LibEpoll::Wait(Int32 millisecondsToWait /*= EpollWaitTimeOutType::BlockNoTimeOut*/)
{
    // EINTR 表示中断，需要再次wait
    Int32 ret = 0;
    while((ret = ::epoll_wait(_evFd, _ev, __LIB_MAX_EVENT_NUM_TO_HANDLE_ONCE_DEF__, millisecondsToWait)) < 0 && errno == EINTR);
    return ret;
}


KERNEL_END

#endif
