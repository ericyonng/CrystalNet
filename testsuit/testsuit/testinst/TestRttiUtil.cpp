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
 * Author: Eric Yonng
 * Date: 2021-01-29 14:45:51
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestRttiUtil.h>

class TestRttiObj
{
public:
    TestRttiObj(){}
    ~TestRttiObj(){
        UNUSED(_raw);
    }

private:
    Byte8 _raw[16];
};

class TestRttiObj2 : public TestRttiObj
{
public:
    TestRttiObj2();
    virtual ~TestRttiObj2(){}

private:
    Byte8 _raw2[16];
};

struct TestRttiStruct
{
    Byte8 _raw[16];
};

void TestRttiUtil::Run() 
{
    TestRttiObj obj;
    const auto &typeName = KERNEL_NS::RttiUtil::GetByType<TestRttiObj>();
    const auto typeId = KERNEL_NS::RttiUtil::GetTypeId<TestRttiObj>();
    const auto typeId2 = KERNEL_NS::RttiUtil::GetTypeId<TestRttiObj>();
    const auto typeId3 = KERNEL_NS::RttiUtil::GetTypeId<Int32>();
    const auto typeId4 = KERNEL_NS::RttiUtil::GetTypeId<TestRttiObj>();
    const auto &typeName2 = KERNEL_NS::RttiUtil::GetByObj(&obj);
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestRttiUtil, "typeName:%s, typeName2:%s, typeId:%llu, typeId2:%llu, typeId3:%llu, typeId4:%llu")
    , typeName.c_str(), typeName2.c_str(), typeId, typeId2, typeId3, typeId4);
    
    std::cout << "TestRttiObj class name = " << KERNEL_NS::RttiUtil::GetByType<TestRttiObj>() << std::endl;
    std::cout << "TestRttiObj2 class name = " << KERNEL_NS::RttiUtil::GetByType<TestRttiObj2>() << std::endl;
    std::cout << "TestRttiStruct class name = " << KERNEL_NS::RttiUtil::GetByType<TestRttiStruct>() << std::endl;
    std::cout << "Byte8 name = " << KERNEL_NS::RttiUtil::GetByType<Byte8>() << std::endl;
    std::cout << "U8 name = " << KERNEL_NS::RttiUtil::GetByType<U8>() << std::endl;
    std::cout << "Int16 name = " << KERNEL_NS::RttiUtil::GetByType<Int16>() << std::endl;
    std::cout << "UInt16 name = " << KERNEL_NS::RttiUtil::GetByType<UInt16>() << std::endl;
    std::cout << "Int32 name = " << KERNEL_NS::RttiUtil::GetByType<Int32>() << std::endl;
    std::cout << "UInt32 name = " << KERNEL_NS::RttiUtil::GetByType<UInt32>() << std::endl;
    std::cout << "Long name = " << KERNEL_NS::RttiUtil::GetByType<Long>() << std::endl;
    std::cout << "ULong name = " << KERNEL_NS::RttiUtil::GetByType<ULong>() << std::endl;
    std::cout << "Int64 name = " << KERNEL_NS::RttiUtil::GetByType<Int64>() << std::endl;
    std::cout << "UInt64 name = " << KERNEL_NS::RttiUtil::GetByType<UInt64>() << std::endl;
    std::cout << "Float name = " << KERNEL_NS::RttiUtil::GetByType<Float>() << std::endl;
    std::cout << "Double name = " << KERNEL_NS::RttiUtil::GetByType<Double>() << std::endl;
    std::cout << "bool name = " << KERNEL_NS::RttiUtil::GetByType<bool>() << std::endl;
    std::cout << "Int32 * name = " << KERNEL_NS::RttiUtil::GetByType<Int32 *>() << std::endl;
    std::cout << "TestRttiObj * name = " << KERNEL_NS::RttiUtil::GetByType<TestRttiObj *>() << std::endl;
    std::cout << "TestRttiObj2 * name = " << KERNEL_NS::RttiUtil::GetByType<TestRttiObj2 *>() << std::endl;
    std::cout << "TestRttiStruct * name = " << KERNEL_NS::RttiUtil::GetByType<TestRttiStruct *>() << std::endl;

    std::cout<< "Test rtti finish" << std::endl;
}
