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
 * Date: 2022-09-25 02:33:48
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestInlineStatic.h>


// 宿主3 简化版的Host
class TestStaticInlineHostC : public KERNEL_NS::CompHostObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, TestStaticInlineHostC);

public:
    TestStaticInlineHostC()
    :KERNEL_NS::CompHostObject(KERNEL_NS::RttiUtil::GetTypeId<TestStaticInlineHostC>())
    {

    }

    ~TestStaticInlineHostC()
    {
        _Clear();
    }

    void Release() override
    {
        TestStaticInlineHostC::Delete_TestStaticInlineHostC(this);
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

POOL_CREATE_OBJ_DEFAULT_IMPL(TestStaticInlineHostC);

void TestStaticInlineHostC::OnRegisterComps()
{
    RegisterComp<KERNEL_NS::PollerFactory>();
    RegisterComp<KERNEL_NS::IpRuleMgrFactory>();
}

void TestInlineStatic::Run()
{
    #if CRYSTAL_TARGET_PLATFORM_LINUX

    // KERNEL_NS::TcpPollerMgr *ptr = NULL;
    // KERNEL_NS::TcpPollerInstConfig *ptr2 = NULL;
    // auto newPoller = KERNEL_NS::EpollTcpPoller::New_EpollTcpPoller(ptr, 0, ptr2);
    // newPoller->Init();
    
    #endif
}

// void TestInlineStatic::Run()
// {
//     auto hostc = TestStaticInlineHostC::New_TestStaticInlineHostC();
//     auto hostc2 = TestStaticInlineHostC::New_TestStaticInlineHostC();
//     auto st = hostc->Init();
//     if(st != Status::Success)
//     {
//         g_Log->Warn(LOGFMT_NON_OBJ_TAG(TestInlineStatic, "hostc init fail st:%d"), st);
//         return;
//     }

//     st = hostc2->Init();
//     if(st != Status::Success)
//     {
//         g_Log->Warn(LOGFMT_NON_OBJ_TAG(TestInlineStatic, "hostc init fail st:%d"), st);
//         return;
//     }

//     st = hostc->Start();
//     if(st != Status::Success)
//     {
//         g_Log->Warn(LOGFMT_NON_OBJ_TAG(TestInlineStatic, "hostc start fail st:%d"), st);
//         return;
//     }

//     st = hostc2->Start();
//     if(st != Status::Success)
//     {
//         g_Log->Warn(LOGFMT_NON_OBJ_TAG(TestInlineStatic, "hostc start fail st:%d"), st);
//         return;
//     }

//     hostc->WillClose();
//     hostc->Close();

//     hostc2->WillClose();
//     hostc2->Close();

//     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestInlineStatic, "test host c finish"));
// }
