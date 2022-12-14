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
 * Date: 2022-03-25 09:20:45
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_IMPL_POLLER_MGR_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_IMPL_POLLER_MGR_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/NetEngine/Poller/interface/IPollerMgr.h>
#include <kernel/comp/NetEngine/Poller/Defs/PollerConfig.h>

KERNEL_BEGIN

class KERNEL_EXPORT PollerMgr : public IPollerMgr
{
    POOL_CREATE_OBJ_DEFAULT_P1(IPollerMgr, PollerMgr);
    
public:
    PollerMgr();
    virtual ~PollerMgr();

public:
    virtual void Clear() override;
    virtual void Release() override;
    virtual void OnRegisterComps() override;
    LibString ToString() const override;

public:
    virtual const PollerConfig *GetConfig() const override;
    virtual void SetConfig(const PollerConfig &cfg) override;
    virtual UInt64 NewSessionId() override;

    virtual void AddSessionPending(UInt64 num) override;
    virtual bool CheckAddSessionPending(UInt64 num, UInt64 &totalSessionNum) override;
    virtual void ReduceSessionPending(UInt64 num) override;
    virtual void AddSessionQuantity(UInt64 num) override;
    virtual void ReduceSessionQuantity(UInt64 num) override;
    virtual UInt64 GetSessionQuantityLimit() const override;

    virtual void SetServiceProxy(IServiceProxy *serviceProxy) override;
    virtual IServiceProxy *GetServiceProxy() override;
    virtual const IServiceProxy *GetServiceProxy() const override;
    virtual void OnMonitor(LibString &info) override;

    // ????????????
    virtual void AddRecvPacketCount(UInt64 num) override;
    virtual void AddRecvBytes(UInt64 num) override;
    virtual void AddSendPacketCount(UInt64 num) override;
    virtual void AddSendBytes(UInt64 num) override;

    virtual void AddAcceptedSessionCount(UInt64 num) override;
    virtual void AddConnectedSessionCount(UInt64 num) override;
    virtual void AddListenerSessionCount(UInt64 num) override;
    virtual void ReduceAcceptedSessionCount(UInt64 num) override;
    virtual void ReduceConnectedSessionCount(UInt64 num) override;
    virtual void ReduceListenerSessionCount(UInt64 num) override;

    virtual void AddLinkerPollerCount(UInt64 num) override;
    virtual void AddDataTransferPollerCount(UInt64 num) override;

    virtual void QuitAllSessions(UInt64 serviceId) override;

protected:
    // ?????????????????????
    virtual Int32 _OnHostInit() override;
    // ????????????????????????
    virtual Int32 _OnCompsCreated() override;
    // ?????????????????????
    virtual Int32 _OnHostWillStart() override;
    // ??????????????????
    virtual Int32 _OnHostStart() override;
    // ?????????willclose??????
    virtual void _OnHostWillClose() override;
    // ?????????Close??????
    virtual void _OnHostClose() override;

private:
    void _Clear();

    PollerConfig *_config;
    std::atomic<UInt64> _maxSessionId;
    IServiceProxy *_serviceProxy;

    // ????????????
    std::atomic<UInt64> _sessionQuantity;
    std::atomic<UInt64> _sessionQuantityPending;

    // ???????????????
    std::atomic<UInt64> _recvPacketCount;           // ???????????????
    std::atomic<UInt64> _recvBytes;                 // ???????????????
    std::atomic<UInt64> _historyRecvPacketCount;    // ???????????????
    std::atomic<UInt64> _historyRecvBytes;          // ???????????????

    // ???????????????
    std::atomic<UInt64> _sendPacketCount;           // ???????????????
    std::atomic<UInt64> _sendBytes;                 // ???????????????
    std::atomic<UInt64> _historySendPacketCount;    // ???????????????
    std::atomic<UInt64> _historySendBytes;          // ???????????????

    // ????????????
    std::atomic<UInt64> _sessionCount;          // ????????????
    std::atomic<UInt64> _onlineSessionCount;    // ??????????????????
    std::atomic<UInt64> _historySessionCount;   // ??????????????????

    // ??????
    std::atomic<UInt64> _acceptedSessionCount;          // ????????????
    std::atomic<UInt64> _onlineAcceptedSessionCount;    // ??????????????????
    std::atomic<UInt64> _historyAcceptedSessionCount;   // ??????????????????

    // ??????
    std::atomic<UInt64> _connectedSessionCount;             // ????????????
    std::atomic<UInt64> _onlineConnectedSessionCount;       // ??????????????????
    std::atomic<UInt64> _historyConnectedSessionCount;       // ??????????????????

    // ??????
    std::atomic<UInt64> _onlineListenerSessionCount;        // ????????????????????????

    // poller
    std::atomic<UInt64> _pollerCounts;      // poller?????????
    std::atomic<UInt64> _linkerCount;       // ???????????????
    std::atomic<UInt64> _dataTransferCount;  // ?????????????????????
};

KERNEL_END

#endif

