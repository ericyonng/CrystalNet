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
 * Date: 2021-01-01 23:09:49
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Utils/TimeUtil.h>

KERNEL_BEGIN


std::atomic<Int32> __g_timezone{0};
Int64 TimeUtil::_systemTimeBegin = 0;
LibCpuCounter TimeUtil::_cpuBegin;

void TimeUtil::SetTimeZone()
{
    // 设置成服务器所在时区
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    tzset();
    time_t now = time(NULL);
    __g_timezone = -1 * static_cast<Int32>(localtime(&now)->tm_gmtoff);         // linux下localtime(&now)->tm_gmtoff是个整数，但是本框架需要取得相反的数，如+8时区，这里需要输出-28800
#else // WIN32
    _tzset();
    __g_timezone = _timezone;
#endif // Non-WIN32
}

// TODO需要验证接口
Int32 TimeUtil::GetTimeZone()
{
    return __g_timezone;
}

Int32 TimeUtil::GetMonthMaxDays(Int32 year, Int32 month)
{
    if(month >= 1 && month <= 7)
    {
        if(month % 2 == 1)
            return 31;
        else if(month == 2)
            return IsLeapYear(year) ? 29 : 28;
        else
            return 30;
    }
    else if(month >= 8 && month <= 12)
    {
        return month % 2 == 0 ? 31 : 30;
    }
    else
    {
        printf("error param");
        return -1;
    }
}

KERNEL_END
