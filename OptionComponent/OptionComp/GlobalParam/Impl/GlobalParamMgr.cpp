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
 * Date: 2026-07-10 09:46:12
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <OptionComp/GlobalParam/Impl/GlobalParamMgr.h>
#include <OptionComp/GlobalParam/Impl/GlobalParamMgrFactory.h>
#include <kernel/comp/Utils/RttiUtil.h>
#include <OptionComp/storage/MongoDB/Interface/IMongoDbMgr.h>

KERNEL_BEGIN

GlobalParamMgr::GlobalParamMgr()
 :IGlobalParamMgr(RttiUtil::GetTypeId<GlobalParamMgr>())
 ,_db("GlobalParam")
 ,_collectionName("GlobalParamMgr")
 ,_uniqueIndexFieldName("ParamId")
 ,_indexName("ParamIdIndex")
{
}

GlobalParamMgr::~GlobalParamMgr()
{
    _Clear();
}

void GlobalParamMgr::Release()
{
    GlobalParamMgr::DeleteByAdapter_GlobalParamMgr(GlobalParamMgrFactory::_buildType.V, this);
}

void GlobalParamMgr::SetMongodbMgr(IMongoDbMgr *mongodbMgr)
{
  _mongodbMgr = mongodbMgr;
}

// 添加参数
CoTask<bool> GlobalParamMgr::UpdateParam(const KERNEL_NS::LibString &paramName, std::map<LibString, Variant> *keyRefValue)
{
    std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> kv;
    kv.emplace(_uniqueIndexFieldName, paramName);
    auto suc = co_await _mongodbMgr->UpdateData(_db, _collectionName, kv, keyRefValue, true);

    CLOG_DEBUG("UpdateParam Param:%s", KERNEL_NS::StringUtil::ToString(kv, ',').c_str());

    co_return suc;
}
    
    
// 原子更新参数
CoTask<bool> GlobalParamMgr::AtomicUpdateParam(const KERNEL_NS::LibString &paramName, std::map<LibString, Variant> *keyRefValue, void *condition, LibString *jsonBackOrigin)
{
    std::map<KERNEL_NS::LibString, KERNEL_NS::Variant> kv;
    kv.emplace(_uniqueIndexFieldName, paramName);
    if (keyRefValue && (!keyRefValue->contains(_uniqueIndexFieldName)))
        keyRefValue->emplace(_uniqueIndexFieldName, KERNEL_NS::Variant(paramName));
    auto ret = co_await _mongodbMgr->UpdateDataIf(_db, _collectionName, kv, keyRefValue, condition, jsonBackOrigin);

    CLOG_DEBUG("AtomicUpdateParam UpdateDataIf kv:%s, jsonBackOrigin:%s, ", KERNEL_NS::StringUtil::ToString(kv, ',').c_str(), jsonBackOrigin ? jsonBackOrigin->c_str():"");

    co_return ret;
}

const KERNEL_NS::LibString &GlobalParamMgr::GetUniqueKeyFieldName() const
{
    return _uniqueIndexFieldName;
}

Int32 GlobalParamMgr::_OnHostInit()
{
    if (!_mongodbMgr)
    {
        CLOG_ERROR("have no mongodb mgr");
        return Status::ConfigError;
    }
    
    // 创建索引
    std::vector<std::pair<KERNEL_NS::LibString, Int32>> fields;
    fields.emplace_back(_uniqueIndexFieldName, 1);
    if (!_mongodbMgr->CreateIndex(_db, _collectionName, _uniqueIndexFieldName, fields, true))
    {
        CLOG_ERROR("create index failed db:%s, collection name:%s, index name:%s", _db.c_str(), _collectionName.c_str(), _uniqueIndexFieldName.c_str());
        return Status::Failed;
    }
    
    return Status::Success;
}

Int32 GlobalParamMgr::_OnHostStart()
{
    return Status::Success;

}
void GlobalParamMgr::_OnHostClose()
{
    
}

void GlobalParamMgr::_Clear()
{
    
}


KERNEL_END