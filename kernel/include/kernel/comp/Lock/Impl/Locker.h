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
 * Date: 2020-10-08 21:38:30
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LOCK_IMPL_LOCKER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LOCK_IMPL_LOCKER_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/common.h>
#include <kernel/comp/Lock/Defs/LockerHandle.h>

KERNEL_BEGIN

class KERNEL_EXPORT Locker
{
public:
    Locker();
    virtual ~Locker();

public:
    void Lock();
    void Unlock();
    bool TryLock();

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    bool IsOtherThreadOwnLock() const;
#endif

protected:
    LockerHandle _handle;
};

inline void Locker::Lock()
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    ::EnterCriticalSection(&(_handle));
#else
    pthread_mutex_lock(&(_handle));
#endif
}

inline void Locker::Unlock()
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    ::LeaveCriticalSection(&(_handle));
#else
    pthread_mutex_unlock(&(_handle));
#endif
}

inline bool Locker::TryLock()
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    return ::TryEnterCriticalSection(&(_handle));
#else
    return ::pthread_mutex_trylock(&(_handle)) == 0;
    // if (LIKELY(ret == 0))
    // {
    //     return true;
    // }
    // else if(ret != EBUSY)
    // {
    //     perror("trylock error!");
    // }

    // return false;
#endif

}

KERNEL_END

#endif
