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
 * Date: 2022-12-16 23:06:22
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestDestructor.h>

class TestType
{
public:
    TestType()
    {

    }

    ~TestType()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("test type destructor."));
    }

    Int32 _array[10] = {0};
};

class TestType2
{
    POOL_CREATE_OBJ_DEFAULT(TestType2);
public:
    TestType2()
    {

    }

    ~TestType2()
    {
        g_Log->Info(LOGFMT_OBJ_TAG("test type destructor."));
    }
    
    Int32 _array[10] = {0};
};

POOL_CREATE_OBJ_DEFAULT_IMPL(TestType2);

void TestDestructor::Run()
{
    // 测试调用析构
    auto mem = reinterpret_cast<TestType *>(KERNEL_NS::KernelAllocMemory<KERNEL_NS::_Build::TL>(sizeof(TestType)));
    KERNEL_NS::Destructor::Invoke(mem);
    KERNEL_NS::KernelFreeMemory<KERNEL_NS::_Build::TL>(mem);

    Int32 *ptr = reinterpret_cast<Int32 *>(KERNEL_NS::KernelAllocMemory<KERNEL_NS::_Build::TL>(sizeof(Int32)));
    KERNEL_NS::Destructor::Invoke(ptr);
    KERNEL_NS::KernelFreeMemory<KERNEL_NS::_Build::TL>(ptr);

    // 测试调用对象池释放
    auto mem2 = TestType2::New_TestType2();
    TestType2::Delete_TestType2(mem2);
}