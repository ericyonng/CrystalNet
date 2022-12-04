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

#include <kernel/kernel_inc.h>
#include <kernel/comp/Lock/Lock.h>
#include <kernel/comp/Delegate/Delegate.h>

KERNEL_BEGIN

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

ALWAYS_INLINE void Statistics::Remove(IDelegate<UInt64, LibString &> *delg)
{
    if(_addrRefDeleg.empty())
    {
        CRYSTAL_TRACE("Statistics _addrRefDeleg empty perhaps have released before");
        return;
    }

    const Int64 delgSize = static_cast<Int64>(_addrRefDeleg.size());
    for(Int64 idx = delgSize - 1; idx >= 0; --idx)
    {
        if(_addrRefDeleg[idx] == delg)
        {
            _addrRefDeleg.erase(_addrRefDeleg.begin() + idx);
            CRYSTAL_RELEASE_SAFE(delg);
            // CRYSTAL_TRACE("Statistics remove a delegate idx = [%lld], left count=[%llu]", idx, static_cast<UInt64>(_addrRefDeleg.size()));
            break;
        }
    }
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
