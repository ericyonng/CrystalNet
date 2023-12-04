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
 * Date: 2021-06-18 01:55:22
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_THREAD_CURRENT_THREAD_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_THREAD_CURRENT_THREAD_H__

#pragma once

#include <kernel/comp/Utils/SystemUtil.h>

KERNEL_BEGIN

namespace CurrentThread
{
    // 采用tls方式缓存当前线程id, 因为线程创建后线程id就不会变,且为了提高性能需要缓存线程id,避免系统调用产生不必要的开销
  void CacheTid();
  // 当前线程是主进程id
  bool IsMainThread();
  
    // linux下 __thread才有效
#if CRYSTAL_TARGET_PLATFORM_LINUX
  // internal
  extern __thread UInt64 _cachedTid;
  extern __thread Byte8 _tidString[32];
  extern __thread Int32 _tidStringLength;
  extern __thread const Byte8 *_threadName;

#else
    // TODO:windows
#endif

  inline UInt64 Tid()
  {
    #if CRYSTAL_TARGET_PLATFORM_LINUX
    if (__builtin_expect(_cachedTid == 0, 0))
    {
      CacheTid();
    }    
    return _cachedTid;
    #else
      return SystemUtil::GetCurrentThreadId();
    #endif
  }

  inline const Byte8 *TidString() // for logging
  {
    #if CRYSTAL_TARGET_PLATFORM_LINUX
        return _tidString;
    #else
    // TODO:windows
    #endif
  }

  inline Int32 TidStringLength() // for logging
  {
    #if CRYSTAL_TARGET_PLATFORM_LINUX
        return _tidStringLength;
    #else
    // TODO:windows
    #endif
  }

  inline const Byte8 *Name()
  {
    #if CRYSTAL_TARGET_PLATFORM_LINUX
        return _threadName;
    #else
    // TODO:windows
    #endif
  }


};

KERNEL_END

#endif