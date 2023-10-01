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
 * Date: 2023-09-27 13:27:00
 * Author: Eric Yonng
 * Description:
*/

#include <pch.h>
#include <OptionComp/BehaviorTree/impl/IBt.h>
#include <OptionComp/BehaviorTree/impl/IBtNode.h>
#include <kernel/comp/Log/log.h>

KERNEL_BEGIN
POOL_CREATE_OBJ_DEFAULT_IMPL(IBt);

IBt::IBt(UInt64 id)
:_isInited(false)
,_isClosed(false)
,_state(BtState::CREATED)
,_isSuccess(false)
,_isFailure(false)
,_id(id)
,_root(NULL)
,_timer(NULL)
,_interval(TimeSlice::FromSeconds(1))
,_timerMgr(NULL)
{

}

IBt::~IBt()
{
    _Clear();
}

Int32 IBt::Init()
{
    if(UNLIKELY(_isInited))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("repeated init"));
        return Status::Repeat;
    }

    _timer = LibTimer::NewThreadLocal_LibTimer(_timerMgr);
    _timer->SetTimeOutHandler(this, &IBt::_OnTick);

    auto err = _OnInit();
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("init bt fail err:%d"), err);
        return err;
    }

    if(LIKELY(_root))
    {
        if(UNLIKELY(_root->GetNodeId() == 0))
            _root->SetNodeId(1);

        err = _root->Init();
        if(err != Status::Success)
        {
            g_Log->Info(LOGFMT_OBJ_TAG("root init fail root:%s, init bt fail:%s"), _root->ToString().c_str(), ToString().c_str());
            return err;
        }
    }

    _isInited = true;
    _isClosed = false;
    _isSuccess = false;
    _isFailure = false;
    SetState(BtState::INITED);

    _timer->Schedule(_interval);
    g_Log->Info(LOGFMT_OBJ_TAG("init bt success:%s"), ToString().c_str());
    return Status::Success;
}

void IBt::Close()
{
    if(UNLIKELY(!_isInited))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("not inited before"));
        return;
    }

    if(UNLIKELY(_isClosed))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("closed before"));
        return;
    }

    g_Log->Info(LOGFMT_OBJ_TAG("bt will close"));

    _BeforeClose();

    if(LIKELY(_root))
        _root->Close();

    _OnBtClose();

    _isClosed = true;
    _isInited = false;

    g_Log->Info(LOGFMT_OBJ_TAG("bt closed:%s"), ToString().c_str());
}

void IBt::SetRootNode(IBtNode *root)
{
    CRYSTAL_RELEASE_SAFE(_root);
    _root = root;
    if(UNLIKELY(_root->GetNodeId() == 0))
        _root->SetNodeId(1);
    _root->SetBt(this);
}

void IBt::SetState(Int32 state)
{
    _state = state;
}

KERNEL_NS::LibString IBt::ToString() const
{
   KERNEL_NS::LibString info;
   info.AppendFormat("bt id:%llu, state:%d,%s, nowTime:%lld, interval ms:%lld"
    , _id, _state, BtState::GetStr(_state), _nowTime.GetMilliTimestamp(), _interval.GetTotalMilliSeconds());

   return info;
}

void IBt::_OnTick(LibTimer *t)
{
    if(UNLIKELY(GetState() <= BtState::INITED))
        SetState(BtState::RUNNING);

    _nowTime.UpdateTime();

    if(LIKELY(_root))
    {
        _root->Tick(_nowTime);

        if(_root->IsFinished())
        {
            SetState(BtState::FINISHED);

            if(_root->IsSuccess())
                _isSuccess = true;
            else
                _isFailure = true;
        }
    }

    if(UNLIKELY(GetState() == BtState::FINISHED))
    {
        _timer->Cancel();

        _OnFinished();
    }
}

void IBt::_Clear()
{
    CRYSTAL_RELEASE_SAFE(_root);

    if(LIKELY(_timer))
    {
        LibTimer::DeleteThreadLocal_LibTimer(_timer);
        _timer = NULL;
    }
}

KERNEL_END
