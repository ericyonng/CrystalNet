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
 * Date: 2023-07-16 14:37:39
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <service/TestService/Comps/DB/interface/IMysqlMgr.h>
#include <protocols/protocols.h>

SERVICE_BEGIN

class TableInfo;
class IStorageInfo;

class MysqlMgr : public IMysqlMgr
{
    POOL_CREATE_OBJ_DEFAULT_P1(IMysqlMgr, MysqlMgr);

public:
    MysqlMgr();
    ~MysqlMgr();

    void Release();
    virtual void OnRegisterComps() override;
    virtual void DefaultMaskReady(bool isReady) override {}

    virtual void RegisterDependence(ILogicSys *obj) override;
    virtual void UnRegisterDependence(const ILogicSys *obj) override;

    // msqQueue:使用指定的消息队列, 用于做同步使用
    virtual Int32 NewRequest(UInt64 &stub, const KERNEL_NS::LibString &dbName, Int32 dbOperatorId, std::vector<KERNEL_NS::SqlBuilder *> &builders, std::vector<KERNEL_NS::Field *> &fields, bool isDestroyHandler, KERNEL_NS::IDelegate<void, KERNEL_NS::MysqlResponse *> *cb, KERNEL_NS::Variant **var = NULL, KERNEL_NS::MysqlMsgQueue *msqQueue = NULL) override;

   virtual void MaskLogicNumberKeyAddDirty(const ILogicSys *logic, UInt64 key, bool isRightRow = false) override;
   virtual void MaskLogicNumberKeyModifyDirty(const ILogicSys *logic, UInt64 key, bool isRightRow = false) override;
   virtual void MaskLogicNumberKeyDeleteDirty(const ILogicSys *logic, UInt64 key, bool isRightRow = false) override;
   virtual void MaskLogicStringKeyAddDirty(const ILogicSys *logic, const KERNEL_NS::LibString &key, bool isRightRow = false) override;
   virtual void MaskLogicStringKeyModifyDirty(const ILogicSys *logic, const KERNEL_NS::LibString &key, bool isRightRow = false) override;
   virtual void MaskLogicStringKeyDeleteDirty(const ILogicSys *logic, const KERNEL_NS::LibString &key, bool isRightRow = false) override;

   virtual Int32 GetSystemDBOperatorId() const override;
   virtual Int32 NewDbOperatorId() override;
   virtual const KERNEL_NS::LibString &GetCurrentServiceDbOption() const override;
   virtual const KERNEL_NS::LibString &GetCurrentServiceDbName() const override;

    // 清洗数据
   virtual void PurgeEndWith(KERNEL_NS::IDelegate<void, Int32> *handler) override;
   
   // 同步接口, 持久化且等完成后回调
   virtual Int32 PurgeAndWaitComplete(ILogicSys *logic) override;

    // 清洗数据
   virtual void Purge(ILogicSys *logic) override;


   virtual Int32 OnSave(const KERNEL_NS::LibString &key, std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> &fieldRefdb) const override;
   virtual Int32 OnLoaded(const KERNEL_NS::LibString &key, const std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> &fieldRefdb) override;

protected:
   Int32 _OnGlobalSysInit() override;
   Int32 _OnGlobalSysCompsCreated() override;

   Int32 _OnHostStart() override;
   void _OnGlobalSysClose() override;

    void _CloseServiceEvent(KERNEL_NS::LibEvent *ev);
    void _OnServiceFrameTick(KERNEL_NS::LibEvent *ev);

    // 数据加载
    bool _LoadSystemTable();
    void _SaveMe();

    void _OnSystemTableBack(KERNEL_NS::MysqlResponse *res);
    void _OnLoadDbTableColumns(KERNEL_NS::MysqlResponse *res);
    void _OnAddNewTableBack(KERNEL_NS::MysqlResponse *res);

    bool _CheckDropTables();
    void _LoadAllPublicData();
    void _OnLoadPublicData(KERNEL_NS::MysqlResponse *res);

    // 脏回调
    void _OnKvSystemNumberAddDirtyHandler(KERNEL_NS::LibDirtyHelper<UInt64, UInt64> *dirtyHelper, UInt64 &key, KERNEL_NS::Variant *params);
    void _OnKvSystemNumberModifyDirtyHandler(KERNEL_NS::LibDirtyHelper<UInt64, UInt64> *dirtyHelper, UInt64 &key, KERNEL_NS::Variant *params);
    void _OnKvSystemNumberDeleteDirtyHandler(KERNEL_NS::LibDirtyHelper<UInt64, UInt64> *dirtyHelper, UInt64 &key, KERNEL_NS::Variant *params);
    void _OnKvSystemNumberReplaceDirtyHandler(KERNEL_NS::LibDirtyHelper<UInt64, UInt64> *dirtyHelper, UInt64 &key, KERNEL_NS::Variant *params);

    void _OnKvSystemStringAddDirtyHandler(KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper, KERNEL_NS::LibString &key, KERNEL_NS::Variant *params);
    void _OnKvSystemStringModifyDirtyHandler(KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper, KERNEL_NS::LibString &key, KERNEL_NS::Variant *params);
    void _OnKvSystemStringDeleteDirtyHandler(KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper, KERNEL_NS::LibString &key, KERNEL_NS::Variant *params);
    void _OnKvSystemStringReplaceDirtyHandler(KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper, KERNEL_NS::LibString &key, KERNEL_NS::Variant *params);

    // 多字段的脏回调
    void _OnNumberAddDirtyHandler(KERNEL_NS::LibDirtyHelper<UInt64, UInt64> *dirtyHelper, UInt64 &key, KERNEL_NS::Variant *params);
    void _OnNumberModifyDirtyHandler(KERNEL_NS::LibDirtyHelper<UInt64, UInt64> *dirtyHelper, UInt64 &key, KERNEL_NS::Variant *params);
    void _OnNumberDeleteDirtyHandler(KERNEL_NS::LibDirtyHelper<UInt64, UInt64> *dirtyHelper, UInt64 &key, KERNEL_NS::Variant *params);
    void _OnNumberReplaceDirtyHandler(KERNEL_NS::LibDirtyHelper<UInt64, UInt64> *dirtyHelper, UInt64 &key, KERNEL_NS::Variant *params);

    void _OnStringAddDirtyHandler(KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper, KERNEL_NS::LibString &key, KERNEL_NS::Variant *params);
    void _OnStringModifyDirtyHandler(KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper, KERNEL_NS::LibString &key, KERNEL_NS::Variant *params);
    void _OnStringDeleteDirtyHandler(KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper, KERNEL_NS::LibString &key, KERNEL_NS::Variant *params);
    void _OnStringReplaceDirtyHandler(KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper, KERNEL_NS::LibString &key, KERNEL_NS::Variant *params);

    void _OnDurtyPurgeFinishHandler(KERNEL_NS::MysqlResponse *res);

    void _InitNumberDirtyHelper(const IStorageInfo *storageInfo, KERNEL_NS::LibDirtyHelper<UInt64, UInt64> *dirtyHelper);
    void _InitStringDirtyHelper(const IStorageInfo *storageInfo, KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *dirtyHelper);

    void _OnPurge(KERNEL_NS::LibTimer *t);

    const ILogicSys *_GetDependenceLogic(const KERNEL_NS::LibString &tableName) const;
    ILogicSys *_GetDependenceLogic(const KERNEL_NS::LibString &tableName);

    void _PurgeAll();

private:
    void _Clear();

    TableInfo *_CreateNewTableInfo(const ILogicSys *logic);
    TableInfo *_GetTableInfo(const KERNEL_NS::LibString &tableName);
    const TableInfo *_GetTableInfo(const KERNEL_NS::LibString &tableName) const;
    void _RemoveTableInfo(const KERNEL_NS::LibString &tableName);

    // 结合storageinfo 和 要持久化的数据分辨出新增, 修改的字段 TODO:
    void _GetTableChanges();

    void _PurgeDirty(const ILogicSys *logic);

    // mysql相关限制
    bool _CheckStorageInfo(const IStorageInfo *storageInfo);
    bool _FillMultiFieldStorageInfo(IStorageInfo *storageInfo);
    bool _FillKvSystemStorageInfo(IStorageInfo *storageInfo);

    bool _GetModifyTableInfo(IStorageInfo *storageInfo, std::map<KERNEL_NS::LibString, std::pair<KERNEL_NS::LibString, Int64>> &originDbTableInfo, std::vector<KERNEL_NS::SqlBuilder *> &builders);
    bool _ModifyDbNumberDataType(IStorageInfo *storageInfo, IStorageInfo *subStorageInfo, const KERNEL_NS::LibString &dataType
                                , const KERNEL_NS::LibString &oldFieldType,  const KERNEL_NS::LibString &fieldName
                                , KERNEL_NS::SmartPtr<KERNEL_NS::AlterTableSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> &alterDropColumn 
                                , KERNEL_NS::SmartPtr<KERNEL_NS::AlterTableSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> &alterModifyColumn 
                                , KERNEL_NS::SmartPtr<KERNEL_NS::AlterTableSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> &alterAddColumn 
                                , bool &hasModify
                                , bool &hasAdd
                                , bool &hasDrop);

    bool _ModifyDbStringDataType(IStorageInfo *storageInfo, IStorageInfo *subStorageInfo, const KERNEL_NS::LibString &dataType
                                , const KERNEL_NS::LibString &oldFieldType, UInt64 oldCapacitySize, const KERNEL_NS::LibString &fieldName
                                , KERNEL_NS::SmartPtr<KERNEL_NS::AlterTableSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> &alterDropColumn 
                                , KERNEL_NS::SmartPtr<KERNEL_NS::AlterTableSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> &alterModifyColumn 
                                , KERNEL_NS::SmartPtr<KERNEL_NS::AlterTableSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> &alterAddColumn 
                                , bool &hasModify
                                , bool &hasAdd
                                , bool &hasDrop);

    bool _ModifyDbBinaryDataType(IStorageInfo *storageInfo, IStorageInfo *subStorageInfo, const KERNEL_NS::LibString &dataType
                                , const KERNEL_NS::LibString &oldFieldType, UInt64 oldCapacitySize, const KERNEL_NS::LibString &fieldName
                                , KERNEL_NS::SmartPtr<KERNEL_NS::AlterTableSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> &alterDropColumn 
                                , KERNEL_NS::SmartPtr<KERNEL_NS::AlterTableSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> &alterModifyColumn 
                                , KERNEL_NS::SmartPtr<KERNEL_NS::AlterTableSqlBuilder, KERNEL_NS::AutoDelMethods::CustomDelete> &alterAddColumn 
                                , bool &hasModify
                                , bool &hasAdd
                                , bool &hasDrop);

private:
    KERNEL_NS::ListenerStub _closeServiceStub;
    KERNEL_NS::ListenerStub _onServiceFrameTickStub;


    // 所有表建立的标脏回调, 系统退出时移除
    std::map<const ILogicSys *, KERNEL_NS::LibDirtyHelper<UInt64, UInt64> *> _logicRefNumberDirtyHelper;
    std::map<const ILogicSys *, KERNEL_NS::LibDirtyHelper<KERNEL_NS::LibString, UInt64> *> _logicRefStringDirtyHelper;
    // 脏表队列
    std::set<const ILogicSys *> _dirtyLogics;
    // 定时purge数据
    KERNEL_NS::LibTimer *_purgeTimer;
    KERNEL_NS::Poller *_poller;

    // 系统表数据
    std::map<KERNEL_NS::LibString, TableInfo *> _tableNameRefTableInfo;
    std::map<KERNEL_NS::LibString, ILogicSys *> _tableNameRefLogic;
    std::set<ILogicSys *> _loadPublicDataPending;

    // 默认初始blob字段大小
    Int64 _defaultBlobOriginSize;
    Int32 _defaultStringKeyOriginSize;
    KERNEL_NS::LibString _currentServiceDBOption;
    KERNEL_NS::LibString _currentServiceDBName;
    Int64 _curVersionNo;
    Int32 _systemOperatorUid;
    Int64 _purgeIntervalMs;
    Int32 _disableSystemTableAutoDrop;
    Int32 _disableAutoDrop;
};

ALWAYS_INLINE const ILogicSys *MysqlMgr::_GetDependenceLogic(const KERNEL_NS::LibString &tableName) const
{
    auto iter = _tableNameRefLogic.find(tableName);
    return iter == _tableNameRefLogic.end() ? NULL : iter->second;
}

ALWAYS_INLINE ILogicSys *MysqlMgr::_GetDependenceLogic(const KERNEL_NS::LibString &tableName)
{
    auto iter = _tableNameRefLogic.find(tableName);
    return iter == _tableNameRefLogic.end() ? NULL : iter->second;
}

ALWAYS_INLINE TableInfo *MysqlMgr::_GetTableInfo(const KERNEL_NS::LibString &tableName)
{
    auto iter = _tableNameRefTableInfo.find(tableName);
    return iter == _tableNameRefTableInfo.end() ? NULL : iter->second;
}

ALWAYS_INLINE const TableInfo *MysqlMgr::_GetTableInfo(const KERNEL_NS::LibString &tableName) const
{
    auto iter = _tableNameRefTableInfo.find(tableName);
    return iter == _tableNameRefTableInfo.end() ? NULL : iter->second;
}

SERVICE_END
