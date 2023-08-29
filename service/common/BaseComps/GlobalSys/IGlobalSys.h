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

#include <kernel/kernel.h>
#include <service_common/ServiceCommon.h>
#include <service/common/SessionType.h>
#include <service/common/PriorityLevelDefine.h>
#include <service/common/BaseComps/LogicSys/LogicSys.h>

SERVICE_BEGIN

class IGlobalSys : public ILogicSys
{
    POOL_CREATE_OBJ_DEFAULT_P1(ILogicSys, IGlobalSys);

public:
    IGlobalSys();
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

   /*
   * 网络相关
   */
public:
  /*
  * 发送消息
  * @return(Int64):返回packetId
  */
  Int64 Send(UInt64 sessionId, KERNEL_NS::LibPacket *packet) const;
  void Send(UInt64 sessionId, const std::list<KERNEL_NS::LibPacket *> &packets) const;
  Int64 Send(UInt64 sessionId, Int32 opcode, const KERNEL_NS::ICoder &coder, Int64 packetId = -1) const;
  
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


    // 组件接口资源
protected:
    /*
    * global 子系统派生类初始化请调用这个
    * Attention: 业务直接互相耦合的，需要放在_OnHostStart做,init不做业务耦合的事情
    */
    virtual Int32 _OnSysInit() final;
    virtual Int32 _OnGlobalSysInit() { return Status::Success; }

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


SERVICE_END