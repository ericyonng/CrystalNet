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

    KERNEL_NS::TlsUtil::DestroyTlsStack();

}
