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
 * Date: 2022-04-22 12:47:05
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_IMPL_SESSION_LIB_SESSION_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_IMPL_SESSION_LIB_SESSION_H__

#pragma once

#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/common/LibObject.h>

#include <kernel/comp/LibString.h>
#include <kernel/comp/Utils/BitUtil.h>
#include <kernel/comp/NetEngine/Poller/impl/Session/SessionOption.h>
#include <kernel/comp/NetEngine/Poller/Defs/CloseSessionInfo.h>
#include <kernel/comp/Cpu/LibCpuCounter.h>
#include <kernel/comp/LibList.h>

KERNEL_BEGIN

class LibSocket;
class IProtocolStack;
class LibPacket;

template<typename BuildType>
class LibStream;

class SessionStreamCtrl
{
public:
    enum ENUMS : UInt64
    {
        NONE = 0,           // 无效
        SEND = 1,           // 发送
        RECV = 2,           // 收
        ACCEPT = 3,         // 接受连入
        CONNECT = 4,        // 连出
        SEND_EAGAIN = 5,    // 发送缓冲区阻塞
        WILL_CLOSE = 6,     // 即将关闭
    };

    static LibString ToString(UInt64 streamMask)
    {
        LibString info;
        info.AppendFormat("stream switch flag:")
            .AppendFormat("recv enable:%d", BitUtil::IsSet(streamMask, SessionStreamCtrl::RECV) ? 0 : 1)
            .AppendFormat(", send enable:%d", BitUtil::IsSet(streamMask, SessionStreamCtrl::SEND) ? 0 : 1)
            .AppendFormat(", send eagain:%d", BitUtil::IsSet(streamMask, SessionStreamCtrl::SEND_EAGAIN) ? 1 : 0)
            .AppendFormat(", accept enable:%d", BitUtil::IsSet(streamMask, SessionStreamCtrl::ACCEPT) ? 0 : 1)
            .AppendFormat(", connect enable:%d", BitUtil::IsSet(streamMask, SessionStreamCtrl::CONNECT) ? 0 : 1)
            .AppendFormat(", session will close:%d", BitUtil::IsSet(streamMask, SessionStreamCtrl::WILL_CLOSE) ? 1 : 0)
            .AppendFormat(".")
            ;

        return info;
    }
};

class KERNEL_EXPORT LibSession
{
    POOL_CREATE_OBJ_DEFAULT(LibSession);

public:
    LibSession(UInt64 sessionId, bool isLinker, bool isConnectToRemote);
    virtual ~LibSession();

    Int32 Init();

    LibSocket *GetSock();
    const LibSocket *GetSock() const;

    // 设置配置
    void SetOption(const SessionOption &option);
    void SetSocket(LibSocket *sock);
    void SetServiceId(UInt64 serviceId);
    void SetPollerId(UInt64 pollerId);
    void SetBufferCapacity(UInt64 bufferCapacity);
    void SetProtocolStack(IProtocolStack *protocolStack);
    void SetPriorityLevel(Int32 level);
    void SetRecvHandleBytesLimit(UInt64 value);
    void SetSendHandleBytesLimit(UInt64 value);
    void SetAcceptHandleCountLimit(UInt64 value);
    // 设置包大小
    void SetMaxPacketSize(UInt64 value);
    // 协议类型
    void SetProtocolType(Int32 type);

    // 获取当前处理数据量
    UInt64 GetCurFrameRecvHandleBytes() const;
    UInt64 GetCurFrameSendHandleBytes() const;
    UInt64 GetCurFrameAcceptHandleCount() const;
    UInt64 &GetCurFrameAcceptHandleCount();
    UInt64 GetFrameAcceptHandleLimit() const;
    // 重置当前处理数据量
    void ResetFrameRecvHandleBytes();
    void ResetFrameSendHandleBytes();
    void ResetFrameAcceptHandleCount();
    // 获取当前剩余可以处理的数据量大小
    UInt64 GetEnableRecvHandleBytesLeft() const;
    UInt64 GetEnableSendHandleBytesLeft() const;
    UInt64 GetEnableAcceptCountLeft() const;
    UInt64 GetServiceId() const;
    UInt64 GetPollerId() const;
    UInt64 GetMaxPacketSize() const;
    UInt32 GetPriorityLevel() const;
    Int32 GetProtocolType() const;
    const SessionOption &GetOption() const;
    Int32 GetSessionType() const;
    Int32 GetProtocolStackType() const;

    const IProtocolStack *GetProtocolStack() const;

    virtual LibString ToString() const;

    virtual void Close();
    
    LibSession &BeginTransaction();
    LibSession &CommitTransaction();

    UInt64 GetId() const;
    void ForbidSend();
    void MaskSendEagain();
    void ClearSendEagain();
    void ForbidRecv();
    void EnableRecv();
    bool CanSend() const;
    bool CanRecv() const;
    bool IsSendEagain() const;
    bool IsLinker() const;
    bool IsConnectToRemote() const;
    bool CanAccept() const;
    void ForbidAccept();
    void MaskClose(UInt64 reason);
    bool WillSessionClose() const;

    bool CheckUpdateRecvSpeed();

private:
    void _Destroy();

protected:
    std::atomic_bool _inited;               // 初始化
    const UInt64 _id;                       // session id
    Int32 _priorityLevel;                   // 优先级
    Int32 _protocolType;                    // 协议类型
    UInt64 _bufferCapacity;                 // 单个缓冲大小
    UInt64 _serviceId;                      // session所在service
    UInt64 _pollerId;                       // session 所在pollerid
    UInt64 _handleRecvBytesPerFrameLimit;   // 单帧最大接收数据量
    UInt64 _handleSendBytesPerFrameLimit;   // 单帧最大发送数据量
    UInt64 _handleAcceptCountPerFrameLimit; // 单帧最大连入连接数
    UInt64 _maxPacketSize;                  // 最大包长度
    IProtocolStack *_protocolStack;         // 协议栈
    const bool _isLinker;                   // 是否连接器或者监听器
    const bool _isConnectToRemote;          // 本地连接远程

    LibSocket *_sock;                       // 套接字

    SessionOption _option;                  // 会话选项
    UInt64 _curFrameRecvHandleBytes;        // 当前帧结束数据字节数
    UInt64 _curFrameSendHandleBytes;        // 当前帧发送字节数
    UInt64 _curFrameAcceptCount;            // 当前连入次数
    LibList<LibPacket *> *_sendPacketList;  // 发送队列
    LibStream<_Build::TL> *_lastSendLeft;   // 剩余的未发
    LibList<LibStream<_Build::TL> *, _Build::TL> *_recvBuffers;    // 接收缓冲区 顺序插入,倒序移除

    UInt64 _streamCtrlMask;                 // 流开关控制
    CloseSessionInfo _closeReason;          // 关闭原因

    // 限速处理
    LibCpuCounter _recvLastCpuTime;
    Int64 _currentTokenNumber;
};

ALWAYS_INLINE LibSocket *LibSession::GetSock()
{
    return _sock;
}

ALWAYS_INLINE const LibSocket *LibSession::GetSock() const
{
    return _sock;
}

ALWAYS_INLINE void LibSession::SetOption(const SessionOption &option)
{
    _option = option;
}

ALWAYS_INLINE void LibSession::SetServiceId(UInt64 serviceId)
{
    _serviceId = serviceId;
}

ALWAYS_INLINE void LibSession::SetPollerId(UInt64 pollerId)
{
    _pollerId = pollerId;
}

ALWAYS_INLINE void LibSession::SetBufferCapacity(UInt64 bufferCapacity)
{
    _bufferCapacity = bufferCapacity;
}

ALWAYS_INLINE void LibSession::SetProtocolStack(IProtocolStack *protocolStack)
{
    _protocolStack = protocolStack;
}

ALWAYS_INLINE void LibSession::SetPriorityLevel(Int32 level)
{
    _priorityLevel = level;
}

ALWAYS_INLINE void LibSession::SetRecvHandleBytesLimit(UInt64 value)
{
    _handleRecvBytesPerFrameLimit = value;
}

ALWAYS_INLINE void LibSession::SetSendHandleBytesLimit(UInt64 value)
{
    _handleSendBytesPerFrameLimit = value;
}

ALWAYS_INLINE void LibSession::SetAcceptHandleCountLimit(UInt64 value)
{
    _handleAcceptCountPerFrameLimit = value;
}

ALWAYS_INLINE void LibSession::SetMaxPacketSize(UInt64 value)
{
    _maxPacketSize = value;
}

ALWAYS_INLINE void LibSession::SetProtocolType(Int32 type)
{
    _protocolType = type;
}

ALWAYS_INLINE UInt64 LibSession::GetCurFrameRecvHandleBytes() const
{
    return _curFrameRecvHandleBytes;
}

ALWAYS_INLINE UInt64 LibSession::GetCurFrameSendHandleBytes() const
{
    return _curFrameSendHandleBytes;
}

ALWAYS_INLINE UInt64 LibSession::GetCurFrameAcceptHandleCount() const
{
    return _curFrameAcceptCount;
}

ALWAYS_INLINE UInt64 &LibSession::GetCurFrameAcceptHandleCount()
{
    return _curFrameAcceptCount;
}

ALWAYS_INLINE UInt64 LibSession::GetFrameAcceptHandleLimit() const
{
    return _handleAcceptCountPerFrameLimit;
}

ALWAYS_INLINE void LibSession::ResetFrameRecvHandleBytes()
{
    _curFrameRecvHandleBytes = 0;
}

ALWAYS_INLINE void LibSession::ResetFrameSendHandleBytes()
{
    _curFrameSendHandleBytes = 0;
}

ALWAYS_INLINE void LibSession::ResetFrameAcceptHandleCount()
{
    _curFrameAcceptCount = 0;
}

ALWAYS_INLINE UInt64 LibSession::GetEnableRecvHandleBytesLeft() const
{
    return _curFrameRecvHandleBytes > _handleRecvBytesPerFrameLimit ? 0 : (_handleRecvBytesPerFrameLimit - _curFrameRecvHandleBytes);
}

ALWAYS_INLINE UInt64 LibSession::GetEnableSendHandleBytesLeft() const
{
    return _curFrameSendHandleBytes > _handleSendBytesPerFrameLimit ? 0 : (_handleSendBytesPerFrameLimit - _curFrameSendHandleBytes);
}

ALWAYS_INLINE UInt64 LibSession::GetEnableAcceptCountLeft() const
{
    return _handleAcceptCountPerFrameLimit - _curFrameAcceptCount;
}

ALWAYS_INLINE UInt64 LibSession::GetServiceId() const
{
    return _serviceId;
}

ALWAYS_INLINE UInt64 LibSession::GetPollerId() const
{
    return _pollerId;
}

ALWAYS_INLINE UInt64 LibSession::GetMaxPacketSize() const
{
    return _maxPacketSize;
}

ALWAYS_INLINE UInt32 LibSession::GetPriorityLevel() const
{
    return _priorityLevel;
}

ALWAYS_INLINE Int32 LibSession::GetProtocolType() const
{
    return _protocolType;
}

ALWAYS_INLINE const SessionOption &LibSession::GetOption() const
{
    return _option;
}

ALWAYS_INLINE Int32 LibSession::GetSessionType() const
{
    return _option._sessionType;
}

ALWAYS_INLINE Int32 LibSession::GetProtocolStackType() const
{
    return _option._protocolStackType;
}

ALWAYS_INLINE const IProtocolStack *LibSession::GetProtocolStack() const
{
    return _protocolStack;
}

ALWAYS_INLINE UInt64 LibSession::GetId() const
{
    return _id;
}

ALWAYS_INLINE void LibSession::ForbidSend()
{
    _streamCtrlMask = BitUtil::Set(_streamCtrlMask, SessionStreamCtrl::SEND);
}

ALWAYS_INLINE void LibSession::MaskSendEagain()
{
    _streamCtrlMask = BitUtil::Set(_streamCtrlMask, SessionStreamCtrl::SEND_EAGAIN);
}

ALWAYS_INLINE void LibSession::ClearSendEagain()
{
    _streamCtrlMask = BitUtil::Clear(_streamCtrlMask, SessionStreamCtrl::SEND_EAGAIN);
}

ALWAYS_INLINE void LibSession::ForbidRecv()
{
    _streamCtrlMask = BitUtil::Set(_streamCtrlMask, SessionStreamCtrl::RECV);
}

ALWAYS_INLINE void LibSession::EnableRecv()
{
    _streamCtrlMask = BitUtil::Clear(_streamCtrlMask, SessionStreamCtrl::RECV);
}

ALWAYS_INLINE bool LibSession::CanSend() const
{
    if(BitUtil::IsSet(_streamCtrlMask, SessionStreamCtrl::SEND))
        return false;

    return true;
}

ALWAYS_INLINE bool LibSession::CanRecv() const
{
    if(BitUtil::IsSet(_streamCtrlMask, SessionStreamCtrl::RECV))
        return false;

    return true;
}

ALWAYS_INLINE bool LibSession::IsSendEagain() const
{
    return BitUtil::IsSet(_streamCtrlMask, SessionStreamCtrl::SEND_EAGAIN);
}

ALWAYS_INLINE bool LibSession::IsLinker() const
{
    return _isLinker;
}

ALWAYS_INLINE bool LibSession::IsConnectToRemote() const
{
    return _isConnectToRemote;
}

ALWAYS_INLINE bool LibSession::CanAccept() const
{
    if(BitUtil::IsSet(_streamCtrlMask, SessionStreamCtrl::ACCEPT))
        return false;

    return true;   
}

ALWAYS_INLINE void LibSession::ForbidAccept()
{
    _streamCtrlMask = BitUtil::Set(_streamCtrlMask, SessionStreamCtrl::ACCEPT);
}

ALWAYS_INLINE void LibSession::MaskClose(UInt64 reason)
{
    _streamCtrlMask = BitUtil::Set(_streamCtrlMask, SessionStreamCtrl::WILL_CLOSE);
    _closeReason.Mask(reason);
}

ALWAYS_INLINE bool LibSession::WillSessionClose() const
{
    return BitUtil::IsSet(_streamCtrlMask, SessionStreamCtrl::WILL_CLOSE);
}

ALWAYS_INLINE bool LibSession::CheckUpdateRecvSpeed()
{
    if(UNLIKELY(_option._sessionRecvPacketSpeedLimit == 0))
        return true;
        
    const auto &current = LibCpuCounter::Current();
    if(current.ElapseMilliseconds(_recvLastCpuTime) >= _option._sessionRecvPacketSpeedTimeUnitMs)
    {// 更新令牌数量
        _currentTokenNumber = _option._sessionRecvPacketSpeedLimit;
        _recvLastCpuTime.Update();
    }

    return (--_currentTokenNumber) >= 0;
}

KERNEL_END

#endif
