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
 * Date: 2023-07-15 20:06:00
 * Author: Eric Yonng
 * Description:
*/

#include <pch.h>
#include <OptionComp/storage/mysql/impl/MysqlDBMgrFactory.h>
#include <OptionComp/storage/mysql/impl/MysqlDBMgr.h>
#include <OptionComp/storage/mysql/impl/MysqlDB.h>
#include <OptionComp/storage/mysql/impl/MysqlMsg.h>
#include <OptionComp/storage/mysql/impl/DbEvent.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(MysqlDBMgr);

MysqlDBMgr::MysqlDBMgr()
:_ini(NULL)
,_maxSeqId(0)
,_msgBackpoller(NULL)
,_msgLevel(0)
,_dbEventType(0)
,_closeMysqlTimer(NULL)
{

}

MysqlDBMgr::~MysqlDBMgr()
{
    _DoDbCloseAll();
    _Clear();
}

void MysqlDBMgr::Release()
{
    MysqlDBMgr::DeleteByAdapter_MysqlDBMgr(MysqlDBMgrFactory::_buildType.V, this);
}

void MysqlDBMgr::OnRegisterComps()
{

}

Int32 MysqlDBMgr::NewOperatorUid(const LibString &dbName)
{
    auto db = GetDB(dbName);
    if(UNLIKELY(!db))
        return 0;

    return db->NewOperatorUid();
}

void MysqlDBMgr::SkipOperatorId(const LibString &dbName, Int32 oid)
{
    auto db = GetDB(dbName);
    if(UNLIKELY(!db))
        return;

    db->SkipOperatorId(oid);
}

void MysqlDBMgr::RemoveSkipOperatorId(const LibString &dbName, Int32 oid)
{
    auto db = GetDB(dbName);
    if(UNLIKELY(!db))
        return;

    db->RemoveSkipOperatorId(oid);
}

void MysqlDBMgr::CloseMysqlAll(bool forceQuit)
{
    g_Log->Info(LOGFMT_OBJ_TAG("will close mysql..."));

    if(UNLIKELY(_closeMysqlTimer && _closeMysqlTimer->IsScheduling()))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("mysql db mgr is in closing..."));
        return;
    }

    if(!IsReady())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("mysql db mgr is already closed."));
        return;
    }

    if(UNLIKELY(!_msgBackpoller || forceQuit))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("have no msg back poller or do force quit and will close immediately forceQuit:%d _pendingSeqs size:%lld."), forceQuit, static_cast<Int64>(_pendingSeqs.size()));

        _DoDbCloseAll();
        return;
    }

    auto timerMgr = _msgBackpoller->GetTimerMgr();
    if(!_closeMysqlTimer)
        _closeMysqlTimer = LibTimer::NewThreadLocal_LibTimer(timerMgr);

    // 可以关闭的条件是: 所有的请求都处理完, 所有的依赖都退出
    _closeMysqlTimer->SetTimeOutHandler([this](LibTimer *t)
    {
        std::vector<LibString> dependenceNames;
        const Int64 pendingCount = static_cast<Int64>(_pendingSeqs.size());

        for(auto obj : _dependence)
            dependenceNames.push_back(obj->GetObjName());

        g_Log->Info(LOGFMT_OBJ_TAG("waiting all dependence:[%s] quit and request complete(request count:%lld) ...")
                , StringUtil::ToString(dependenceNames, ",").c_str(), pendingCount);

        // 是否可以退出
        if(!_CanQuit())
            return;
            
        // 退出
        _DoDbCloseAll();

        _closeMysqlTimer = NULL;
        LibTimer::DeleteThreadLocal_LibTimer(t);
    });

    _closeMysqlTimer->Schedule(1000);
}

bool MysqlDBMgr::PushRequest(MysqlDB *db, MysqlRequest *req)
{
    if(UNLIKELY(!db))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("db is null"));
        return false;
    }

    if(UNLIKELY(!req))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("req is null"));
        return false;
    }

    _pendingSeqs.insert(req->_seqId);

    if(!db->PushRequest(req))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("db:%s push request fail req:%s"), db->GetConfig()._dbName.c_str(), req->ToString().c_str());
        _pendingSeqs.erase(req->_seqId);
        return false;
    }

    g_Log->Info(LOGFMT_OBJ_TAG("db:%s push request success seq id:%llu"), db->GetConfig()._dbName.c_str(), req->_seqId);
    return true;
}

Int32 MysqlDBMgr::_OnHostInit()
{
    if(UNLIKELY(!_ini))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("have no config ini file, please check."));
        return Status::Failed;
    }

    if(UNLIKELY(!_msgBackpoller))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("have no msg back poller, please check."));
        return Status::Failed;
    }

    if(UNLIKELY(_dbEventType == 0))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("need set db event type before init, please check."));
        return Status::Failed;
    }

    // 读取数据库列表项
    for(auto &dbSeg : _dbConfigSegments)
    {
        MysqlConfig config;
        auto errCode = _ReadToConfig(dbSeg, config);
        if(errCode != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("read db config fail errCode:%d, dbSeg:%s, ini path:%s"), errCode, dbSeg.c_str(), _ini->GetPath().c_str());
            return errCode;
        }

        if(_dbNameRefMysqlConfig.find(config._dbName) != _dbNameRefMysqlConfig.end())
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("read a repeat db config %s please check db config segments:%s")
                    , config._dbName.c_str(), StringUtil::ToString(_dbConfigSegments, ",").c_str());
            return Status::Repeat;
        }

        _dbNameRefMysqlConfig.insert(std::make_pair(config._dbName, config));
    }

    // 创建db对象
    _CreateDb();
    
    auto err = _InitDb();
    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("init db fail"));
        return err;
    }

    if(UNLIKELY(_dbNameRefMysqlDB.empty()))
        g_Log->Warn(LOGFMT_OBJ_TAG("have no any db please check if dont need db."));

    g_Log->Info(LOGFMT_OBJ_TAG("mysql db mgr init success."));

    return Status::Success;
}

Int32 MysqlDBMgr::_OnCompsCreated()
{
    _RegisterPollerEvents();
    return Status::Success;
}

Int32 MysqlDBMgr::_OnHostStart()
{
    if(UNLIKELY(_dbNameRefMysqlDB.empty()))
    {
        MaskReady(true);
        return Status::Success;
    }

    for(auto iter : _dbNameRefMysqlDB)
    {
        auto err = iter.second->Start();
        if(err != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("start db fail err:%d, config:%s"), err, iter.second->GetConfig().ToString().c_str());
            return err;
        }
    }

    // 等待所有db ready
    for(;!_IsAllDbReady();)
        SystemUtil::ThreadSleep(1000);

    MaskReady(true);
    g_Log->Info(LOGFMT_OBJ_TAG("all db is start."));

    return Status::Success;
}

void MysqlDBMgr::_OnHostBeforeCompsWillClose()
{
    // 已经退出不需要再执行退出
    if(UNLIKELY(!IsReady()))
        return;

    // 需要等待可以退出的条件
    if(!_CanQuit())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("mysql db mgr waiting for quit..."));

        // 之前没有启动定时器, 此时需要启动定时器来检测是否可以退出
        if(!_closeMysqlTimer || !_closeMysqlTimer->IsScheduling())
            CloseMysqlAll();

        return;
    }

    CloseMysqlAll(true);
}

void MysqlDBMgr::_OnHostClose()
{

}

void MysqlDBMgr::_DoDbCloseAll()
{
    if(UNLIKELY(!IsReady()))
        return;

    for(auto iter : _dbNameRefMysqlDB)
        iter.second->WillClose();

    for(auto iter : _dbNameRefMysqlDB)
        iter.second->Close();

    // 等待所有都退出
    for(;!_IsAllDBStop();)
        SystemUtil::ThreadSleep(1000);

    _UnRegisterPollerEvents();
    MaskReady(false);
    g_Log->Info(LOGFMT_OBJ_TAG("all db quit."));
}

bool MysqlDBMgr::_CanQuit() const
{
    if(!_dependence.empty())
        return false;

    if(!_pendingSeqs.empty())
        return false;

    return true;
}

void MysqlDBMgr::_Clear()
{
    ContainerUtil::DelContainer(_dbNameRefMysqlDB, [](MysqlDB *db){
        MysqlDB::Delete_MysqlDB(db);
    });

    _dbNameRefMysqlConfig.clear();

    if(_closeMysqlTimer)
    {
        LibTimer::DeleteThreadLocal_LibTimer(_closeMysqlTimer);
        _closeMysqlTimer = NULL;
    }
}

void MysqlDBMgr::_RegisterPollerEvents()
{
    if(LIKELY(_msgBackpoller))
        _msgBackpoller->Subscribe(_dbEventType, this, &MysqlDBMgr::_OnDbEvent);
}

void MysqlDBMgr::_UnRegisterPollerEvents()
{
    if(LIKELY(_msgBackpoller))
        _msgBackpoller->UnSubscribe(_dbEventType);
}

void MysqlDBMgr::_OnDbEvent(PollerEvent *dbEvent)
{
    auto ev = dbEvent->CastTo<DbEvent>();
    auto res = ev->_res;
    g_Log->Debug(LOGFMT_OBJ_TAG("db event res:%s"), res->ToString().c_str());

    if(LIKELY(res->_handler))
    {
        res->_handler->Invoke(res);
        if(!res->_isDestroyHandler)
            res->_handler = NULL;
    }

    _pendingSeqs.erase(res->_seqId);
}

Int32 MysqlDBMgr::_ReadToConfig(const LibString &dbSeg, MysqlConfig &config) const
{
    {// db名
        LibString value;
        if(UNLIKELY(!_ini->ReadStr(dbSeg.c_str(), "Host", value)))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("config error have no Host key in segment:%s, ini file:%s")
                        , dbSeg.c_str(), _ini->GetPath().c_str());

            return Status::Failed;
        }

        if(UNLIKELY(value.empty()))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("config error Host is empty in segment:%s, ini file:%s")
                        , dbSeg.c_str(), _ini->GetPath().c_str());

            return Status::Failed;
        }

        config._host = value;
    }

    {// db名
        LibString value;
        if(UNLIKELY(!_ini->ReadStr(dbSeg.c_str(), "DB", value)))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("config error have no DB key in segment:%s, ini file:%s")
                        , dbSeg.c_str(), _ini->GetPath().c_str());

            return Status::Failed;
        }

        if(UNLIKELY(value.empty()))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("config error db name is empty key:DB in segment:%s, ini file:%s")
                        , dbSeg.c_str(), _ini->GetPath().c_str());

            return Status::Failed;
        }

        config._dbName = value;
    }

    {// 用户名
        LibString value;
        if(UNLIKELY(!_ini->ReadStr(dbSeg.c_str(), "User", value)))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("config error have no User key in segment:%s, ini file:%s")
                        , dbSeg.c_str(), _ini->GetPath().c_str());

            return Status::Failed;
        }

        if(UNLIKELY(value.empty()))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("config error User is empty in segment:%s, ini file:%s")
                        , dbSeg.c_str(), _ini->GetPath().c_str());

            return Status::Failed;
        }

        config._user = value;
    }

    {// 密码
        LibString value;
        if(UNLIKELY(!_ini->ReadStr(dbSeg.c_str(), "Pwd", value)))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("config error have no Pwd key in segment:%s, ini file:%s")
                        , dbSeg.c_str(), _ini->GetPath().c_str());

            return Status::Failed;
        }

        if(UNLIKELY(value.empty()))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("config error Pwd is empty in segment:%s, ini file:%s")
                        , dbSeg.c_str(), _ini->GetPath().c_str());

            return Status::Failed;
        }

        config._pwd = value;
    }

    {// Port
        UInt64 port = 0;
        if(UNLIKELY(!_ini->CheckReadNumber(dbSeg.c_str(), "Port", port)))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("config error have no Port key in segment:%s, ini file:%s")
                        , dbSeg.c_str(), _ini->GetPath().c_str());

            return Status::Failed;
        }

        if(UNLIKELY(port == 0))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("config error Port is zero in segment:%s, ini file:%s")
                        , dbSeg.c_str(), _ini->GetPath().c_str());

            return Status::Failed;
        }

        config._port = static_cast<UInt16>(port);
    }

    {// 绑定的ip 没有设置则使用默认:127.0.0.1
        LibString value = "127.0.0.1";
        if(UNLIKELY(_ini->ReadStr(dbSeg.c_str(), "BindIp", value)))
        {
            if(UNLIKELY(value.empty()))
            {
                value = "127.0.0.1";
                g_Log->Warn(LOGFMT_OBJ_TAG("BindIp is empty and will use %s"), value.c_str());
            }
        }

        config._bindIp = value;
    }

    {// 编码 没有设置则使用默认:utf8mb4
        LibString value = "utf8mb4";
        if(UNLIKELY(_ini->ReadStr(dbSeg.c_str(), "CharacterSet", value)))
        {
            if(UNLIKELY(value.empty()))
            {
                value = "utf8mb4";
                g_Log->Warn(LOGFMT_OBJ_TAG("CharacterSet is empty and will use %s"), value.c_str());
            }
        }

        config._charset = value;
        config._dbCharset = value;
    }

    {// 编码COLLATE 没有设置则使用默认:utf8mb4_bin
        LibString value = "utf8mb4_bin";
        if(UNLIKELY(_ini->ReadStr(dbSeg.c_str(), "COLLATE", value)))
        {
            if(UNLIKELY(value.empty()))
            {
                value = "utf8mb4_bin";
                g_Log->Warn(LOGFMT_OBJ_TAG("COLLATE is empty and will use %s"), value.c_str());
            }
        }

        config._dbCollate = value;
    }

    {// 自动重连 没有设置则使用默认:1
        Int64 value = 1;
        _ini->CheckReadNumber(dbSeg.c_str(), "AutoReconnect", value);
        config._autoReconnect = static_cast<Int32>(value);
    }

    {// 自动重连 没有设置则使用默认:2GB
        UInt64 value = 2147483648;
        _ini->CheckReadNumber(dbSeg.c_str(), "MaxPacketSize", value);
        config._maxPacketSize = value;
    }

    {// 自动重连 没有设置则使用默认:开启
        Int64 value = 1;
        _ini->CheckReadNumber(dbSeg.c_str(), "EnableMultiStatements", value);
        config._enableMultiStatements = (value != 0);
    }

    {// 错误重试次数
        Int32 value = 10;
        _ini->CheckReadNumber(dbSeg.c_str(), "RetryTimesWhenNetInterrupt", value);
        config._retryWhenError = value;
    }

    {// 线程数量
        Int32 value = 10;
        _ini->CheckReadNumber(dbSeg.c_str(), "DbThreadNum", value);
        config._dbThreadNum = value;
    }

    {// Ping时间间隔
        Int64 value = 3600;
        _ini->CheckReadNumber(dbSeg.c_str(), "PingIntervalSeconds", value);
        config._pingIntervalSeconds = value;
    }

    return Status::Success;
}

void MysqlDBMgr::_CreateDb()
{
    for(auto &iter : _dbNameRefMysqlConfig)
    {
        auto newDb = MysqlDB::New_MysqlDB(this);
        newDb->SetConfig(iter.second);
        newDb->SetDbMsgLevel(_msgLevel);
        newDb->SetTargetPoller(_msgBackpoller);
        newDb->SetEventType(_dbEventType);

        _dbNameRefMysqlDB.insert(std::make_pair(iter.second._dbName, newDb));
    }
}

// 初始化db
Int32 MysqlDBMgr::_InitDb()
{
    for(auto iter : _dbNameRefMysqlDB)
    {
        auto err = iter.second->Init();
        if(err != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("db init fail err:%d, db config:%s")
                        ,err, iter.second->GetConfig().ToString().c_str());
            return err;
        }
    }

    return Status::Success;
}

bool MysqlDBMgr::_IsAllDbReady() const
{
    for(auto iter : _dbNameRefMysqlDB)
    {
        if(!iter.second->IsReady())
            return false;
    }

    return true;
}

bool MysqlDBMgr::_IsAllDBStop() const
{
    for(auto iter : _dbNameRefMysqlDB)
    {
        if(iter.second->IsReady())
            return false;
    }

    return true;
}

KERNEL_END
