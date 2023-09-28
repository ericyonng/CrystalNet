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
 * Date: 2023-09-26 14:17:00
 * Author: Eric Yonng
 * Description:
*/

#include <pch.h>
#include <OptionComp/BehaviorTree/impl/IBtNode.h>
#include <OptionComp/BehaviorTree/impl/IBt.h>

KERNEL_BEGIN
POOL_CREATE_OBJ_DEFAULT_IMPL(IBtNode);

IBtNode::IBtNode(Int32 nodeType, IBtNode *parent)
:_isInited(false)
,_isClosed(false)
,_bt(NULL)
,_parent(parent)
{
    _nodeData._type = nodeType;
}

IBtNode::~IBtNode()
{
    _Clear();
}

Int32 IBtNode::Init()
{
    if(UNLIKELY(_isInited))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("repeated init bt node id:%llu"), _nodeData._id);
        return Status::Repeat;
    }

    _isClosed = false;
    auto err = _OnInit();
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("init fail node id:%llu"), _nodeData._id);
        return err;
    }

    const Int32 count = static_cast<Int32>(_suns.size());
    for(Int32 idx = 0; idx < count; ++idx)
    {
        auto node = _suns[idx];
        err = node->Init();
        if(err != Status::Success)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("bt id:%llu, sun node init fail sun node:%s, current node:%s")
            , _bt->GetBtId(), node->ToString().c_str(), ToString().c_str());

            return err;
        }
    }

    _isInited = true;
    SetState(BtNodeState::INITED);

    g_Log->Info(LOGFMT_OBJ_TAG("bt node inited:%s"), ToString().c_str());
    return Status::Success;
}

void IBtNode::Close()
{
    if(UNLIKELY(!_isInited))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("not init before bt node id:%llu"), _nodeData._id);
        return;
    }

    if(UNLIKELY(_isClosed))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("closed before bt node id:%llu"), _nodeData._id);
        return;
    }

    for(auto sun : _suns)
        sun->Close();

    _isClosed = true;
    _isInited = false;

    g_Log->Info(LOGFMT_OBJ_TAG("bt node closed:%s"), ToString().c_str());
}

void IBtNode::Tick(const KERNEL_NS::LibTime &nowTime)
{
    if(UNLIKELY(GetState() <= BtNodeState::INITED))
        SetState(BtNodeState::RUNNING);

    if(UNLIKELY(IsFinished()))
        return;

    // tick
    const auto state = GetState();
    _OnTick(nowTime);

    // 状态切换后执行
    if((state != BtNodeState::SUCCESS) && IsSuccess())
        _OnSuccess();
    else if((state != BtNodeState::FAILURE) && IsFailure())
        _OnFailure();
}

KERNEL_NS::LibString IBtNode::ToString() const
{
    KERNEL_NS::LibString info;
    info.AppendFormat("bt id:%llu, have parent:%d, parent node id:%llu, suns count:%d, [node id:%llu, type:%d, flags:%llx, state:%d,%s]"
    , _bt->GetBtId(), _parent ? 1 : 0, _parent ? _parent->GetNodeId() : 0, static_cast<Int32>(_suns.size())
    , _nodeData._id, _nodeData._type, _nodeData._flags, _nodeData._state, BtNodeState::GetStr(_nodeData._state));

    return info;
}


void IBtNode::_Clear()
{
    for(auto sun : _suns)
        sun->Release();

    _suns.clear();
}

KERNEL_END
