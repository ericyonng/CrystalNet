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
 * Date: 2023-07-14 13:46:00
 * Author: Eric Yonng
 * Description: 
*/

#include<pch.h>
#include <OptionComp/storage/mysql/impl/Field.h>
#include <OptionComp/storage/mysql/impl/Record.h>
#include <OptionComp/storage/mysql/impl/MysqlMsg.h>
#include <OptionComp/storage/mysql/impl/SqlBuilder.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(MysqlMsgQueue);
POOL_CREATE_OBJ_DEFAULT_IMPL(MysqlRequest);
POOL_CREATE_OBJ_DEFAULT_IMPL(MysqlResponse);
POOL_CREATE_OBJ_DEFAULT_IMPL(MysqlSqlBuilderInfo);

MysqlMsgQueue::MysqlMsgQueue()
:_msgQueue(KERNEL_NS::ConcurrentPriorityQueue<MysqlResponse *>::New_ConcurrentPriorityQueue())
{
    _msgQueue->SetMaxLevel(0);
    _msgQueue->Init();
}

MysqlMsgQueue::~MysqlMsgQueue()
{
    if(LIKELY(_msgQueue))
        _msgQueue->Destroy();
}

MysqlSqlBuilderInfo::MysqlSqlBuilderInfo()
:_builder(NULL)
{

}

MysqlSqlBuilderInfo::~MysqlSqlBuilderInfo()
{
    CRYSTAL_RELEASE_SAFE(_builder);
    ContainerUtil::DelContainer2(_fields);
}

void MysqlSqlBuilderInfo::Release()
{
    MysqlSqlBuilderInfo::DeleteThreadLocal_MysqlSqlBuilderInfo(this);
}

MysqlSqlBuilderInfo *MysqlSqlBuilderInfo::Create()
{
    return MysqlSqlBuilderInfo::NewThreadLocal_MysqlSqlBuilderInfo();
}

LibString MysqlSqlBuilderInfo::Dump() const
{
    LibString info;
    info.AppendData(_builder->Dump()).AppendFormat("\n");

    for(auto field:_fields)
        info.AppendData(field->Dump()).AppendFormat("\n");

    return info;
}

MysqlRequest::MysqlRequest()
:_dbOperatorId(0)
,_seqId(0)
,_stub(0)
,_msgType(MysqlMsgType::Stmt)
,_handler(NULL)
,_isDestroyHandler(true)
,_var(NULL)
,_msgQueue(NULL)
{

}

MysqlRequest::~MysqlRequest()
{
    ContainerUtil::DelContainer2(_builderInfos);

    if(_isDestroyHandler)
        CRYSTAL_RELEASE_SAFE(_handler);
    _handler = NULL;

    if(_var)
    {
        Variant::DeleteThreadLocal_Variant(_var);
        _var = NULL;
    }

    // msg queue是不释放的
    _msgQueue = NULL;
}

LibString MysqlRequest::ToString() const
{
    // sqls
    std::vector<LibString> multiSql;
    for(auto b : _builderInfos)
        multiSql.push_back(b->Dump());

    return LibString().AppendFormat("db name:%s, operator id:%d, seq id:%llu, msg type:%d,%s, _stub:%llu, _isDestroyHandler:%d sql:\n", _dbName.c_str(), _dbOperatorId, _seqId, _msgType, MysqlMsgType::ToString(_msgType), _stub, _isDestroyHandler)
                    .AppendData(StringUtil::ToString(multiSql, ";"));
}

LibString MysqlRequest::Dump() const
{
    LibString info;

    std::vector<LibString> multiSql;
    for(auto b : _builderInfos)
        multiSql.push_back(b->Dump());

    info.AppendFormat("db name:%s, _dbOperatorId:%d,  seqId:%llu, stub:%llu, msg type:%d,%s, call back owner:%s, callback:%s, ", _dbName.c_str(), _dbOperatorId, _seqId, _stub, _msgType, MysqlMsgType::ToString(_msgType)
        , _handler ? _handler->GetOwnerRtti() : "",  _handler ? _handler->GetCallbackRtti() : "");

    info.AppendFormat("\nsql:").AppendData(KERNEL_NS::StringUtil::ToString(multiSql, ";\n"));

    return info;
}

MysqlResponse::MysqlResponse()
:_dbOperatorId(0)
,_seqId(0)
,_stub(0)
,_msgType(MysqlMsgType::Stmt)
,_maxInsertId(0)
,_affectedRows(0)
,_errCode(Status::Success)
,_mysqlErrno(0)
,_isRequestSendToMysql(false)
,_handler(NULL)
,_isDestroyHandler(true)
,_var(NULL)
{

}

MysqlResponse::~MysqlResponse()
{
    if(_isDestroyHandler)
        CRYSTAL_RELEASE_SAFE(_handler);
    _handler = NULL;

    if(_var)
    {
        Variant::DeleteThreadLocal_Variant(_var);
        _var = NULL;
    }
}

LibString MysqlResponse::ToString() const
{
    // fields
    std::vector<LibString> records;
    for(auto &d : _datas)
        records.push_back(d->ToString());

    return LibString().AppendFormat("db name:%s _dbOperatorId:%d seq id:%llu, msg type:%d,%s, _stub:%llu max insert id:%lld, affected rows:%lld, err code:%d, mysql errno:%u, is request send to mysql:%d, _isDestroyHandler:%d datas:\n"
                        ,_dbName.c_str(), _dbOperatorId, _seqId, _msgType, MysqlMsgType::ToString(_msgType), _stub, _maxInsertId, _affectedRows, _errCode, _mysqlErrno, _isRequestSendToMysql, _isDestroyHandler)
                        .AppendData(StringUtil::ToString(records, ";"));
}

KERNEL_END
