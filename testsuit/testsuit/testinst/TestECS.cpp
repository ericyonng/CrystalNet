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
    {
        g_Log->Info(LOGFMT_OBJ_TAG("CompA constructor."));
    }

    ~CompA()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("CompA destructor."));
    }

    void Release()
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
        g_Log->Info(LOGFMT_OBJ_TAG("%s on update"), KERNEL_NS::RttiUtil::GetByObj(this));
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

POOL_CREATE_OBJ_DEFAULT_IMPL(CompA);


// 组件B
class CompB : public KERNEL_NS::CompObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, CompB);
public:
    CompB()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("CompB constructor."));
    }

    ~CompB()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("CompB destructor."));
    }
    void Release()
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
        g_Log->Info(LOGFMT_OBJ_TAG("%s on update"), KERNEL_NS::RttiUtil::GetByObj(this));
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

POOL_CREATE_OBJ_DEFAULT_IMPL(CompB);

// 故障组件3
class CompFault : public KERNEL_NS::CompObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, CompFault);
public:
    CompFault()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("CompFault constructor."));
    }
    ~CompFault()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("CompFault destructor."));
    }
    
    void Release()
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
        g_Log->Info(LOGFMT_OBJ_TAG("%s on update"), KERNEL_NS::RttiUtil::GetByObj(this));
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

POOL_CREATE_OBJ_DEFAULT_IMPL(CompFault);


// 宿主
class HostA : public KERNEL_NS::CompHostObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, HostA);
public:
    HostA()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("HostA constructor."));
    }
    ~HostA()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("HostA destructor."));
    }
    void Release()
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

    virtual void OnRegisterComps();

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
    virtual void _OnHostClose()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on host close."), ToString().c_str());
    }

    // 在组件更新之前
    virtual void _OnWillHostUpdate() 
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on will host update before comps."), ToString().c_str());
    }

    // 在组件更新之后
    virtual void _OnHostUpdate() 
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

POOL_CREATE_OBJ_DEFAULT_IMPL(HostA);


// 宿主2
class HostB : public KERNEL_NS::CompHostObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, HostB);

public:
    HostB()
    {

    }
    ~HostB()
    {

    }

    void Release()
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
    
    virtual void OnRegisterComps();

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
    virtual void _OnHostClose()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on host close."), ToString().c_str());
    }

    // 在组件更新之前
    virtual void _OnWillHostUpdate() 
    {
        g_Log->Info(LOGFMT_OBJ_TAG("%s on will host update before comps."), ToString().c_str());
    }

    // 在组件更新之后
    virtual void _OnHostUpdate() 
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

POOL_CREATE_OBJ_DEFAULT_IMPL(HostB);

class ICompC : public KERNEL_NS::CompObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, ICompC);
public:
    ICompC()
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
    {
        g_Log->Info(LOGFMT_OBJ_TAG("CompC constructor"));
    }

    ~CompC()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("CompC destructor"));
        _Clear();
    }

    void Release()
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
POOL_CREATE_OBJ_DEFAULT_IMPL(ICompC);
POOL_CREATE_OBJ_DEFAULT_IMPL(CompC);


// 宿主3 简化版的Host
class HostC : public KERNEL_NS::CompHostObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, HostC);

public:
    HostC()
    {

    }

    ~HostC()
    {
        _Clear();
    }

    void Release()
    {
        HostC::Delete_HostC(this);
    }
    
    virtual void OnRegisterComps();

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
    virtual void _OnHostClose()
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

POOL_CREATE_OBJ_DEFAULT_IMPL(HostC);


// 工厂
class CompAFactory : public KERNEL_NS::CompFactory
{
public:
    static constexpr KERNEL_NS::_Build::MT _buildType{};

    static KERNEL_NS::CompFactory *FactoryCreate()
    {
       return KERNEL_NS::ObjPoolWrap<CompAFactory>::NewByAdapter(_buildType.V);
    }

    void Release()
    {
        KERNEL_NS::ObjPoolWrap<CompAFactory>::DeleteByAdapter(_buildType.V, this);
    }


    virtual KERNEL_NS::CompObject *Create() const
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

    void Release()
    {
        KERNEL_NS::ObjPoolWrap<CompBFactory>::DeleteByAdapter(_buildType.V, this);
    }

    virtual KERNEL_NS::CompObject *Create() const
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

    void Release()
    {
        KERNEL_NS::ObjPoolWrap<CompFaultFactory>::DeleteByAdapter(_buildType.V, this);
    }
    virtual KERNEL_NS::CompObject *Create() const
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
    void Release()
    {
        KERNEL_NS::ObjPoolWrap<CompCFactory>::DeleteByAdapter(_buildType.V, this);
    }


    virtual KERNEL_NS::CompObject *Create() const
    {
        return CompC::NewByAdapter_CompC(_buildType.V);
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
    void Release()
    {
        KERNEL_NS::ObjPoolWrap<HostAFactory>::DeleteByAdapter(_buildType.V, this);
    }
    virtual KERNEL_NS::CompObject *Create() const
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
    void Release()
    {
        KERNEL_NS::ObjPoolWrap<HostBFactory>::DeleteByAdapter(_buildType.V, this);
    }
    virtual KERNEL_NS::CompObject *Create() const
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
    void Release()
    {
        KERNEL_NS::ObjPoolWrap<HostCFactory>::DeleteByAdapter(_buildType.V, this);
    }
    virtual KERNEL_NS::CompObject *Create() const
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
        // KERNEL_NS::CompObject *compc = CompC::NewByAdapter_CompC(CompCFactory::_buildType.V);
        // compc->Init();
        // compc->Start();

        // hostb->ReplaceComp(compc);

        // 动态移除组件
        hostb->RemoveComp<CompC>();

        getchar();

        hostb->WillClose();

        hostb->Close();

        hostb->Release();

        /* code */
    } while (0);
    
}