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
 * Date: 2020-12-06 20:44:54
 * Author: Eric Yonng
 * Description: 
*/

#ifdef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_DELEGATE_LIB_DELEGATE_H__

KERNEL_BEGIN

// 类委托
template <typename ObjType, class Rtn, typename... Args>
ALWAYS_INLINE DelegateClass<ObjType, Rtn, Args...>::DelegateClass(ObjType *t, Rtn(ObjType::*f)(Args...))
    :_obj(t)
    ,_f(f)
{
}

template <typename ObjType, class Rtn, typename... Args>
ALWAYS_INLINE DelegateClass<ObjType, Rtn, Args...>::DelegateClass(const ObjType *t, Rtn(ObjType::*f)(Args...) const)
    :_obj(const_cast<ObjType *>(t))
    , _f(decltype(_f)(f))
{
}

template <typename ObjType, typename Rtn, typename... Args>
ALWAYS_INLINE DelegateClass<ObjType, Rtn, Args...>::~DelegateClass()
{
}

template <typename ObjType, typename Rtn, typename... Args>
ALWAYS_INLINE Rtn DelegateClass<ObjType, Rtn, Args...>::Invoke(Args... args)
{
    return (_obj->*_f)(std::forward<Args>(args)...);
}

template <typename ObjType, typename Rtn, typename... Args>
ALWAYS_INLINE Rtn DelegateClass<ObjType, Rtn, Args...>::Invoke(Args... args) const
{
    return (_obj->*_f)(std::forward<Args>(args)...);
}

template <typename ObjType, typename Rtn, typename... Args>
ALWAYS_INLINE IDelegate<Rtn, Args...> *DelegateClass<ObjType, Rtn, Args...>::CreateNewCopy() const
{
    return DelegateFactory::Create(_obj, _f);
}

// 类委托带删除对象
template <typename ObjType, class Rtn, typename... Args>
ALWAYS_INLINE DelegateClassDelObj<ObjType, Rtn, Args...>::DelegateClassDelObj(ObjType *t, Rtn(ObjType::*f)(Args...))
    :_obj(t)
    ,_f(f)
{
}

template <typename ObjType, class Rtn, typename... Args>
ALWAYS_INLINE DelegateClassDelObj<ObjType, Rtn, Args...>::DelegateClassDelObj(const ObjType *t, Rtn(ObjType::*f)(Args...) const)
    :_obj(const_cast<ObjType *>(t))
    , _f(decltype(_f)(f))
{
}

template <typename ObjType, typename Rtn, typename... Args>
ALWAYS_INLINE DelegateClassDelObj<ObjType, Rtn, Args...>::~DelegateClassDelObj()
{
    CRYSTAL_DELETE_SAFE(_obj);
}

template <typename ObjType, typename Rtn, typename... Args>
ALWAYS_INLINE Rtn DelegateClassDelObj<ObjType, Rtn, Args...>::Invoke(Args... args)
{
    return (_obj->*_f)(std::forward<Args>(args)...);
}

template <typename ObjType, typename Rtn, typename... Args>
ALWAYS_INLINE Rtn DelegateClassDelObj<ObjType, Rtn, Args...>::Invoke(Args... args) const
{
    return (_obj->*_f)(std::forward<Args>(args)...);
}

// 函数委托
template <typename Rtn, typename... Args>
ALWAYS_INLINE DelegateFunction<Rtn, Args...>::DelegateFunction(Rtn(*f)(Args...))
    :_f(f)
{
}

template <typename Rtn, typename... Args>
ALWAYS_INLINE DelegateFunction<Rtn, Args...>::~DelegateFunction()
{
}

template <typename Rtn, typename... Args>
ALWAYS_INLINE Rtn DelegateFunction<Rtn, Args...>::Invoke(Args... args)
{
    return (*_f)(std::forward<Args>(args)...);
}

template <typename Rtn, typename... Args>
ALWAYS_INLINE Rtn DelegateFunction<Rtn, Args...>::Invoke(Args... args) const
{
    return (*_f)(std::forward<Args>(args)...);
}

template <typename Rtn, typename... Args>
ALWAYS_INLINE IDelegate<Rtn, Args...> *DelegateFunction<Rtn, Args...>::CreateNewCopy() const
{
    return DelegateFactory::Create(_f);
}


// 闭包委托

// template <typename ClosureFuncType, typename Rtn, typename... Args>
// inline DelegateClosureFunc<ClosureFuncType, Rtn, Args...>::DelegateClosureFunc(ClosureFuncType &&closureFunc)
//     :_closureFun(std::forward<ClosureFuncType>(closureFunc))
// {
// 
// }
// 
// template <typename ClosureFuncType, typename Rtn, typename... Args>
// inline DelegateClosureFunc<ClosureFuncType, Rtn, Args...>::DelegateClosureFunc(ClosureFuncType const&closureFunc)
//     :_closureFun(closureFunc)
// {
// 
// }
// 

template <typename ClosureFuncType, typename Rtn, typename... Args>
ALWAYS_INLINE DelegateClosureFunc<ClosureFuncType, Rtn, Args...>::DelegateClosureFunc(RemoveReferenceType<ClosureFuncType> &&closureFunc)
    :_closureFun(closureFunc)
{
}

template <typename ClosureFuncType, typename Rtn, typename... Args>
ALWAYS_INLINE DelegateClosureFunc<ClosureFuncType, Rtn, Args...>::DelegateClosureFunc(const RemoveReferenceType<ClosureFuncType> &closureFunc)
    :_closureFun(closureFunc)
{
}

template <typename ClosureFuncType, typename Rtn, typename... Args>
ALWAYS_INLINE DelegateClosureFunc<ClosureFuncType, Rtn, Args...>::~DelegateClosureFunc()
{
}

template <typename ClosureFuncType, typename Rtn, typename... Args>
ALWAYS_INLINE Rtn DelegateClosureFunc<ClosureFuncType, Rtn, Args...>::Invoke(Args... args)
{
    return _closureFun(std::forward<Args>(args)...);
}

template <typename ClosureFuncType, typename Rtn, typename... Args>
ALWAYS_INLINE Rtn DelegateClosureFunc<ClosureFuncType, Rtn, Args...>::Invoke(Args... args) const
{
    return _closureFun(std::forward<Args>(args)...);
}

template <typename ClosureFuncType, typename Rtn, typename... Args>
ALWAYS_INLINE IDelegate<Rtn, Args...> *DelegateClosureFunc<ClosureFuncType, Rtn, Args...>::CreateNewCopy() const
{
    return DelegateFactory::Create<decltype(_closureFun), Rtn, Args...>(_closureFun);
}


// 委托工厂方法
template <typename ObjType, typename Rtn, typename... Args>
ALWAYS_INLINE IDelegate<Rtn, Args...> *DelegateFactory::Create(ObjType *obj, Rtn(ObjType::*f)(Args...))
{
    return new DelegateClass<ObjType, Rtn, Args...>(obj, f);
}

template <typename ObjType, typename Rtn, typename... Args>
ALWAYS_INLINE const IDelegate<Rtn, Args...> *DelegateFactory::Create(const ObjType *obj, Rtn(ObjType::*f)(Args...) const)
{
    return new DelegateClass<ObjType, Rtn, Args...>(obj, f);
}

template <typename ObjType, typename Rtn, typename... Args>
ALWAYS_INLINE IDelegate<Rtn, Args...> *DelegateFactory::CreateAndHelpDelObj(ObjType *obj, Rtn(ObjType::*f)(Args...))
{
    return new DelegateClassDelObj<ObjType, Rtn, Args...>(obj, f);
}

template <typename ObjType, typename Rtn, typename... Args>
ALWAYS_INLINE const IDelegate<Rtn, Args...> *DelegateFactory::CreateAndHelpDelObj(const ObjType *obj, Rtn(ObjType::*f)(Args...) const)
{
    return new DelegateClassDelObj<ObjType, Rtn, Args...>(obj, f);
}


template <typename Rtn, typename... Args>
ALWAYS_INLINE IDelegate<Rtn, Args...> *DelegateFactory::Create(Rtn(*f)(Args...))
{
    return new DelegateFunction<Rtn, Args...>(f);
}

template <typename ClosureFuncType, typename Rtn, typename... Args>
ALWAYS_INLINE IDelegate<Rtn, Args...> *DelegateFactory::Create(ClosureFuncType &&func)
{
    return new DelegateClosureFunc<ClosureFuncType, Rtn, Args...>(std::forward<ClosureFuncType>(func));
}

template <typename ClosureFuncType, typename Rtn, typename... Args>
ALWAYS_INLINE IDelegate<Rtn, Args...> *DelegateFactory::Create(ClosureFuncType const&func)
{
    return new DelegateClosureFunc<ClosureFuncType, Rtn, Args...>(std::forward<decltype(func)>(func));
}

KERNEL_END

#endif
