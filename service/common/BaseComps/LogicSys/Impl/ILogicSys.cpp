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
 * Date: 2022-09-22 19:39:30
 * Author: Eric Yonng
 * Description: 逻辑系统
*/

#include <pch.h>
#include <service/common/BaseComps/LogicSys/Impl/ILogicSys.h>
#include <service/common/BaseComps/Event/Defs/EventEnums.h>
#include <service/common/BaseComps/ServiceCompType.h>
#include <service/common/BaseComps/Storage/storage.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(ILogicSys);

ILogicSys::ILogicSys()
:_service(NULL)
,_serviceProxy(NULL)
,_app(NULL)
,_timerMgr(NULL)
,_eventMgr(NULL)
,_quitServiceEventDefaltStub(INVALID_LISTENER_STUB)
,_storageOperatorId(0)
,_storage(NULL)
{
    _SetType(ServiceCompType::LOGIC_SYS);
}

ILogicSys::~ILogicSys()
{
    _Clear();
}

SERVICE_COMMON_NS::IService *&ILogicSys::GetCurrentService()
{
    DEF_STATIC_THREAD_LOCAL_DECLEAR SERVICE_COMMON_NS::IService *s_currentService = NULL;

    return s_currentService;
}

KERNEL_NS::LibString ILogicSys::ToString() const 
{
    return KERNEL_NS::CompHostObject::ToString();
}

void ILogicSys::OnRegisterComps()
{

}

Int32 ILogicSys::OnLoaded(const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db)
{
    g_Log->Warn(LOGFMT_OBJ_TAG("need override OnLoaded interface obj:%s, when loaded storage data, storage info:%s"), GetObjName().c_str(), _storage ? _storage->ToString().c_str() : "NO STORAGE INFO");
    return Status::Failed;
}

Int32 ILogicSys::OnLoaded(UInt64 key, const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db)
{
    g_Log->Warn(LOGFMT_OBJ_TAG("need override OnLoaded interface obj:%s, when loaded storage data, storage info:%s"), GetObjName().c_str(), _storage ? _storage->ToString().c_str() : "NO STORAGE INFO");
    return Status::Failed;
}

Int32 ILogicSys::OnLoaded(UInt64 key, const std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> &fieldRefdb)
{
    g_Log->Warn(LOGFMT_OBJ_TAG("need override OnLoaded interface obj:%s, when loaded storage data, storage info:%s"), GetObjName().c_str(), _storage ? _storage->ToString().c_str() : "NO STORAGE INFO");
    return Status::Failed;
}

Int32 ILogicSys::OnLoaded(const KERNEL_NS::LibString &key, const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db)
{
    g_Log->Warn(LOGFMT_OBJ_TAG("need override OnLoaded interface obj:%s, when loaded storage data, storage info:%s"), GetObjName().c_str(), _storage ? _storage->ToString().c_str() : "NO STORAGE INFO");
    return Status::Failed;
}

Int32 ILogicSys::OnLoaded(const KERNEL_NS::LibString &key, const std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> &fieldRefdb)
{
    g_Log->Warn(LOGFMT_OBJ_TAG("need override OnLoaded interface obj:%s, when loaded storage data, storage info:%s"), GetObjName().c_str(), _storage ? _storage->ToString().c_str() : "NO STORAGE INFO");
    return Status::Failed;
}

Int32 ILogicSys::OnSave(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) const
{
    g_Log->Warn(LOGFMT_OBJ_TAG("need override OnSave interface obj:%s, when save storage data, storage info:%s"), GetObjName().c_str(), _storage ? _storage->ToString().c_str() : "NO STORAGE INFO");
    return Status::Failed;
}

Int32 ILogicSys::OnSave(UInt64 key, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) const
{
    g_Log->Warn(LOGFMT_OBJ_TAG("need override OnSave interface obj:%s, when save storage data, storage info:%s"), GetObjName().c_str(), _storage ? _storage->ToString().c_str() : "NO STORAGE INFO");
    return Status::Failed;
}

Int32 ILogicSys::OnSave(UInt64 key, std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> &fieldRefdb) const
{
    g_Log->Warn(LOGFMT_OBJ_TAG("need override OnSave interface obj:%s, when save storage data, storage info:%s"), GetObjName().c_str(), _storage ? _storage->ToString().c_str() : "NO STORAGE INFO");
    return Status::Failed;
}

Int32 ILogicSys::OnSave(const KERNEL_NS::LibString &key, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) const
{
    g_Log->Warn(LOGFMT_OBJ_TAG("need override OnSave interface obj:%s, when save storage data, storage info:%s"), GetObjName().c_str(), _storage ? _storage->ToString().c_str() : "NO STORAGE INFO");
    return Status::Failed;
}

Int32 ILogicSys::OnSave(const KERNEL_NS::LibString &key, std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> &fieldRefdb) const
{
    g_Log->Warn(LOGFMT_OBJ_TAG("need override OnSave interface obj:%s, when save storage data, storage info:%s"), GetObjName().c_str(), _storage ? _storage->ToString().c_str() : "NO STORAGE INFO");
    return Status::Failed;
}

Int32 ILogicSys::_OnHostCreated()
{
    return Status::Success;
}

Int32 ILogicSys::_OnHostInit()
{
    _service = GetCurrentService();
    _serviceProxy = _service->GetServiceProxy();
    _app = _service->GetApp();
    _timerMgr = _service->GetTimerMgr();
    _eventMgr = GetService()->GetEventMgr();

    FocusMethod(LogicInterestMethod::ON_PASS_DAY);

    // 向service注册关注的组件,在系统关闭的时候可以正常check退出
    _service->RegisterFocusServiceModule(this);
    
    auto st = _OnSysInit();
    if(st != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("global sys init fail st:%d, %s"), st, GetObjName().c_str());
        return st;
    }

    // 注册事件
    _RegisterLogicEvents();  

    return Status::Success;
}

Int32 ILogicSys::_OnCompsCreated()
{ 
    _storage = GetCompByType(ServiceCompType::STORAGE_COMP)->CastTo<IStorageInfo>();

    auto st = _OnSysCompsCreated();
    if(st != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("_OnSysCompsCreated fail st:%d, %s"), st, GetObjName().c_str());
        return st;
    }

    return Status::Success; 
}

void ILogicSys::_OnHostClose()
{
    _OnSysClose();

    _Clear();
}

void ILogicSys::_OnQuitServiceEventDefault(KERNEL_NS::LibEvent *ev)
{
    GetService()->MaskServiceModuleQuitFlag(this);
    g_Log->Info(LOGFMT_OBJ_TAG("use default quit service handle to quit logic sys:%s"), GetObjName().c_str());
}

void ILogicSys::_RegisterLogicEvents()
{
    if(!_eventMgr->IsListening(EventEnums::QUIT_SERVICE_EVENT, this))
    {
        if(_quitServiceEventDefaltStub == INVALID_LISTENER_STUB)
            _quitServiceEventDefaltStub = _eventMgr->AddListener(EventEnums::QUIT_SERVICE_EVENT, this, &ILogicSys::_OnQuitServiceEventDefault);
    }
}

void ILogicSys::_UnRegisterLogicEvents()
{
    if(_quitServiceEventDefaltStub != INVALID_LISTENER_STUB)
        _eventMgr->RemoveListenerX(_quitServiceEventDefaltStub);
}

void ILogicSys::_Clear()
{
    _UnRegisterLogicEvents();
}

SERVICE_END
