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
    class GlobalIdMgr : public IGlobalIdMgr
{
    POOL_CREATE_OBJ_DEFAULT_P1(IGlobalIdMgr, GlobalIdMgr);
public:
    GlobalIdMgr();
    ~GlobalIdMgr() override;

    void Release() override;

    void OnRegisterComps() override;

    virtual Int64 NewId() override;

    virtual void SetMongoProxy(IMongodbProxy *mongoProxy) override;

private:
    Int32 _OnAfterCompsInit() override;
    Int32 _OnHostWillStart() override;
    void _OnHostClose() override;

    // 1. 注册机器id
    // 2. 定时同步时间部分
    // 3. 维持心跳续期

    void RegisterMachine();

    Int64 TryOccupiedMachine(std::map<KERNEL_NS::LibString, MongoSerializeInfo> *dict, const LibTime &nowByBase);

private:
    std::atomic<Int64> _lastId;
    IMongodbProxy* _mongoProxy;

    // owner
    const KERNEL_NS::LibString _ownerId;
    // 心跳
    KERNEL_NS::LibTime _heartbeatTime;

    // 过期时间
    KERNEL_NS::TimeSlice _invalidTime;

    // 基准时间 2026.01.01 00:00:00
    const KERNEL_NS::LibTime _baseTime;

    // lockPre
    const KERNEL_NS::LibString _lockPrefix;
};

KERNEL_END


#endif
