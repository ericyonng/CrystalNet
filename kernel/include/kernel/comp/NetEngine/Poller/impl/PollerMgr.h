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

#include <kernel/comp/NetEngine/Poller/interface/IPollerMgr.h>
#include <atomic>

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

    // 统计数据
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
    virtual void AddPollerCount(UInt64 num) override;

    virtual void QuitAllSessions(UInt64 serviceId) override;

protected:
    // 在组件初始化前
    virtual Int32 _OnHostInit() override;
    // 所有组件创建完成
    virtual Int32 _OnCompsCreated() override;
    // 在组件启动之前
    virtual Int32 _OnHostWillStart() override;
    // 组件启动之后
    virtual Int32 _OnHostStart() override;
    // 在组件willclose之后
    virtual void _OnHostWillClose() override;
    // 在组件Close之后
    virtual void _OnHostClose() override;

private:
    void _Clear();

    PollerConfig *_config;
    std::atomic<UInt64> _maxSessionId;
    IServiceProxy *_serviceProxy;

    // 会话总数
    std::atomic<UInt64> _sessionQuantity;
    std::atomic<UInt64> _sessionQuantityPending;

    // 收到的数据
    std::atomic<UInt64> _recvPacketCount;           // 单帧包数量
    std::atomic<UInt64> _recvBytes;                 // 单帧字节数
    std::atomic<UInt64> _historyRecvPacketCount;    // 历史包总量
    std::atomic<UInt64> _historyRecvBytes;          // 历史总流量

    // 发送的数据
    std::atomic<UInt64> _sendPacketCount;           // 单帧包数量
    std::atomic<UInt64> _sendBytes;                 // 单帧字节数
    std::atomic<UInt64> _historySendPacketCount;    // 历史包总量
    std::atomic<UInt64> _historySendBytes;          // 历史总流量

    // 所有会话
    std::atomic<UInt64> _sessionCount;          // 单帧数量
    std::atomic<UInt64> _onlineSessionCount;    // 在线会话数量
    std::atomic<UInt64> _historySessionCount;   // 历史会话总数

    // 连入
    std::atomic<UInt64> _acceptedSessionCount;          // 单帧数量
    std::atomic<UInt64> _onlineAcceptedSessionCount;    // 在线会话数量
    std::atomic<UInt64> _historyAcceptedSessionCount;   // 历史会话总数

    // 连出
    std::atomic<UInt64> _connectedSessionCount;             // 单帧数量
    std::atomic<UInt64> _onlineConnectedSessionCount;       // 在线会话数量
    std::atomic<UInt64> _historyConnectedSessionCount;       // 历史会话总数

    // 监听
    std::atomic<UInt64> _onlineListenerSessionCount;        // 在线监听会话数量

    // poller
    std::atomic<UInt64> _pollerCounts;      // poller总数量
    std::atomic<UInt64> _linkerCount;       // 连接器数量
    std::atomic<UInt64> _dataTransferCount;  // 数据传输器数量
};

KERNEL_END

#endif

