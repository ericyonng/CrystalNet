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
 * Description: mysql 数据库
 * 
 * 1.每个对象只管理一个数据库
 * 2.支持mysqldump等
*/


#ifndef __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MYSQL_IMPL_MYSQL_DB_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MYSQL_IMPL_MYSQL_DB_H__

#pragma once

#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Lock/Lock.h>
#include <kernel/comp/ConcurrentPriorityQueue/ConcurrentPriorityQueue.h>
#include <OptionComp/storage/mysql/impl/MysqlConfig.h>

KERNEL_BEGIN

class MysqlConnect;
class LibThread;
class MysqlRequest;
class Poller;
class MysqlDBMgr;
class Variant;

class DBBalanceInfo
{
    POOL_CREATE_OBJ_DEFAULT(DBBalanceInfo);
public:
    DBBalanceInfo(Int32 idx);
    ~DBBalanceInfo();

    LibString ToString() const;

    const Int32 _index;
    MysqlConnect *_conn;
    LibThread *_thread;
    ConcurrentPriorityQueue<MysqlRequest *> *_msgQueue;
    ConditionLocker _eventGuard;   
};

class MysqlDB
{
    POOL_CREATE_OBJ_DEFAULT(MysqlDB);

public:
    MysqlDB(MysqlDBMgr *owner);
    ~MysqlDB();

    Int32 Init();
    Int32 Start();
    void WillClose();
    void Close();

    void SetConfig(const MysqlConfig &config);
    const MysqlConfig &GetConfig() const;

    void SetTargetPoller(Poller *poller);
    void SetEventType(Int32 evType);

    bool IsReady() const;

    Int32 Dump(const LibString &dumpFilePath);

    bool PushRequest(MysqlRequest *req);

    LibString ToString() const;

    // 每次操作必须持有operator id
    Int32 NewOperatorUid();
    // NewOperatorUid 的时候跳过该id, 保证该id使用者不受其他使用者干扰
    void SkipOperatorId(Int32 oid);
    void RemoveSkipOperatorId(Int32 oid);

    Int32 GetThreadNum() const;

private:
    void _OnWorker(LibThread *t, KERNEL_NS::Variant *var);

    // request 处理
    void _StmtHandler(MysqlConnect *curConn, MysqlRequest *req, Int64 &pingExpireTime);
    void _NormalSqlHandler(MysqlConnect *curConn, MysqlRequest *req, Int64 &pingExpireTime);
    void _SqlWithTransActionSqlHandler(MysqlConnect *curConn, MysqlRequest *req, Int64 &pingExpireTime);

    void _Clear();

private:
    MysqlDBMgr *_owner;
    std::atomic_bool _isReady;
    MysqlConfig _cfg;
    UInt64 _maxId;
    Int32 _curMaxOperatorUid;
    std::unordered_set<Int32> _skipOperatorId;   // 当线程数量大于_disableOperatorId的时候启用

    // 从thread 线程创建于销毁
    std::vector<DBBalanceInfo *> _workerBalance;
    // MysqlConnect *_conn;

    // LibThread *_worker;
    // ConcurrentPriorityQueue<MysqlRequest *> *_msgQueue;
    // ConditionLocker _eventGuard;   

    Poller *_targetPoller;  // db 返回消息的poller
    Int32 _eventType;   // 事件类型   
    
    typedef void (MysqlDB::*MsgHandler)(MysqlConnect *conn, MysqlRequest *, Int64 &);
    std::vector<MsgHandler> _msgHandler;
};

ALWAYS_INLINE void MysqlDB::SetConfig(const MysqlConfig &config)
{
    _cfg = config;
}

ALWAYS_INLINE const MysqlConfig &MysqlDB::GetConfig() const
{
    return _cfg;
}

ALWAYS_INLINE void MysqlDB::SetTargetPoller(Poller *poller)
{
    _targetPoller = poller;
}

ALWAYS_INLINE void MysqlDB::SetEventType(Int32 evType)
{
    _eventType = evType;
}

ALWAYS_INLINE bool MysqlDB::IsReady() const
{
    return _isReady.load(std::memory_order_acquire);
}

ALWAYS_INLINE Int32 MysqlDB::NewOperatorUid()
{
    const Int32 count = static_cast<Int32>(_workerBalance.size());

    // 所有id可能都被禁用了, 此时不跳过任何id
    if(UNLIKELY(count == static_cast<Int32>(_skipOperatorId.size())))
    {
        ++_curMaxOperatorUid;
        if(_curMaxOperatorUid == count)
            _curMaxOperatorUid = 0;

        return _curMaxOperatorUid;
    }

    for(;;)
    {
        ++_curMaxOperatorUid;
        if(_curMaxOperatorUid == count)
            _curMaxOperatorUid = 0;

        // 只有不在跳过的id列表中即可使用
        if(UNLIKELY(_skipOperatorId.find(_curMaxOperatorUid) == _skipOperatorId.end()))
            break;
    }

    return _curMaxOperatorUid;
}

ALWAYS_INLINE void MysqlDB::SkipOperatorId(Int32 oid)
{
    _skipOperatorId.insert(oid);
}

ALWAYS_INLINE void MysqlDB::RemoveSkipOperatorId(Int32 oid)
{
    _skipOperatorId.erase(oid);
}

ALWAYS_INLINE Int32 MysqlDB::GetThreadNum() const
{
    return _cfg._dbThreadNum;
}

KERNEL_END

#endif
