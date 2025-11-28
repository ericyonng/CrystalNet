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
 * Date: 2022-01-08 04:29:08
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Cpu/LibCpuCounter.h>
#include <utility>

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    #include <WinSock2.h>
    #include <profileapi.h> // cpucounter
#endif

KERNEL_BEGIN

UInt64 LibCpuFrequency::_countPerSecond = 0; 

void LibCpuFrequency::InitFrequancy()
{
    _countPerSecond = KERNEL_NS::CrystalGetCpuCounterFrequancy();
//     _countPerMillisecond = std::max<UInt64>(_countPerSecond / TimeDefs::MILLI_SECOND_PER_SECOND, 1);
//     _countPerMicroSecond = std::max<UInt64>(_countPerSecond / TimeDefs::MICRO_SECOND_PER_SECOND, 1);
//
// #if CRYSTAL_TARGET_PLATFORM_LINUX
//     _countPerNanoSecond = std::max<UInt64>(_countPerSecond / TimeDefs::NANO_SECOND_PER_SECOND, 1);
// #else
//     _countPerNanoSecond = _countPerMicroSecond * 1000;
// #endif // CRYSTAL_TARGET_PLATFORM_LINUX
}

POOL_CREATE_OBJ_DEFAULT_IMPL(LibCpuSlice);



POOL_CREATE_OBJ_DEFAULT_IMPL(LibCpuCounter);

#if CRYSTAL_TARGET_PLATFORM_WINDOWS

void LibCpuCounter::_UpdateWin()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    _count = li.QuadPart;
}
#endif

KERNEL_END
