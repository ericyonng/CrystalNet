// MIT License
// 
// Copyright (c) 2020 ericyonng<120453674@qq.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// Date: 2026-07-06 23:07:54
// Author: Eric Yonng
// Description:


#ifndef __CRYSTAL_NET_OPTION_COMPONENT_OPTIONCOMP_GLOBAL_ID_IMPL_GLOBAL_ID_MGR_MONGO_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_OPTIONCOMP_GLOBAL_ID_IMPL_GLOBAL_ID_MGR_MONGO_H__

#pragma once

#include <OptionComp/storage/MongoDB/Impl/IMongodbStorageInfo.h>

KERNEL_BEGIN

class GlobalIdMgrMongo : public KERNEL_NS::IMongodbStorageInfo
{
    POOL_CREATE_OBJ_DEFAULT_P1(IMongodbStorageInfo, GlobalIdMgrMongo);
    
public:
    GlobalIdMgrMongo();
    ~GlobalIdMgrMongo() override;

    virtual void Release() override;
    virtual Int32 _OnHostInit() override;

    /** 字段名 **/
    static constexpr const Byte8 *KeyName = "MachineId";
    // 当前时间 心跳时间 TimePart的最大值
    static constexpr const Byte8 *TimePartName = "TimePart";
    // 心跳时间, 续期保活
    static constexpr const Byte8 *HeartbeatTimeName = "HeartbeatTime";
    // 当前owner
    static constexpr const Byte8 *CurOwnerName = "CurOwner";

    static constexpr const Byte8 *DbName = "GlobalIdDb";
};

KERNEL_END

#endif
