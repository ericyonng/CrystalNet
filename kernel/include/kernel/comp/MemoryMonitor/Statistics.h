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
 * Date: 2021-03-19 22:43:52
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMORY_MONITOR_STATISTICS_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMORY_MONITOR_STATISTICS_H__

#pragma once

#include <vector>

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>

#include <kernel/comp/Lock/Impl/SpinLock.h>

KERNEL_BEGIN

class LibString;

template <typename Rtn, typename... Args>
class IDelegate;

// 只提供注册不提供移除,移除会有性能上的损耗
class KERNEL_EXPORT Statistics
{
public:
    // Rtn:返回Buffer占用的内存大小
    typedef std::vector<IDelegate<UInt64, LibString &> *> Seq;

public:
    Statistics(){}
    ~Statistics();

public:
    Seq &GetDict();
    // 会直接释放delg,外部不可自己释放delg
    void Remove(IDelegate<UInt64, LibString &> *delg);

    void Lock();
    void Unlock();

private:
    SpinLock _lck;
    Seq _addrRefDeleg;
};

ALWAYS_INLINE Statistics::Seq &Statistics::GetDict()
{
    return _addrRefDeleg;
}

ALWAYS_INLINE void Statistics::Lock()
{
    _lck.Lock();
}

ALWAYS_INLINE void Statistics::Unlock()
{
    _lck.Unlock();
}

KERNEL_END

#endif
