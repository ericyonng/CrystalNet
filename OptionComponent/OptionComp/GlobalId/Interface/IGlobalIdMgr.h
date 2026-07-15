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
 * Date: 2026-07-06 16:59:12
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_OPTIONCOMP_GLOBAL_ID_INTERFRACE_GLOBAL_ID_MGR_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_OPTIONCOMP_GLOBAL_ID_INTERFRACE_GLOBAL_ID_MGR_H__

#pragma once

#include <kernel/comp/CompObject/CompHostObject.h>

KERNEL_BEGIN

class IMongoDbMgr;
class IGlobalParamMgr;

class IGlobalIdMgr : public CompHostObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, IGlobalIdMgr);
    
public:
    IGlobalIdMgr(UInt64 objTypeId) : CompHostObject(objTypeId){}

    // 定时5秒持久化, 最多丢5秒的数据, 除非mongodb集群全挂了
    // 一个机器id被占用, 会在12小时后才可用, 丢5秒是可控的(时间数据会取时间数据和当前时间的最大值, 12小时后可用可以覆盖掉丢失的5秒时间计数)
    // 接口最高2亿qps, 按照丢失5秒数据算, 时间位最多会丢8小时, 在12小时范围内,12小时后方可再次使用, 会取时间戳和时间位数据最大值,最大值覆盖了8小时的时间
    // id 理论上可以使用543年
    // [63bit] - [        34位       ] - [   14位   ] - [   15位   ]
    //   [0]      时间bit(相对基准时间)      machine id      seq id
    virtual Int64 NewId() = 0;

    // 基于mongodb
    virtual void SetMongodbMgr(IMongoDbMgr *mongodbMgr) = 0;
    // 设置GlobalParam
    virtual void SetGlobalParamMgr(IGlobalParamMgr *globalParamMgr) = 0;

    // 机器id
    virtual Int64 GetMachineId() const = 0;
    // 时间位
    virtual Int64 GetTimePart() const = 0;
    // owner
    virtual const KERNEL_NS::LibString &GetOwnerId() const = 0;

    // 强行占据机器id
    // 返回原owner id
    virtual CoTask<KERNEL_NS::LibString> ForceOccupyMachineId(Int64 machineId) = 0;

    // 最大机器id
    virtual Int64 GetMaxMachineId() const = 0;
};

KERNEL_END

#endif