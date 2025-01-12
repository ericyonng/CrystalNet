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
 * Date: 2025-01-05 15:41:13
 * Author: Eric Yonng
 * Description:
 * 分布式id构成：
 * [号段位] + [机器id] + [序列号]
 * 号段位: 当序列号位满时进一位
 * 机器id: 向中心注册机器id, 然后本地化生成, 为保证
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_IDGENERATOR_IMPL_IDGENERATOR_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_IDGENERATOR_IMPL_IDGENERATOR_H__

#pragma once

#include <kernel/comp/CompObject/CompObject.h>
#include <kernel/comp/Delegate/IDelegate.h>
#include <concepts>

KERNEL_BEGIN

struct KERNEL_EXPORT IdGeneratorEception : std::exception 
{
    // id占位异常
    [[nodiscard]] const char* what() const noexcept override 
    {
        return "IdGeneratorEception";
    }
};

// 首先UInt64 服务全球100亿人口不现实, 即使是国内15亿人, 64bit按照每人50qps那么也将在7年后耗尽, 所以不现实
// 按照2w一个节点, 分布式id能服务的人数其实仅限于, machine id的位数, 如果要同时服务3亿人需要14bit, 即16,384个节点
// 需要machine id 因为其实machine id是为了控制节点的并发消耗id的速度,如果没有machine id那么每启动一个就要占用一个号段, 如果并发几万个进程, 一下子就消耗光了，machine id是为了控制消耗速度,应该给machine id划分10bit
// id 结构 = [1bit符号位 = 0] + [35bit 号段位] + [10bit 机器id位] + [18bit 自增序列号位]
// 原理:采用提前占用号段位的方式来保证提前占用的号段一定是唯一的,序列号用完则向远程申请一个单位的号段, 使用machine id是因为避免启动很多个进程又关闭很多个进程后, id段被过快消耗掉
// 基础保障: 需要远程id号段分配节点是高可用的不允许服务时间内全部节点不可用 
// 单个节点理论消耗id 段个数: 按照网络来回50ms, 那么1秒内最多消耗20个id段
// 理论性能: 5,242,880 qps
// id可使用: 可以使用 54 年
class KERNEL_EXPORT IdGenerator : public CompObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, IdGenerator);

public:
    // id结构
    enum ID_PART_TYPE
    {
        // 有效位总共63位
        ID_VALID_MAX_BITS = 63,
        
        // 序列号起始位置
        SEQUANCE_ID_PART_START_POS = 0,
        // 序列号宽度
        SEQUANCE_ID_PART_WIDTH = 18,
        // 机器位起始
        MACHINE_ID_PART_START_POS = SEQUANCE_ID_PART_START_POS + SEQUANCE_ID_PART_WIDTH,
        // 机器位宽度
        MACHINE_ID_PART_WIDTH = 10,
        // 号段起始位置
        NUMBER_SEGMENT_PART_START_POS = MACHINE_ID_PART_START_POS + MACHINE_ID_PART_WIDTH,
        // 号段宽度
        NUMBER_SEGMENT_PART_WIDTH = ID_VALID_MAX_BITS - SEQUANCE_ID_PART_WIDTH - MACHINE_ID_PART_WIDTH,
        // 符号位起始位置
        SIGNAL_FLAG_START_POS = NUMBER_SEGMENT_PART_START_POS + NUMBER_SEGMENT_PART_WIDTH,
        SIGNAL_FLAG_WIDTH = 1,
    };

    // 最大序列id
    static constexpr UInt64 MAX_SEQUANCE_ID = (1LLU << SEQUANCE_ID_PART_WIDTH) - 1;
    // 最大机器id
    static constexpr UInt64 MAX_MACHINE_ID = (1LLU << MACHINE_ID_PART_WIDTH) - 1;

    #pragma region bignum
    static constexpr UInt64 BIG_NUM_MAX_SEQUANCE_ID = (1LLU << 32) - 1;
    static constexpr Int32 BIG_NUM_MACHINE_ID_POS = 32;

    #pragma endregion
    
public:
    IdGenerator();
    ~IdGenerator() override;

    virtual void Release() override;
    LibString ToString() const override;

    // id 结构 = [1bit符号位 = 0] + [35bit 号段位] + [10bit 机器id位] + [18bit 自增序列号位]
    // 原理:采用提前占id段的方式保证, id段范围内一定是唯一的,使用machine id是避免机器不断重启导致id段过快消耗
    // 基础保障: 需要远程id号段分配节点是高可用的不允许服务时间内全部节点不可用 
    // 每秒消耗id段: 按照网络来回50ms, 那么1秒内最多消耗20个id段
    // 理论性能: 5,242,880 qps
    // id可使用: 可以使用54年
    UInt64 NewId();

    // 占用号段回调, 如果没有设置的话, 那么会使用_DefaultOccupancyNumberSegmentMethod, 且生成的id符号位为1, 以便与正式的区别
    // @param UInt64&: 符号位
    // @param UInt64&: 号段
    // @param UInt64&: 机器id
    // @return bool: 是否成功 
    void SetOccupancyNumberSegmentDelegate(IDelegate<bool, UInt64&, UInt64 &, UInt64 &> *delg);

    // 更新号段
    void UpdateOccupancyNumberSegment();

protected:
    Int32 _OnCreated() override;
    Int32 _OnInit() override;
    Int32 _OnStart() override;
    void _OnWillClose() override;
    void _OnClose() override;

    // 默认占用id段的方法, 如果没有调用SetOccupancyNumberSegmentDelegate设置占用号段的回调,那么会使用默认的
    static bool _DefaultOccupancyNumberSegmentMethod(UInt64 &signalFlag, UInt64 &segment, UInt64 &machineId);
    
protected:
    // 序列号位
    UInt64 _lastSequanceId;
    // 号段位
    UInt64 _lastNumberSegment;
    // 机器id段
    UInt64 _machineId;
    // 符号位
    UInt64 _signalFlag;

    // 提前占用号段回调
    // @param UInt64&: 符号位
    // @param UInt64&: 号段
    // @param UInt64&: 机器id
    // @return bool: 是否成功 
    IDelegate<bool, UInt64&, UInt64 &, UInt64 &> *_occupancyNumberSegmentDelegate;
};

ALWAYS_INLINE UInt64 IdGenerator::NewId()
{
    // id用尽需要重新获取id段
    if(UNLIKELY((_lastSequanceId ^ MAX_SEQUANCE_ID) == 0LLU))
    {
        UpdateOccupancyNumberSegment();
        _lastSequanceId = 0;
    }

    return _signalFlag | (_lastNumberSegment << NUMBER_SEGMENT_PART_START_POS) | (_machineId << MACHINE_ID_PART_START_POS) | (++_lastSequanceId);
}

ALWAYS_INLINE void IdGenerator::SetOccupancyNumberSegmentDelegate(IDelegate<bool, UInt64&, UInt64 &, UInt64 &> *delg)
{
    if(_occupancyNumberSegmentDelegate)
        _occupancyNumberSegmentDelegate->Release();
    
    _occupancyNumberSegmentDelegate = delg;
}

ALWAYS_INLINE void IdGenerator::UpdateOccupancyNumberSegment()
{
    if(UNLIKELY(!_occupancyNumberSegmentDelegate->Invoke(_signalFlag, _lastNumberSegment, _machineId)))
    {
        throw IdGeneratorEception();
    }
}


KERNEL_END

#endif
