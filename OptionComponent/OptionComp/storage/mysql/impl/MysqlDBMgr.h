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
 * Date: 2023-07-07 21:00:00
 * Author: Eric Yonng
 * Description: mysql 数据库管理
 * db的退出是需要其他所有依赖db的模块都退出
*/

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MYSQL_IMPL_MYSQL_DB_MGR_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MYSQL_IMPL_MYSQL_DB_MGR_H__

#pragma once

#include <kernel/comp/LibString.h>
#include <kernel/comp/CompObject/CompObjectInc.h>
#include <OptionComp/storage/mysql/impl/MysqlConfig.h>

KERNEL_BEGIN

class MysqlDB;
class Poller;
class LibIniFile;
struct PollerEvent;
class LibTimer;
class MysqlRequest;

class MysqlDBMgr : public CompHostObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, MysqlDBMgr);

public:
    MysqlDBMgr();
    ~MysqlDBMgr();
    // 释放
    virtual void Release() override;
    // 需要手动设置ready
    virtual void DefaultMaskReady(bool isReady) {}
    virtual void OnRegisterComps() override;  

    // 公共接口
public:
    // 新建序列号id
    UInt64 NewSeqId();
    void RemovePendingSeqId(UInt64 seqId);
    // 系统,用户若需要操作数据库都需要持有一个操作id, 用于负载均衡
    Int32 NewOperatorUid(const LibString &dbName);
    // NewOperatorUid 的时候跳过该id, 保证该id使用者不受其他使用者干扰
    void SkipOperatorId(const LibString &dbName, Int32 oid);
    void RemoveSkipOperatorId(const LibString &dbName, Int32 oid);

    // 设置消息接收的poller
    void SetMsgBackPoller(Poller *poller);

    // 设置消息接收的消息等级
    void SetMsgLevel(Int32 level);

    // 设置需要监听的db事件类型
    void SetDbEventType(Int32 dbEventType);

    // 设置配置
    void SetIniFile(const LibIniFile *iniFile);

    // 设置需要加载的数据库段名列表
    void SetDbSegmentList(const std::vector<LibString> &dbConfigSegments);

    // 外部依赖注册
    void RegisterDependence(const CompObject *obj);
    void UnRegisterDependence(const CompObject *obj);

    // 关闭mysql
    void CloseMysqlAll(bool forceQuit = false);

    // 获取db对象
    MysqlDB *GetDB(const KERNEL_NS::LibString &dbName);
    const MysqlDB *GetDB(const KERNEL_NS::LibString &dbName) const;

    // 消息
    bool PushRequest(MysqlDB *db, MysqlRequest *req);

    // 获取所有依赖
    const std::set<const CompObject *> &GetDependence() const;
    std::set<const CompObject *> GetDependence(const std::set<const CompObject *> &excludes) const;

    // 获取配置
    const std::unordered_map<LibString, MysqlConfig> &GetConfigs() const;
    const MysqlConfig &GetConfig(const LibString &dbName) const;
    bool HasPendings() const;

protected:
    virtual Int32 _OnHostInit() override;
    virtual Int32 _OnCompsCreated() override;
    virtual Int32 _OnHostStart() override;
    virtual void _OnHostBeforeCompsWillClose() override;
    virtual void _OnHostClose() override;
    void _DoDbCloseAll();
    bool _CanQuit() const;

    void _Clear();

    // 注册poller消息处理
    void _RegisterPollerEvents();
    void _UnRegisterPollerEvents();
    void _OnDbEvent(PollerEvent *dbEvent);

    // 读取db配置
    Int32 _ReadToConfig(const LibString &dbSeg, MysqlConfig &config) const;
    // 创建mysql对象
    void _CreateDb();
    // 初始化db
    Int32 _InitDb();
    // db ready
    bool _IsAllDbReady() const;
    bool _IsAllDBStop() const;

private:
    // 配置表
    const LibIniFile *_ini;
    // db对象
    std::unordered_map<LibString, MysqlDB *> _dbNameRefMysqlDB;
    // db配置
    std::unordered_map<LibString, MysqlConfig> _dbNameRefMysqlConfig;
    // 当前最大的序列号id
    UInt64 _maxSeqId;
    // mysql处理完后消息的接收者poller
    Poller *_msgBackpoller;
    // 消息优先级
    Int32 _msgLevel;
    // db事件类型
    Int32 _dbEventType;
    // 配置表中db的段名列表
    std::vector<LibString> _dbConfigSegments;
    // 还未完成的请求
    std::set<UInt64> _pendingSeqs;
    // 依赖模块列表
    std::set<const CompObject *> _dependence;

    // 关闭定时器
    LibTimer *_closeMysqlTimer;
};

ALWAYS_INLINE UInt64 MysqlDBMgr::NewSeqId()
{
    return ++_maxSeqId;
}

ALWAYS_INLINE void MysqlDBMgr::RemovePendingSeqId(UInt64 seqId)
{
    _pendingSeqs.erase(seqId);
}

ALWAYS_INLINE void MysqlDBMgr::SetMsgBackPoller(Poller *poller)
{
    _msgBackpoller = poller;
}

ALWAYS_INLINE void MysqlDBMgr::SetMsgLevel(Int32 level)
{
    _msgLevel = level;
}

ALWAYS_INLINE void MysqlDBMgr::SetDbEventType(Int32 dbEventType)
{
    _dbEventType = dbEventType;
}

ALWAYS_INLINE void MysqlDBMgr::SetIniFile(const LibIniFile *iniFile)
{
    _ini = iniFile;
}

ALWAYS_INLINE void MysqlDBMgr::SetDbSegmentList(const std::vector<LibString> &dbConfigSegments)
{
    _dbConfigSegments = dbConfigSegments;
}

ALWAYS_INLINE void MysqlDBMgr::RegisterDependence(const CompObject *obj)
{
    _dependence.insert(obj);
}

ALWAYS_INLINE void MysqlDBMgr::UnRegisterDependence(const CompObject *obj)
{
    _dependence.erase(obj);
}

ALWAYS_INLINE MysqlDB *MysqlDBMgr::GetDB(const KERNEL_NS::LibString &dbName)
{
    auto iter = _dbNameRefMysqlDB.find(dbName);
    return iter == _dbNameRefMysqlDB.end() ? NULL : iter->second;
}

ALWAYS_INLINE const MysqlDB *MysqlDBMgr::GetDB(const KERNEL_NS::LibString &dbName) const
{
    auto iter = _dbNameRefMysqlDB.find(dbName);
    return iter == _dbNameRefMysqlDB.end() ? NULL : iter->second;
}

ALWAYS_INLINE const std::set<const CompObject *> &MysqlDBMgr::GetDependence() const
{
    return _dependence;
}

ALWAYS_INLINE std::set<const CompObject *> MysqlDBMgr::GetDependence(const std::set<const CompObject *> &excludes) const
{
    std::set<const CompObject *> objs;
    for(auto obj : _dependence)
    {
        if(excludes.find(obj) != excludes.end())
            continue;

        objs.insert(obj);
    }

    return objs;
}

ALWAYS_INLINE const std::unordered_map<LibString, MysqlConfig> &MysqlDBMgr::GetConfigs() const
{
    return _dbNameRefMysqlConfig;
}

ALWAYS_INLINE const MysqlConfig &MysqlDBMgr::GetConfig(const LibString &dbName) const
{
    static const MysqlConfig s_empty;
    auto iter = _dbNameRefMysqlConfig.find(dbName);
    return iter == _dbNameRefMysqlConfig.end() ? s_empty : iter->second;
}

ALWAYS_INLINE bool MysqlDBMgr::HasPendings() const
{
    return !_pendingSeqs.empty();
}

KERNEL_END

#endif
