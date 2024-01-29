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
 * Date: 2022-03-25 09:16:30
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_INTERFACE_IPOLLER_MGR_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_INTERFACE_IPOLLER_MGR_H__

#pragma once

#include <kernel/comp/CompObject/CompHostObject.h>

KERNEL_BEGIN

struct PollerConfig;
class IServiceProxy;

struct PollerMgrStatisticsInfo;

class KERNEL_EXPORT IPollerMgr : public CompHostObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, IPollerMgr);

public:
    IPollerMgr(){}
    virtual ~IPollerMgr(){}

    virtual const PollerConfig *GetConfig() const = 0;
    virtual void SetConfig(const PollerConfig &cfg) = 0;

    virtual UInt64 NewSessionId() = 0;

    virtual void AddSessionPending(UInt64 num) = 0;
    virtual bool CheckAddSessionPending(UInt64 num, UInt64 &totalSessionNum) = 0;
    virtual void ReduceSessionPending(UInt64 num) = 0;
    virtual void AddSessionQuantity(UInt64 num) = 0;
    virtual void ReduceSessionQuantity(UInt64 num) = 0;
    virtual UInt64 GetSessionQuantityLimit() const = 0;

    virtual void SetServiceProxy(IServiceProxy *serviceProxy) = 0;
    virtual IServiceProxy *GetServiceProxy() = 0;
    virtual const IServiceProxy *GetServiceProxy() const = 0;
    virtual void OnMonitor(PollerMgrStatisticsInfo &statistics) = 0;

    // 收发统计
    virtual void AddRecvPacketCount(UInt64 num) = 0;
    virtual void AddRecvBytes(UInt64 num) = 0;
    virtual void AddSendPacketCount(UInt64 num) = 0;
    virtual void AddSendBytes(UInt64 num) = 0;

    // 会话统计
    virtual void AddAcceptedSessionCount(UInt64 num) = 0;
    virtual void AddConnectedSessionCount(UInt64 num) = 0;
    virtual void AddListenerSessionCount(UInt64 num) = 0;
    virtual void ReduceAcceptedSessionCount(UInt64 num) = 0;
    virtual void ReduceConnectedSessionCount(UInt64 num) = 0;
    virtual void ReduceListenerSessionCount(UInt64 num) = 0;

    // poller统计
    virtual void AddLinkerPollerCount(UInt64 num) = 0;
    virtual void AddDataTransferPollerCount(UInt64 num) = 0;
    virtual void AddPollerCount(UInt64 num) = 0;

    // 剔除所有session
    virtual void QuitAllSessions(UInt64 serviceId) = 0;
};

KERNEL_END

#endif
