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
 * Date: 2022-02-24 13:38:41
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LOCK_IMPL_LOCK_WRAP_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LOCK_IMPL_LOCK_WRAP_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/Lock/Impl/LockAdapter.h>

KERNEL_BEGIN

template<LockParticleType::ENUMS ParticleType, typename BuildType>
struct LockTypeWrap
{
    static constexpr LockType::ENUMS _type = LockType::Unknown;
};

// 单线程使用dummylock
template<LockParticleType::ENUMS ParticleType>
struct LockTypeWrap<ParticleType, _Build::TL>
{
    static constexpr LockType::ENUMS _type = LockType::DummyLock;
};

// 多线程轻量级,spinlock
template<>
struct KERNEL_EXPORT LockTypeWrap<LockParticleType::Light, _Build::MT>
{
    static constexpr LockType::ENUMS _type = LockType::SpinLock;
};

// 多线程中度,MutexLock
template<>
struct KERNEL_EXPORT LockTypeWrap<LockParticleType::Middle, _Build::MT>
{
    static constexpr LockType::ENUMS _type = LockType::MutexLock;
};

// 多线程重度,ConditionLock
template<>
struct KERNEL_EXPORT LockTypeWrap<LockParticleType::Heavy, _Build::MT>
{
    static constexpr LockType::ENUMS _type = LockType::ConditionLock;
};

// lock包装
template<typename BuildType, LockParticleType::ENUMS ParticleType = LockParticleType::Light>
using LockWrap = LockAdapter<LockTypeWrap<ParticleType, BuildType>::_type>;

KERNEL_END

#endif
