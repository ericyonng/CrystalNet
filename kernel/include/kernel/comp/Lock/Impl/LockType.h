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
 * Date: 2022-02-24 13:53:52
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LOCK_IMPL_LOCKE_TYPE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LOCK_IMPL_LOCKE_TYPE_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>

KERNEL_BEGIN

class KERNEL_EXPORT LockType
{
public:
    enum ENUMS
    {
        SpinLock = 0,           // 自选锁
        MutexLock = 1,          // 互斥锁
        ConditionLock = 2,      // 条件变量互斥锁
        DummyLock = 3,          // 假锁 TL版本适用
        Unknown = 4,            // 未知
    };
};

// 锁粒度
class KERNEL_EXPORT LockParticleType
{
public:
    enum ENUMS
    {
        Light = 0,          // 轻量级, 使用SpinLock
        Middle = 1,         // 中度 使用Mutex
        Heavy = 2,          // 重度,使用ConditionLock
    };
};

KERNEL_END

#endif
