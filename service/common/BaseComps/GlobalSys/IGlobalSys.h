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
 * Date: 2022-09-18 19:53:32
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <service/common/SessionType.h>
#include <service/common/PriorityLevelDefine.h>
#include <service/common/BaseComps/LogicSys/LogicSys.h>
#include <kernel/comp/Delegate/LibDelegate.h>
#include <service/common/status.h>
#include <kernel/comp/Event/Defs.h>
#include <service_common/service/service.h>

#include <list>

KERNEL_BEGIN

struct IpControlInfo;
class LibPacket;
class ICoder;
class LibEvent;

KERNEL_END

SERVICE_BEGIN

template<bool T>
concept NeedReponseAdapter = std::bool_constant<T>::value;

class IGlobalSys : public ILogicSys
{
    POOL_CREATE_OBJ_DEFAULT_P1(ILogicSys, IGlobalSys);

public:
    IGlobalSys(UInt64 objTypeId);
    ~IGlobalSys();

   void Release() = 0;

    /*
    * 获取其他global系统
    */
   template<typename SysType>
   SysType *GetGlobalSys();
   template<typename SysType>
   const SysType *GetGlobalSys() const;

    /*
    * 系统将要启动（表示数据等都准备就绪, 但只针对当前服务节点来说准备好了） 
    */
    virtual void OnWillStartup() {}
    
    /*
    * 系统启动
    */
    virtual void OnStartup() {}

    static const KERNEL_NS::LibString OpcodeToString(Int32 opcode);

   /*
   * 网络相关
   */
public:
    /*
    * 发送消息
    * @return(Int64):返回packetId
    */
    void Send(UInt64 sessionId, KERNEL_NS::LibPacket *packet) const;
    void Send(UInt64 sessionId, const std::list<KERNEL_NS::LibPacket *> &packets) const;
    // packetId 为0表示无效, 其他都是有效的
    void Send(UInt64 sessionId, Int32 opcode, const KERNEL_NS::ICoder &coder, Int64 packetId = 0) const;

    // 使用协程发送和接收返回消息, 由于会动态订阅Response消息, 那么之前已经订阅了Response消息的则会失效, 必须设置packetId用于回调
    template<typename CoderType>
    CRYSTAL_NET::kernel::CoTask<KERNEL_NS::SmartPtr<KERNEL_NS::LibPacket, KERNEL_NS::AutoDelMethods::Release>> SendCo(UInt64 sessionId, Int32 opcode, KERNEL_NS::SmartPtr<CoderType, KERNEL_NS::AutoDelMethods::Release> coder, Int64 packetId)
    {
        // 1.发送数据包
        Send(sessionId, opcode, *coder, packetId);

        // 2.关联response消息回调
        KERNEL_NS::SmartPtr<KERNEL_NS::LibPacket *,  KERNEL_NS::AutoDelMethods::CustomDelete> ptr(KERNEL_NS::KernelCastTo<KERNEL_NS::LibPacket*>(
            kernel::KernelAllocMemory<KERNEL_NS::_Build::TL>(sizeof(KERNEL_NS::LibPacket **))));
        ptr.SetClosureDelegate([](void *p)
        {
            // 释放packet
            auto castP = KERNEL_NS::KernelCastTo<KERNEL_NS::LibPacket*>(p);
            if(*castP)
                (*castP)->Release();

            KERNEL_NS::KernelFreeMemory<KERNEL_NS::_Build::TL>(castP);
        });
        *ptr = NULL;

        // TODO:通过创建一个CoTask Waiter将CoParam传到lambda中
        KERNEL_NS::SmartPtr<KERNEL_NS::TaskParamRefWrapper, KERNEL_NS::AutoDelMethods::Release> params = KERNEL_NS::TaskParamRefWrapper::NewThreadLocal_TaskParamRefWrapper();
        SubscribePacket(sessionId, packetId, [ptr, params](KERNEL_NS::LibPacket *&packet) mutable 
        {
            // 将结果带出去
            *ptr = packet;
            
            // 后续协程释放packet
            packet = NULL;

            // 唤醒Waiter
            auto &coParam = params->_params;
            if(coParam && coParam->_handle)
                coParam->_handle->ForceAwake();
        });

        // 等待Waiter
        co_await KERNEL_NS::Waiting().SetDisableSuspend().GetParam(params);

        if(params->_params)
        {
            auto &pa = params->_params; 
            if(pa->_errCode != Status::Success)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("waiting err:%d, session id:%llu, opcode:%d,%s, data:%s, packetId:%lld")
                    , pa->_errCode, sessionId, opcode, OpcodeToString(opcode).c_str(), coder->ToJsonString().c_str(), packetId);
                UnSubscribePacket(sessionId, packetId);
            }

            // 销毁waiting协程
            if(pa->_handle)
                pa->_handle->DestroyHandle(pa->_errCode);
        }
        
        // 3.将消息回调中的Packet引用设置成空
        auto packet = *ptr;
        *ptr = NULL;
        co_return KERNEL_NS::SmartPtr<KERNEL_NS::LibPacket,  KERNEL_NS::AutoDelMethods::Release>(packet);
    }

    Int64 NewPacketId(UInt64 sessionId) const;
    
    /*
    * 关闭会话
    */
    void CloseSession(UInt64 sessionId, Int64 closeMillisecondTimeDelay, bool forbidRead, bool forbidWrite) const;

    /*
    * ip 黑白名单控制
    */
    void AddWhite(const KERNEL_NS::LibString &ip, Int32 level = PriorityLevelDefine::INNER);
    void AddBlack(const KERNEL_NS::LibString &ip, Int32 level = PriorityLevelDefine::INNER);
    void EraseWhite(const KERNEL_NS::LibString &ip, Int32 level = PriorityLevelDefine::INNER);
    void EraseBlack(const KERNEL_NS::LibString &ip, Int32 level = PriorityLevelDefine::INNER);
    void AddWhite(const std::list<KERNEL_NS::LibString> &ips, Int32 level = PriorityLevelDefine::INNER);
    void AddBlack(const std::list<KERNEL_NS::LibString> &ips, Int32 level = PriorityLevelDefine::INNER);
    void EraseWhite(const std::list<KERNEL_NS::LibString> &ips, Int32 level = PriorityLevelDefine::INNER);
    void EraseBlack(const std::list<KERNEL_NS::LibString> &ips, Int32 level = PriorityLevelDefine::INNER);
    void ControlIpPipline(const std::list<KERNEL_NS::IpControlInfo *> &controlInfoList, Int32 level = PriorityLevelDefine::INNER);

    template<typename ObjType>
    void Subscribe(Int32 opcodeId, ObjType *obj, void (ObjType::*Handler)(KERNEL_NS::LibPacket *&));
    void Subscribe(Int32 opcodeId, KERNEL_NS::IDelegate<void, KERNEL_NS::LibPacket *&> *deleg);
    template<typename LambadaType>
    void Subscribe(Int32 opcodeId, LambadaType &&lamb);
    // 监听packet回包
    template<typename LambadaType>
    void SubscribePacket(UInt64 sessionId, Int64 packetId, LambadaType &&lamb);
    void UnSubscribePacket(UInt64 sessionId, Int64 packetId);

    // 组件接口资源
protected:
    /*
    * global 子系统派生类初始化请调用这个
    * Attention: 业务直接互相耦合的，需要放在_OnHostStart做,init不做业务耦合的事情
    */
    virtual Int32 _OnSysInit() final;
    virtual Int32 _OnGlobalSysInit() { return Status::Success; }

    virtual Int32 _OnHostWillStart() final;
    virtual Int32 _OnGlobalSysWillStart() { return Status::Success; }

    virtual Int32 _OnSysCompsCreated() final;
    virtual Int32 _OnGlobalSysCompsCreated() { return Status::Success; }

    /*
    * 在组件 close 之后 global close
    * Attention:给IGlobalSys使用 派生类使用：_OnGlobalSysClose
    */
    // 在组件Close之后
    virtual void _OnSysClose() final;
    virtual void _OnGlobalSysClose();

private:
    void _Clear();

    void _OnWillStartupEv(KERNEL_NS::LibEvent *ev);
    void _OnStartupEv(KERNEL_NS::LibEvent *ev);

    KERNEL_NS::ListenerStub _onServiceWillStartupStub;
    KERNEL_NS::ListenerStub _onServiceStartupStub;
};

template<typename SysType>
ALWAYS_INLINE SysType *IGlobalSys::GetGlobalSys()
{
    return GetService()->GetComp<SysType>();
}

template<typename SysType>
ALWAYS_INLINE const SysType *IGlobalSys::GetGlobalSys() const
{
    return GetService()->GetComp<SysType>();
}

template<typename ObjType>
ALWAYS_INLINE void IGlobalSys::Subscribe(Int32 opcodeId, ObjType *obj, void (ObjType::*Handler)(KERNEL_NS::LibPacket *&))
{
    auto delg = KERNEL_NS::DelegateFactory::Create(obj, Handler);
    Subscribe(opcodeId, delg);
}

template<typename LambadaType>
ALWAYS_INLINE void IGlobalSys::Subscribe(Int32 opcodeId, LambadaType &&lamb)
{
    auto delg = KERNEL_CREATE_CLOSURE_DELEGATE(lamb, void, KERNEL_NS::LibPacket *&);
    Subscribe(opcodeId, delg);
}

ALWAYS_INLINE void IGlobalSys::Subscribe(Int32 opcodeId, KERNEL_NS::IDelegate<void, KERNEL_NS::LibPacket *&> *deleg)
{
    GetService()->Subscribe(opcodeId, deleg);
}

template<typename LambadaType>
ALWAYS_INLINE void IGlobalSys::SubscribePacket(UInt64 sessionId, Int64 packetId, LambadaType &&lamb)
{
    auto deleg = KERNEL_CREATE_CLOSURE_DELEGATE(lamb, void, KERNEL_NS::LibPacket *&);
    GetService()->RegisterPacketMsg(sessionId, packetId, deleg);
}

ALWAYS_INLINE void IGlobalSys::UnSubscribePacket(UInt64 sessionId, Int64 packetId)
{
    GetService()->UnRegisterPacketMsg(sessionId, packetId);
}


SERVICE_END