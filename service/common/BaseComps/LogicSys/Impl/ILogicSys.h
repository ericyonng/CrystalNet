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
 * Date: 2022-09-22 19:39:30
 * Author: Eric Yonng
 * Description: 逻辑系统，一定是service下的组件
*/

#pragma once

#include <kernel/kernel.h>
#include <service_common/ServiceCommon.h>
#include <service/common/SessionType.h>
#include <service/common/PriorityLevelDefine.h>

SERVICE_BEGIN

class LogicInterestMethod
{
public: 
    enum ENUMS
    {
        BEGIN = 0,

        ON_PASS_DAY = BEGIN,    // 跨天接口关注
        ON_PASS_WEEK = 1,       // 跨周 国外以星期天为第一天，国内以星期一为第一天
        ON_PASS_MONTH = 2,      // 跨月
        ON_PASS_YEAR = 3,       // 跨年
        ON_STORAGE_SUPPORT = 4,  // 存储支持
        END,
    };
};

class ILogicSys : public KERNEL_NS::CompHostObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, ILogicSys);

public:
    ILogicSys();
    ~ILogicSys();

    static SERVICE_COMMON_NS::IService *&GetCurrentService();

    // api
public:
    /*
    * global 信息
    */
    virtual KERNEL_NS::LibString ToString() const override;

    /*
    * 注册组件
    */
    virtual void OnRegisterComps() override;

    
    // 逻辑功能
public:
    /*
    * 获取所在的服务
    */
    SERVICE_COMMON_NS::IService *GetService();
    const SERVICE_COMMON_NS::IService *GetService() const;

    /*
    * 获取所在的服务代理
    */
    SERVICE_COMMON_NS::ServiceProxy *GetServiceProxy();
    const SERVICE_COMMON_NS::ServiceProxy *GetServiceProxy() const;

    /*
    * 获取所在的app
    */
    SERVICE_COMMON_NS::Application *GetApp();
    const SERVICE_COMMON_NS::Application *GetApp() const;

    /*
    * 获取事件管理器
    */
    KERNEL_NS::EventManager *GetEventMgr();
    const KERNEL_NS::EventManager *GetEventMgr() const;
    void SetEventMgr(KERNEL_NS::EventManager *eventMgr);

    /*
    * 定时器管理
    */
   KERNEL_NS::TimerMgr *GetTimerMgr();
   const KERNEL_NS::TimerMgr *GetTimerMgr() const;

   /*
   * 获取 子系统
   */
   template<typename SysType>
   SysType *GetSys();
   template<typename SysType>
   const SysType *GetSys() const;

   // 关注的接口
   void FocusMethod(Int32 methodEnum);
   bool IsMethodFocus(Int32 methodEnum) const;

   /*
   * 数据加载
   * TODO: 需要 FocusMethod ON_STORAGE_SUPPORT 才生效
   */
   virtual void OnLoaded(const KERNEL_NS::LibString &db);

   /*
   * 数据持久化
   * TODO: 需要FocusMethod ON_STORAGE_SUPPORT 才生效
   */
   virtual void OnStorage(KERNEL_NS::LibString &db);

   /*
   * 跨天 默认关注
   * @param(params):NULL
   */
  virtual void OnPassDay(KERNEL_NS::Variant *params) {}

   /*
   * 跨周 需要 FocusMethod 才生效
   * @param(params):NULL
   */
  virtual void OnPassWeek(KERNEL_NS::Variant *params) {}

   /*
   * 跨月 需要 FocusMethod 才生效
   * @param(params):NULL
   */
  virtual void OnPassMonth(KERNEL_NS::Variant *params) {}

  /*
  * 跨年 需要 FocusMethod 才生效
  * @param(params):NULL
  */
 virtual void OnPassYear(KERNEL_NS::Variant *params) {}

 
    // 组件接口资源
protected:
    /*
    * global 被创建出来时
    */
    virtual Int32 _OnHostCreated() override;

    /*
    * 初始化
    * Attention: 本接口只提供给ILogicSys调用，用于初始化相关资源
    */
    virtual Int32 _OnHostInit() final;

    /*
    * 子系统派生类初始化请调用这个
    */
    virtual Int32 _OnSysInit() { return Status::Success; }

    /*
    * sys 所有组件创建出来时
    */
    virtual Int32 _OnCompsCreated() override { return Status::Success; }

    /*
    * sys 将要启动,并且在组件启动之前,
    * Attention:勿在此之前启动线程,此时是线程不安全的状态
    */
    virtual Int32 _OnHostWillStart() override { return Status::Success; }

    /*
    * 组件启动之后 sys 启动, 此时可以启动线程
    */
    virtual Int32 _OnHostStart() override { return Status::Success; }

    /*
    * 在组件will close之前 sys will close
    * Attention: 若内部有多线程, 应该在此时结束线程,避免后续流程线程不安全
    */
    virtual void _OnHostBeforeCompsWillClose() override {}

    /*
    * 在组件will close 之后 sys will close
    */
    virtual void _OnHostWillClose() override {}

    /*
    * 在组件 close 之前 sys close
    */
    virtual void _OnHostBeforeCompsClose() override {}

    /*
    * 在组件 close 之后 sys close
    * Attention:给ILogicSys使用 派生类使用：_OnSysClose
    */
    // 在组件Close之后
    virtual void _OnHostClose() final;
    virtual void _OnSysClose() {}

    /*
    * 在组件 update 之前 sys update
    * Attention:接口被调用的前提是，需要在 _OnCreated或者在构造中设置:
    *       SetFocus(KERNEL_NS::ObjFocusInterfaceFlag::ON_UPDATE);
    *       方可生效
    */
    virtual void _OnWillHostUpdate() override { }

    /*
    * 在组件 update 之后 sys update
    * Attention:接口被调用的前提是，需要在 _OnCreated或者在构造中设置:
    *       SetFocus(KERNEL_NS::ObjFocusInterfaceFlag::ON_UPDATE);
    *       方可生效
    */
    virtual void _OnHostUpdate() override { }

private:
    SERVICE_COMMON_NS::IService *_service;
    SERVICE_COMMON_NS::ServiceProxy *_serviceProxy;
    SERVICE_COMMON_NS::Application *_app;
    KERNEL_NS::TimerMgr *_timerMgr;
    KERNEL_NS::EventManager *_eventMgr;

    // 关注的接口
    std::unordered_set<Int32> _intrestMethods;
};

ALWAYS_INLINE SERVICE_COMMON_NS::IService *ILogicSys::GetService()
{
    return _service;
}

ALWAYS_INLINE const SERVICE_COMMON_NS::IService *ILogicSys::GetService() const
{
    return _service;
}

ALWAYS_INLINE SERVICE_COMMON_NS::ServiceProxy *ILogicSys::GetServiceProxy()
{
    return _serviceProxy;
}

ALWAYS_INLINE const SERVICE_COMMON_NS::ServiceProxy *ILogicSys::GetServiceProxy() const
{
    return _serviceProxy;
}

ALWAYS_INLINE SERVICE_COMMON_NS::Application *ILogicSys::GetApp()
{
    return _app;
}

ALWAYS_INLINE const SERVICE_COMMON_NS::Application *ILogicSys::GetApp() const
{
    return _app;
}

ALWAYS_INLINE KERNEL_NS::EventManager *ILogicSys::GetEventMgr()
{
    return _eventMgr;
}

ALWAYS_INLINE const KERNEL_NS::EventManager *ILogicSys::GetEventMgr() const
{
    return _eventMgr;
}

ALWAYS_INLINE void ILogicSys::SetEventMgr(KERNEL_NS::EventManager *eventMgr)
{
    _eventMgr = eventMgr;
}

ALWAYS_INLINE KERNEL_NS::TimerMgr *ILogicSys::GetTimerMgr()
{
    return _timerMgr;
}

ALWAYS_INLINE const KERNEL_NS::TimerMgr *ILogicSys::GetTimerMgr() const
{
    return _timerMgr;
}

template<typename SysType>
ALWAYS_INLINE SysType *ILogicSys::GetSys()
{
    return GetComp<SysType>();
}

template<typename SysType>
ALWAYS_INLINE const SysType *ILogicSys::GetSys() const
{
    return GetComp<SysType>();
}

ALWAYS_INLINE void ILogicSys::FocusMethod(Int32 methodEnum)
{
    _intrestMethods.insert(methodEnum);
}

ALWAYS_INLINE bool ILogicSys::IsMethodFocus(Int32 methodEnum) const
{
    return _intrestMethods.find(methodEnum) != _intrestMethods.end();
}

SERVICE_END
