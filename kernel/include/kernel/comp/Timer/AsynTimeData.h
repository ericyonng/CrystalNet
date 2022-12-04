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
 * 
 * Author: Eric Yonng
 * Date: 2021-03-17 15:05:35
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TIMER_ASYN_TIME_DATA_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TIMER_ASYN_TIME_DATA_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/memory.h>
#include <kernel/comp/Utils/BitUtil.h>

KERNEL_BEGIN

class TimeData;

class KERNEL_EXPORT AsynOpType
{
public:
    enum
    {
        OP_NONE = 0,            // 无效
        OP_REGISTER = 1,        // 注册
        OP_UNREGISTER = 2,      // 反注册
        OP_DESTROY = 3,         // 销毁
    };
};

class KERNEL_EXPORT AsynTimeData
{
    POOL_CREATE_OBJ_DEFAULT(AsynTimeData);

public:
    AsynTimeData(TimeData *data);
    void Release();

public:
    void MaskRegister(Int64 expireTime, Int64 newPeriod);
    void MaskUnRegister();
    void MaskDestroy();
    void Reset();
    bool IsMaskRegister() const;

public:
    UInt32 _flag;                   // 操作位标记 AsynOpType
    Int64 _newExpiredTime;          // 当前过期时间 微妙 修改数据时候使用
    Int64 _newPeriod;               // 定时周期 微妙    修改数据时候使用
    TimeData *_data;                // 原始的定时数据
};

inline AsynTimeData::AsynTimeData(TimeData *data)
    :_flag(AsynOpType::OP_NONE)
    ,_newExpiredTime(0)
    ,_newPeriod(0)
    ,_data(data)
{

}

inline void AsynTimeData::Release()
{
    AsynTimeData::DeleteThreadLocal_AsynTimeData(this);
}

inline void AsynTimeData::MaskRegister(Int64 expireTime, Int64 newPeriod)
{
    // 移除删除与修改
    _flag = BitUtil::Set(_flag, AsynOpType::OP_REGISTER);
    _newExpiredTime = expireTime;
    _newPeriod = newPeriod;
}

inline void AsynTimeData::MaskUnRegister()
{
    // 移除添加与修改
    _flag = BitUtil::Clear(_flag, AsynOpType::OP_REGISTER);
    _flag = BitUtil::Set(_flag, AsynOpType::OP_UNREGISTER);
}

inline void AsynTimeData::MaskDestroy()
{
    _flag = BitUtil::Clear(_flag, AsynOpType::OP_REGISTER);
    _flag = BitUtil::Clear(_flag, AsynOpType::OP_UNREGISTER);
    _flag = BitUtil::Set(_flag, AsynOpType::OP_DESTROY);
}

inline void AsynTimeData::Reset()
{
    _flag = 0;
}

inline bool AsynTimeData::IsMaskRegister() const
{
    return BitUtil::IsSet(_flag, AsynOpType::OP_REGISTER);
}

KERNEL_END


#endif


