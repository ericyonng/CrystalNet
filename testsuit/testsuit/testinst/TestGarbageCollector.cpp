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
 * Date: 2020-12-09 01:37:35
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestGarbageCollector.h>

static inline Int64 NowTime()
{
    // return std::chrono::system_clock::now().time_since_epoch().count() / 10;
    return KERNEL_NS::TimeUtil::GetMicroTimestamp();
}

class TestGarbageObj
{
    POOL_CREATE_OBJ_DEFAULT(TestGarbageObj);
public:
    TestGarbageObj(){}
    ~TestGarbageObj(){}

    Byte8 str[4096];
};



class TestSysTemStringObj
{
public:
    TestSysTemStringObj()
    {}
    ~TestSysTemStringObj()
    {
    }

public:
    Byte8 str[4096];
};

#define TEST_OBJ_ALLOCTOR_NUM 1024000

void TestGarbageCollector::Run()
{
    // 启动垃圾回收线程
    KERNEL_NS::GarbageThread::GetInstence()->Start();

    std::vector<TestGarbageObj *> *testStrPtrArr = new std::vector<TestGarbageObj *>;
    std::vector<TestSysTemStringObj *> *testSysPtrArr = new std::vector<TestSysTemStringObj *>;

     UInt64 beginTime = NowTime();
     for (Int32 i = 0; i < TEST_OBJ_ALLOCTOR_NUM; ++i)
     {
         testStrPtrArr->push_back(new TestGarbageObj);
     }

     UInt64 endTime = NowTime();
     printf("obj pool alloc ==== interval = %lld us\n", endTime - beginTime);

     beginTime = NowTime();
     for (Int32 i = 0; i < TEST_OBJ_ALLOCTOR_NUM; ++i)
     {
         testSysPtrArr->push_back(new TestSysTemStringObj);
     }

     endTime = NowTime();
     printf("system alloc ==== interval = %lld us\n", endTime - beginTime);

     printf("will free objs\n");
     getchar();

    beginTime = NowTime();
    for(auto &ptr : *testStrPtrArr)
    {
        CRYSTAL_DELETE_SAFE(ptr);
    }
    endTime = NowTime();
    printf("obj pool free ==== interval = %lld us\n", endTime - beginTime);

    beginTime = NowTime();
    for (auto &ptr : *testSysPtrArr)
    {
        CRYSTAL_DELETE_SAFE(ptr);
    }
    endTime = NowTime();
    printf("system free ==== interval = %lld us\n", endTime - beginTime);

    KERNEL_NS::GarbageThread::GetInstence()->Close();
}