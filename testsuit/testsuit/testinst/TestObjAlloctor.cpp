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
 * Date: 2020-12-02 00:27:24
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestObjAlloctor.h>


static inline Int64 NowTime()
{
    // return std::chrono::system_clock::now().time_since_epoch().count() / 10;
    return KERNEL_NS::TimeUtil::GetMicroTimestamp();
}

class TestString
{
    POOL_CREATE_OBJ_DEFAULT(TestString);

public:
    TestString()
    {
        // std::cout << "test string construct" << std::endl;

    }
    virtual ~TestString()
    {
        //std::cout << "test string deconstruct" << std::endl;
    }
public:
   Byte8 str[4096];
};

POOL_CREATE_OBJ_DEFAULT_IMPL(TestString);

class ChildTestString : public TestString
{
    POOL_CREATE_OBJ_DEFAULT_P1(TestString, ChildTestString);

public:
    ChildTestString()
    {
        //std::cout << "ChildTestString construct" << std::endl;

    }
    virtual ~ChildTestString()
    {
       // std::cout << " ChildTestString deconstruct" << std::endl;
    }

public:
   Byte8 str2[128];
};

POOL_CREATE_OBJ_DEFAULT_IMPL(ChildTestString);


class TestSysTemStringObj
{
public:
    TestSysTemStringObj()
    {}
    virtual ~TestSysTemStringObj()
    {
    }

public:
    Byte8 str[4096];
};

class TestDeriveSysTemStringObj : public TestSysTemStringObj
{
public:
    TestDeriveSysTemStringObj()
    {}
    virtual ~TestDeriveSysTemStringObj()
    {}

public:
    Byte8 str2[128];
};

class TestDeriveAncestor
{
    POOL_CREATE_OBJ_DEFAULT(TestDeriveAncestor);

public:
    Byte8 str2[128];
};

POOL_CREATE_OBJ_DEFAULT_IMPL(TestDeriveAncestor);


class TestDeriveAncestorDerive : public TestDeriveAncestor
{
    POOL_CREATE_OBJ_DEFAULT_P1(TestDeriveAncestor, TestDeriveAncestorDerive);

public:
    TestDeriveAncestorDerive()
    {

    }
    ~TestDeriveAncestorDerive()
    {
        str3[0] = 1;
    }
    
	std::string ToString() const
	{
		return str3;
	}
public:
    Byte8 str3[128];
};

POOL_CREATE_OBJ_DEFAULT_IMPL(TestDeriveAncestorDerive);



class TestDeriveAncestorDerive2 : public TestDeriveAncestorDerive
{
    POOL_CREATE_OBJ_DEFAULT_P1(TestDeriveAncestorDerive, TestDeriveAncestorDerive2);

public:
    Byte8 str4[128];
};

class SmartPtrRaw 
{
    POOL_CREATE_OBJ_DEFAULT(SmartPtrRaw);

public:
    SmartPtrRaw()
    {

    }
};
POOL_CREATE_OBJ_DEFAULT_IMPL(TestDeriveAncestorDerive2);

POOL_CREATE_OBJ_DEFAULT_IMPL(SmartPtrRaw);

// TODO:测试性能+测试相邻内存的内存践踏，测试继承的内存践踏 性能与内存践踏已通过测试

#define  TEST_OBJ_ALLOCTOR_NUM 10000
void TestObjAlloctor::Run()
{
    // KERNEL_NS::ObjAlloctor<Int32> int32Alloctor; 
    // std::cout << int32Alloctor.ToString().c_str() << std::endl;

    // Int32 *int32Obj = int32Alloctor.NewNoConstruct();
    // *int32Obj = 15;
    // std::cout << "int obj=" << *int32Obj << std::endl;

    // std::cout << int32Alloctor.ToString().c_str() << std::endl;

    // int32Alloctor.DeleteNoDestructor(int32Obj);

    // std::cout << int32Alloctor.ToString().c_str() << std::endl;

     std::vector<ChildTestString *> *testStrPtrArr = new std::vector<ChildTestString *>;
     std::vector<TestDeriveSysTemStringObj *> *testSysStrPtrArr = new std::vector<TestDeriveSysTemStringObj *>;
     
    //  auto newPtr = SmartPtrRaw::NewByAdapter_SmartPtrRaw(KERNEL_NS::_Build::TL::V);
    //  newPtr = SmartPtrRaw::NewByAdapter_SmartPtrRaw(KERNEL_NS::_Build::MT::V);
    //  newPtr = SmartPtrRaw::New_SmartPtrRaw();
    //  newPtr = SmartPtrRaw::NewThreadLocal_SmartPtrRaw();

    // auto newDevi = TestDeriveAncestorDerive2::NewThreadLocal_TestDeriveAncestorDerive2();
//     auto size1 = sizeof(TestString);
//     auto size2 = sizeof(ChildTestString);
//     TestString *str1 = new TestString;
//     ChildTestString *str2 = new ChildTestString;
//     str1->str[4095] = 17;
//     ChildTestString::Delete(str2);

     std::cout << "begin obj pool info:" << std::endl;
     // std::cout << ChildTestString::GetAlloctor__ChildTestStringobjAlloctor().ToString().c_str() << std::endl;
     UInt64 beginTime = NowTime();
     for (Int32 i = 0; i < TEST_OBJ_ALLOCTOR_NUM; ++i)
     {
         testStrPtrArr->push_back(ChildTestString::New_ChildTestString());
     }

     UInt64 endTime = NowTime();
     printf("obj pool alloc ==== interval = %lld us\n", endTime - beginTime);

     beginTime = NowTime();
     for (Int32 i = 0; i < TEST_OBJ_ALLOCTOR_NUM; ++i)
     {
         testSysStrPtrArr->push_back(new TestDeriveSysTemStringObj);
     }
     endTime = NowTime();
     printf("system alloc ==== interval = %lld us\n", endTime - beginTime);

     printf("will free objs\n");

     beginTime = NowTime();
     for (Int32 i = 0; i < TEST_OBJ_ALLOCTOR_NUM; ++i)
     {
         ChildTestString::Delete_ChildTestString((*testStrPtrArr)[i]);
     }
     endTime = NowTime();
     printf("obj pool free ==== interval = %lld us\n", endTime - beginTime);


     beginTime = NowTime();
     for (Int32 i = 0; i < TEST_OBJ_ALLOCTOR_NUM; ++i)
     {
         delete (*testSysStrPtrArr)[i];
     }
     endTime = NowTime();
     printf("system free ==== interval = %lld us\n", endTime - beginTime);

    // 分配（触发了NewBuffer）系统是pool的2.6倍
    // 释放pool是系统的1.9倍
    
     std::cout << "obj pool info:" << std::endl;
    // std::cout << ChildTestString::GetAlloctor__ChildTestStringobjAlloctor().ToString().c_str() << std::endl;
    // ChildTestString *testRef = new ChildTestString();
    // testRef->AddRef();

    // ChildTestString testRef2 = *testRef;
    // bool isTest = *testRef;
     // getchar();
    // for (auto ptr : *testStrPtrArr)
    // {
    //     delete ptr;
    // }

     auto newDrive = new TestDeriveAncestorDerive;
     KERNEL_NS::AlloctorInfoCollector collector;
     newDrive->ObjPoolCollectThreadLocal_TestDeriveAncestorDerive(collector);
     std::cout<< collector.Result() << std::endl;

    auto newObj = TestDeriveAncestorDerive::New_TestDeriveAncestorDerive();
    newObj->AddRef_TestDeriveAncestorDerive();
    newObj->AddRef_TestDeriveAncestorDerive();
    newObj->AddRef_TestDeriveAncestorDerive();
    newObj->AddRef_TestDeriveAncestorDerive();

    TestDeriveAncestorDerive::Delete_TestDeriveAncestorDerive(newObj);
    TestDeriveAncestorDerive::Delete_TestDeriveAncestorDerive(newObj);
    TestDeriveAncestorDerive::Delete_TestDeriveAncestorDerive(newObj);
    TestDeriveAncestorDerive::Delete_TestDeriveAncestorDerive(newObj);
    TestDeriveAncestorDerive::Delete_TestDeriveAncestorDerive(newObj);
    
}