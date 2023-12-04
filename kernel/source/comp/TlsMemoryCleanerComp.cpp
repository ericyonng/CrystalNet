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
 * Date: 2023-07-02 21:08:0
 * Author: Eric Yonng
 * Description: 
*/
#include <pch.h>
#include <kernel/comp/TlsMemoryCleanerComp.h>
#include <kernel/comp/Timer/Timer.h>
#include <kernel/comp/Log/log.h>
#include <kernel/comp/Tls/TlsDefaultObj.h>
#include <kernel/comp/Utils/TlsUtil.h>
#include <kernel/comp/Utils/SystemUtil.h>
#include <kernel/comp/Lock/Impl/SpinLock.h>
#include <kernel/comp/memory/ObjPoolWrap.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(TlsMemoryCleanerComp);

TlsMemoryCleanerComp::TlsMemoryCleanerComp()
:_timerMgr(NULL)
,_intervalMs(60 * 1000) // 默认1分钟清理一次
,_timer(NULL)
,_tlsDefaultObj(NULL)
,_isManualStart(false)
{

}

TlsMemoryCleanerComp::~TlsMemoryCleanerComp()
{
    _Clear();
}

void TlsMemoryCleanerComp::Release()
{
    TlsMemoryCleanerComp::DeleteByAdapter_TlsMemoryCleanerComp(TlsMemoryCleanerCompFactory::_buildType.V, this);
}

Int32 TlsMemoryCleanerComp::ManualStart()
{
    if(!_isManualStart)
    {
        if(g_Log->IsEnable(LogLevel::Warn))
            g_Log->Info(LOGFMT_OBJ_TAG("tls memory cleaner not in manual mode please check."));

        return Status::Failed;
    }

    return _DoStart();
}

void TlsMemoryCleanerComp::ManualClose()
{
    if(!_isManualStart)
    {
        if(g_Log->IsEnable(LogLevel::Warn))
            g_Log->Info(LOGFMT_OBJ_TAG("tls memory cleaner not in manual mode please check."));

        return;
    }

    // 关闭前也清理下内存
    _PurgeTlsMemory();

    _Clear();

    g_Log->Info(LOGFMT_OBJ_TAG("tls memory cleaner comp will ManualClose success thread id:%llu."), SystemUtil::GetCurrentThreadId());
}

Int32 TlsMemoryCleanerComp::_OnInit()
{
    if(_isManualStart)
    {
        if(g_Log->IsEnable(LogLevel::Warn))
            g_Log->Info(LOGFMT_OBJ_TAG("tls memory cleaner switch manual mode start."));

        return Status::Success;
    }

    return Status::Success;
}

Int32 TlsMemoryCleanerComp::_OnStart()
{
    if(_isManualStart)
    {
        if(g_Log->IsEnable(LogLevel::Warn))
            g_Log->Info(LOGFMT_OBJ_TAG("tls memory cleaner switch manual mode start."));

        return Status::Success;
    }

    return _DoStart();
}

void TlsMemoryCleanerComp::_OnWillClose()
{
    if(_isManualStart)
    {
        if(g_Log->IsEnable(LogLevel::Warn))
            g_Log->Info(LOGFMT_OBJ_TAG("tls memory cleaner switch manual mode and will do manual close please check."));

        return;
    }

    // 关闭前也清理下内存
    _PurgeTlsMemory();

    _Clear();
    g_Log->Info(LOGFMT_OBJ_TAG("tls memory cleaner comp will close success thread id:%llu."), SystemUtil::GetCurrentThreadId());
}

void TlsMemoryCleanerComp::_Clear()
{
    if(LIKELY(_timer))
    {
        LibTimer::DeleteThreadLocal_LibTimer(_timer);
        _timer = NULL;
    }
}

void TlsMemoryCleanerComp::_OnCleanTimer(LibTimer *timer)
{
    _PurgeTlsMemory();
}

Int32 TlsMemoryCleanerComp::_DoStart()
{
    if(UNLIKELY(!_timerMgr))
    {
        if(g_Log->IsEnable(LogLevel::Warn))
            g_Log->Warn(LOGFMT_OBJ_TAG("need timer mgr please check."));

        return Status::Failed;
    }

    _timer = LibTimer::NewThreadLocal_LibTimer(_timerMgr);
    _timer->SetTimeOutHandler(this, &TlsMemoryCleanerComp::_OnCleanTimer);
    _timer->Schedule(_intervalMs);

    _tlsDefaultObj = TlsUtil::GetDefTls();

    g_Log->Info(LOGFMT_OBJ_TAG("tls memory cleaner comp start success thread id:%llu."), SystemUtil::GetCurrentThreadId());

    return Status::Success;
}

void TlsMemoryCleanerComp::_PurgeTlsMemory()
{
    _tlsDefaultObj->_lck->Lock();
    auto toSwap = _tlsDefaultObj->_durtyList;
    _tlsDefaultObj->_durtyList = _tlsDefaultObj->_durtyListSwap;
    _tlsDefaultObj->_durtyListSwap = toSwap;
    _tlsDefaultObj->_lck->Unlock();

    for(auto iter = toSwap->begin(); iter != toSwap->end();)
    {
        auto node = *iter;
        node->ForceMergeBlocks();
        iter = toSwap->erase(iter);
    }
}

KERNEL_NS::CompFactory *TlsMemoryCleanerCompFactory::FactoryCreate()
{
    return KERNEL_NS::ObjPoolWrap<TlsMemoryCleanerCompFactory>::NewByAdapter(_buildType.V);
}

void TlsMemoryCleanerCompFactory::Release()
{
    KERNEL_NS::ObjPoolWrap<TlsMemoryCleanerCompFactory>::DeleteByAdapter(_buildType.V, this);
}
    
KERNEL_NS::CompObject *TlsMemoryCleanerCompFactory::Create() const
{
    return TlsMemoryCleanerComp::NewByAdapter_TlsMemoryCleanerComp(_buildType.V);
}

KERNEL_NS::CompObject *TlsMemoryCleanerCompFactory::StaticCreate()
{
    return TlsMemoryCleanerComp::NewByAdapter_TlsMemoryCleanerComp(_buildType.V);
}

KERNEL_END
