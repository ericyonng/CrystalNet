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
 * Date: 2021-01-24 21:16:08
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestStatic.h>

static KERNEL_NS::SpinLock s_TestStaticLck;
static std::atomic<Int32> s_threadCount{0};
#define TEST_STATIC_THREAD_NUM 2

class TestStaticObj2
{
public:
    TestStaticObj2(const Byte8 *name);

    void Print();

private:
    std::string _name;
};

TestStaticObj2::TestStaticObj2(const Byte8 *name)
    :_name(name)
{
    s_TestStaticLck.Lock();
    std::cout<< "TestStaticObj2 constructor create from thread:"<< name << std::endl;
    s_TestStaticLck.Unlock();
}

void TestStaticObj2::Print()
{
    s_TestStaticLck.Lock();
    std::cout<< "Print TestStaticObj2 create from thread:"<< _name.c_str() << std::endl;
    s_TestStaticLck.Unlock();
}

class TestStaticObj
{
public:
    static void Test(const Byte8 *ThreadName);
};

void TestStaticObj::Test(const Byte8 *ThreadName)
{
    static TestStaticObj2 *obj = new TestStaticObj2(ThreadName);
    obj->Print();
}

static void ThreadWorker1(KERNEL_NS::LibThread *thread)
{
    KERNEL_NS::LibString name;
    name.AppendFormat("ThreadWorker1 thread id[%llu] ", thread->GetTheadId());

    ++s_threadCount;

    // 保证两个线程同时进入
    while (s_threadCount.load() < TEST_STATIC_THREAD_NUM);
    
    TestStaticObj::Test(name.c_str());
}

static void ThreadWorker2(KERNEL_NS::LibThread *thread)
{
    KERNEL_NS::LibString name;
    name.AppendFormat("ThreadWorker2 thread id[%llu] ", thread->GetTheadId());

    ++s_threadCount;

    // 保证两个线程同时进入
    while (s_threadCount.load() < TEST_STATIC_THREAD_NUM);

    TestStaticObj::Test(name.c_str());
}

void TestStatic::Run() 
{
    KERNEL_NS::LibThread thread1;
    KERNEL_NS::LibThread thread2;
    thread1.AddTask(&ThreadWorker1);
    thread2.AddTask(&ThreadWorker2);

    thread1.Start();
    thread2.Start();

    getchar();

    thread1.HalfClose();
    thread2.HalfClose();

    thread1.FinishClose();
    thread2.FinishClose();

}
