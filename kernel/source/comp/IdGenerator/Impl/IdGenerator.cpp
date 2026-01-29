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
 * Date: 2025-01-05 17:01:13
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/IdGenerator/Impl/IdGenerator.h>
#include <kernel/comp/IdGenerator/Impl/IdGeneratorFactory.h>
#include <kernel/comp/Utils/RttiUtil.h>

#include "kernel/comp/LibTraceId.h"
#include "kernel/comp/Log/ILog.h"

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(IdGenerator);

IdGenerator::IdGenerator()
 :CompObject(RttiUtil::GetTypeId<IdGenerator>())
,_lastSequanceId(0)
,_lastNumberSegment(0)
,_occupancyNumberSegmentDelegate(NULL)
{
    _occupancyNumberSegmentDelegate = DelegateFactory::Create(&IdGenerator::_DefaultOccupancyNumberSegmentMethod);
}

IdGenerator::~IdGenerator()
{
  CRYSTAL_RELEASE_SAFE(_occupancyNumberSegmentDelegate);
}

void IdGenerator::Release()
{
    IdGenerator::DeleteByAdapter_IdGenerator(IdGeneratorFactory::_buildType.V, this);
}

LibString IdGenerator::ToString() const
{
    return LibString("IdGenerator").AppendFormat("-[NumberSegment]:%llu, [SequanceId]:%llu", _lastNumberSegment, _lastSequanceId);
}

Int32 IdGenerator::_OnCreated()
{
    return Status::Success;
}

Int32 IdGenerator::_OnInit()
{
    if(!_occupancyNumberSegmentDelegate)
    {
        if (g_Log)
            g_Log->Error(LOGFMT_OBJ_TAG("have no occupancy number segment delegate, please check"));
        return Status::Failed;
    }
    
    return Status::Success;
}

Int32 IdGenerator::_OnStart()
{
    // 初始化占用一个id段
    UpdateOccupancyNumberSegment();

    // 当前trace, 更新
    auto trace  = LibTraceId::GetCurrentTrace();
    trace->UpdateMain();
    trace->UpdateSub();
    return Status::Success;
}

void IdGenerator::_OnWillClose()
{
    
}

void IdGenerator::_OnClose()
{
    
}

bool IdGenerator::_DefaultOccupancyNumberSegmentMethod(UInt64 &segment, UInt64 &machineId)
{
    static std::atomic<UInt64> s_maxNumberSegment {0};
    static std::atomic<UInt64> s_machineId {0};

    // 机器id
    auto origin = s_machineId.load(std::memory_order_acquire);
    machineId = origin % MAX_MACHINE_ID + 1;
    while (!s_machineId.compare_exchange_weak(origin, machineId, std::memory_order_acq_rel))
        machineId = origin % MAX_MACHINE_ID + 1;
    
    // id段
    segment = s_maxNumberSegment.fetch_add(1, std::memory_order_release) + 1;

    return true;
}

KERNEL_END
