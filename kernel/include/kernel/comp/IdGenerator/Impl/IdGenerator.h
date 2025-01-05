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
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_IDGENERATOR_IMPL_IDGENERATOR_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_IDGENERATOR_IMPL_IDGENERATOR_H__

#pragma once

#include <kernel/comp/CompObject/CompObject.h>
#include <kernel/comp/Delegate/IDelegate.h>

KERNEL_BEGIN


// id 结构 = [1bit符号位 = 0] + [44bit号段位] + [19bit 自增序列号位]
// 原理: 移除machine id位, 放开对节点数量的限制, 采用提前占用号段位的方式来保证提前占用的号段一定是唯一的,17bit序列号用完则向远程申请一个单位的号段
// 基础保障: 需要远程id号段分配节点是高可用的不允许服务时间内全部节点不可用
// 理论性能: 如果考虑占位网络传输50ms, 那么可以达到, 10485760 qps, 如果不考虑网络因素, 最高 524288000 qps
// id可使用: 500年
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
        SEQUANCE_ID_PART_WIDTH = 19,
        // 号段起始位置
        NUMBER_SEGMENT_PART_START_POS = SEQUANCE_ID_PART_START_POS + SEQUANCE_ID_PART_WIDTH,
        // 号段宽度
        NUMBER_SEGMENT_PART_WIDTH = ID_VALID_MAX_BITS - SEQUANCE_ID_PART_WIDTH,
        // 符号位起始位置
        SIGNAL_FLAG_START_POS = NUMBER_SEGMENT_PART_START_POS + NUMBER_SEGMENT_PART_WIDTH,
        SIGNAL_FLAG_WIDTH = 1,
    };

    // 最大序列id
    static constexpr UInt64 MAX_SEQUANCE_ID = (1LLU << SEQUANCE_ID_PART_WIDTH) - 1;
    
public:
    IdGenerator();
    ~IdGenerator() override;

    virtual void Release() override;
    LibString ToString() const override;
    virtual void DefaultMaskReady(bool isReady) override {}

    // id 结构 = [1bit符号位 = 0] + [44bit号段位] + [19bit 自增序列号位]
    // 原理: 移除machine id位, 放开对节点数量的限制, 采用提前占用号段位的方式来保证提前占用的号段一定是唯一的,17bit序列号用完则向远程申请一个单位的号段
    // 基础保障: 需要远程id号段分配节点是高可用的不允许服务时间内全部节点不可用
    // 理论性能: 如果考虑占位网络传输50ms, 那么可以达到, 10485760 qps, 如果不考虑网络因素, 最高 524288000 qps
    // id可使用: 500年
    UInt64 NewId();

    // 占用号段回调, 如果没有设置的话, 那么会使用_DefaultOccupancyNumberSegmentMethod, 且生成的id符号位为1, 以便与正式的区别
    void SetOccupancyNumberSegmentDelegate(IDelegate<UInt64> *delg);

    // 更新号段
    void UpdateOccupancyNumberSegment();

protected:
    Int32 _OnCreated() override;
    Int32 _OnInit() override;
    Int32 _OnStart() override;
    void _OnWillClose() override;
    void _OnClose() override;

    // 默认占用id段的方法, 如果没有调用SetOccupancyNumberSegmentDelegate设置占用号段的回调,那么会使用默认的
    static UInt64 _DefaultOccupancyNumberSegmentMethod();
    
protected:
    // 序列号位
    UInt64 _lastSequanceId;
    // 号段位
    UInt64 _lastNumberSegment;
    // 符号位
    UInt64 _signalFlag;

    // 提前占用号段回调
    IDelegate<UInt64> *_occupancyNumberSegmentDelegate;
};

ALWAYS_INLINE UInt64 IdGenerator::NewId()
{
    // id用尽需要重新获取id段
    if(UNLIKELY((_lastSequanceId ^ MAX_SEQUANCE_ID) == 0LLU))
    {
        UpdateOccupancyNumberSegment();
        _lastSequanceId = 0;
    }

    return _signalFlag | (_lastNumberSegment << NUMBER_SEGMENT_PART_START_POS) | (++_lastSequanceId);
}

ALWAYS_INLINE void IdGenerator::SetOccupancyNumberSegmentDelegate(IDelegate<UInt64> *delg)
{
    if(_occupancyNumberSegmentDelegate)
        _occupancyNumberSegmentDelegate->Release();
    
    _occupancyNumberSegmentDelegate = delg;
}

ALWAYS_INLINE void IdGenerator::UpdateOccupancyNumberSegment()
{
    _lastNumberSegment = _occupancyNumberSegmentDelegate->Invoke();
    _signalFlag = (1LLU << SIGNAL_FLAG_START_POS) & _lastNumberSegment;
    _lastNumberSegment &= ~(1LLU << SIGNAL_FLAG_START_POS);
}


KERNEL_END

#endif
