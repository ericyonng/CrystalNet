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
 * Date: 2020-10-08 16:31:18
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_SINGLETON_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_SINGLETON_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/common.h>
#include <kernel/comp/AutoDel.h>
#include <kernel/comp/SmartPtr.h>
#include <kernel/comp/Lock/Lock.h>

KERNEL_BEGIN

template<typename ObjType, AutoDelMethods::Way delMethod = AutoDelMethods::Delete>
class Singleton
{
public:
    static ObjType *GetInstance()
    {
        // 静态局部变量内部自动加锁,线程安全
        static SmartPtr<ObjType, delMethod> _obj = new ObjType();

        return _obj;
    }

    template<typename... Args>
    static ObjType *GetInstance(Args&&... args)
    {
        // 静态局部变量内部自动加锁,线程安全
        static SmartPtr<ObjType, delMethod> _obj = new ObjType(std::forward<Args>(args)...);

        return _obj;
    }

private:
    Singleton(void) = delete;
    ~Singleton(void) = delete;
    Singleton(const Singleton<ObjType, delMethod>&) = delete;
    Singleton(Singleton<ObjType, delMethod>&&) = delete;
    Singleton<ObjType, delMethod>& operator = (const Singleton<ObjType, delMethod>&) = delete;

private:
};


KERNEL_END

#endif
