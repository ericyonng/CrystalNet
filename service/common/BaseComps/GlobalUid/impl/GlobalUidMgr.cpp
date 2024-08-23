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
 * Date: 2023-08-05 20:48:05
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/kernel.h>
#include <service_common/ServiceCommon.h>

#include <service/common/BaseComps/GlobalUid/impl/GlobalUidMgr.h>
#include <service/common/BaseComps/GlobalUid/impl/GlobalUidMgrFactory.h>
#include <service/common/BaseComps/GlobalUid/impl/GlobalUidMgrStorageFactory.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(IGlobalUidMgr);
POOL_CREATE_OBJ_DEFAULT_IMPL(GlobalUidMgr);

GlobalUidMgr::GlobalUidMgr()
:IGlobalUidMgr(KERNEL_NS::RttiUtil::GetTypeId<GlobalUidMgr>())
,_curAllocUid(0)
,_maxUid(0)
,_aheadCount(1000)
,_machineId(0)
,_synStorageCb(NULL)
{

}

GlobalUidMgr::~GlobalUidMgr()
{
    _Clear();
}

void GlobalUidMgr::Release() 
{
    GlobalUidMgr::DeleteByAdapter_GlobalUidMgr(GlobalUidMgrFactory::_buildType.V, this);
}

void GlobalUidMgr::OnRegisterComps()
{
    RegisterComp<GlobalUidMgrStorageFactory>();
}

Int32 GlobalUidMgr::OnLoaded(UInt64 key, const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db)
{
    Int64 bytes = static_cast<Int64>(sizeof(_maxUid));
    bytes = bytes > db.GetReadableSize() ? db.GetReadableSize() : bytes;
    if(LIKELY(bytes > 0))
        db.Read(&_maxUid, bytes);

    _InitGuid();
    return Status::Success;
}

Int32 GlobalUidMgr::OnSave(UInt64 key, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) const
{
    if(!db.Write(&_maxUid, static_cast<Int64>(sizeof(_maxUid))))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("write data fail key:%llu, maxUid:%llu"), key, _maxUid);
        return Status::SerializeFail;
    }

    return Status::Success;
}

UInt64 GlobalUidMgr::NewGuid()
{
    if(UNLIKELY(_maxUid == 0))
        _InitGuid();

    if(UNLIKELY(_curAllocUid >= _maxUid))
    {
        _curAllocUid = KERNEL_NS::GuidUtil::Snowflake(_snowflakInfo);
        _maxUid = KERNEL_NS::GuidUtil::Snowflake(_snowflakInfo, _aheadCount - 1);

        #if CRYSTAL_STORAGE_ENABLE
            MaskNumberKeyModifyDirty(static_cast<UInt64>(_machineId));
        #endif
        auto err = _synStorageCb->Invoke(this);
        if(err != Status::Success)
            g_Log->Error(LOGFMT_OBJ_TAG("storage guid fail err:%d, guid mgr:%s"), err, ToString().c_str());

        return _curAllocUid;
    }

    return ++_curAllocUid;
}

void GlobalUidMgr::SetGetIdAheadCount(Int32 aheadCount)
{
    _aheadCount = static_cast<UInt64>(aheadCount > 0 ? aheadCount : 1);
}

void GlobalUidMgr::SetUpdateLastIdCallback(KERNEL_NS::IDelegate<Int32, ILogicSys *> *deleg)
{
    if(UNLIKELY(_synStorageCb))
        _synStorageCb->Release();

    _synStorageCb = deleg;
}

void GlobalUidMgr::OnStartup()
{
    _InitGuid();

    g_Log->Info(LOGFMT_OBJ_TAG("_maxUid :%llu, _aheadCount:%llu, _machineId:%u"), _maxUid, _aheadCount, _machineId);
}

KERNEL_NS::LibString GlobalUidMgr::ToString() const
{
    KERNEL_NS::LibString info;
    info.AppendFormat("max uid:%llu, cur alloc uid:%llu, ahead count:%llu, machineId:%u", _maxUid, _curAllocUid, _aheadCount, _machineId);
    return info;
}

Int32 GlobalUidMgr::_OnGlobalSysInit()
{
    _machineId = GetApp()->GetMachineId();

    if(UNLIKELY(!_synStorageCb))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("need a syn storage callback please check."));
        return Status::Failed;
    }
    return Status::Success;
}

void GlobalUidMgr::_OnGlobalSysClose()
{
    _Clear();
}

void GlobalUidMgr::_Clear()
{
    CRYSTAL_RELEASE_SAFE(_synStorageCb);
}

void GlobalUidMgr::_InitGuid()
{
    if(UNLIKELY(_maxUid == 0))
    {
        KERNEL_NS::GuidUtil::InitSnowFlake(_snowflakInfo, _machineId);
        _maxUid = _snowflakInfo.ToId();

        #if CRYSTAL_STORAGE_ENABLE
         MaskNumberKeyAddDirty(static_cast<UInt64>(_machineId));
        #endif
        
        _synStorageCb->Invoke(this);
    }
    else
    {
        KERNEL_NS::GuidUtil::InitSnowFlakeById(_snowflakInfo, _maxUid);
    }

    _curAllocUid = _maxUid;
}



SERVICE_END
