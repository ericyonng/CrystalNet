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
 * Date: 2025-10-14 14:15:00
 * Author: Eric Yonng
 * Description: mongodb数据库管理
*/
#include <pch.h>
#include <OptionComp/storage/MongoDB/Impl/MongoDbMgr.h>
#include <OptionComp/storage/MongoDB/Impl/MongoDbMgrFactory.h>
#include <mongocxx/exception/exception.hpp>

KERNEL_BEGIN

mongocxx::instance MongoDbMgr::_instance;

MongoDbMgr::MongoDbMgr()
 :IMongoDbMgr(KERNEL_NS::RttiUtil::GetTypeId<MongoDbMgr>())
,_connectionPool(NULL)
{
 
}

MongoDbMgr::~MongoDbMgr()
{
    _Clear();
}

void MongoDbMgr::Release()
{
    MongoDbMgr::DeleteByAdapter_MongoDbMgr(MongoDbMgrFactory::_buildType.V, this);
}

void MongoDbMgr::OnRegisterComps()
{

}

void MongoDbMgr::SetUri(const KERNEL_NS::LibString &uri)
{
    _uri = uri;
}

void MongoDbMgr::SetAccountPwd(const KERNEL_NS::LibString &account, const KERNEL_NS::LibString &pwd)
{
    _account = account;

    // 转义
    _pwd = pwd.escape("", '%');
}

void MongoDbMgr::SetSrvHostName(const KERNEL_NS::LibString &hostName)
{
    _srvHostName = hostName;
}

#ifdef CRYSTAL_NET_CPP20

KERNEL_NS::CoTask<bool> MongoDbMgr::Query(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collection, KERNEL_NS::LibString keyName, UInt64 keyValue)
{
    co_return false;
}

KERNEL_NS::CoTask<bool> MongoDbMgr::Query(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collection, KERNEL_NS::LibString keyName, KERNEL_NS::LibString keyValue)
{
    co_return false;
}

#endif

Int32 MongoDbMgr::_OnHostInit()
{
    try
    {
        // 外部没有指定uri则使用srv的设置, 如果还是没有则报错
        if(_uri.empty())
        {
            if(_account.empty() || _pwd.empty() || _srvHostName.empty())
            {
                CLOG_ERROR("have no mongodb+srv uri config, please check, account:%s, pwd:%s, srvHostName:%s", _account.c_str(), _pwd.c_str(), _srvHostName.c_str());
                return Status::ConfigError;
            }

            _uri.AppendFormat("mongodb+srv://%s:%s@%s/?authSource=admin&w=majority&journal=true&readConcernLevel=majority&maxPoolSize=100&connectTimeoutMS=10000&socketTimeoutMS=30000&retryWrites=true&retryReads=true", _account.c_str(), _pwd.c_str(), _srvHostName.c_str());
        }

        auto uri = mongocxx::uri(_uri.GetRaw());
        _connectionPool = new mongocxx::pool(uri);
    }
    catch (const mongocxx::exception &e)
    {
        CLOG_ERROR("init mongodb mongocxx exception: %s, uri:%s account:%s, pwd:%s, srvHostName:%s", e.code().message().c_str(), _uri.c_str(), _account.c_str(), _pwd.c_str(), _srvHostName.c_str());
        return Status::ConfigError;
    }
    catch (const std::exception &e)
    {
        CLOG_ERROR("init mongodb std exception: %s, uri:%s account:%s, pwd:%s, srvHostName:%s", e.what(), _uri.c_str(), _account.c_str(), _pwd.c_str(), _srvHostName.c_str());
        return Status::ConfigError;
    }
    catch (...)
    {
        CLOG_ERROR("unknown exception: uri:%s account:%s, pwd:%s, srvHostName:%s", _uri.c_str(), _account.c_str(), _pwd.c_str(), _srvHostName.c_str());
        return Status::Failed;
    }
    
    return Status::Success;
}

Int32 MongoDbMgr::_OnCompsCreated()
{
    return Status::Success;
}

Int32 MongoDbMgr::_OnHostStart()
{
    return Status::Success;
}

void MongoDbMgr::_OnHostBeforeCompsWillClose()
{
}

void MongoDbMgr::_OnHostClose()
{
    _Clear();
}

void MongoDbMgr::_Clear()
{
    CRYSTAL_DELETE_SAFE(_connectionPool);
}

KERNEL_END

