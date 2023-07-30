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
 * Date: 2023-07-14 20:45:00
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/ThreadTlsMemoryCleanerGroup.h>
#include <kernel/comp/TlsMemoryCleanerComp.h>
#include <kernel/comp/Timer/Timer.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(ThreadTlsMemoryCleanerGroup);

ThreadTlsMemoryCleanerGroup::ThreadTlsMemoryCleanerGroup()
:_timerMgr(NULL)
,_isTimerAttach(false)
,_intervalMs(60 * 1000) // 默认1分钟清理一次
,_isManualStart(false)
{

}

ThreadTlsMemoryCleanerGroup::~ThreadTlsMemoryCleanerGroup()
{
    _Clear();
}

void ThreadTlsMemoryCleanerGroup::Release()
{
    ThreadTlsMemoryCleanerGroup::DeleteByAdapter_ThreadTlsMemoryCleanerGroup(ThreadTlsMemoryCleanerGroupFactory::_buildType.V, this);
}

void ThreadTlsMemoryCleanerGroup::OnRegisterComps()
{
    RegisterComp<TlsMemoryCleanerCompFactory>();
}

void ThreadTlsMemoryCleanerGroup::Drive()
{
    _timerMgr->Drive();
}

Int32 ThreadTlsMemoryCleanerGroup::_OnHostInit()
{
    if(!_timerMgr)
        _timerMgr = TimerMgr::New_TimerMgr();
    
    _timerMgr->Launch(NULL);
    return Status::Success;
}

Int32 ThreadTlsMemoryCleanerGroup::_OnCompsCreated()
{
    auto cleaner = GetComp<TlsMemoryCleanerComp>();
    cleaner->SetIntervalMs(_intervalMs);
    cleaner->SetManualStart(_isManualStart);
    cleaner->SetTimerMgr(_timerMgr);

    return Status::Success;
}

Int32 ThreadTlsMemoryCleanerGroup::_OnHostStart()
{
    return Status::Success;
}

void ThreadTlsMemoryCleanerGroup::_OnHostClose()
{
    _Clear();
}

void ThreadTlsMemoryCleanerGroup::_Clear()
{
    if(_timerMgr)
    {
        if(!_isTimerAttach)
            TimerMgr::Delete_TimerMgr(_timerMgr);

        _timerMgr = NULL;
    }

    _isTimerAttach = false;
}

KERNEL_NS::CompFactory *ThreadTlsMemoryCleanerGroupFactory::FactoryCreate()
{
    return KERNEL_NS::ObjPoolWrap<ThreadTlsMemoryCleanerGroupFactory>::NewByAdapter(_buildType.V);
}

void ThreadTlsMemoryCleanerGroupFactory::Release()
{
    KERNEL_NS::ObjPoolWrap<ThreadTlsMemoryCleanerGroupFactory>::DeleteByAdapter(_buildType.V, this);
}
    
KERNEL_NS::CompObject *ThreadTlsMemoryCleanerGroupFactory::Create() const
{
    return ThreadTlsMemoryCleanerGroup::NewByAdapter_ThreadTlsMemoryCleanerGroup(_buildType.V);
}

KERNEL_NS::CompObject *ThreadTlsMemoryCleanerGroupFactory::StaticCreate()
{
    return ThreadTlsMemoryCleanerGroup::NewByAdapter_ThreadTlsMemoryCleanerGroup(_buildType.V);
}

ThreadTlsMemoryCleanerGroup *ThreadTlsMemoryCleanerGroupFactory::StaticCreateAs()
{
    return ThreadTlsMemoryCleanerGroup::NewByAdapter_ThreadTlsMemoryCleanerGroup(_buildType.V);
}

KERNEL_END
