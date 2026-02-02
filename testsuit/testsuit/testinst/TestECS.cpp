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
 * Date: 2022-09-03 21:36:05
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestECS.h>

// 组件A
class CompA : public KERNEL_NS::CompObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, CompA);

public:
    CompA()
    :KERNEL_NS::CompObject(KERNEL_NS::RttiUtil::GetTypeId<CompA>())
    {
        g_Log->Info(LOGFMT_OBJ_TAG("CompA constructor."));
    }

    ~CompA()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("CompA destructor."));
    }

    void Release() override
    {
        CompA::Delete_CompA(this);
    }

    // // api
public:
    virtual KERNEL_NS::LibString ToString() const override
    {
        return KERNEL_NS::CompObject::ToString().AppendFormat("comp name:%s", _name.c_str());
    }

    virtual void Clear() override
    {
        _Clear();
        KERNEL_NS::CompObject::Clear();
    }

    virtual void OnUpdate() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on update"), KERNEL_NS::RttiUtil::GetByObj(this).c_str());
    }

    // 组件接口资源
protected:
    virtual Int32 _OnCreated() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on created"), ToString().c_str());
        return Status::Success;
    }

    virtual Int32 _OnInit() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on init"), ToString().c_str());
        return Status::Success;
    }

    // start 可以启动线程，再此之前都不可以启动线程
    virtual Int32 _OnStart() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on start"), ToString().c_str());
        return Status::Success;
    }

    virtual void _OnWillClose() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on will close"), ToString().c_str());
    }

    virtual void _OnClose() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on close"), ToString().c_str());
    }

private:
    void _Clear()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s _Clear"), ToString().c_str());
    }


private:
    KERNEL_NS::LibString _name = "CompA name field";
};




// 组件B
class CompB : public KERNEL_NS::CompObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, CompB);
public:

    CompB()
    :KERNEL_NS::CompObject(KERNEL_NS::RttiUtil::GetTypeId<CompB>())
    {
        g_Log->Info(LOGFMT_OBJ_TAG("CompB constructor."));
    }

    ~CompB()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("CompB destructor."));
    }
    void Release() override
    {
        CompB::Delete_CompB(this);
    }

    // // api
public:
    virtual KERNEL_NS::LibString ToString() const override
    {
        return KERNEL_NS::CompObject::ToString().AppendFormat("comp name:%s", _name.c_str());
    }

    virtual void Clear() override
    {
        _Clear();
        KERNEL_NS::CompObject::Clear();
    }

    virtual void OnUpdate() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on update"), KERNEL_NS::RttiUtil::GetByObj(this).c_str());
    }

    // 组件接口资源
protected:
    virtual Int32 _OnCreated() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on created"), ToString().c_str());
        return Status::Success;
    }

    virtual Int32 _OnInit() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on init"), ToString().c_str());
        return Status::Success;
    }

    // start 可以启动线程，再此之前都不可以启动线程
    virtual Int32 _OnStart() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on start"), ToString().c_str());
        return Status::Success;
    }

    virtual void _OnWillClose() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on will close"), ToString().c_str());
    }

    virtual void _OnClose() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on close"), ToString().c_str());
    }

private:
    void _Clear()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s _Clear"), ToString().c_str());
    }
    
private:
    KERNEL_NS::LibString _name = "CompB name field";
};


// 故障组件3
class CompFault : public KERNEL_NS::CompObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, CompFault);
public:
    CompFault()
    :KERNEL_NS::CompObject(KERNEL_NS::RttiUtil::GetTypeId<CompFault>())
    {
        g_Log->Info(LOGFMT_OBJ_TAG("CompFault constructor."));
    }
    ~CompFault()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("CompFault destructor."));
    }
    
    void Release() override
    {
        CompFault::Delete_CompFault(this);
    }


    // // api
public:
    virtual KERNEL_NS::LibString ToString() const override
    {
        return KERNEL_NS::CompObject::ToString().AppendFormat("comp name:%s", _name.c_str());
    }

    virtual void Clear() override
    {
        _Clear();
        KERNEL_NS::CompObject::Clear();
    }

    virtual void OnUpdate() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on update"), KERNEL_NS::RttiUtil::GetByObj(this).c_str());
    }

    // 组件接口资源
protected:
    virtual Int32 _OnCreated() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on created"), ToString().c_str());
        return Status::Success;
    }

    virtual Int32 _OnInit() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on init"), ToString().c_str());
        // SetErrCode(NULL, Status::Failed);
        // return Status::Failed;
        return Status::Success;
    }

    // start 可以启动线程，再此之前都不可以启动线程
    virtual Int32 _OnStart() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on start"), ToString().c_str());
        return Status::Success;
    }

    virtual void _OnWillClose() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on will close"), ToString().c_str());
    }

    virtual void _OnClose() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on close"), ToString().c_str());
    }

private:
    void _Clear()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s _Clear"), ToString().c_str());
    }

private:
    KERNEL_NS::LibString _name = "CompFault name field";
};




// 宿主
class HostA : public KERNEL_NS::CompHostObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, HostA);
public:
    HostA()
    :KERNEL_NS::CompHostObject(KERNEL_NS::RttiUtil::GetTypeId<HostA>())
    {
        g_Log->Info(LOGFMT_OBJ_TAG("HostA constructor."));
    }
    ~HostA()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("HostA destructor."));
    }
    void Release() override
    {
        HostA::Delete_HostA(this);
    }


    // // api
public:
    virtual KERNEL_NS::LibString ToString() const override
    {
        return KERNEL_NS::CompHostObject::ToString().AppendFormat("comp name:%s", _name.c_str());
    }

    virtual void Clear() override
    {
        _Clear();
        KERNEL_NS::CompHostObject::Clear();
    }

    virtual void OnRegisterComps() override;

    // 组件接口资源
protected:
    virtual Int32 _OnHostCreated() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on created"), ToString().c_str());
        return Status::Success;
    }

    // 在组件初始化前 必须重写
    virtual Int32 _OnHostInit() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on host init."), ToString().c_str());
        return Status::Success;
    }

    // 所有组件创建完成
    virtual Int32 _OnCompsCreated() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on host comps created."), ToString().c_str());
        return Status::Success;
    }

    // 在组件启动之前 请勿在WillStart及之前的接口启动线程，此时都是线程不安全的状态
    virtual Int32 _OnHostWillStart() override
    { 
        g_Log->Info(LOGFMT_OBJ_TAG("%s on host will start."), ToString().c_str());
        return Status::Success; 
    }

    // 组件启动之后 此时可以启动线程 必须重写
    virtual Int32 _OnHostStart() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on host start."), ToString().c_str());
        return Status::Success;
    }

    // 在组件willclose之前 host若内部有多线程,应该要在组件的willclose之前结束掉线程,避免资源的竞争
    virtual void _OnHostBeforeCompsWillClose() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on host before comps will close."), ToString().c_str());
    }

    // 在组件willclose之后
    virtual void _OnHostWillClose() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on host will close."), ToString().c_str());
    }

    // 在组件close之前
    virtual void _OnHostBeforeCompsClose() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on host before comps close."), ToString().c_str());
    }

    // 在组件Close之后
    virtual void _OnHostClose() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on host close."), ToString().c_str());
    }

    // 在组件更新之前
    virtual void _OnWillHostUpdate() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on will host update before comps."), ToString().c_str());
    }

    // 在组件更新之后
    virtual void _OnHostUpdate() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on host update after comps."), ToString().c_str());
    }

private:
    void _Clear()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s _Clear"), ToString().c_str());
    }

private:
    KERNEL_NS::LibString _name = "HostA name field";
};




// 宿主2
class HostB : public KERNEL_NS::CompHostObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, HostB);

public:
    HostB()
    :KERNEL_NS::CompHostObject(KERNEL_NS::RttiUtil::GetTypeId<HostB>())
    {

    }
    ~HostB()
    {

    }

    void Release() override
    {
        HostB::Delete_HostB(this);
    }


    // api
public:
    virtual KERNEL_NS::LibString ToString() const override
    {
        return KERNEL_NS::CompHostObject::ToString().AppendFormat("comp name:%s", _name.c_str());
    }

    virtual void Clear() override
    {
        _Clear();
        KERNEL_NS::CompHostObject::Clear();
    }
    
    virtual void OnRegisterComps() override;

    // 组件接口资源
protected:
    virtual Int32 _OnHostCreated() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on created"), ToString().c_str());
        return Status::Success;
    }

    // 在组件初始化前 必须重写
    virtual Int32 _OnHostInit() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on host init."), ToString().c_str());
        return Status::Success;
    }

    // 所有组件创建完成
    virtual Int32 _OnCompsCreated() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on host comps created."), ToString().c_str());
        return Status::Success;
    }

    // 在组件启动之前 请勿在WillStart及之前的接口启动线程，此时都是线程不安全的状态
    virtual Int32 _OnHostWillStart() override
    { 
        g_Log->Info(LOGFMT_OBJ_TAG("%s on host will start."), ToString().c_str());
        return Status::Success; 
    }

    // 组件启动之后 此时可以启动线程 必须重写
    virtual Int32 _OnHostStart() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on host start."), ToString().c_str());
        return Status::Success;
    }

    // 在组件willclose之前 host若内部有多线程,应该要在组件的willclose之前结束掉线程,避免资源的竞争
    virtual void _OnHostBeforeCompsWillClose() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on host before comps will close."), ToString().c_str());
    }

    // 在组件willclose之后
    virtual void _OnHostWillClose() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on host will close."), ToString().c_str());
    }

    // 在组件close之前
    virtual void _OnHostBeforeCompsClose() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on host before comps close."), ToString().c_str());
    }

    // 在组件Close之后
    virtual void _OnHostClose() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on host close."), ToString().c_str());
    }

    // 在组件更新之前
    virtual void _OnWillHostUpdate() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on will host update before comps."), ToString().c_str());
    }

    // 在组件更新之后
    virtual void _OnHostUpdate() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on host update after comps."), ToString().c_str());
    }

private:
    void _Clear()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s _Clear"), ToString().c_str());
    }

private:
    KERNEL_NS::LibString _name = "HostB name field";
};



class ICompC : public KERNEL_NS::CompObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, ICompC);
public:
    ICompC(UInt64 objTypeId)
    :KERNEL_NS::CompObject(objTypeId)
    {

    }

    ~ICompC()
    {

    }

};


// 简化版 组件C
class CompC : public ICompC
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, CompC);

public:
    CompC()
    :ICompC(KERNEL_NS::RttiUtil::GetTypeId<CompC>())
    {
        g_Log->Info(LOGFMT_OBJ_TAG("CompC constructor"));
    }

    ~CompC()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("CompC destructor"));
        _Clear();
    }

    void Release() override
    {
        CompC::Delete_CompC(this);
    }

protected:
    virtual Int32 _OnInit() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on init"), ToString().c_str());
        return Status::Success;
    }

private:
    void _Clear()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s _Clear"), ToString().c_str());
    }

private:
    KERNEL_NS::LibString _name = "CompC name field";
    
};



// 简化版 组件C
class CompD : public KERNEL_NS::CompObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, CompD);

public:
    CompD()
    // 故意传错
    :KERNEL_NS::CompObject(KERNEL_NS::RttiUtil::GetTypeId<CompD>())
    {
        g_Log->Info(LOGFMT_OBJ_TAG("CompD constructor"));
    }

    ~CompD()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("CompD destructor"));
    }

    void Release() override
    {
        CompD::Delete_CompD(this);
    }

    void Hello()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("hello compd"));
    }
};


// 简化版 组件C
class CompE : public KERNEL_NS::CompObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, CompE);

public:
    CompE()
    // 故意传错
    :KERNEL_NS::CompObject(KERNEL_NS::RttiUtil::GetTypeId<CompE>())
    {
        g_Log->Info(LOGFMT_OBJ_TAG("CompE constructor"));
    }

    ~CompE()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("CompE destructor"));
    }

    void Release() override
    {
        CompE::Delete_CompE(this);
    }

    void Hello()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("hello CompE"));
    }
};



// 宿主3 简化版的Host
class HostC : public KERNEL_NS::CompHostObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, HostC);

public:
    HostC()
    :KERNEL_NS::CompHostObject(KERNEL_NS::RttiUtil::GetTypeId<HostC>())
    {

    }

    ~HostC()
    {
        _Clear();
    }

    void Release() override
    {
        HostC::Delete_HostC(this);
    }
    
    virtual void OnRegisterComps() override;

    // 组件接口资源
protected:

    // 在组件初始化前 必须重写
    virtual Int32 _OnHostInit() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on host init."), ToString().c_str());
        return Status::Success;
    }

    // 组件启动之后 此时可以启动线程 必须重写
    virtual Int32 _OnHostStart() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on host start."), ToString().c_str());
        return Status::Success;
    }

    // 在组件Close之后
    virtual void _OnHostClose() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on host close."), ToString().c_str());
    }

private:
    void _Clear()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s _Clear"), ToString().c_str());
    }

private:
    KERNEL_NS::LibString _name = "HostC name field";
};




// 工厂
class CompAFactory : public KERNEL_NS::CompFactory
{
public:
    static constexpr KERNEL_NS::_Build::MT _buildType{};

    static KERNEL_NS::CompFactory *FactoryCreate()
    {
       return KERNEL_NS::ObjPoolWrap<CompAFactory>::NewByAdapter(_buildType.V);
    }

    void Release() override
    {
        KERNEL_NS::ObjPoolWrap<CompAFactory>::DeleteByAdapter(_buildType.V, this);
    }


    virtual KERNEL_NS::CompObject *Create() const override
    {
        return CompA::NewByAdapter_CompA(_buildType.V);
    }


};



class CompBFactory : public KERNEL_NS::CompFactory
{
public:
    static constexpr KERNEL_NS::_Build::MT _buildType{};
    
    static KERNEL_NS::CompFactory *FactoryCreate()
    {
       return KERNEL_NS::ObjPoolWrap<CompBFactory>::NewByAdapter(_buildType.V);
    }

    void Release() override
    {
        KERNEL_NS::ObjPoolWrap<CompBFactory>::DeleteByAdapter(_buildType.V, this);
    }

    virtual KERNEL_NS::CompObject *Create() const override
    {
        return CompB::NewByAdapter_CompB(_buildType.V);
    }


};

class CompFaultFactory : public KERNEL_NS::CompFactory
{
public:
    static constexpr KERNEL_NS::_Build::MT _buildType{};
    static KERNEL_NS::CompFactory *FactoryCreate()
    {
       return KERNEL_NS::ObjPoolWrap<CompFaultFactory>::NewByAdapter(_buildType.V);
    }

    void Release() override
    {
        KERNEL_NS::ObjPoolWrap<CompFaultFactory>::DeleteByAdapter(_buildType.V, this);
    }
    virtual KERNEL_NS::CompObject *Create() const override
    {
        return CompFault::NewByAdapter_CompFault(_buildType.V);
    }


};


class CompCFactory : public KERNEL_NS::CompFactory
{
public:
    static constexpr KERNEL_NS::_Build::MT _buildType{};
    static KERNEL_NS::CompFactory *FactoryCreate()
    {
       return KERNEL_NS::ObjPoolWrap<CompCFactory>::NewByAdapter(_buildType.V);
    }
    void Release() override
    {
        KERNEL_NS::ObjPoolWrap<CompCFactory>::DeleteByAdapter(_buildType.V, this);
    }


    virtual KERNEL_NS::CompObject *Create() const override
    {
        CREATE_CRYSTAL_COMP(comp, CompC);
        return comp;
    }

};


class HostAFactory : public KERNEL_NS::CompFactory
{
public:
    static constexpr KERNEL_NS::_Build::MT _buildType{};
    static KERNEL_NS::CompFactory *FactoryCreate()
    {
       return KERNEL_NS::ObjPoolWrap<HostAFactory>::NewByAdapter(_buildType.V);
    }
    void Release() override
    {
        KERNEL_NS::ObjPoolWrap<HostAFactory>::DeleteByAdapter(_buildType.V, this);
    }
    virtual KERNEL_NS::CompObject *Create() const override
    {
        return HostA::NewByAdapter_HostA(_buildType.V);
    }

};


class HostBFactory : public KERNEL_NS::CompFactory
{
public:
    static constexpr KERNEL_NS::_Build::MT _buildType{};
    static KERNEL_NS::CompFactory *FactoryCreate()
    {
       return KERNEL_NS::ObjPoolWrap<HostAFactory>::NewByAdapter(_buildType.V);
    }
    void Release() override
    {
        KERNEL_NS::ObjPoolWrap<HostBFactory>::DeleteByAdapter(_buildType.V, this);
    }
    virtual KERNEL_NS::CompObject *Create() const override
    {
        return HostB::NewByAdapter_HostB(_buildType.V);
    }

};


class HostCFactory : public KERNEL_NS::CompFactory
{
public:
    static constexpr KERNEL_NS::_Build::MT _buildType{};
    static KERNEL_NS::CompFactory *FactoryCreate()
    {
       return KERNEL_NS::ObjPoolWrap<HostCFactory>::NewByAdapter(_buildType.V);
    }
    void Release() override
    {
        KERNEL_NS::ObjPoolWrap<HostCFactory>::DeleteByAdapter(_buildType.V, this);
    }
    virtual KERNEL_NS::CompObject *Create() const override
    {
        return HostC::NewByAdapter_HostC(_buildType.V);
    }
};



void HostA::OnRegisterComps()
{
    RegisterComp<CompAFactory>();
    RegisterComp<CompBFactory>();
    RegisterComp<CompCFactory>();

    // 测试循环依赖！！！，错误的示范：A <= C <= A ...
    RegisterComp<HostCFactory>();
}

void HostB::OnRegisterComps()
{   
    RegisterComp<CompAFactory>();
    RegisterComp<CompBFactory>();
    // RegisterComp<CompFaultFactory>();
    // RegisterComp<CompCFactory>();
}

void HostC::OnRegisterComps()
{
    // 注意循环依赖,若HostC中有HostA, HostA中也有HostC那么将导致死循环
    RegisterComp<CompAFactory>();
    RegisterComp<CompBFactory>();
    RegisterComp<HostAFactory>();
    RegisterComp<CompCFactory>();
}

// 测试循环依赖: HostA <= HostC <= HostA <= ...
// void TestECS::Run()
// {
//     do
//     {
//         auto hosta = HostA::New_HostA();
//         auto st = hosta->Init();
//         if(st != Status::Success)
//         {
//             g_Log->Error(LOGFMT_NON_OBJ_TAG(TestECS, "host a init fail st:%d"), st);
//             break;
//         }

//         st = hosta->Start();
//         if(st != Status::Success)
//         {
//             g_Log->Error(LOGFMT_NON_OBJ_TAG(TestECS, "hosta start fail st:%d"), st);
//             break;
//         }

//         getchar();

//         hosta->WillClose();

//         hosta->Close();

//         hosta->Release();

//         /* code */
//     } while (0);
    
// }
// void TestECS::Run()
// {
//     do
//     {
//         KERNEL_NS::SmartPtr<HostA, KERNEL_NS::AutoDelMethods::Release> hosta = HostA::New_HostA();
//         auto st = hosta->Init();
//         if(st != Status::Success)
//         {
//             g_Log->Error(LOGFMT_NON_OBJ_TAG(TestECS, "host a init fail st:%d"), st);
//             break;
//         }

//         st = hosta->Start();
//         if(st != Status::Success)
//         {
//             g_Log->Error(LOGFMT_NON_OBJ_TAG(TestECS, "host a start fail st:%d"), st);
//             break;
//         }

//         KERNEL_NS::SmartPtr<HostB, KERNEL_NS::AutoDelMethods::Release> hostb = HostB::New_HostB();
//         st = hostb->Init();
//         if(st != Status::Success)
//         {
//             g_Log->Error(LOGFMT_NON_OBJ_TAG(TestECS, "host b init fail st:%d"), st);
//             break;
//         }
//         st = hostb->Start();
//         if(st != Status::Success)
//         {
//             g_Log->Error(LOGFMT_NON_OBJ_TAG(TestECS, "host b init start st:%d"), st);
//             break;
//         }

//         KERNEL_NS::SmartPtr<HostC, KERNEL_NS::AutoDelMethods::Release> hostc = HostC::New_HostC();
//         st = hostc->Init();
//         if(st != Status::Success)
//         {
//             g_Log->Error(LOGFMT_NON_OBJ_TAG(TestECS, "host b init fail st:%d"), st);
//             break;
//         }
//         st = hostc->Start();
//         if(st != Status::Success)
//         {
//             g_Log->Error(LOGFMT_NON_OBJ_TAG(TestECS, "host b init start st:%d"), st);
//             break;
//         }

//         hosta->WillClose();
//         hosta->Close();

//         hostb->WillClose();
//         hostb->Close();

//         hostc->WillClose();
//         hostc->Close();
//     } while (0);
// }

// 测试替换组件
void TestECS::Run()
{
    do
    {
        auto hostb = HostB::New_HostB();
        
        // 动态添加组件
        KERNEL_NS::CompObject *compc = CompC::NewByAdapter_CompC(CompCFactory::_buildType.V);
        hostb->RegisterComp(compc);

        hostb->RegisterComp<CompFaultFactory>();

        auto st = hostb->Init();
        if(st != Status::Success)
        {
            g_Log->Error(LOGFMT_NON_OBJ_TAG(TestECS, "hostb init fail st:%d"), st);
            break;
        }

        st = hostb->Start();
        if(st != Status::Success)
        {
            g_Log->Error(LOGFMT_NON_OBJ_TAG(TestECS, "hostb start fail st:%d"), st);
            break;
        }

        // 替换组件c
        compc = CompC::NewByAdapter_CompC(CompCFactory::_buildType.V);
        compc->Init();
        compc->Start();

        hostb->ReplaceComp(compc);

        // 动态移除组件
        hostb->RemoveComp<CompC>();

        auto compd = CompD::New_CompD();
        compd->Init();
        compd->Start();
        hostb->AttachComp(compd);
        auto attachD = hostb->GetComp<CompD>();
        if(attachD)
            attachD->Hello();

        auto compE = CompE::New_CompE();
        compE->Init();
        compE->Start();
        hostb->AddComp(compE);
        auto compE2 = CompE::New_CompE();
        compE2->Init();
        compE2->Start();
        hostb->AttachComp(compE2);
        auto e = hostb->GetComp<CompE>();
        e->Hello();

        auto popD = hostb->PopComp<CompD>();
        popD->WillClose();
        popD->Close();
        CompD::Delete_CompD(popD);
        auto d = hostb->GetComp<CompD>();
        d->Hello();

        getchar();

        hostb->WillClose();

        hostb->Close();

        hostb->Release();

        /* code */
    } while (0);
    
}