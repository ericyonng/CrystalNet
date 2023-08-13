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
 * Date: 2022-06-26 19:05:01
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <service_common/ServiceCommon.h>
#include <service/common/common.h>

SERVICE_BEGIN

struct ServiceConfig;

class MyTestService : public SERVICE_COMMON_NS::IService
{
    POOL_CREATE_OBJ_DEFAULT_P1(IService, MyTestService);

public:
    MyTestService();
    ~MyTestService();
    void Release();

    // 协议栈
    virtual KERNEL_NS::IProtocolStack *GetProtocolStack(KERNEL_NS::LibSession *session) final;
    virtual const KERNEL_NS::IProtocolStack *GetProtocolStack(KERNEL_NS::LibSession *session) const final;
    virtual KERNEL_NS::IProtocolStack *GetProtocolStack(Int32 prototalStackType) final;
    virtual const KERNEL_NS::IProtocolStack *GetProtocolStack(Int32 prototalStackType) const final;

    // 获取定时器
    KERNEL_NS::TimerMgr *GetTimerMgr();
    const KERNEL_NS::TimerMgr *GetTimerMgr() const;

    // 协议订阅 已经存在的订阅会被新的覆盖并报warn
    virtual void Subscribe(Int32 opcodeId, KERNEL_NS::IDelegate<void, KERNEL_NS::LibPacket *&> *deleg);

    KERNEL_NS::EventManager *GetEventMgr();
    const KERNEL_NS::EventManager *GetEventMgr() const;

    // 获取会话类型
    Int32 GetSessionTypeByPort(UInt16 port) const;

    // 获取配置
    const ServiceConfig *GetServiceConfig() const;

    UInt64 GetSessionAmount() const override;

    // 派生接口
protected:
    // 清理数据
    virtual void _OnServiceClear() final;
    // 注册组件
    virtual void _OnServiceRegisterComps() final;
    // 服务初始化 配置
    virtual Int32 _OnServiceInit() final;
    // 服务组件创建完成
    virtual Int32 _OnServiceCompsCreated() final;
    // 服务完全启动
    virtual Int32 _OnServiceStartup() final;
    // 服务即将关闭
    virtual void _OnServiceWillClose() final;
    // 服务完成关闭
    virtual void _OnServiceClosed() final;

    // 会话创建
    virtual void _OnSessionCreated(KERNEL_NS::PollerEvent *msg) override;
    // 会话销毁
    virtual void _OnSessionDestroy(KERNEL_NS::PollerEvent *msg) override;
    // 连接回调
    virtual void _OnAsynConnectRes(KERNEL_NS::PollerEvent *msg) override;
    // 监听回调
    virtual void _OnAddListenRes(KERNEL_NS::PollerEvent *msg) override;
    // 收到网络消息回调
    virtual void _OnRecvMsg(KERNEL_NS::PollerEvent *msg) override;
    // 退出服务
    void _OnQuitingService(KERNEL_NS::PollerEvent *msg) override;


    // 初始化相关
    virtual bool _OnPollerPrepare(KERNEL_NS::Poller *poller) final;
    // 销毁相关
    virtual void _OnPollerWillDestroy(KERNEL_NS::Poller *poller) final;

    // 获取消息处理器
    KERNEL_NS::IDelegate<void, KERNEL_NS::LibPacket *&> *_GetMsgHandler(Int32 opcode);
    const KERNEL_NS::IDelegate<void, KERNEL_NS::LibPacket *&> *_GetMsgHandler(Int32 opcode) const;

    virtual void _OnEventLoopStart() override;

private:
    void _Clear();
    void _OnFrameTimer(KERNEL_NS::LibTimer *timer);
    
    Int32 _InitProtocolStack();

    bool _CheckOpcode(Int32 opcode, KERNEL_NS::LibString &errInfo);
    void _GetOpcodeInfo(Int32 opcode, KERNEL_NS::LibString &opcodeInfo);
    bool _CheckOpcodeEnable(Int32 opcode);

private:
    KERNEL_NS::TimerMgr *_timerMgr;
    KERNEL_NS::LibTimer *_updateTimer;
    Int64 _frameUpdateTimeMs;                       // 帧更新间隔ms
    KERNEL_NS::EventManager *_eventMgr;      // 事件管理器

    // 配置
    ServiceConfig *_serviceConfig;

    std::unordered_map<Int32, KERNEL_NS::IProtocolStack *> _stackTypeRefProtocolStack;
    KERNEL_NS::IProtocolStack *_defaultStack;

    // 协议消息处理器
    std::unordered_map<Int32, KERNEL_NS::IDelegate<void, KERNEL_NS::LibPacket *&> *> _opcodeRefHandler;
};

ALWAYS_INLINE KERNEL_NS::TimerMgr *MyTestService::GetTimerMgr()
{
    return _timerMgr;
}

ALWAYS_INLINE const KERNEL_NS::TimerMgr *MyTestService::GetTimerMgr() const
{
    return _timerMgr;
}

ALWAYS_INLINE KERNEL_NS::EventManager *MyTestService::GetEventMgr()
{
    return _eventMgr;
}

ALWAYS_INLINE const KERNEL_NS::EventManager *MyTestService::GetEventMgr() const
{
    return _eventMgr;
}

ALWAYS_INLINE KERNEL_NS::IDelegate<void, KERNEL_NS::LibPacket *&> *MyTestService::_GetMsgHandler(Int32 opcode)
{
    auto iter = _opcodeRefHandler.find(opcode);
    return iter == _opcodeRefHandler.end() ? NULL : iter->second;
}

ALWAYS_INLINE const KERNEL_NS::IDelegate<void, KERNEL_NS::LibPacket *&> *MyTestService::_GetMsgHandler(Int32 opcode) const
{
    auto iter = _opcodeRefHandler.find(opcode);
    return iter == _opcodeRefHandler.end() ? NULL : iter->second;
}

SERVICE_END
