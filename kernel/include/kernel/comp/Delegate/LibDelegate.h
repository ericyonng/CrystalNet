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
 *              完美转发利用 &&引用折叠,以及std::forward实现
*/
#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_DELEGATE_LIB_DELEGATE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_DELEGATE_LIB_DELEGATE_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/Utils/RttiUtil.h>

KERNEL_BEGIN

// R回调返回值类型，Args回调函数参数包 委托基类 用于解耦具体类型，创建类型无关的委托
template <typename Rtn, typename... Args>
class IDelegate
{
public:
    IDelegate();
    virtual ~IDelegate();
    // 左值会绑定成左值引用，右值会绑定成右值引用
    // 请注意引用折叠适当使用std::forward可以完美的将参数传入，原来什么类型传入后绑定的就是什么类型
    virtual Rtn Invoke(Args... args) = 0;   // 为什么不使用 && 因为为了匹配被委托的参数类型, 只要内部使用std::forward即可完美转发
    virtual Rtn Invoke(Args... args) const = 0;
    virtual IDelegate<Rtn, Args...> *CreateNewCopy() const = 0;
    virtual void Release();

    // 判断是否是某个对象的回调
    virtual bool IsBelongTo(void *obj) const { return false; }
    virtual const void *GetOwner() const { return NULL; }
    virtual void *GetOwner() { return NULL; }
    virtual const Byte8 * GetOwnerRtti() { return ""; }
};

////////////////////
// 类委托
template <typename ObjType, typename Rtn, typename... Args>
class DelegateClass : public IDelegate<Rtn, Args...>
{
public:
    DelegateClass(ObjType *t, Rtn(ObjType::*f)(Args...));
    DelegateClass(ObjType *t, Rtn(ObjType::*f)(Args...) const);
    virtual ~DelegateClass();

    virtual Rtn Invoke(Args... args);
    virtual Rtn Invoke(Args... args) const;
    virtual IDelegate<Rtn, Args...> *CreateNewCopy() const;
    virtual bool IsBelongTo(void *obj) const override { return reinterpret_cast<void *>(_obj) == obj; }
    virtual const void *GetOwner() const override { return _obj; }
    virtual void *GetOwner() override { return _obj; }
    virtual const Byte8 * GetOwnerRtti() override { return RttiUtil::GetByObj(_obj); }

private:
    mutable ObjType *_obj;
    mutable Rtn(ObjType::*_f)(Args...);
    // mutable R(T::*_fconst)(Args...) const;
};


////////////////////
// 帮助释放obj,但是不可以创建obj的副本,避免不可控
template <typename ObjType, typename Rtn, typename... Args>
class DelegateClassDelObj : public IDelegate<Rtn, Args...>
{
public:
    DelegateClassDelObj(ObjType *t, Rtn(ObjType::*f)(Args...));
    DelegateClassDelObj(ObjType *t, Rtn(ObjType::*f)(Args...) const);
    virtual ~DelegateClassDelObj();

    virtual Rtn Invoke(Args... args);
    virtual Rtn Invoke(Args... args) const;
    virtual bool IsBelongTo(void *obj) const override { return reinterpret_cast<void *>(_obj) == obj; }
    virtual const void *GetOwner() const override { return _obj; }
    virtual void *GetOwner() override { return _obj; }
    virtual const Byte8 * GetOwnerRtti() override { return RttiUtil::GetByObj(_obj); }

    // 禁用拷贝与赋值，以及创建副本
private:
    DelegateClassDelObj(const DelegateClassDelObj<ObjType, Rtn, Args...> &) {}
    DelegateClassDelObj<ObjType, Rtn, Args...> &operator=(const DelegateClass<ObjType, Rtn, Args...> &) {}
    virtual IDelegate<Rtn, Args...> *CreateNewCopy() const { return NULL; }

private:
    mutable ObjType * _obj;
    mutable Rtn(ObjType::*_f)(Args...);
    // mutable R(T::*_fconst)(Args...) const;
};


////////////////////
// 函数委托
template <typename Rtn, typename... Args>
class DelegateFunction : public IDelegate<Rtn, Args...>
{
public:
    DelegateFunction(Rtn(*f)(Args...));
    virtual ~DelegateFunction();

    virtual Rtn Invoke(Args... args);
    virtual Rtn Invoke(Args... args) const;
    virtual IDelegate<Rtn, Args...> *CreateNewCopy() const;
    virtual bool IsBelongTo(void *f) const { return f == reinterpret_cast<void *>(_f); }
    virtual const void *GetOwner() const { return _f; }
    virtual void *GetOwner() override { return _f; }
    virtual const Byte8 * GetOwnerRtti() override { return RttiUtil::GetByObj(_f); }

private:
    mutable Rtn(*_f)(Args...);
};


////////////////////
// 支持lambda表达式,std::function等闭包委托
template <typename ClosureFuncType, typename Rtn, typename... Args>
class DelegateClosureFunc : public IDelegate<Rtn, Args...>
{
public:
    //DelegateClosureFunc(ClosureFuncType &&closureFunc);
    // DelegateClosureFunc(ClosureFuncType const&closureFunc);
    DelegateClosureFunc(const ClosureFuncType &closureFunc);
    virtual ~DelegateClosureFunc();

    virtual Rtn Invoke(Args... args);
    virtual Rtn Invoke(Args... args) const;
    virtual IDelegate<Rtn, Args...> *CreateNewCopy() const;
    virtual const Byte8 * GetOwnerRtti() override { return RttiUtil::GetByObj(&_closureFun); }

private:
    mutable RemoveReferenceType<ClosureFuncType> _closureFun;
};


////////////////////
// 委托工厂
class KERNEL_EXPORT DelegateFactory
{
public:
    // 委托,委托释放时不释放obj对象
    template <typename ObjType, typename Rtn, typename... Args>
    static IDelegate<Rtn, Args...> *Create(ObjType *obj, Rtn(ObjType::*f)(Args...));
    template <typename ObjType, typename Rtn, typename... Args>
    static const IDelegate<Rtn, Args...> *Create(ObjType *obj, Rtn(ObjType::*f)(Args...) const);

    // 委托,委托释放时候将释放obj对象 TODO:待测试,确认obj是否被释放
    template <typename ObjType, typename Rtn, typename... Args>
    static IDelegate<Rtn, Args...> *CreateAndHelpDelObj(ObjType *obj, Rtn(ObjType::*f)(Args...));
    template <typename ObjType, typename Rtn, typename... Args>
    static const IDelegate<Rtn, Args...> *CreateAndHelpDelObj(ObjType *obj, Rtn(ObjType::*f)(Args...) const);

    // 普通函数
    template <typename Rtn, typename... Args>
    static IDelegate<Rtn, Args...> *Create(Rtn(*f)(Args...));

    // 绑定lambda,std::function,如：DelegateFactory::Create<decltype(func), void, int>(func);
    template <typename ClosureFuncType /* = decltype(func) */, typename Rtn, typename... Args>
    static IDelegate<Rtn, Args...> *Create(ClosureFuncType &&func);

    // 绑定lambda,std::function,如：DelegateFactory::Create<decltype(func), void, int>(func);
    template <typename ClosureFuncType /* = decltype(func) */, typename Rtn, typename... Args>
    static IDelegate<Rtn, Args...> *Create(ClosureFuncType const&func);
};

KERNEL_END

// 闭包委托创建
#undef KERNEL_CREATE_CLOSURE_DELEGATE
#define KERNEL_CREATE_CLOSURE_DELEGATE(closureTypeFunc, Rtn, ...)    \
KERNEL_NS::DelegateFactory::Create<decltype(closureTypeFunc), Rtn, ##__VA_ARGS__>(closureTypeFunc)

#include <kernel/comp/Delegate/LibDelegateImpl.h>

#endif
