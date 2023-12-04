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
 * Date: 2020-12-06 20:28:10
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_DELEGATE_IDELEGATE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_DELEGATE_IDELEGATE_H__

#pragma once

#include <kernel/common/BaseMacro.h>

KERNEL_BEGIN

// R回调返回值类型，Args回调函数参数包 委托基类 用于解耦具体类型，创建类型无关的委托
template <typename Rtn, typename... Args>
class IDelegate
{
public:
    IDelegate(){}
    virtual ~IDelegate(){}

    // 左值会绑定成左值引用，右值会绑定成右值引用
    // 请注意引用折叠适当使用std::forward可以完美的将参数传入，原来什么类型传入后绑定的就是什么类型
    virtual Rtn Invoke(Args... args) = 0;   // 为什么不使用 && 因为为了匹配被委托的参数类型, 只要内部使用std::forward即可完美转发
    virtual Rtn Invoke(Args... args) const = 0;
    virtual IDelegate<Rtn, Args...> *CreateNewCopy() const = 0;
    virtual void Release(){ delete this; }

    // 判断是否是某个对象的回调
    virtual bool IsBelongTo(void *obj) const { return false; }
    virtual const void *GetOwner() const { return NULL; }
    virtual void *GetOwner() { return NULL; }
    virtual const Byte8 * GetOwnerRtti() { return ""; }
    virtual const Byte8 * GetCallbackRtti() { return ""; }
};

KERNEL_END

#endif
