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
 * Date: 2020-10-08 16:04:25
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_AUTO_DEL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_AUTO_DEL_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/comp/Delegate/Delegate.h>
#include <kernel/comp/Utils/BackTraceUtil.h>

KERNEL_BEGIN

class KERNEL_EXPORT AutoDelMethods
{
public:
    enum Way
    {
        Delete = 0,         // delete 释放对象
        NoSafeDelete,       // delete 对象但不赋空值
        MultiDelete,        // delete [] 释放对象
        Release,            // 调用对象的Release方法释放对象
        ReleaseSafe,        // 调用对象的Release 并置空方法释放对象
        DoNothing,          // 不做任何操作
        SetNull,            // 不释放 但置空
        CustomDelete,       // 自定义删除器 需要配合Delegate
    };
};

// auto free obj
template<AutoDelMethods::Way del>
class AutoDel
{
public:
    template<typename _Type>
    ALWAYS_INLINE void Release(_Type &val);
    ALWAYS_INLINE void Cancel() {}
    ALWAYS_INLINE void *pop()
    {
        return NULL;
    }
    ALWAYS_INLINE void Takeover(void *deleg){}
};

// 实现


template<>
class KERNEL_EXPORT AutoDel<AutoDelMethods::Delete>
{
public:
    template<typename _Type>
    ALWAYS_INLINE void Release(_Type &p)
    {
        CRYSTAL_DELETE_SAFE(p);
        // CRYSTAL_TRACE("single delete when release");
    }
    ALWAYS_INLINE void Cancel() {}
    ALWAYS_INLINE void *pop()
    {
        return NULL;
    }
    ALWAYS_INLINE void Takeover(void *deleg){}
};

template<>
class KERNEL_EXPORT AutoDel<AutoDelMethods::NoSafeDelete>
{
public:
    template<typename _Type>
    ALWAYS_INLINE void Release(_Type &p)
    {
        CRYSTAL_DELETE(p);
        // CRYSTAL_TRACE("single NoSafeDelete when release");
    }

    ALWAYS_INLINE void Cancel() {}
    ALWAYS_INLINE void *pop()
    {
        return NULL;
    }
    ALWAYS_INLINE void Takeover(void *deleg){}
};

template<>
class KERNEL_EXPORT AutoDel<AutoDelMethods::MultiDelete>
{
public:
    template<typename _Type>
    ALWAYS_INLINE void Release(_Type &p)
    {
        CRYSTAL_MULTI_DELETE_SAFE(p);
        // CRYSTAL_TRACE("multi delete when release");
    }
    ALWAYS_INLINE void Cancel() {}
    ALWAYS_INLINE void *pop()
    {
        return NULL;
    }
    ALWAYS_INLINE void Takeover(void *deleg){}
};

template<>
class KERNEL_EXPORT AutoDel<AutoDelMethods::Release>
{
public:
    template<typename _Type>
    ALWAYS_INLINE void Release(_Type &p)
    {
        CRYSTAL_RELEASE(p);
        // CRYSTAL_TRACE("invoke release method when release");
    }
    ALWAYS_INLINE void Cancel() {}
    ALWAYS_INLINE void *pop()
    {
        return NULL;
    }
    ALWAYS_INLINE void Takeover(void *deleg){}
};

template<>
class KERNEL_EXPORT AutoDel<AutoDelMethods::ReleaseSafe>
{
public:
    template<typename _Type>
    ALWAYS_INLINE void Release(_Type &p)
    {
        CRYSTAL_RELEASE_SAFE(p);
        // CRYSTAL_TRACE("invoke release method when release");
    }
    ALWAYS_INLINE void Cancel() {}
    ALWAYS_INLINE void *pop()
    {
        return NULL;
    }
    ALWAYS_INLINE void Takeover(void *deleg){}
};

template<>
class KERNEL_EXPORT AutoDel<AutoDelMethods::DoNothing>
{
public:
    template<typename _Type>
    ALWAYS_INLINE void Release(_Type &p)
    {
        // 不释放也不置空
        // CRYSTAL_TRACE("do nothing when release");
    }

    ALWAYS_INLINE void Cancel() {}
    ALWAYS_INLINE void *pop()
    {
        return NULL;
    }
    ALWAYS_INLINE void Takeover(void *deleg){}
};

template<>
class KERNEL_EXPORT AutoDel<AutoDelMethods::SetNull>
{
public:
    ALWAYS_INLINE AutoDel(IDelegate<void, void *> *delg)
    {
        // static_assert(AutoDelMethods::SetNull == AutoDelMethods::CustomDelete, "auto del with delegate, AutoDelMethods must be custom type");
    }

    template<typename _Type>
    ALWAYS_INLINE void Release(_Type &p)
    {
        p = NULL;
        // CRYSTAL_TRACE("set null when release");
    }
    ALWAYS_INLINE void Cancel() {}
    ALWAYS_INLINE void *pop()
    {
        return NULL;
    }
    ALWAYS_INLINE void Takeover(void *deleg){}
};

template<>
class KERNEL_EXPORT AutoDel<AutoDelMethods::CustomDelete>
{
    // AutoDel(const AutoDel<AutoDelMethods::CustomDelete> &other) = delete;
    AutoDel(AutoDel<AutoDelMethods::CustomDelete> &&other) = delete;
    // AutoDel<AutoDelMethods::CustomDelete> &operator= (AutoDel<AutoDelMethods::CustomDelete> &other) = delete;
    AutoDel<AutoDelMethods::CustomDelete> &operator= (AutoDel<AutoDelMethods::CustomDelete> &&other) = delete;

public:
    ALWAYS_INLINE AutoDel()
        :_delg(NULL)
    {

    }

    ALWAYS_INLINE ~AutoDel()
    {
        CRYSTAL_RELEASE_SAFE(_delg);
    }

    // 注意不建议拷贝,因为delegate 如果是lambda，它是一个特有的闭包环境,它只允许对相同指针进行释放，不允许对不同指针进行释放(因为条件可能不同)
    ALWAYS_INLINE AutoDel(const AutoDel<AutoDelMethods::CustomDelete> &other)
        :_delg(NULL)
    {
        if(LIKELY(other._delg))
            _delg = other._delg->CreateNewCopy();
    }

    ALWAYS_INLINE AutoDel<AutoDelMethods::CustomDelete> &operator= (const AutoDel<AutoDelMethods::CustomDelete> &other)
    {
        Cancel();
        if(LIKELY(other._delg))
            _delg = other._delg->CreateNewCopy();
        
        return *this;
    }

    ALWAYS_INLINE void SetDelegate(IDelegate<void, void *> *delg)
    {
        CRYSTAL_RELEASE_SAFE(_delg);
        _delg = delg;
    }

    template<typename _Type>
    ALWAYS_INLINE void Release(_Type &&p)
    {
        if (LIKELY(_delg))
            _delg->Invoke(p);
        else
        {

            CRYSTAL_TRACE("AutoDel delegate is nil p:%p, _Type:%s, \nbacktrace:\n%s", p, typeid(_Type).name(), BackTraceUtil::CrystalCaptureStackBackTrace().c_str());
        }
    }

    ALWAYS_INLINE void Cancel() 
    {
        CRYSTAL_RELEASE_SAFE(_delg);
    }

    ALWAYS_INLINE IDelegate<void, void *> *pop()
    {
        auto delg = _delg;
        _delg = NULL;
        return delg;
    }
    
    ALWAYS_INLINE void Takeover(void *deleg)
    {
        CRYSTAL_RELEASE_SAFE(_delg);
        _delg = reinterpret_cast<IDelegate<void, void *> *>(deleg);
    }

    IDelegate<void, void *> *_delg;
};



KERNEL_END

#endif
