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

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MONGODB_IMPL_MONGODB_PROXY_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MONGODB_IMPL_MONGODB_PROXY_H__

#pragma once

#include <OptionComp/storage/MongoDB/Interface/IMongodbProxy.h>
#include <OptionComp/storage/MongoDB/Impl/MongodbConfig.h>
#include <unordered_map>
#include <map>
#include <kernel/comp/FileMonitor/FileMonitor.h>
#include <kernel/comp/LibDirtyHelper.h>

KERNEL_BEGIN

class LibTimer;

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

// 有状态每个线程一个Proxy,会检查Owner层级所有有 IMongodbStorageInfo 组件的组件
class MongodbProxy : public IMongodbProxy
{
    POOL_CREATE_OBJ_DEFAULT_P1(IMongodbProxy, MongodbProxy);
    
public:
    MongodbProxy();
    virtual ~MongodbProxy() override;
    
    void Release() override;

    // 标脏
    virtual void MaskLogicNumberKeyAddDirty(const CompHostObject *logic, Int64 key) override;
    virtual void MaskLogicNumberKeyModifyDirty(const CompHostObject *logic, Int64 key) override;
    virtual void MaskLogicNumberKeyDeleteDirty(const CompHostObject *logic, Int64 key) override;
    virtual void MaskLogicStringKeyAddDirty(const CompHostObject *logic, const KERNEL_NS::LibString &key) override;
    virtual void MaskLogicStringKeyModifyDirty(const CompHostObject *logic, const KERNEL_NS::LibString &key) override;
    virtual void MaskLogicStringKeyDeleteDirty(const CompHostObject *logic, const KERNEL_NS::LibString &key) override;

    // 等待落库完成
    virtual KERNEL_NS::CoTask<> Purge() override;
    // 等待logic落库完成
    virtual KERNEL_NS::CoTask<> Purge(const CompHostObject *logic) override;
    
    virtual KERNEL_NS::CoTask<bool> Query(const CompHostObject *logic, Int64 key, std::map<KERNEL_NS::LibString, KERNEL_NS::MongoSerializeInfo> *fieldNameRefDataResult) override;
    virtual KERNEL_NS::CoTask<bool> Query(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, KERNEL_NS::LibString keyName, Int64 key, std::map<KERNEL_NS::LibString, KERNEL_NS::MongoSerializeInfo> *fieldNameRefDataResult) override;
    virtual KERNEL_NS::CoTask<bool> Query(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, KERNEL_NS::LibString keyName, KERNEL_NS::LibString key, std::map<KERNEL_NS::LibString, KERNEL_NS::MongoSerializeInfo> *fieldNameRefDataResult) override;

    virtual void SetMongodbMgr(KERNEL_NS::IMongoDbMgr *mongodbMgr) override;
    // 设置检查依赖是否退出
    virtual void SetCheckDependenceQuit(KERNEL_NS::IDelegate<bool, const KERNEL_NS::CompHostObject *> *cb) override;
    // 设置自己完成回调(告知自己完成了, 可以退出,因为proxy要等待所有依赖都退出后才可退出)
    virtual void SetMongoProxyMaskQuit(KERNEL_NS::IDelegate<void, const KERNEL_NS::CompHostObject *> *cb) override;
    // 设置关注的组件回调, 以便Host在退出的时候检查是否Proxy要退出, 只有等待Proxy也退出, Host才可以退出, 避免数据没完全落地Host就退出了
    virtual void SetRegisterFocus(KERNEL_NS::IDelegate<void, const KERNEL_NS::CompHostObject *> *cb) override;
    // 监听关闭
    virtual void ListenClose(KERNEL_NS::EventManager *eventMgr, Int32 eventType) override;

protected:
    Int32 _OnHostInit() override;
    Int32 _OnCompsCreated() override;

    virtual Int32 _OnHostWillStart() override;
    Int32 _OnHostStart() override;
    void _OnHostClose() override;
    
    void _OnCloseEvent(KERNEL_NS::LibEvent *ev);
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

    KERNEL_NS::CoTask<> _DorPurgeNumber(const CompHostObject *sys);
    KERNEL_NS::CoTask<> _DorPurgeString(const CompHostObject *sys);

    bool _CheckLogic(const CompHostObject *logic, KERNEL_NS::LibString &err) const;
    void _BuildSysFields(const CompHostObject *logic);

    bool _TryGetStorageType(const CompHostObject *logic, const KERNEL_NS::LibString &fieldName, Int32 &storageType) const;
private:

    // 所有表建立的标脏回调, 系统退出时移除
    std::map<const CompHostObject *, KERNEL_NS::LibDirtyHelper<Int64, UInt64> *> _logicRefNumberDirtyHelper;
    std::map<const CompHostObject *, KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *> _logicRefStringDirtyHelper;
    // 脏表队列
    std::map<const CompHostObject *, bool> _dirtyLogicRefIsNumberKey;
    // 需要持久化系统的组件信息 string:GetSimpleTypeName int32:storageType
    std::unordered_map<const CompHostObject *, std::unordered_map<KERNEL_NS::LibString, Int32>> _sysRefFieldStorageType;
    
    // 定时purge数据
    KERNEL_NS::LibTimer *_purgeTimer;
    KERNEL_NS::FileMonitor<KERNEL_NS::MongodbConfig, KERNEL_NS::YamlDeserializer> *_options;
    // 是否正在检查退出
    bool _checkQuit;

    // 依赖mongodbMgr
    KERNEL_NS::IMongoDbMgr *_mongodbMgr;
    // mongodb配置
    SourceWrap _configSource;
    LibString _keyName;

    // 检查Host注册的mongodb依赖组件是否退出, 都退出, 那么Proxy就可以退出
    KERNEL_NS::IDelegate<bool, const KERNEL_NS::CompHostObject *> * _checkDependenceQuit;
    // 告知Host Proxy已经完成落地可以退出
    KERNEL_NS::IDelegate<void, const KERNEL_NS::CompHostObject *> * _maskSelfQuit;
    // 向Host注册关注,让Host等待Proxy所有数据落地完成后才能退出
    KERNEL_NS::IDelegate<void, const KERNEL_NS::CompHostObject *> * _registerFocus;

    // 监听退出, 准备退出的数据落地
    KERNEL_NS::ListenerStub _closeEventStub;

    // 依赖mongodb的组件
    std::set<KERNEL_NS::CompHostObject *> _dependence;
};

KERNEL_END

#endif

