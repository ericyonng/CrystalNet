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

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(ILogicSys);


ILogicSys::ILogicSys()
:_service(NULL)
,_serviceProxy(NULL)
,_app(NULL)
,_timerMgr(NULL)
,_eventMgr(NULL)
{

}

ILogicSys::~ILogicSys()
{

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

void ILogicSys::OnLoaded(const KERNEL_NS::LibString &db)
{
    // TODO:
}

void ILogicSys::OnStorage(KERNEL_NS::LibString &db)
{
    // TODO:
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

    FocusMethod(LogicInterestMethod::ON_PASS_DAY);
    
    auto st = _OnSysInit();
    if(st != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("global sys init fail st:%d, %s"), st, GetObjName().c_str());
        return st;
    }

    return Status::Success;
}

void ILogicSys::_OnHostClose()
{
    _OnSysClose();
}


SERVICE_END
