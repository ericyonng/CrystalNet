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
 * Date: 2025-01-05 18:09:13
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_KERNEL_FINALLY_KERNEL_FINALLY_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_KERNEL_FINALLY_KERNEL_FINALLY_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>

#include "kernel/comp/Delegate/IDelegate.h"
#include "kernel/comp/Delegate/LibDelegate.h"

#ifdef CRYSTAL_NET_CPP20
 #include <concepts>
#endif

KERNEL_BEGIN

/*
 * 宏使用:
 *  KERNEL_FINALLY_BEGIN(arg)
    {
        To do something finally.
    }
    KERNEL_FINALLY_END();
 */
class KERNEL_EXPORT KernelFinally
{
public:
    explicit KernelFinally(IDelegate<void> *delg)
    :_finalAction(delg)
    {
        
    }

    template<typename LambdaType>
#ifdef CRYSTAL_NET_CPP20
    requires requires(LambdaType l)
    {
        {l()}->std::same_as<void>;
    }
#endif
    KernelFinally(LambdaType &&lamb)
    :_finalAction(KERNEL_CREATE_CLOSURE_DELEGATE(lamb, void))
    {
      
    }
    
    ~KernelFinally();

private:
 IDelegate<void> *_finalAction;
};

KERNEL_END

#ifndef KERNEL_FINALLY_BEGIN
/*
 * 宏使用:
 *  KERNEL_FINALLY_BEGIN(arg)
    {
        To do something finally.
    }
    KERNEL_FINALLY_END();
 */
 #define KERNEL_FINALLY_BEGIN(Arg) KERNEL_NS::KernelFinally Arg([&]
#endif


#ifndef KERNEL_FINALLY_END
/*
 * 宏使用:
 *  KERNEL_FINALLY_BEGIN(arg)
    {
        To do something finally.
    }
    KERNEL_FINALLY_END();
 */
 #define KERNEL_FINALLY_END() )
#endif

#endif