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
 * Date: 2023-08-05 20:44:02
 * Author: Eric Yonng
 * Description: 采用预先向数据库拿81
*/

#pragma once

#include <service/common/BaseComps/GlobalUid/interface/IGlobalUidMgr.h>

SERVICE_BEGIN

class GlobalUidMgr : public IGlobalUidMgr
{
    POOL_CREATE_OBJ_DEFAULT_P1(IGlobalUidMgr, GlobalUidMgr);

public:
    GlobalUidMgr();
    ~GlobalUidMgr();
    void Release() override;

    void OnRegisterComps() override;

    Int32 OnLoaded(UInt64 key, const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) override;
    Int32 OnSave(UInt64 key, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) const override;

    // 全球唯一id
    virtual UInt64 NewGuid() override;

    // 提前取的id数量
    virtual void SetGetIdAheadCount(Int32 aheadCount) override;

    // 设置更新最大uid回调 同步调用(直到持久化完毕)
    virtual void SetUpdateLastIdCallback(KERNEL_NS::IDelegate<Int32, ILogicSys *> *deleg) override; 

    virtual void OnStartup() override;

    virtual KERNEL_NS::LibString ToString() const override;

protected:
    Int32 _OnGlobalSysInit() override;
    void _OnGlobalSysClose() override;
    void _Clear();

    UInt64 _curAllocUid;
    UInt64 _maxUid;
    UInt64 _aheadCount;
    UInt32 _machineId;
    KERNEL_NS::IDelegate<Int32, ILogicSys *> *_synStorageCb;

    KERNEL_NS::SnowflakeInfo _snowflakInfo;
};

SERVICE_END
