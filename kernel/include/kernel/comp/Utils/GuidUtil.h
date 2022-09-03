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
 * Date: 2021-01-06 01:23:02
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_GUID_UTIL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_GUID_UTIL_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/Utils/Defs/LibGuidDefs.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/LibTime.h>
#include <kernel/comp/Lock/Lock.h>
#include <kernel/comp/Utils/SystemUtil.h>
#include <kernel/comp/Utils/Defs/UidDefs.h>

KERNEL_BEGIN

// guid是业务相关不建议底层使用
class KERNEL_EXPORT GuidUtil
{
public:
    /**
     * Generate GUID.
     * @return LibGuid - GUID value.
     */
    static LibGuid Gen();

    /**
     * Format GUID.
     * @param[in] guid - GUID value.
     * @return LibString - formatted GUID value.
     */
    static LibString Format(const LibGuid &guid);

    /**
     * Generate GUID and Format the GUID structure data to string format.
     * @return FS_String - the string format guid.
     */
    static LibString GenStr();

    // 初始化雪花算法数据 建议时间使用秒做单位 maskInfo修改 uidmask maskInfo 默认使用默认值 依赖存储最后一个uid
    static bool InitSnowFlake(SnowflakeInfo &historyInfo
    , UInt64 instanceId
    , UInt64 lastTime = 0
    , UInt64 lastSeq = 0
    , UIDMaskInfo *maskInfo = NULL);
    // 初始化雪花算法数据 建议时间使用秒做单位 maskInfo修改 uidmask maskInfo 默认使用默认值 依赖存储最后一个uid
    static bool InitSnowFlakeById(SnowflakeInfo &historyInfo, UInt64 lastId, UIDMaskInfo *maskInfo = NULL);

    // 雪花算法生成分布式id [time] + [instance] + [seq]
    // 不依赖于时钟的雪花算法,可重入
    // 满序列号触发时钟递增, 
    // 建议外部将SnowflakeHistoryInfo持久化处理避免id重复 线程安全
    // 单实例 性能是1500w+/s 调用耗时在66ns左右 , 在高并发(最高性能情况)下至少提供26年(起始时间设置成当下)的id使用
    // ,理论最高可以使用69000+年 建议时间单位：秒 
    static UInt64 Snowflake(SnowflakeInfo &snowflakeInfo);

    // 获取时间,实例id, seqid
    static void GetSnowflakeIdPart(const UIDMaskInfo &maskInfo, UInt64 uid, UInt64 &idTime, UInt64 &instanceId, UInt64 &seq);

private:
    // 波动到下一id
    static void ToggleToNext(SnowflakeInfo &snowflakeInfo);

public:
    // 依赖系统时间的雪花算法 目的上个版本的雪花算法不足之处是可以利用两个id相减计算出一段时间内的生成量,这是不被允许的
    // 单一实例 支持69年,时间采用毫秒级,并发性能：430w+/s(windows) linux下（193w+）在于时钟调用的复杂性, 依赖系统时钟,时钟回拨方法调用会失败
    // 指定startMicroSecondTime可以提升雪花算法使用年限
    // 返回0表示失败 调用耗时最大在513ns左右
    static bool InitSnowflakeBaseSysTime(SnowflakeInfo &historyInfo, UInt64 instanceId, UInt64 startMicroSecondTime = 0, UIDMaskInfo *maskInfo = NULL);
    static UInt64 SnowflakeBaseSysTime(SnowflakeInfo &snowflakeInfo);

};

ALWAYS_INLINE LibGuid GuidUtil::Gen()
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

ALWAYS_INLINE LibString GuidUtil::Format(const LibGuid &guid)
{
    LibString str;
    str.AppendFormat("%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
               static_cast<UInt32>(guid.Data1),
               static_cast<UInt32>(guid.Data2),
               static_cast<UInt32>(guid.Data3),
               static_cast<UInt32>(guid.Data4[0]),
               static_cast<UInt32>(guid.Data4[1]),
               static_cast<UInt32>(guid.Data4[2]),
               static_cast<UInt32>(guid.Data4[3]),
               static_cast<UInt32>(guid.Data4[4]),
               static_cast<UInt32>(guid.Data4[5]),
               static_cast<UInt32>(guid.Data4[6]),
               static_cast<UInt32>(guid.Data4[7]));

    return str;
}

ALWAYS_INLINE LibString GuidUtil::GenStr()
{
    return Format(Gen());
}

ALWAYS_INLINE bool GuidUtil::InitSnowFlake(SnowflakeInfo &historyInfo
, UInt64 instanceId
, UInt64 lastTime
, UInt64 lastSeq
, UIDMaskInfo *maskInfo)
{
    if(historyInfo._isInit)
        return true;

    if(UNLIKELY(!instanceId || !lastTime))
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

    historyInfo._instanceId = instanceId;
    historyInfo._lastSeq = lastSeq;
    historyInfo._lastTime = lastTime;

    return true;
}

ALWAYS_INLINE bool GuidUtil::InitSnowFlakeById(SnowflakeInfo &historyInfo, UInt64 lastId, UIDMaskInfo *maskInfo)
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

ALWAYS_INLINE UInt64 GuidUtil::Snowflake(SnowflakeInfo &snowflakeInfo)
{
    // 1.序号满,递增个时间单位,并重置成1
    ++snowflakeInfo._lastSeq;
    const auto &maskInfo = snowflakeInfo._maskInfo;
    if(UNLIKELY((snowflakeInfo._lastSeq & maskInfo._seqBitsLimitMask) == 0))
    {
        snowflakeInfo._lastTime += 1;
        snowflakeInfo._lastSeq = 1;
    }

    // 3.组装id [time] + [instance] + [seq]
    return snowflakeInfo.ToId();
}

ALWAYS_INLINE void GuidUtil::GetSnowflakeIdPart(const UIDMaskInfo &maskInfo, UInt64 uid, UInt64 &idTime, UInt64 &instanceId, UInt64 &seq)
{
    idTime = (uid & maskInfo._timePosMask) >> (maskInfo._instanceBits + maskInfo._seqBits);
    instanceId = (uid & maskInfo._instancePosMask) >> maskInfo._seqBits;
    seq = uid & maskInfo._seqPosMask;
}

ALWAYS_INLINE void GuidUtil::ToggleToNext(SnowflakeInfo &snowflakeInfo)
{
    // 1.序号满,递增个时间单位,并重置成1
    ++snowflakeInfo._lastSeq;
    if(UNLIKELY((snowflakeInfo._lastSeq & snowflakeInfo._maskInfo._seqBitsLimitMask) == 0))
    {
        snowflakeInfo._lastTime += 1;
        snowflakeInfo._lastSeq = 1;
    }
}

ALWAYS_INLINE bool GuidUtil::InitSnowflakeBaseSysTime(SnowflakeInfo &historyInfo, UInt64 instanceId, UInt64 startMicroSecondTime, UIDMaskInfo *maskInfo)
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
    const UInt64 nowTime = static_cast<UInt64>(TimeUtil::GetMicroTimestamp());
    if(startMicroSecondTime > nowTime)
        return false;

    // 极小概率发生,程序崩溃后拉起的lastTime和最后一个uid的lastTime一样,若一样则可能造成uid重复
    historyInfo._startMicroSecondTime = startMicroSecondTime;
    historyInfo._instanceId = instanceId;
    historyInfo._lastSeq = 0;
    historyInfo._lastTime = static_cast<UInt64>(((nowTime - startMicroSecondTime) / TimeDefs::MICRO_SECOND_PER_MILLI_SECOND));

    return true;
}

ALWAYS_INLINE UInt64 GuidUtil::SnowflakeBaseSysTime(SnowflakeInfo &snowflakeInfo)
{
    static const UInt64 microSecPerMilli = static_cast<Int64>(TimeDefs::MICRO_SECOND_PER_MILLI_SECOND);
    const auto &maskInfo = snowflakeInfo._maskInfo;

    // GetMicroTimestamp 耗时比较大 TODO:
    const UInt64 timePart = snowflakeInfo._lastTime;
    const UInt64 startMicroTime = snowflakeInfo._startMicroSecondTime;
    UInt64 nowMicro = static_cast<UInt64>(TimeUtil::GetMicroTimestamp()) - startMicroTime;
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
            nowMicro = static_cast<UInt64>(TimeUtil::GetMicroTimestamp()) - startMicroTime;
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
                nowMicro = static_cast<UInt64>(TimeUtil::GetMicroTimestamp()) - startMicroTime;
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

#endif
