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

                    (this->*handler)(workerBalance->_conn, req);
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

void MysqlDB::_StmtHandler(MysqlConnect *curConn, MysqlRequest *req)
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
        if(errCode != Status::Success)
        {
            g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), LibString().AppendFormat("sql using stmt fail, errCode:%d, seq id:%llu, maxInsertId:%lld, totalAffectedRows:%lld req:"
            , errCode, seqId, res->_maxInsertId, res->_affectedRows), req->Dump());

            res->_errCode = errCode;
            res->_mysqlErrno = mysqlErrno;
            res->_isRequestSendToMysql = isSendToMysql;

            return;
        }

        if(res->_maxInsertId < insertId)
            res->_maxInsertId = insertId;

        res->_affectedRows += affectedRows;

        for(auto &rec : records)
            res->_datas.push_back(rec);

        res->_isRequestSendToMysql = isSendToMysql;
    };

    Int32 idx = 0;
    const Int32 count = static_cast<Int32>(req->_builders.size());
    for(; idx < count; ++idx)
    {
        auto builder = req->_builders[idx];
        if(!curConn->ExecuteSqlUsingStmt(*builder, req->_seqId, req->_fields, cb))
        {
            // 因为网络断开需要重试
            if(IS_MYSQL_NETWORK_ERROR(res->_mysqlErrno))
            {
                bool isSuccess = false;
                const Int32 leftRetryTimes = _cfg._retryWhenError;
                if(leftRetryTimes > 0)
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("mysql network interrupt and will retry %d times mostly"), leftRetryTimes);

                    for(Int32 idx = 0; idx < leftRetryTimes; ++idx)
                    {
                        if(curConn->ExecuteSqlUsingStmt(*builder, req->_seqId, req->_fields, cb))
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
                        if(curConn->ExecuteSqlUsingStmt(*builder, req->_seqId, req->_fields, cb))
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

    // 有失败, 则打印剩余失败的sql
    if(idx != count)
    {
        LibString failBuilder;
        for(;idx < count; ++idx)
        {
            auto builder = req->_builders[idx];
            failBuilder.AppendData(builder->Dump()).AppendFormat("\n");
        }
        g_Log->FailSql(LOGFMT_OBJ_TAG_NO_FMT(), KERNEL_NS::LibString().AppendFormat("\nmysql sql excute error mysql db:%s connection:%s request dump:\n", ToString().c_str(), curConn->ToString().c_str())
                    , req->Dump(), LibString().AppendFormat("\nfail sqls:\n"), failBuilder);
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
void MysqlDB::_NormalSqlHandler(MysqlConnect *curConn, MysqlRequest *req)
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

    Int32 idx = 0;
    const Int32 count = static_cast<Int32>(req->_builders.size());
    for(; idx < count; ++idx)
    {
        auto builder = req->_builders[idx];
        if(!curConn->ExecuteSql(*builder, req->_seqId, cb))
        {
            // 因为网络断开需要重试
            if(IS_MYSQL_NETWORK_ERROR(res->_mysqlErrno))
            {
                bool isSuccess = false;
                const Int32 leftRetryTimes = _cfg._retryWhenError;
                if(leftRetryTimes > 0)
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("mysql network interrupt and will retry %d times mostly"), leftRetryTimes);

                    for(Int32 idx = 0; idx < leftRetryTimes; ++idx)
                    {
                        if(curConn->ExecuteSql(*builder, req->_seqId, cb))
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
                        if(curConn->ExecuteSql(*builder, req->_seqId, cb))
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

    // 有失败, 则打印剩余失败的sql
    if(idx != count)
    {
        LibString failBuilder;
        for(;idx < count; ++idx)
        {
            auto builder = req->_builders[idx];
            failBuilder.AppendData(builder->Dump()).AppendFormat("\n");
        }
        g_Log->FailSql(LOGFMT_OBJ_TAG_NO_FMT(), KERNEL_NS::LibString().AppendFormat("\nmysql sql excute error mysql:%s connection:%s request dump:\n", ToString().c_str(), curConn->ToString().c_str())
                    , req->Dump(), LibString().AppendFormat("\nfail sqls:\n"), failBuilder);
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
void MysqlDB::_SqlWithTransActionSqlHandler(MysqlConnect *curConn, MysqlRequest *req)
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

    if(!curConn->ExecuteSqlUsingTransAction(req->_builders, req->_seqId, cb))
    {
        const Int32 count = static_cast<Int32>(req->_builders.size());
        g_Log->FailSql(LOGFMT_OBJ_TAG_NO_FMT(), KERNEL_NS::LibString().AppendFormat("\nmysql sql excute error  mysql:%s, connection:%s request dump:\n", ToString().c_str(), curConn->ToString().c_str())
                    , req->Dump());

        g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), LibString().AppendFormat("ExecuteSql fail req:"), req->Dump());
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
