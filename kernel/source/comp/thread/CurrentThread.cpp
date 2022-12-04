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
 * Date: 2021-06-18 02:06:28
 * Author: Eric Yonng
 * Description: 
*/
#include <pch.h>
#include <kernel/comp/thread/CurrentThread.h>
#include <kernel/comp/Utils/SystemUtil.h>

KERNEL_BEGIN

namespace CurrentThread
{
#if CRYSTAL_TARGET_PLATFORM_LINUX
    __thread UInt64 _cachedTid = 0;
    __thread Byte8 _tidString[32];
    __thread Int32 _tidStringLength = 6;
    __thread const Byte8 *_threadName = "unknown";
#else
    // TODO: windows
#endif

  void CacheTid()
  {
#if CRYSTAL_TARGET_PLATFORM_LINUX
        _cachedTid = SystemUtil::GetCurrentThreadId();
        _tidStringLength = sprintf(_tidString, "%5llu", _cachedTid);
#else
    // TODO:windows
#endif

  }

  bool IsMainThread()
  {
#if CRYSTAL_TARGET_PLATFORM_LINUX
    return Tid() == static_cast<UInt64>(SystemUtil::GetCurProcessId());
#else
    // TODO:windows
      return Tid() == static_cast<UInt64>(SystemUtil::GetCurProcessId());
#endif
  }

};


KERNEL_END

