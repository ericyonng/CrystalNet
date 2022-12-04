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
 * Date: 2020-12-27 23:19:58
 * Author: Eric Yonng
 * Description: TODO: 可以设置一个id缓冲池,从一个地方集中的拿1个小时的id,拿回本地后,全局id就从池子中取 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_DEFS_UID_DEFS_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_DEFS_UID_DEFS_H__

#pragma once

#include <kernel/kernel_inc.h>

KERNEL_BEGIN

// 默认的id分布 选项
class KERNEL_EXPORT UIDMask
{
    // 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111
public:
    // [41BITS TIME] + [10BITS INSTANCE] + [13BITS SEQ]
    enum LEN_LIMIT : UInt64
    {
        TIME_BITS = 41,                             // 时间戳长度 高41位 建议用秒,可以使用比较久,毫秒太占用id位数
        INSTANCE_BITS = 10,                         // 实例id位长
        SEQ_BITS = 13,                              // 自增id位长
    };

    enum MASK : UInt64
    {
        TIME_BITS_LIMIT_MASK     = ((1llu << LEN_LIMIT::TIME_BITS) - 1),       // 默认的时间戳长度限制 41位
        TIME_POS_MASK            = TIME_BITS_LIMIT_MASK << (LEN_LIMIT::INSTANCE_BITS + LEN_LIMIT::SEQ_BITS),     // 默认的时间戳长度限制 高41位全1 time值掩码, uid & TIME_POS_MASK 可以获得time
        INSTANCE_BITS_LIMIT_MASK  = ((1llu << LEN_LIMIT::INSTANCE_BITS) - 1),                     // 机器码id长度限制
        INSTANCE_POS_MASK         = (INSTANCE_BITS_LIMIT_MASK << LEN_LIMIT::SEQ_BITS),  // instanceId值掩码, uid & INSTANCE_POS_MASK 可以获得instanceId
        SEQ_BITS_LIMIT_MASK      = ((1llu << LEN_LIMIT::SEQ_BITS) - 1),
        SEQ_POS_MASK             = (SEQ_BITS_LIMIT_MASK),   // seq值掩码, uid & SEQ_POS_MASK 可以获得seq
    };
};

struct KERNEL_EXPORT UIDMaskInfo
{
    UInt64 _timeBits = UIDMask::TIME_BITS;
    UInt64 _timeMovePos = UIDMask::INSTANCE_BITS + UIDMask::SEQ_BITS;
    UInt64 _instanceBits = UIDMask::INSTANCE_BITS;
    UInt64 _seqBits = UIDMask::SEQ_BITS;
    UInt64 _timeBitsLimitMask = UIDMask::TIME_BITS_LIMIT_MASK;
    UInt64 _timePosMask = UIDMask::TIME_POS_MASK;
    UInt64 _instanceBitsLimitMask = UIDMask::INSTANCE_BITS_LIMIT_MASK;
    UInt64 _instancePosMask = UIDMask::INSTANCE_POS_MASK;
    UInt64 _seqBitsLimitMask = UIDMask::SEQ_BITS_LIMIT_MASK;
    UInt64 _seqPosMask = UIDMask::SEQ_POS_MASK;
};

// 雪花算法历史信息 [time] + [instanceId] + [seq] 为保证唯一性,请保证一个进程一个instanceId，必须持久化,优点不依赖时钟,且全球唯一
struct KERNEL_EXPORT SnowflakeInfo
{
    SnowflakeInfo(UInt64 instanceId = 0, UInt64 startMicroSecondTime = 0, UInt64 lastTime = 0, UInt64 seq = 0)
        :_startMicroSecondTime(startMicroSecondTime)
        ,_lastTime(lastTime)
        , _instanceId(instanceId)
        , _lastSeq(seq)
        ,_isInit(false)
    {

    }

    UInt64 ToId() const;

    // // id位分布一个实例统一使用,可自行调整规则UIDMaskInfo,默认使用UIDMask
    UIDMaskInfo _maskInfo;
    

    UInt64 _startMicroSecondTime;           // 用于依赖系统时间,指定一个起始的时间戳（微妙时间戳）
    UInt64 _lastTime;            // 历史时间
    UInt64 _instanceId;          // 机器实例id
    UInt64 _lastSeq;             // 序列号
    bool _isInit;                // 是否初始化
};

ALWAYS_INLINE UInt64 SnowflakeInfo::ToId() const
{
    // 组装id [time] + [instance] + [seq]
    return  (_lastTime << _maskInfo._timeMovePos) |
            ((_instanceId & _maskInfo._instanceBitsLimitMask) << _maskInfo._seqBits) | 
            (_lastSeq & _maskInfo._seqBitsLimitMask);
}

KERNEL_END

#endif
