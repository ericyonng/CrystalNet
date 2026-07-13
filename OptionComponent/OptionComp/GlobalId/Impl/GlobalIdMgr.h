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

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_OPTIONCOMP_GLOBAL_ID_IMPL_GLOBAL_ID_MGR_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_OPTIONCOMP_GLOBAL_ID_IMPL_GLOBAL_ID_MGR_H__

#pragma once

#include <OptionComponent/OptionComp/GlobalId/Interface/IGlobalIdMgr.h>
#include <atomic>

#include "OptionComp/storage/MongoDB/Impl/MongoSerializeInfo.h"


KERNEL_BEGIN

class LibTimer;

class GlobalIdMgr : public IGlobalIdMgr
{
    POOL_CREATE_OBJ_DEFAULT_P1(IGlobalIdMgr, GlobalIdMgr);
    
public:
    // 时间位宽
    static constexpr Int32 TIME_PART_WIDTH = 31;
    // 机器id位宽 14BIT
    static constexpr Int32 MACHINE_ID_WIDTH = 14;
    static constexpr Int32 SEQ_WIDTH = 18;

    // 时间位
    static constexpr Int64 TIME_PART_POS = SEQ_WIDTH + MACHINE_ID_WIDTH;
    // 机器id位
    static constexpr Int64 MACHINE_ID_POS = SEQ_WIDTH;
    // 时间掩码
    static constexpr Int64 TIME_PART_MASK = ~((1LL << TIME_PART_POS) - 1);

    // 序号位宽
    static constexpr Int64 SEQ_MASK = (1LL << SEQ_WIDTH) - 1;
    static constexpr Int64 SEQ_MOD = (1LL << SEQ_WIDTH);
    // 最大机器id
    static constexpr Int64 MAX_MACHINE_ID = (1LL << MACHINE_ID_WIDTH) - 1;
    // 机器id模
    static constexpr Int64 MACHINE_ID_MOD = (1LL << MACHINE_ID_WIDTH);
    // 机器id掩码
    static constexpr Int64 MACHINE_ID_MASK = ((1LL << MACHINE_ID_WIDTH) - 1) << SEQ_WIDTH;

    // 最大时间位值, 溢出时启动失败, 提示需要扩展id了
    static constexpr Int64 TIME_PART_MAX_ID = (1LL << TIME_PART_WIDTH);

    GlobalIdMgr();
    ~GlobalIdMgr() override;

    void Release() override;

    void OnRegisterComps() override;

    virtual Int64 NewId() override;

    virtual void SetMongodbMgr(IMongoDbMgr *mongodbMgr) override;
    virtual void SetGlobalParamMgr(IGlobalParamMgr *globalParamMgr) override;
    // 机器id
    virtual Int64 GetMachineId() const override;
    // 时间位
    virtual Int64 GetTimePart() const override;
    // 获取owner
    virtual const KERNEL_NS::LibString &GetOwnerId() const override;

private:
    Int32 _OnAfterCompsInit() override;
    Int32 _OnHostWillStart() override;
    void _OnHostClose() override;

    // 定时落地并更新心跳
    // 关闭时候必定不会触发 _OnTimerSave 因为HostClose时候销毁定时器
    void _OnTimerSave(LibTimer *t);

    // 线程池执行 数据落地
    CoTask<> _SaveData(bool isClose);

    KERNEL_NS::CoTask<bool> RegisterMachine();

private:
    alignas(SYSTEM_ALIGN_SIZE) IMongoDbMgr * _mongodbMgr;
    alignas(SYSTEM_ALIGN_SIZE) std::atomic<Int64> _lastId;
    alignas(SYSTEM_ALIGN_SIZE) IGlobalParamMgr *_globalParamMgr;
    
    // owner
    const KERNEL_NS::LibString _ownerId;
    // 过期时间
    KERNEL_NS::TimeSlice _invalidTime;

    // 基准时间 2026.01.01 00:00:00
    const KERNEL_NS::LibTime _baseTime;

    static const KERNEL_NS::LibString ParamCollectionFieldName;
    static const KERNEL_NS::LibString ParamCollectionKeyValue;

    /** GlobalId字段名 **/
    static const KERNEL_NS::LibString GlobalIdKeyName;
    
    // 当前时间 心跳时间 TimePart的最大值
    static const KERNEL_NS::LibString TimePartName;
    // 心跳时间, 续期保活
    static const KERNEL_NS::LibString HeartbeatTimeName;
    // 当前owner
    static const KERNEL_NS::LibString CurOwnerName;

    static const KERNEL_NS::LibString DbName;
    static const KERNEL_NS::LibString CollectionName;
    static const KERNEL_NS::LibString UniqueIndexName;

    // timer
    std::atomic<LibTimer *> _saveDataTime;
    std::atomic<Poller *> _saveDataPoller;

    // 定时同步时间间隔
    KERNEL_NS::TimeSlice _saveInterval;
};

KERNEL_END


#endif
