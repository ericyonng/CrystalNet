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
 * Date: 2021-01-12 23:59:25
 * Author: Eric Yonng
 * Description: TODO:等待日志模块完善
*/

#include <pch.h>
#include <testsuit/testinst/TestTls.h>


class BaseType
{
public:
    BaseType()
    {
    }

    virtual ~BaseType()
    {
        //InvokeDestruct();
        std::cout << "base free" << std::endl;
    }

    // virtual void InvokeDestruct() = 0;

    Byte8 _cha555;

};

class DeriveType : public BaseType
{
public:
    DeriveType()
    {
    }

    virtual ~DeriveType()
    {
        std::cout << "DeriveType free" << std::endl;
    }

//     virtual void InvokeDestruct()
//     {
//         this->~DeriveType();
//     }
public:
    Byte8 _cha;
};

void TestTls::Run() 
{
//     BaseType *ptr = reinterpret_cast<BaseType *>(new DeriveType());
//     ptr->~BaseType();

    // 初始化tls
    KERNEL_NS::TlsUtil::GetUtileTlsHandle();

    // 获取tlsstack
    auto tlsStack = KERNEL_NS::TlsUtil::GetTlsStack();
    auto tlsDef = tlsStack->GetDef();

    std::cout<< "tls def size = "<< sizeof(*tlsDef) << std::endl; 

    // 测试tls obj pool
    {
        auto objPool = KERNEL_NS::TlsUtil::GetTlsStack()->New<KERNEL_NS::TlsObjectPool<KERNEL_NS::ObjAlloctor<DeriveType>>>();
        auto pool = objPool->GetPool(1, KERNEL_NS::MemoryAlloctorConfig(sizeof(DeriveType), 1));
        auto obj = pool->New();

        pool->Delete(obj);
    }

    // 测试 tls memory pool
    {
        auto tlsPool = KERNEL_NS::TlsUtil::GetTlsStack()->New<KERNEL_NS::TlsMemoryPool>()->GetPoolAndCreate<KERNEL_NS::MemoryPool, KERNEL_NS::InitMemoryPoolInfo>("test tls");
        auto buffer = tlsPool->Alloc(1024);

        tlsPool->Free(buffer);

    }

    // 测试 tls memory alloctor
    {
        auto tlsAlloctor = KERNEL_NS::TlsUtil::GetTlsStack()->New<KERNEL_NS::TlsMemoryAlloctor>()->GetMemoryAlloctorAndCreate<KERNEL_NS::MemoryAlloctor, KERNEL_NS::MemoryAlloctorConfig>(1024, 1, "test tls");

        auto buffer = tlsAlloctor->Alloc(1024);

        tlsAlloctor->Free(buffer);
    }

    KERNEL_NS::TlsUtil::DestroyTlsStack();

}
