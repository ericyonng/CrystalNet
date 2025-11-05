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
 * Date: 2022-03-23 15:00:00
 * Author: Eric Yonng
 * Description: 
*/
#include <pch.h>
#include <kernel/comp/Poller/PollerEvent.h>
#include <kernel/comp/Utils/RttiUtil.h>
#include <kernel/comp/Coroutines/AsyncTask.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(PollerEvent);
POOL_CREATE_OBJ_DEFAULT_IMPL(ActionPollerEvent);
POOL_CREATE_OBJ_DEFAULT_IMPL(EmptyPollerEvent);
POOL_CREATE_OBJ_DEFAULT_IMPL(AsyncTaskPollerEvent);

PollerEvent::PollerEvent(Int32 type)
    :_type(type)
{

}

PollerEvent::~PollerEvent()
{
    
}

LibString PollerEvent::ToString() const
{
    return LibString().AppendFormat("poller event type:%d, obj name:%s", _type, RttiUtil::GetByObj(this).c_str());
}

ActionPollerEvent::ActionPollerEvent()
:PollerEvent(PollerEventInternalType::ActionPollerEventType)
,_action(NULL)
{

}

ActionPollerEvent::~ActionPollerEvent()
{
    if(_action)
        _action->Release();

    _action = NULL;
}

void ActionPollerEvent::Release()
{
    ActionPollerEvent::Delete_ActionPollerEvent(this);
}

LibString ActionPollerEvent::ToString() const 
{
    LibString info;
    info.AppendFormat("%s\n", PollerEvent::ToString().c_str());

    return info;
}

EmptyPollerEvent::EmptyPollerEvent()
:PollerEvent(PollerEventInternalType::EmptyPollerEventType)
{

}

AsyncTaskPollerEvent::AsyncTaskPollerEvent()
:PollerEvent(PollerEventInternalType::AsyncTaskType)
{

}

AsyncTaskPollerEvent::~AsyncTaskPollerEvent()
{
    CRYSTAL_RELEASE_SAFE(_asyncTask);
}

void AsyncTaskPollerEvent::Release()
{
    AsyncTaskPollerEvent::Delete_AsyncTaskPollerEvent(this);
}

POOL_CREATE_OBJ_DEFAULT_IMPL(StubPollerEvent);

StubPollerEvent::StubPollerEvent(Int32 type, UInt64 stub, UInt64 objTypeId)
    :PollerEvent(type)
    ,_stub(stub)
    ,_isResponse(false)
,_objTypeId(objTypeId)
{
    
}

StubPollerEvent::~StubPollerEvent()
{
    
}

LibString StubPollerEvent::ToString() const
{
    LibString info;
    info.AppendFormat("%s, stub:%llu, _isResponse:%d, _objTypeId:%llu\n", PollerEvent::ToString().c_str(), _stub, (_isResponse ? 1:0), _objTypeId);

    return info;
}

KERNEL_END
