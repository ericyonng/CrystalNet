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
 * Date: 2023-11-19 00:03:49
 * Author: Eric Yonng
 * Description: 
*/


#include <pch.h>
#include <Comps/SystemLog/Impl/SystemLogGlobal.h>
#include <Comps/SystemLog/Impl/SystemLogGlobalStorageFactory.h>
#include <Comps/SystemLog/Impl/SystemLogGlobalStorage.h>
#include <Comps/SystemLog/Impl/SystemLogGlobalFactory.h>
#include <Comps/config/config.h>
#include <protocols/protocols.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(ISystemLogGlobal);
POOL_CREATE_OBJ_DEFAULT_IMPL(SystemLogGlobal);

SystemLogGlobal::SystemLogGlobal()
{

}

SystemLogGlobal::~SystemLogGlobal()
{

}

void SystemLogGlobal::Release()
{
    SystemLogGlobal::DeleteByAdapter_SystemLogGlobal(SystemLogGlobalFactory::_buildType.V, this);
}

void SystemLogGlobal::OnRegisterComps()
{
    RegisterComp<SystemLogGlobalStorageFactory>();
}

Int32 SystemLogGlobal::_OnGlobalSysInit()
{
    Subscribe(Opcodes::OpcodeConst::OPCODE_SystemLogDataListReq, this, &SystemLogGlobal::_OnSystemLogDataListReq);
    
    return Status::Success;
}

Int32 SystemLogGlobal::OnLoaded(UInt64 key, const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db)
{
    return Status::Success;
}

Int32 SystemLogGlobal::OnSave(UInt64 key, std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> &fieldRefdb) const
{
    auto iter = _idRefSystemLogData.find(key);
    if(iter == _idRefSystemLogData.end())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("system log not exists key:%llu"), key);
        return Status::NotFound;
    }

    auto data = iter->second;

    {// id
        auto newStream = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
        newStream->Init(sizeof(UInt64));
        newStream->WriteUInt64(key);
        fieldRefdb.insert(std::make_pair(SystemLogGlobalStorage::ID, newStream));
    }

    {// libraryId
        auto newStream = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
        newStream->Init(sizeof(UInt64));
        newStream->WriteUInt64(data->libraryid());
        fieldRefdb.insert(std::make_pair(SystemLogGlobalStorage::LIBRARY_ID_NAME, newStream));
    }

    {// data

        auto newStream = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
        fieldRefdb.insert(std::make_pair(SystemLogGlobalStorage::LOG_DATA_NAME, newStream));
        if(!data->Encode(*newStream))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("encode fail key:%llu"), key);
            return Status::SerializeFail;
        }
    }

    _idRefSystemLogData.erase(iter);

    data->Release();

    return Status::Success;
}

void SystemLogGlobal::AddLog(UInt64 libraryId, const KERNEL_NS::LibString &titleWordId, const std::vector<VariantParam> &titleParams, const KERNEL_NS::LibString &contentWordId, const std::vector<VariantParam> &contentParams)
{
    auto uid = GetGlobalSys<IGlobalUidMgr>()->NewGuid();
    auto newData = new SystemLogData();
    newData->set_id(uid);
    newData->set_libraryid(libraryId);
    newData->set_titlewordid(titleWordId.GetRaw());

    for(auto &param : titleParams)
        *newData->add_titleparams() = param;

    newData->set_contentwordid(contentWordId.GetRaw());

    for(auto &param : contentParams)
        *newData->add_contentparams() = param;

    _idRefSystemLogData.insert(std::make_pair(uid, newData));

    MaskNumberKeyAddDirty(uid);
}

void SystemLogGlobal::_OnSystemLogDataListReq(KERNEL_NS::LibPacket *&packet)
{
    // 
    
}

SERVICE_END
