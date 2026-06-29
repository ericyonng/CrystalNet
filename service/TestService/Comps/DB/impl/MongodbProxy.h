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
// Date: 2026-06-24 22:06:31
// Author: Eric Yonng
// Description:
// 存储
// |--- A, 有B组件需要存储, 那么B作为字段BCompData整体存储，AUniqueId: A的唯一索引, _Id: Mongodb自动生成的唯一id
//
// doc:
// {
//     _Id:xxx,
//     AUniqueId:xxx,
//     BCompData:xxx,
// }

#pragma once

#include <Comps/DB/interface/IMongodbProxy.h>
#include <OptionComp/storage/MongoDB/MongoDBComp.h>
#include <unordered_set>
#include <unordered_map>
#include <map>

SERVICE_BEGIN

class StorageMode
{
public:
    enum ENUMS
    {
        BEGIN = 1,
        ADD_DATA = BEGIN,
        UPDATE_DATA,
        DEL_DATA,
        REPLACE_DATA,
        MAX_TYPE,
    };
};

class MongodbProxy : public IMongodbProxy
{
    POOL_CREATE_OBJ_DEFAULT_P1(IMongodbProxy, MongodbProxy);
    
public:
    MongodbProxy();
    virtual ~MongodbProxy() override;
    
    void Release() override;
    virtual void OnRegisterComps() override;
    virtual void DefaultMaskReady(bool isReady) override {}

    // 添加依赖
    virtual void RegisterDependence(ILogicSys *obj) override;
    virtual void UnRegisterDependence(const ILogicSys *obj) override;

    // 标脏
    virtual void MaskLogicNumberKeyAddDirty(const ILogicSys *logic, Int64 key) override;
    virtual void MaskLogicNumberKeyModifyDirty(const ILogicSys *logic, Int64 key) override;
    virtual void MaskLogicNumberKeyDeleteDirty(const ILogicSys *logic, Int64 key) override;
    virtual void MaskLogicStringKeyAddDirty(const ILogicSys *logic, const KERNEL_NS::LibString &key) override;
    virtual void MaskLogicStringKeyModifyDirty(const ILogicSys *logic, const KERNEL_NS::LibString &key) override;
    virtual void MaskLogicStringKeyDeleteDirty(const ILogicSys *logic, const KERNEL_NS::LibString &key) override;

    // 等待落库完成
    virtual KERNEL_NS::CoTask<> Purge() override;
    // 等待logic落库完成
    virtual KERNEL_NS::CoTask<> Purge(const ILogicSys *logic) override;
    
    virtual KERNEL_NS::CoTask<bool> Query(KERNEL_NS::LibString dbName, const ILogicSys *logic, Int64 key, std::map<KERNEL_NS::LibString, KERNEL_NS::MongoSerializeInfo> *fieldNameRefDataResult) override;
    virtual KERNEL_NS::CoTask<bool> Query(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, KERNEL_NS::LibString keyName, Int64 key, std::map<KERNEL_NS::LibString, KERNEL_NS::MongoSerializeInfo> *fieldNameRefDataResult) override;
    virtual KERNEL_NS::CoTask<bool> Query(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, KERNEL_NS::LibString keyName, KERNEL_NS::LibString key, std::map<KERNEL_NS::LibString, KERNEL_NS::MongoSerializeInfo> *fieldNameRefDataResult) override;

protected:
    Int32 _OnGlobalSysInit() override;
    Int32 _OnGlobalSysCompsCreated() override;

    Int32 _OnHostStart() override;
    void _OnGlobalSysClose() override;

    void _CloseServiceEvent(KERNEL_NS::LibEvent *ev);
    void _OnPurgeTimer(KERNEL_NS::LibTimer *t);

    void _CheckQuit();

    void _Clear();

    void _InitDirtyHelper(KERNEL_NS::LibDirtyHelper<Int64, UInt64> *dirtyHelper);
    void _InitDirtyHelper(KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper);

    // 多字段的脏回调 params和dirtyHelper均可能失效,需要注意
    KERNEL_NS::CoTask<> _OnNumberAddDirtyHandler(KERNEL_NS::LibDirtyHelper<Int64, UInt64> *dirtyHelper, Int64 &key, KERNEL_NS::Variant *params);
    KERNEL_NS::CoTask<> _OnNumberModifyDirtyHandler(KERNEL_NS::LibDirtyHelper<Int64, UInt64> *dirtyHelper, Int64 &key, KERNEL_NS::Variant *params);
    KERNEL_NS::CoTask<> _OnNumberDeleteDirtyHandler(KERNEL_NS::LibDirtyHelper<Int64, UInt64> *dirtyHelper, Int64 &key, KERNEL_NS::Variant *params);
    KERNEL_NS::CoTask<> _OnNumberReplaceDirtyHandler(KERNEL_NS::LibDirtyHelper<Int64, UInt64> *dirtyHelper, Int64 &key, KERNEL_NS::Variant *params);

    KERNEL_NS::CoTask<> _OnStringAddDirtyHandler(KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper, KERNEL_NS::LibString &key, KERNEL_NS::Variant *params);
    KERNEL_NS::CoTask<> _OnStringModifyDirtyHandler(KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper, KERNEL_NS::LibString &key, KERNEL_NS::Variant *params);
    KERNEL_NS::CoTask<> _OnStringDeleteDirtyHandler(KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper, KERNEL_NS::LibString &key, KERNEL_NS::Variant *params);
    KERNEL_NS::CoTask<> _OnStringReplaceDirtyHandler(KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper, KERNEL_NS::LibString &key, KERNEL_NS::Variant *params);

    KERNEL_NS::CoTask<> _DorPurgeNumber(const ILogicSys *sys);
    KERNEL_NS::CoTask<> _DorPurgeString(const ILogicSys *sys);

    bool _CheckLogic(const ILogicSys *logic, KERNEL_NS::LibString &err) const;
    void _BuildSysFields(const ILogicSys *logic);
private:
    // 关服时需要执行所有数据落地
    KERNEL_NS::ListenerStub _closeServiceStub;

    // 所有表建立的标脏回调, 系统退出时移除
    std::map<const ILogicSys *, KERNEL_NS::LibDirtyHelper<Int64, UInt64> *> _logicRefNumberDirtyHelper;
    std::map<const ILogicSys *, KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *> _logicRefStringDirtyHelper;
    // 脏表队列
    std::map<const ILogicSys *, bool> _dirtyLogicRefIsNumberKey;
    // 需要持久化系统的组件信息 int32:storageType
    std::unordered_map<const ILogicSys *, std::unordered_map<KERNEL_NS::LibString, Int32>> _sysRefFieldStorageType;
    
    // 定时purge数据
    KERNEL_NS::LibTimer *_purgeTimer;
    KERNEL_NS::FileMonitor<KERNEL_NS::MongodbConfig, KERNEL_NS::YamlDeserializer> *_options;
    // 是否正在检查退出
    bool _checkQuit;
};

SERVICE_END
