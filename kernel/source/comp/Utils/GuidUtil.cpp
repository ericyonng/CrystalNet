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
 * Date: 2023-11-26 15:13:50
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Utils/GuidUtil.h>
#include <string.h>
#include <kernel/comp/Utils/SystemUtil.h>
#include <kernel/comp/Utils/TimeUtil.h>

#if CRYSTAL_TARGET_PLATFORM_LINUX
    #include <uuid/uuid.h> // 真正在uuid/uuid.h
#endif

KERNEL_BEGIN

LibGuid GuidUtil::Gen()
{
    LibGuid guid;
    ::memset(&guid, 0, sizeof(LibGuid));

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    uuid_generate(reinterpret_cast<unsigned char *>(&guid));
#else
    ::CoCreateGuid(&guid);
#endif

    return guid;
}

bool GuidUtil::InitSnowFlake(SnowflakeInfo &historyInfo
, UInt64 instanceId
, UInt64 lastTime
, UInt64 lastSeq
, UIDMaskInfo *maskInfo)
{
    if(historyInfo._isInit)
        return true;

    historyInfo._isInit = true;
    if(maskInfo)
    {
        auto &historyMaskInfo = historyInfo._maskInfo;
        historyMaskInfo._timeBits = maskInfo->_timeBits;        
        historyMaskInfo._instanceBits = maskInfo->_instanceBits;
        historyMaskInfo._seqBits = maskInfo->_seqBits;
        historyMaskInfo._timeBitsLimitMask = maskInfo->_timeBitsLimitMask;
        historyMaskInfo._timePosMask = maskInfo->_timePosMask;
        historyMaskInfo._instanceBitsLimitMask = maskInfo->_instanceBitsLimitMask;
        historyMaskInfo._instancePosMask = maskInfo->_instancePosMask;
        historyMaskInfo._seqBitsLimitMask = maskInfo->_seqBitsLimitMask;
        historyMaskInfo._seqPosMask = maskInfo->_seqPosMask;
    }

    historyInfo._instanceId = instanceId;
    historyInfo._lastSeq = lastSeq;
    historyInfo._lastTime = lastTime;

    return true;
}

bool GuidUtil::InitSnowFlakeById(SnowflakeInfo &historyInfo, UInt64 lastId, UIDMaskInfo *maskInfo)
{
    if(UNLIKELY(!lastId))
        return false;

    // 获取lastId各部分值
    auto &historyMaskInfo = historyInfo._maskInfo;
    UInt64 lastTime = 0;
    UInt64 instanceId = 0;
    UInt64 lastSeq = 0;
    GetSnowflakeIdPart(historyMaskInfo, lastId, lastTime, instanceId, lastSeq);
    return InitSnowFlake(historyInfo, instanceId, lastTime, lastSeq, maskInfo);
}

bool GuidUtil::InitSnowflakeBaseSysTime(SnowflakeInfo &historyInfo, UInt64 instanceId, UInt64 startMicroSecondTime, UIDMaskInfo *maskInfo)
{
    if(historyInfo._isInit)
        return true;

    if(UNLIKELY(!instanceId))
        return false;

    historyInfo._isInit = true;
    if(maskInfo)
    {
        auto &historyMaskInfo = historyInfo._maskInfo;
        historyMaskInfo._timeBits = maskInfo->_timeBits;        
        historyMaskInfo._instanceBits = maskInfo->_instanceBits;
        historyMaskInfo._seqBits = maskInfo->_seqBits;
        historyMaskInfo._timeBitsLimitMask = maskInfo->_timeBitsLimitMask;
        historyMaskInfo._timePosMask = maskInfo->_timePosMask;
        historyMaskInfo._instanceBitsLimitMask = maskInfo->_instanceBitsLimitMask;
        historyMaskInfo._instancePosMask = maskInfo->_instancePosMask;
        historyMaskInfo._seqBitsLimitMask = maskInfo->_seqBitsLimitMask;
        historyMaskInfo._seqPosMask = maskInfo->_seqPosMask;
    }

    // 起始时间不能大于当前时间
    const UInt64 nowTime = static_cast<UInt64>(TimeUtil::GetFastMicroTimestamp());
    if(startMicroSecondTime > nowTime)
        return false;

    // 极小概率发生,程序崩溃后拉起的lastTime和最后一个uid的lastTime一样,若一样则可能造成uid重复
    historyInfo._startMicroSecondTime = startMicroSecondTime;
    historyInfo._instanceId = instanceId;
    historyInfo._lastSeq = 0;
    historyInfo._lastTime = static_cast<UInt64>(((nowTime - startMicroSecondTime) / TimeDefs::MICRO_SECOND_PER_MILLI_SECOND));

    return true;
}

UInt64 GuidUtil::SnowflakeBaseSysTime(SnowflakeInfo &snowflakeInfo)
{
    static const UInt64 microSecPerMilli = static_cast<Int64>(TimeDefs::MICRO_SECOND_PER_MILLI_SECOND);
    const auto &maskInfo = snowflakeInfo._maskInfo;

    // GetMicroTimestamp 耗时比较大 TODO:
    const UInt64 timePart = snowflakeInfo._lastTime;
    const UInt64 startMicroTime = snowflakeInfo._startMicroSecondTime;
    UInt64 nowMicro = static_cast<UInt64>(TimeUtil::GetFastMicroTimestamp()) - startMicroTime;
    UInt64 nowMilli = nowMicro / microSecPerMilli;
    const bool willOverSeq = ((snowflakeInfo._lastSeq + 1) & maskInfo._seqBitsLimitMask) == 0;

    if(UNLIKELY(timePart > nowMilli))
    {// 时钟回拨需要追时
        const UInt64 needNowTimeAtLeast = willOverSeq ? (timePart + 1) : timePart;
        const UInt64 needNowTimeMicroAtLeast = needNowTimeAtLeast * microSecPerMilli;
        while (UNLIKELY(nowMilli < needNowTimeAtLeast))
        {
            CRYSTAL_TRACE("system time back nowMicro = [%llu] _lastTime = [%llu]", nowMicro, snowflakeInfo._lastTime);
            SystemUtil::ThreadSleep(0, needNowTimeMicroAtLeast - nowMicro);
            nowMicro = static_cast<UInt64>(TimeUtil::GetFastMicroTimestamp()) - startMicroTime;
            nowMilli = nowMicro / microSecPerMilli;
            // std::cout << "system time back nowMicro = " << nowMicro << std::endl;
            // std::cout << "system time back _lastTime = " << snowflakeInfo._lastTime << std::endl;
            // lck.Unlock();
            // return 0;
        }

        if(willOverSeq)
            snowflakeInfo._lastSeq = 0;
    }
    else
    {
        // 序号溢出等待下一毫秒
        if(UNLIKELY(willOverSeq))
        {
            const UInt64 needNowTimeAtLeast = timePart + 1;
            const UInt64 needNowTimeMicroAtLeast = needNowTimeAtLeast * microSecPerMilli;
            while (UNLIKELY(nowMilli < needNowTimeAtLeast))
            {
                CRYSTAL_TRACE("system time back nowMicro = [%llu] _lastTime = [%llu]", nowMicro, snowflakeInfo._lastTime);
                SystemUtil::ThreadSleep(0, needNowTimeMicroAtLeast - nowMicro);
                nowMicro = static_cast<UInt64>(TimeUtil::GetFastMicroTimestamp()) - startMicroTime;
                nowMilli = nowMicro / microSecPerMilli;
                // std::cout << "system time back nowMicro = " << nowMicro << std::endl;
                // std::cout << "system time back _lastTime = " << snowflakeInfo._lastTime << std::endl;
                // lck.Unlock();
                // return 0;
            }

            snowflakeInfo._lastSeq = 0;
        }
    }

    ++snowflakeInfo._lastSeq;
    snowflakeInfo._lastTime = nowMilli;

    return snowflakeInfo.ToId();
}

KERNEL_END
