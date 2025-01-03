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
 * Date: 2020-10-08 18:53:58
 * Author: Eric Yonng
 * Description: 锁句柄
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LOCK_DEFS_LOCKER_HANDLE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LOCK_DEFS_LOCKER_HANDLE_H__

#pragma once

#include <kernel/common/compile.h>
#include <kernel/common/BaseMacro.h>

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
 #include <pthread.h>
#endif

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
 #include <windows.h>
 #include <synchapi.h>
#endif

KERNEL_BEGIN

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    typedef CRITICAL_SECTION LockerHandle;
#else
    typedef pthread_mutex_t LockerHandle;
#endif

KERNEL_END

#endif
