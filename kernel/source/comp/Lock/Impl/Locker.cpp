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
 * Date: 2020-10-08 21:47:40
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Lock/Impl/Locker.h>
#include <kernel/comp/Utils/CountUtil.h>

KERNEL_BEGIN

Locker::Locker()
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    static const UInt32 maxDwordCount1 = CountUtil::Count1InBinary((std::numeric_limits<DWORD>::max)());
#endif

    ::memset(&_handle, 0, sizeof(_handle));
    
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    // 自旋次数（最高位置1）
    DWORD spinCnt = static_cast<DWORD>(SPINNING_COUNT);
    spinCnt |= static_cast<DWORD>(1 << (maxDwordCount1 - 1));

    // 创建自旋锁，避免线程频繁挂起
    if(!::InitializeCriticalSectionAndSpinCount(&(_handle), spinCnt))
        printf("MetaLocker create spinlock fail spinCnt[%lu]", spinCnt);
#else
    auto ret = pthread_mutex_init(&(_handle), NULL);
    if(ret != 0)
        perror("\nMetaLocker create fail");

#endif // _WIN32
}

Locker::~Locker()
{
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    auto ret = pthread_mutex_destroy(&_handle);
    if(ret != 0)
        perror("pthread_mutex_destroy fail");

#else
    ::DeleteCriticalSection(&_handle);
#endif
}

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
bool Locker::IsOtherThreadOwnLock() const
{
    auto curThread = ULongToHandle(static_cast<ULong>(KernelGetCurrentThreadId()));
    return _handle.OwningThread && (_handle.OwningThread != curThread);
}
#endif
KERNEL_END

