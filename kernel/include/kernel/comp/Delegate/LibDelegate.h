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

#include <kernel/common/RemoveReference.h>
#include <kernel/comp/Utils/RttiUtil.h>
#include <kernel/comp/Delegate/IDelegate.h>

KERNEL_BEGIN

////////////////////
// 类委托
template <typename ObjType, typename Rtn, typename... Args>
class DelegateClass : public IDelegate<Rtn, Args...>
{
public:
    DelegateClass(ObjType *t, Rtn(ObjType::*f)(Args...));
    DelegateClass(const ObjType *t, Rtn(ObjType::*f)(Args...) const);
    virtual ~DelegateClass();

    virtual Rtn Invoke(Args... args) override;
    virtual Rtn Invoke(Args... args) const  override;
    virtual IDelegate<Rtn, Args...> *CreateNewCopy() const override;
    virtual bool IsBelongTo(void *obj) const override { return reinterpret_cast<void *>(_obj) == obj; }
    virtual const void *GetOwner() const override { return _obj; }
    virtual void *GetOwner() override { return _obj; }
    virtual LibString GetOwnerRtti() override { return RttiUtil::GetByObj(_obj); }
    virtual LibString GetCallbackRtti() override { return RttiUtil::GetByObj(&_f); }

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
    DelegateClassDelObj(const ObjType *t, Rtn(ObjType::*f)(Args...) const);
    virtual ~DelegateClassDelObj();

    virtual Rtn Invoke(Args... args) override;
    virtual Rtn Invoke(Args... args) const override;
    virtual bool IsBelongTo(void *obj) const override { return reinterpret_cast<void *>(_obj) == obj; }
    virtual const void *GetOwner() const override { return _obj; }
    virtual void *GetOwner() override { return _obj; }
    virtual LibString GetOwnerRtti() override { return RttiUtil::GetByObj(_obj); }
    virtual LibString GetCallbackRtti() override { return RttiUtil::GetByObj(&_f); }

    // 禁用拷贝与赋值，以及创建副本
private:
    DelegateClassDelObj(const DelegateClassDelObj<ObjType, Rtn, Args...> &) {}
    DelegateClassDelObj<ObjType, Rtn, Args...> &operator=(const DelegateClass<ObjType, Rtn, Args...> &) {}
    virtual IDelegate<Rtn, Args...> *CreateNewCopy() const override { return NULL; }

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

    virtual Rtn Invoke(Args... args) override;
    virtual Rtn Invoke(Args... args) const override;
    virtual IDelegate<Rtn, Args...> *CreateNewCopy() const override;
    virtual bool IsBelongTo(void *f) const override { return f == reinterpret_cast<void *>(_f); }
    virtual const void *GetOwner() const override { return reinterpret_cast<const void *>(_f); }
    virtual void *GetOwner() override { return reinterpret_cast<void *>(_f); }
    virtual LibString GetOwnerRtti() override { return RttiUtil::GetByObj(_f); }
    virtual LibString GetCallbackRtti() override { return RttiUtil::GetByObj(_f); }

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

    virtual Rtn Invoke(Args... args) override;
    virtual Rtn Invoke(Args... args) const override;
    virtual IDelegate<Rtn, Args...> *CreateNewCopy() const override;
    virtual LibString GetOwnerRtti() override { return RttiUtil::GetByObj(&_closureFun); }
    virtual LibString GetCallbackRtti() override { return RttiUtil::GetByObj(&_closureFun); }

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
    static const IDelegate<Rtn, Args...> *Create(const ObjType *obj, Rtn(ObjType::*f)(Args...) const);

    // 委托,委托释放时候将释放obj对象 TODO:待测试,确认obj是否被释放
    template <typename ObjType, typename Rtn, typename... Args>
    static IDelegate<Rtn, Args...> *CreateAndHelpDelObj(ObjType *obj, Rtn(ObjType::*f)(Args...));
    template <typename ObjType, typename Rtn, typename... Args>
    static const IDelegate<Rtn, Args...> *CreateAndHelpDelObj(const ObjType *obj, Rtn(ObjType::*f)(Args...) const);

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
