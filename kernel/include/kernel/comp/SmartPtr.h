/*!
 * MIT License
 *  
 * Copyright (c) 2020 Eric Yonng<120453674@qq.com>
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *  
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *  
 * 
 * Date: 2020-10-08 18:11:32
 * Author: Eric Yonng
 * Description: 
 *              1.由于AutoDel的特殊性（尤其针对闭包,有封闭的环境,所以不可任何情况下的转移,拷贝，以及赋值,要转移请pop）
 *              2. SmartPtr的引用计数线程不安全
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_SMART_PTR_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_SMART_PTR_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/common.h>
#include <kernel/comp/AutoDel.h>

KERNEL_BEGIN

// 同时只能支持在同一个线程,不可进行多线程并发操作
template<typename ObjType, AutoDelMethods::Way delMethod = AutoDelMethods::Delete>
class SmartPtr
{
public:
    SmartPtr()
        :_ptr(NULL)
        ,_ref(NULL)
    {

    }

    SmartPtr(ObjType *ptr)
        :_ptr(ptr)
        ,_ref(NULL)
    {
        if(LIKELY(_ptr))
        {
            _ref = CRYSTAL_NEW(UInt64);
            *_ref = 1;
        }
    }

    SmartPtr(const SmartPtr<ObjType, delMethod> &obj)        
        :_ptr(NULL)
        ,_ref(NULL)
    {
        _CopyFrom(obj);
    }

    SmartPtr(SmartPtr<ObjType, delMethod> &&obj)
    {
        _MoveFrom(obj);
    }

    ~SmartPtr()
    {
        Release();
    }

    // popDelegate 将释放的方法也弹出去（只针对CustomerDelete有效）
    // 引用计数不为1时,pop返回NULL,因为还有其他智能指针hold住了ObjType,还是有可能被其他智能指针释放，故而无法彻底弹出
    ObjType *pop(IDelegate<void, void *&> **popDelegate = NULL)
    {
        // 还有引用不可抛出去
        if(_ref && *_ref > 1)
        {
            CRYSTAL_TRACE("ptr is hold by other smartptr objtype:%s, ref:%llu", typeid(ObjType).name(), *_ref);
            return NULL;
        }

        ObjType *ptr = _ptr;
        _ptr = NULL;

        if(UNLIKELY(popDelegate))
        {
            *popDelegate = reinterpret_cast<IDelegate<void, void *&> *>(_del.pop());
        }
        else
        {
            _del.Cancel();
        }

        // 没有引用了
        if(UNLIKELY(!_ref))
            return ptr;

        CRYSTAL_DELETE_SAFE(_ref);
        return ptr;
    }

    void Release()
    {
        // 说明被转移了
        if(UNLIKELY(!_ref || (*_ref == 0)))
        {
            _del.Cancel();

            CRYSTAL_DELETE_SAFE(_ref);

            return;
        }

        if(--(*_ref) != 0)
        {
            _ptr = NULL;
            _ref = NULL;
            _del.Cancel();
            return;
        }

        if(LIKELY(_ptr))
            _del.Release(_ptr);
        _del.Cancel();

        CRYSTAL_DELETE_SAFE(_ref);
        _ref = NULL;
        _ptr = NULL;
    }

    SmartPtr<ObjType, delMethod> &operator =(ObjType *ptr)
    {
        if(_ptr == ptr)
            return *this;

        Release();
        _ptr = ptr;
        if(LIKELY(_ptr))
        {
            _ref = CRYSTAL_NEW(UInt64);
            *_ref = 1;
        }

        return *this;
    }
    
    SmartPtr<ObjType, delMethod> &operator =(const SmartPtr<ObjType, delMethod> &other)
    {
        _CopyFrom(other);

        return *this;
    }

    SmartPtr<ObjType, delMethod> &operator =(SmartPtr<ObjType, delMethod> &&ptr)
    {
        _MoveFrom(ptr);

        return *this;
    }

    operator void *()
    {
        return _ptr;
    }

    operator const void *() const
    {
        return _ptr;
    }

    operator ObjType *()
    {
        return _ptr;
    }

    operator const ObjType *() const
    {
        return _ptr;
    }

    ObjType *operator->()
    {
        return _ptr;
    }

    const ObjType *operator->() const
    {
        return _ptr;
    }

    // no out-range detect
    ObjType& operator [](Int32 index)    
    {
        ObjType *ptr = _ptr;
        if(ptr)
            ptr += index;

        return *ptr;
    }

    // no out-range detect
    const ObjType& operator [](Int32 index) const
    {
        const ObjType *ptr = _ptr;
        if(ptr)
            ptr += index;

        return *ptr;
    }   

    ObjType &operator *()
    {
        return *_ptr;
    }

    const ObjType &operator *() const
    {
        return *_ptr;
    }

    operator bool()
    {
        return _ptr != NULL;
    }

    ObjType *AsSelf()
    {
        return _ptr;
    }

    const ObjType *AsSelf() const
    {
        return _ptr;
    }

    template<typename SpecifyType>
    SpecifyType *Cast()
    {
        return reinterpret_cast<SpecifyType *>(_ptr);
    }

    template<typename SpecifyType>
    const SpecifyType *Cast() const
    {
        return reinterpret_cast<SpecifyType *>(_ptr);
    }
    
    // 利用模版,在调用的地方静态断言删除类型:void func(void *&)
    template<AutoDelMethods::Way ImplType = AutoDelMethods::CustomDelete>
    void SetDelegate(IDelegate<void, void *> *delg)
    {
        // static_assert(ImplType == delMethod, "ImplType must be delMethod");
        static_assert(ImplType == AutoDelMethods::CustomDelete, "SmartPtr SetDelegate only for CustomDelete");
        _del.SetDelegate(delg);
    }

	// void func(void *)
    template<typename ClosureType, AutoDelMethods::Way ImplType = AutoDelMethods::CustomDelete>
    void SetClosureDelegate(ClosureType &&closureFun)
    {
        // static_assert(ImplType == delMethod, "ImplType must be delMethod");
        static_assert(ImplType == AutoDelMethods::CustomDelete, "SmartPtr SetDelegate only for CustomDelete");
        auto delg = KERNEL_CREATE_CLOSURE_DELEGATE(closureFun, void, void *);
        _del.SetDelegate(delg);
    }

    UInt64 GetRefCount() const
    {
        return _ref ? *_ref : 0;
    }

private:
    void _CopyFrom(const SmartPtr<ObjType, delMethod> &other)
    {
        if(this == &other)
            return;

        // 内部地址一样说明已经拷贝给当前智能指针过了
        if (_ptr == other._ptr)
            return;

        Release();

        _ptr = other._ptr;
        _ref = other._ref;
        _del = other._del;

        // 引用计数更新
        if(LIKELY(_ptr))
            ++(*_ref);
    }

    void _MoveFrom(SmartPtr<ObjType, delMethod> &obj)
    {
        if(this == &obj)
            return;

        if(_ptr != obj._ptr)
        {
            Release();
        }

        _ptr = obj._ptr;
        _ref = obj._ref;
        _del.Takeover(obj._del.pop());

        obj._ptr = NULL;
        obj._ref = NULL;
    }

protected:
    mutable ObjType *_ptr;
    mutable UInt64 *_ref;
    mutable AutoDel<delMethod> _del;
};

KERNEL_END

#endif
