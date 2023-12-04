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

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <kernel/common/BaseType.h>

#include <kernel/comp/Utils/Defs/LibGuidDefs.h>
#include <kernel/comp/LibString.h>
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
    static UInt64 Snowflake(SnowflakeInfo &snowflakeInfo, UInt64 addNum);

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

ALWAYS_INLINE UInt64 GuidUtil::Snowflake(SnowflakeInfo &snowflakeInfo, UInt64 addNum)
{
    // 1.序号满,递增个时间单位,并重置成1
    const auto &maskInfo = snowflakeInfo._maskInfo;
    const auto lastSeq = addNum + snowflakeInfo._lastSeq;
    const auto multi = lastSeq / maskInfo._seqBitsLimitMask;
    const auto left = lastSeq % maskInfo._seqBitsLimitMask;
    snowflakeInfo._lastTime += multi;
    snowflakeInfo._lastSeq = left;

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

KERNEL_END

#endif
