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
 * Date: 2021-09-07 12:39:10
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestDelegate.h>
#include<functional>

// class member
class DelegateClass1
{
public:
    void Print(Int32 idx)
    {
        g_Log->Info(LOGFMT_OBJ_TAG("idx=%d"), idx);
    }
};


// global function
static void Print(Int32 idx)
{
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestDelegate, "global scope idx:%d"), idx);
}

// lambda, std::function

void TestDelegate::Run()
{
    DelegateClass1 class1;
    {
        KERNEL_NS::SmartPtr<KERNEL_NS::IDelegate<void, Int32>, KERNEL_NS::AutoDelMethods::Release> delg = KERNEL_NS::DelegateFactory::Create(&class1, &DelegateClass1::Print);
        delg->Invoke(1);

        KERNEL_NS::SmartPtr<KERNEL_NS::IDelegate<void, Int32>, KERNEL_NS::AutoDelMethods::Release> copyDeleg = delg->CreateNewCopy();
        copyDeleg->Invoke(2);
    }

    {
        KERNEL_NS::SmartPtr<KERNEL_NS::IDelegate<void, Int32>, KERNEL_NS::AutoDelMethods::Release> delg = KERNEL_NS::DelegateFactory::Create(&Print);
        delg->Invoke(3);
    }

    {
        auto __invokeLambda = [](Int32 idx)->void
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestDelegate, "lambda idx:%d"), idx);
        };
        KERNEL_NS::SmartPtr<KERNEL_NS::IDelegate<void, Int32>, KERNEL_NS::AutoDelMethods::Release> delg = KERNEL_CREATE_CLOSURE_DELEGATE(__invokeLambda, void, Int32);
        delg->Invoke(4);
    }

        {
        auto __invokeLambda = [](Int32 idx)->void
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestDelegate, "lambda idx:%d"), idx);
        };
        std::function<void(Int32)> dynFunc = __invokeLambda;

        KERNEL_NS::SmartPtr<KERNEL_NS::IDelegate<void, Int32>, KERNEL_NS::AutoDelMethods::Release> delg = KERNEL_NS::DelegateFactory::Create<decltype(dynFunc), void, Int32>(dynFunc);
        delg->Invoke(5);
    }
}