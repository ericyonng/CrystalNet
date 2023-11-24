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
*/

#include <pch.h>
#include <OptionComp/storage/mysql/impl/MysqlDB.h>
#include <OptionComp/storage/mysql/impl/MysqlMsg.h>
#include <OptionComp/storage/mysql/impl/DbEvent.h>
#include <OptionComp/storage/mysql/impl/MysqlConnect.h>
#include <OptionComp/storage/mysql/impl/MysqlDBMgr.h>
#include <mysql.h>
#include <OptionComp/storage/mysql/impl/Defs.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(DBBalanceInfo);

DBBalanceInfo::DBBalanceInfo(Int32 idx)
:_index(idx)
,_conn(NULL)
,_thread(NULL)
,_msgQueue(NULL)
{

}

DBBalanceInfo::~DBBalanceInfo()
{
    CRYSTAL_DELETE_SAFE(_thread);
    if(_msgQueue)
    {
        ConcurrentPriorityQueue<MysqlRequest *>::Delete_ConcurrentPriorityQueue(_msgQueue);
        _msgQueue = NULL;
    }

    if(_conn)
    {
        MysqlConnect::Delete_MysqlConnect(_conn);
        _conn = NULL;
    }
}

LibString DBBalanceInfo::ToString() const
{
    LibString info;
    info.AppendFormat("index:%d, connection:%s", _index, _conn->ToString().c_str());
    return info;
}

POOL_CREATE_OBJ_DEFAULT_IMPL(MysqlDB);

MysqlDB::MysqlDB(MysqlDBMgr *owner)
:_owner(owner)
,_isReady{false}
,_maxId(0)
,_curMaxOperatorUid(0)
,_targetPoller(NULL)
,_msgLevel(0)
,_eventType(0)
{

}

MysqlDB::~MysqlDB()
{
    _Clear();
}

Int32 MysqlDB::Init()
{
    if(UNLIKELY(_cfg._dbThreadNum < 1))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("mysql need one thread to run at least _threadNum:%d"), _cfg._dbThreadNum);
        return Status::Failed;
    }

    if(UNLIKELY(!_workerBalance.empty()))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("init repeated"));
        return Status::Repeat;
    }

    _workerBalance.resize(_cfg._dbThreadNum);
    for(Int32 idx = 0; idx < _cfg._dbThreadNum; ++idx)
    {
        auto newBalance = DBBalanceInfo::New_DBBalanceInfo(idx);
        _workerBalance[idx] = newBalance;

        // 线程
        auto newVar = KERNEL_NS::Variant::New_Variant();
        (*newVar) = newBalance;
        auto newThread = new LibThread;
        newBalance->_thread = newThread;
        newThread->AddTask2(this, &MysqlDB::_OnWorker, newVar);

        // 消息队列
        auto msgQueue = ConcurrentPriorityQueue<MysqlRequest *>::New_ConcurrentPriorityQueue();
        newBalance->_msgQueue =  msgQueue;
        msgQueue->SetMaxLevel(0);
        msgQueue->Init();

        // msyql连接
        newBalance->_conn = MysqlConnect::New_MysqlConnect(++_maxId);
        newBalance->_conn->SetConfig(_cfg);
        newThread->SetThreadName(KERNEL_NS::LibString().AppendFormat("MysqlDB_%d", _maxId));
    }

    _msgHandler.resize(MysqlMsgType::Max);
    _msgHandler[MysqlMsgType::Stmt] = &MysqlDB::_StmtHandler;
    _msgHandler[MysqlMsgType::NormalSql] = &MysqlDB::_NormalSqlHandler;
    _msgHandler[MysqlMsgType::SqlWithTransAction] = &MysqlDB::_SqlWithTransActionSqlHandler;

    if(_targetPoller == NULL)
        g_Log->Warn(LOGFMT_OBJ_TAG("have no target poller and cant send db msg back"));

    return Status::Success;
}

Int32 MysqlDB::Start()
{
    if(UNLIKELY(_workerBalance.empty()))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("mysql not init before _threadNum:%d"), _cfg._dbThreadNum);
        return Status::NotInit;
    }

    for(Int32 idx = 0; idx < _cfg._dbThreadNum; ++idx)
    {
        auto balance = _workerBalance[idx];
        balance->_thread->Start();
    }

    return Status::Success;
}

void MysqlDB::WillClose()
{
    if(UNLIKELY(_workerBalance.empty()))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("mysql not init before _threadNum:%d"), _cfg._dbThreadNum);
        return;
    }

    for(auto balance : _workerBalance)
        balance->_thread->HalfClose();
}

void MysqlDB::Close()
{
    for(auto balance : _workerBalance)
        balance->_thread->FinishClose();

    _Clear();
}

Int32 MysqlDB::Dump(const LibString &dumpFilePath)
{
    // 选择一个可工作的线程
    if(UNLIKELY(_workerBalance.empty()))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("mysql worker not start dumpFilePath:%s."), dumpFilePath.c_str());
        return Status::Failed;
    }

    bool isWorking = false;
    for(auto balance : _workerBalance)
    {
        if(!balance->_thread->IsDestroy())
        {
            isWorking = true;
            break;
        }
    }
    if(!isWorking)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("have no any woker ini working dumpFilePath:%s."), dumpFilePath.c_str());
        return Status::Failed;
    }

    g_Log->Info(LOGFMT_OBJ_TAG("begin dump mysql config:%s dump file:%s"), _cfg.ToString().c_str(), dumpFilePath.c_str());

    auto dbDumpCmd = LibString().AppendFormat(
        "mysqldump --single-transaction --set-gtid-purged=OFF -h\"%s\" -P%hu -u\"%s\" -p\"%s\" \"%s\" > \"%s\"",
        _cfg._host.c_str(), _cfg._port, _cfg._user.c_str(), _cfg._pwd.c_str(), _cfg._dbName.c_str(), dumpFilePath.c_str());

    auto ret = ::system(dbDumpCmd.c_str());
    if (ret != 0)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("Execute mysql dump command failed, cmd:%s, ret:%d"), dbDumpCmd.c_str(), ret);
        return Status::Failed;
    }

    g_Log->Info(LOGFMT_OBJ_TAG("Dump database %s finished"), _cfg._dbName.c_str());
    return Status::Success;
}

bool MysqlDB::PushRequest(MysqlRequest *req)
{
    if(UNLIKELY(!_isReady))
    {
        g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), KERNEL_NS::LibString().AppendFormat("mysql db is not ready mysql db:%s, req:", ToString().c_str()), req->Dump());
        return false;
    }

    if(UNLIKELY(req->_dbOperatorId >= static_cast<Int32>(_workerBalance.size())))
    {
        g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), KERNEL_NS::LibString().AppendFormat("operator id:%d is bad, cant over balance size mysql db:%s, req:", req->_dbOperatorId, ToString().c_str()), req->Dump());
        return false;
    }

    auto balance = _workerBalance[req->_dbOperatorId];
    if(UNLIKELY(!balance))
    {
        g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), KERNEL_NS::LibString().AppendFormat("have no worker by operator id:%d, mysql db:%s, req:", req->_dbOperatorId, ToString().c_str()), req->ToString());
        return false;
    }

    balance->_msgQueue->PushQueue(0, req);
    balance->_eventGuard.Sinal();

    return true;
}

LibString MysqlDB::ToString() const
{
    LibString info;
    info.AppendFormat("db config:%s", _cfg.ToString().c_str());

    return info;
}

void MysqlDB::_OnWorker(LibThread *t, Variant *var)
{
    auto workerBalance = var->AsPtr<DBBalanceInfo>();

    // 线程内存整理
    SmartPtr<ThreadTlsMemoryCleanerGroup, AutoDelMethods::CustomDelete> cleanerGroup = ThreadTlsMemoryCleanerGroupFactory::StaticCreateAs();
    cleanerGroup.SetClosureDelegate([](void *p){
        auto ptr = reinterpret_cast<ThreadTlsMemoryCleanerGroup *>(p);
        ptr->Release();
    });

    do
    {
        auto ret = cleanerGroup->Init();
        if(ret != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("memory cleaner init fail ret:%d worker balance:%s, cleanerGroup:%s")
                        , ret, workerBalance->ToString().c_str(), cleanerGroup->ToString().c_str());
            break;
        }

        ret = cleanerGroup->Start();
        if(ret != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("memory cleaner start fail ret:%d worker balance:%s,cleanerGroup:%s")
                    , ret, workerBalance->ToString().c_str(), cleanerGroup->ToString().c_str());
            break;
        }

        ret = workerBalance->_conn->Init();
        if(ret != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("mysql connect init fail ret:%d workerBalance:%s,"), ret, workerBalance->ToString().c_str());
            break;
        }

        ret = workerBalance->_conn->Start();
        if(ret != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("mysql connect start fail ret:%d, workerBalance:%s "), ret, workerBalance->ToString().c_str());
            break;
        }

        std::vector<LibList<MysqlRequest *> *> reqList; 
        reqList.resize(1);
        reqList[0] = LibList<MysqlRequest *>::New_LibList();

        _isReady = true;

        g_Log->Info(LOGFMT_OBJ_TAG("db is ready config:%s"), _cfg.ToString().c_str());

        auto msgQueue = workerBalance->_msgQueue;
        auto &eventGuard = workerBalance->_eventGuard;
        
        Int64 pingExpireTime = 0;
        while (!t->IsDestroy())
        {
            if(msgQueue->IsEmpty())
            {
                eventGuard.Lock();
                eventGuard.TimeWait(50);
                eventGuard.Unlock();
            }

            msgQueue->SwapAll(reqList);

            auto eventList = reqList[0];
            for(auto node = eventList->Begin(); node;)
            {
                auto req = node->_data;

                // 处理
                do
                {
                    if(req->_msgType >= static_cast<Int32>(_msgHandler.size()))
                    {
                        g_Log->FailSql(LOGFMT_OBJ_TAG_NO_FMT(), KERNEL_NS::LibString("bad mysql request :"), req->Dump());
                        break;
                    }

                    auto handler = _msgHandler[req->_msgType];
                    if(!handler)
                    {
                        g_Log->FailSql(LOGFMT_OBJ_TAG_NO_FMT(), KERNEL_NS::LibString("have no mysql msg handler request :"), req->Dump());
                        break;
                    }

                    auto outputLogFunc = [&req](UInt64 costMs)
                    {
                        g_Log->DumpSql(LOGFMT_NON_OBJ_TAG_NO_FMT(MysqlDB), KERNEL_NS::LibString().AppendFormat("seq id:%llu, cost %llu ms", req->_seqId, costMs));
                    };
                    
                    // TODO:性能监控日志输出优化
                    PERFORMANCE_RECORD_DEF(pr, outputLogFunc, 100);
                    (this->*handler)(workerBalance->_conn, req, pingExpireTime);
                } while (false);

                MysqlRequest::Delete_MysqlRequest(req);
                node = eventList->Erase(node);
            }

            cleanerGroup->Drive();
        }

    } while (false);
    
    workerBalance->_conn->Close();
    workerBalance->_msgQueue->Destroy();

    cleanerGroup->WillClose();
    cleanerGroup->Close();

    g_Log->Info(LOGFMT_OBJ_TAG("db is stop config:%s, workerBalance:%s"), _cfg.ToString().c_str(), workerBalance->ToString().c_str());

    _isReady = false;
}

void MysqlDB::_StmtHandler(MysqlConnect *curConn, MysqlRequest *req, Int64 &pingExpireTime)
{
    SmartPtr<MysqlResponse, AutoDelMethods::CustomDelete> res = MysqlResponse::New_MysqlResponse();
    res.SetClosureDelegate([](void *p){
        auto ptr = reinterpret_cast<MysqlResponse *>(p);
        MysqlResponse::Delete_MysqlResponse(ptr);
    });

    res->_dbOperatorId = req->_dbOperatorId;
    res->_seqId = req->_seqId;
    res->_stub = req->_stub;
    res->_msgType = req->_msgType;
    res->_handler = req->_handler;
    req->_handler = NULL;
    res->_isDestroyHandler = req->_isDestroyHandler;
    res->_dbName = req->_dbName;
    res->_var = req->_var;
    req->_var = NULL;

    UInt32 mysqlDbErr = 0;
    Int32 finalErrCode = Status::Success;
    auto &&cb = [this, req, &res, &mysqlDbErr, &finalErrCode](MysqlConnect *conn, UInt64 seqId, Int32 errCode, UInt32 mysqlErrno, bool isSendToMysql, 
    Int64 insertId, Int64 affectedRows, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &records)
    {
        if(errCode != Status::Success)
        {
            g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), LibString().AppendFormat("sql using stmt fail, errCode:%d, mysqlErrno:%u seq id:%llu, maxInsertId:%lld, totalAffectedRows:%lld req:"
            , errCode, mysqlErrno, seqId, res->_maxInsertId, res->_affectedRows), req->Dump());

            finalErrCode = errCode;
            mysqlDbErr = mysqlErrno;

            res->_errCode = errCode;
            res->_mysqlErrno = mysqlErrno;
            res->_isRequestSendToMysql = isSendToMysql;

            return;
        }

        res->_errCode = Status::Success;
        res->_mysqlErrno = 0;

        if(res->_maxInsertId < insertId)
            res->_maxInsertId = insertId;

        res->_affectedRows += affectedRows;

        for(auto &rec : records)
            res->_datas.push_back(rec);

        res->_isRequestSendToMysql = isSendToMysql;
    };

    const auto &curTime = KERNEL_NS::LibTime::NowTimestamp();
    bool isReconnected = true;
    if((pingExpireTime == 0) || (curTime >= pingExpireTime) || !curConn->IsConnected())
    {
        pingExpireTime = curConn->GetConfig()._pingIntervalSeconds + curTime;

        curConn->OnMysqlDisconnect();

        // 重连
        auto leftPingTimes = _cfg._retryWhenError;
        isReconnected = false;
        const auto &pingContent = KERNEL_NS::LibString().AppendFormat("ExecuteSqlUsingStmt fail of network disconnected, try reconnect seq id:%llu", req->_seqId);
        for(Int32 retryIndex = 0; retryIndex < leftPingTimes; ++retryIndex)
        {
            if(curConn->Ping(pingContent))
            {
                isReconnected = true;
                break;
            }

            g_Log->Info(LOGFMT_OBJ_TAG("ExecuteSqlUsingStmt fail try reconnect seq id:%llu, mysql connection:%s"), req->_seqId, ToString().c_str());
            KERNEL_NS::SystemUtil::ThreadSleep(1000);
        }
    }

    Int32 idx = 0;
    const Int32 count = static_cast<Int32>(req->_builderInfos.size());
    if(LIKELY(isReconnected))
    {
        for(; idx < count; ++idx)
        {
            auto builderInfo = req->_builderInfos[idx];
            if(!curConn->ExecuteSqlUsingStmt(*builderInfo->_builder, req->_seqId, builderInfo->_fields, cb))
            {
                // 因为网络断开需要重试
                if(IS_MYSQL_NETWORK_ERROR(res->_mysqlErrno))
                {
                    // 重连
                    auto leftPingTimes = _cfg._retryWhenError;
                    bool isReconnected = false;
                    const auto &pingContent = KERNEL_NS::LibString().AppendFormat("ExecuteSqlUsingStmt fail of network disconnected, try reconnect seq id:%llu", req->_seqId);
                    for(Int32 retryIndex = 0; retryIndex < leftPingTimes; ++retryIndex)
                    {
                        if(curConn->Ping(pingContent))
                        {
                            isReconnected = true;
                            break;
                        }

                        g_Log->Info(LOGFMT_OBJ_TAG("ExecuteSqlUsingStmt fail try reconnect seq id:%llu, mysql connection:%s"), req->_seqId, ToString().c_str());
                        KERNEL_NS::SystemUtil::ThreadSleep(1000);
                    }
                    
                    if(!isReconnected)
                    {
                        g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), LibString().AppendFormat("ExecuteSqlUsingStmt fail of mysql disconnected req:"), req->Dump());
                        break;
                    }

                    // 断开的错误重连后不算需要重置
                    finalErrCode = Status::Success;
                    mysqlDbErr = 0;

                    bool isSuccess = false;
                    const Int32 leftRetryTimes = _cfg._retryWhenError;
                    if(leftRetryTimes > 0)
                    {
                        g_Log->Warn(LOGFMT_OBJ_TAG("mysql network interrupt and will retry %d times mostly"), leftRetryTimes);

                        for(Int32 idx = 0; idx < leftRetryTimes; ++idx)
                        {
                            if(curConn->ExecuteSqlUsingStmt(*builderInfo->_builder, req->_seqId, builderInfo->_fields, cb))
                            {
                                isSuccess = true;
                                break;
                            }
                        }
                    }
                    else
                    {
                        g_Log->Warn(LOGFMT_OBJ_TAG("mysql network interrupt and will retry many times util success mysql:%s, connection:%s."), ToString().c_str(), curConn->ToString().c_str());
                        
                        for(;;)
                        {
                            if(curConn->ExecuteSqlUsingStmt(*builderInfo->_builder, req->_seqId, builderInfo->_fields, cb))
                            {
                                isSuccess = true;
                                break;
                            }
                        }
                    }

                    // 成功则继续下一个sql, 错误则中断并打印未执行成功的sql
                    if(isSuccess)
                    continue;
                }

                g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), LibString().AppendFormat("ExecuteSql fail req:"), req->Dump());
                break;
            }
        }
    }
    else
    {
        g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), LibString().AppendFormat("ExecuteSqlUsingStmt fail of mysql disconnected req:"), req->Dump());
    }

    g_Log->DumpSql(LOGFMT_OBJ_TAG_NO_FMT(),  KERNEL_NS::LibString().AppendFormat("seq id:%llu db name:%s affected rows:%lld, final insert id:%lld, is req send to mysql:%d, errCode:%d, mysql err:%u"
                    , res->_seqId, res->_dbName.c_str(), res->_affectedRows, res->_maxInsertId, res->_isRequestSendToMysql, res->_errCode, res->_mysqlErrno));

    // 有失败, 则打印剩余失败的sql
    if(idx != count)
    {
        LibString failBuilder;
        for(;idx < count; ++idx)
        {
            auto builderInfo = req->_builderInfos[idx];
            failBuilder.AppendData(builderInfo->Dump()).AppendFormat("\n");
        }

        if((mysqlDbErr != 0) && (res->_mysqlErrno == 0))
        {
            res->_mysqlErrno = mysqlDbErr;
        }

        if(finalErrCode != Status::Success)
            res->_errCode = finalErrCode;
        else if(res->_errCode == Status::Success)
        {
            res->_errCode = Status::Failed;
        }
        g_Log->FailSql(LOGFMT_OBJ_TAG_NO_FMT(), KERNEL_NS::LibString().AppendFormat("\nmysql sql excute error seq id:%llu mysql db:%s connection:%s errCode:%d, mysqlerrno:%u, finalErrCode:%d, mysqlDbErr:%u request dump:\n",  res->_seqId, ToString().c_str(), curConn->ToString().c_str(), res->_errCode, res->_mysqlErrno, finalErrCode, mysqlDbErr)
                    , req->Dump(), LibString().AppendFormat("\nfail sqls:\n"), failBuilder);
    }

    // 使用指定的消息队列
    if(req->_msgQueue)
    {
        req->_msgQueue->PushQueue(res.pop());
        return;
    }

    // 返回res
    if(LIKELY(_targetPoller && (_eventType > 0)))
    {
        auto dbEvent = DbEvent::New_DbEvent(_eventType);
        dbEvent->_res = res.AsSelf();
        _targetPoller->Push(_msgLevel, dbEvent);
        res.pop();
    }
}

// TODO:失败时需要重试, 以及dump失败的sql
void MysqlDB::_NormalSqlHandler(MysqlConnect *curConn, MysqlRequest *req, Int64 &pingExpireTime)
{
    SmartPtr<MysqlResponse, AutoDelMethods::CustomDelete> res = MysqlResponse::New_MysqlResponse();
    res.SetClosureDelegate([](void *p){
        auto ptr = reinterpret_cast<MysqlResponse *>(p);
        MysqlResponse::Delete_MysqlResponse(ptr);
    });

    res->_dbOperatorId = req->_dbOperatorId;
    res->_seqId = req->_seqId;
    res->_stub = req->_stub;
    res->_msgType = req->_msgType;
    res->_handler = req->_handler;
    req->_handler = NULL;
    res->_isDestroyHandler = req->_isDestroyHandler;
    res->_dbName = req->_dbName;
    res->_var = req->_var;
    req->_var = NULL;

    UInt32 mysqlDbErr = 0;
    UInt32 finalErrCode = Status::Success;
    auto &&cb = [this, req, &res, &mysqlDbErr, &finalErrCode](MysqlConnect *conn, UInt64 seqId, Int32 errCode, UInt32 mysqlErrno, bool isSendToMysql, 
    Int64 insertId, Int64 affectedRows, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &records)
    {
        if(res->_maxInsertId < insertId)
            res->_maxInsertId = insertId;

        res->_affectedRows += affectedRows;

        if(errCode != Status::Success)
        {
            g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), LibString().AppendFormat("sql using stmt fail, errCode:%d, seq id:%llu, maxInsertId:%lld, totalAffectedRows:%lld req:"
            , errCode, seqId, res->_maxInsertId, res->_affectedRows), req->ToString().c_str());

            finalErrCode = errCode;
            mysqlDbErr = mysqlErrno;

            res->_errCode = errCode;
            res->_mysqlErrno = mysqlErrno;
            res->_isRequestSendToMysql = isSendToMysql;


            return;
        }

        res->_errCode = Status::Success;
        res->_mysqlErrno = 0;

        for(auto &rec : records)
            res->_datas.push_back(rec);

        res->_isRequestSendToMysql = isSendToMysql;
    };


    const auto &curTime = KERNEL_NS::LibTime::NowTimestamp();
    bool isReconnected = true;
    if((pingExpireTime == 0) || (curTime >= pingExpireTime) || !curConn->IsConnected())
    {
        pingExpireTime = curConn->GetConfig()._pingIntervalSeconds + curTime;

        curConn->OnMysqlDisconnect();

        // 重连
        auto leftPingTimes = _cfg._retryWhenError;
        isReconnected = false;
        const auto &pingContent = KERNEL_NS::LibString().AppendFormat("ExecuteSql fail of network disconnected, try reconnect seq id:%llu", req->_seqId);
        for(Int32 retryIndex = 0; retryIndex < leftPingTimes; ++retryIndex)
        {
            if(curConn->Ping(pingContent))
            {
                isReconnected = true;
                break;
            }

            g_Log->Info(LOGFMT_OBJ_TAG("ExecuteSql fail try reconnect seq id:%llu, mysql connection:%s"), req->_seqId, ToString().c_str());
            KERNEL_NS::SystemUtil::ThreadSleep(1000);
        }
    }

    Int32 idx = 0;
    const Int32 count = static_cast<Int32>(req->_builderInfos.size());

    if(LIKELY(isReconnected))
    {
        for(; idx < count; ++idx)
        {
            auto builderInfo = req->_builderInfos[idx];
            if(!curConn->ExecuteSql(*builderInfo->_builder, req->_seqId, cb))
            {
                // 因为网络断开需要重试
                if(IS_MYSQL_NETWORK_ERROR(res->_mysqlErrno))
                {
                    // 重连
                    bool isReconnected = false;
                    auto leftPingTimes = _cfg._retryWhenError;
                    const auto &pingContent = KERNEL_NS::LibString().AppendFormat("ExecuteSql fail of network disconnected, try reconnect seq id:%llu", req->_seqId);
                    for(Int32 retryIndex = 0; retryIndex < leftPingTimes; ++retryIndex)
                    {
                        if(curConn->Ping(pingContent))
                        {
                            isReconnected = true;
                            break;
                        }

                        g_Log->Info(LOGFMT_OBJ_TAG("ExecuteSql fail try reconnect seq id:%llu, mysql connection:%s"), req->_seqId, ToString().c_str());
                        KERNEL_NS::SystemUtil::ThreadSleep(1000);
                    }

                    if(!isReconnected)
                    {
                        g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), LibString().AppendFormat("ExecuteSql fail of mysql disconnected req:"), req->Dump());
                        break;
                    }

                    // 断开的错误重连后不算需要重置
                    finalErrCode = Status::Success;
                    mysqlDbErr = 0;

                    bool isSuccess = false;
                    const Int32 leftRetryTimes = _cfg._retryWhenError;
                    if(leftRetryTimes > 0)
                    {
                        g_Log->Warn(LOGFMT_OBJ_TAG("mysql network interrupt and will retry %d times mostly"), leftRetryTimes);

                        for(Int32 idx = 0; idx < leftRetryTimes; ++idx)
                        {
                            if(curConn->ExecuteSql(*builderInfo->_builder, req->_seqId, cb))
                            {
                                isSuccess = true;
                                break;
                            }
                        }
                    }
                    else
                    {
                        g_Log->Warn(LOGFMT_OBJ_TAG("mysql network interrupt and will retry many times util success mysql:%s connection:%s."), ToString().c_str(), curConn->ToString().c_str());
                        
                        for(;;)
                        {
                            if(curConn->ExecuteSql(*builderInfo->_builder, req->_seqId, cb))
                            {
                                isSuccess = true;
                                break;
                            }
                        }
                    }

                    // 成功则继续下一个sql, 错误则中断并打印未执行成功的sql
                    if(isSuccess)
                    continue;
                }

                g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), LibString().AppendFormat("ExecuteSql fail req:"), req->Dump());
                break;
            }
        }
    }
    else
    {
        g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), LibString().AppendFormat("ExecuteSql fail of mysql disconnected req:"), req->Dump());
    }

    g_Log->DumpSql(LOGFMT_OBJ_TAG_NO_FMT(),  KERNEL_NS::LibString().AppendFormat("seq id:%llu db name:%s affected rows:%lld, final insert id:%lld, is req send to mysql:%d, errCode:%d, mysql err:%u"
                    , res->_seqId, res->_dbName.c_str(), res->_affectedRows, res->_maxInsertId, res->_isRequestSendToMysql, res->_errCode, res->_mysqlErrno));

    // 有失败, 则打印剩余失败的sql
    if(idx != count)
    {
        LibString failBuilder;
        for(;idx < count; ++idx)
        {
            auto builderInfo = req->_builderInfos[idx];
            failBuilder.AppendData(builderInfo->Dump()).AppendFormat("\n");
        }

        if((mysqlDbErr != 0) && (res->_mysqlErrno == 0))
        {
            res->_mysqlErrno = mysqlDbErr;
        }

        if(finalErrCode != Status::Success)
            res->_errCode = finalErrCode;
        else if(res->_errCode == Status::Success)
        {
            res->_errCode = Status::Failed;
        }

        g_Log->FailSql(LOGFMT_OBJ_TAG_NO_FMT(), KERNEL_NS::LibString().AppendFormat("\nmysql sql excute error seq id:%llu mysql:%s connection:%s errCode:%d, mysqlerrno:%u, finalErrCode:%d, mysqlDbErr:%u request dump:\n",  res->_seqId, ToString().c_str(), curConn->ToString().c_str(), res->_errCode, res->_mysqlErrno, finalErrCode, mysqlDbErr)
                    , req->Dump(), LibString().AppendFormat("\nfail sqls:\n"), failBuilder);
    }

    // 使用指定的消息队列
    if(req->_msgQueue)
    {
        req->_msgQueue->PushQueue(res.pop());
        return;
    }

    // 返回res
    if(LIKELY(_targetPoller && (_eventType > 0)))
    {
        auto dbEvent = DbEvent::New_DbEvent(_eventType);
        dbEvent->_res = res.AsSelf();
        _targetPoller->Push(_msgLevel, dbEvent);

        res.pop();
    }
}

// TODO:失败时需要重试, 以及dump失败的sql, 因为开启事务执行的是多条sql, 所以不建议失败重试, 否则会造成不一致的情况(sql被其他线程执行覆盖)
void MysqlDB::_SqlWithTransActionSqlHandler(MysqlConnect *curConn, MysqlRequest *req, Int64 &pingExpireTime)
{
   SmartPtr<MysqlResponse, AutoDelMethods::CustomDelete> res = MysqlResponse::New_MysqlResponse();
    res.SetClosureDelegate([](void *p){
        auto ptr = reinterpret_cast<MysqlResponse *>(p);
        MysqlResponse::Delete_MysqlResponse(ptr);
    });

    res->_dbOperatorId = req->_dbOperatorId;
    res->_seqId = req->_seqId;
    res->_stub = req->_stub;
    res->_msgType = req->_msgType;
    res->_handler = req->_handler;
    req->_handler = NULL;
    res->_isDestroyHandler = req->_isDestroyHandler;
    res->_dbName = req->_dbName;
    res->_var = req->_var;
    req->_var = NULL;
    
    auto &&cb = [this, req, &res](MysqlConnect *conn, UInt64 seqId, Int32 errCode, UInt32 mysqlErrno, bool isSendToMysql, 
    Int64 insertId, Int64 affectedRows, std::vector<SmartPtr<Record, AutoDelMethods::CustomDelete>> &records)
    {
        if(res->_maxInsertId < insertId)
            res->_maxInsertId = insertId;

        res->_affectedRows += affectedRows;

        if(errCode != Status::Success)
        {
            g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), LibString().AppendFormat("sql using stmt fail, errCode:%d, seq id:%llu, maxInsertId:%lld, totalAffectedRows:%lld req:"
            , errCode, seqId, res->_maxInsertId, res->_affectedRows), req->ToString().c_str());

            res->_errCode = errCode;
            res->_mysqlErrno = mysqlErrno;
            res->_isRequestSendToMysql = isSendToMysql;

            return;
        }

        for(auto &rec : records)
            res->_datas.push_back(rec);

        res->_isRequestSendToMysql = isSendToMysql;
    };

    // 先执行ping
    const auto &curTime = KERNEL_NS::LibTime::NowTimestamp();
    bool isReconnected = true;
    auto leftPingTimes = _cfg._retryWhenError;
    if((pingExpireTime == 0) || (curTime >= pingExpireTime) || !curConn->IsConnected())
    {
        const auto &pingStr = KERNEL_NS::LibString().AppendFormat("Before ExecuteSqlUsingTransAction detect connected seq id:%llu", req->_seqId);
        pingExpireTime = curConn->GetConfig()._pingIntervalSeconds + curTime;

        curConn->OnMysqlDisconnect();

        // 重连
        isReconnected = false;
        for(Int32 retryIndex = 0; retryIndex < leftPingTimes; ++retryIndex)
        {
            if(curConn->Ping(pingStr))
            {
                isReconnected = true;
                break;
            }

            g_Log->Info(LOGFMT_OBJ_TAG("_SqlWithTransActionSqlHandler fail try reconnect seq id:%llu, mysql connection:%s"), req->_seqId, ToString().c_str());
            KERNEL_NS::SystemUtil::ThreadSleep(1000);
        }
    }

    do
    {

        if(UNLIKELY(!isReconnected))
        {
            res->_errCode = Status::Failed;
            g_Log->Warn(LOGFMT_OBJ_TAG("ping mysql fail please check use ping times:%d seq id:%llu."), leftPingTimes, req->_seqId);
            break;
        }

        std::vector<KERNEL_NS::SqlBuilder *> builders;
        for(auto &builder : req->_builderInfos)
            builders.push_back(builder->_builder);

        if(!curConn->ExecuteSqlUsingTransAction(builders, req->_seqId, cb))
        {
            g_Log->FailSql(LOGFMT_OBJ_TAG_NO_FMT(), KERNEL_NS::LibString().AppendFormat("\nmysql sql excute error  mysql:%s, connection:%s request dump:\n", ToString().c_str(), curConn->ToString().c_str())
                        , req->Dump());

            g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), LibString().AppendFormat("ExecuteSql fail req:"), req->Dump());
        }


    } while (false);

    g_Log->DumpSql(LOGFMT_OBJ_TAG_NO_FMT(),  KERNEL_NS::LibString().AppendFormat("seq id:%llu db name:%s affected rows:%lld, final insert id:%lld, is req send to mysql:%d, errCode:%d, mysql err:%u"
                    , res->_seqId, res->_dbName.c_str(), res->_affectedRows, res->_maxInsertId, res->_isRequestSendToMysql, res->_errCode, res->_mysqlErrno));

    // 有失败, 则打印剩余失败的sql
    if(res->_errCode != Status::Success)
    {
        LibString failBuilder;
        const Int32 count = static_cast<Int32>(req->_builderInfos.size());
        for(Int32 idx = 0; idx < count; ++idx)
        {
            auto builderInfo = req->_builderInfos[idx];
            failBuilder.AppendData(builderInfo->Dump()).AppendFormat("\n");
        }

        g_Log->FailSql(LOGFMT_OBJ_TAG_NO_FMT(), KERNEL_NS::LibString().AppendFormat("\nmysql sql excute error seq id:%llu mysql db:%s connection:%s errCode:%d, mysqlerrno:%u, request dump:\n",  res->_seqId, ToString().c_str(), curConn->ToString().c_str(), res->_errCode, res->_mysqlErrno)
                    , req->Dump(), LibString().AppendFormat("\nfail sqls:\n"), failBuilder);
    }

    // 使用指定的消息队列
    if(req->_msgQueue)
    {
        req->_msgQueue->PushQueue(res.pop());
        return;
    }
    
    // 返回res
    if(LIKELY(_targetPoller && (_eventType > 0)))
    {
        auto dbEvent = DbEvent::New_DbEvent(_eventType);
        dbEvent->_res = res.AsSelf();
        _targetPoller->Push(_msgLevel, dbEvent);

        res.pop();
    }
}

void MysqlDB::_Clear()
{
    ContainerUtil::DelContainer(_workerBalance, [](DBBalanceInfo *ptr){
        DBBalanceInfo::Delete_DBBalanceInfo(ptr);
    });
}

KERNEL_END
