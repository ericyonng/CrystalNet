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
 * Date: 2021-03-19 21:16:03
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Lock/Impl/SpinLock.h>

// 平台有关库
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    #include <WinSock2.h>
#endif

KERNEL_BEGIN

#if CRYSTAL_TARGET_PLATFORM_LINUX

SpinLock::SpinLock()
{
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
pthread_spin_init(&_handle, 0);
#else
    ::memset(&_handle, 0, sizeof(_handle));
    // static const UInt32 maxDwordCount1 = CountUtil::Count1InBinary((std::numeric_limits<DWORD>::max)());
    // 自旋次数（最高位置1）
    DWORD spinCnt = static_cast<DWORD>(SPINNING_COUNT);
    // 创建自旋锁，避免线程频繁挂起
    if(!::InitializeCriticalSectionAndSpinCount(&(_handle), spinCnt))
        CRYSTAL_TRACE("SpinLock create spinlock fail spinCnt[%lu] last error:%d", spinCnt, GetLastError());
#endif

}

#endif

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
void SpinLock::Lock(Int64 loop_cnt_to_yield)
{
    // 自旋休眠次数
    const Int64 loop_count_to_yield = loop_cnt_to_yield;

    // test and wait 直到flag被clear
    bool exp = false;
    while(!_flag.compare_exchange_weak(exp, true, std::memory_order_acq_rel))
    {
        exp = false;

        // _mm_pause();
        if (LIKELY(--loop_cnt_to_yield))
            continue;

        // 自旋次数达到则切换线程
        #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
        #if CRYSTAL_TARGET_PLATFORM_LINUX || CRYSTAL_TARGET_PLATFORM_ANDROID || CRYSTAL_TARGET_PLATFORM_MAC
            asm volatile ("rep;nop" : : : "memory");
        #else
            asm volatile ("nop");
        #endif
        #else // WINDOWS platform
            YieldProcessor();
        #endif // Non-WINDOWS platform

        #ifdef _DEBUG
            printf("\nperhaps in dead loop!\n");
        #endif
        loop_cnt_to_yield = loop_count_to_yield;
    }
}
#endif

KERNEL_END
