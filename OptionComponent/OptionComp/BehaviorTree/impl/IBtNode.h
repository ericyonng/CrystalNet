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

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_BEHAVIOR_TREE_IMPL_IBTNODE_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_BEHAVIOR_TREE_IMPL_IBTNODE_H__

#pragma once

#include <kernel/comp/LibTime.h>
#include <OptionComp/BehaviorTree/impl/BtNodeData.h>

KERNEL_BEGIN

class IBt;

class IBtNode
{
    POOL_CREATE_OBJ_DEFAULT(IBtNode);

public:
    IBtNode(Int32 nodeType, IBtNode *parent);
    virtual ~IBtNode();
    virtual void Release() = 0;

    virtual Int32 Init() final;
    virtual void Close() final;
    virtual void Tick(const KERNEL_NS::LibTime &nowTime) final;

    bool IsFinished() const;
    bool IsStarted() const;
    bool IsRunning() const;
    bool IsSuccess() const;
    bool IsFailure() const;

    void SetBt(IBt *bt);
    const IBt *GetBt() const;
    IBt *GetBt();
    
    void SetParent(IBtNode *parent);
    const IBtNode *GetParent() const;
    IBtNode *GetParent();
    
    void SetNodeId(UInt64 nodeId);
    UInt64 GetNodeId() const;

    bool AddNode(IBtNode *node);

    void SetState(Int32 state);
    Int32 GetState() const;

    const std::vector<IBtNode *> &GetSuns() const;
    std::vector<IBtNode *> &GetSuns();

    virtual KERNEL_NS::LibString ToString() const;

protected:
    virtual void _OnTick(const KERNEL_NS::LibTime &nowTime) = 0;
    virtual Int32 _OnInit() { return Status::Success; }
    virtual void _OnClose() { }

    // 节点状态切换成成功
    virtual void _OnSuccess() {}
    // 节点状态切换成失败
    virtual void _OnFailure() {}

private:
    void _Clear();
    
protected:
    bool _isInited;
    bool _isClosed;

    BtNodeData _nodeData;

    IBt *_bt;
    IBtNode *_parent;
    std::vector<IBtNode *> _suns;
};

ALWAYS_INLINE bool IBtNode::IsFinished() const
{
    return (_nodeData._state == BtNodeState::SUCCESS) || (_nodeData._state == BtNodeState::FAILURE);
}

ALWAYS_INLINE bool IBtNode::IsStarted() const
{
    return _nodeData._state > BtNodeState::INITED;
}

ALWAYS_INLINE bool IBtNode::IsRunning() const
{
    return _nodeData._state == BtNodeState::RUNNING;
}

ALWAYS_INLINE bool IBtNode::IsSuccess() const
{
    return _nodeData._state == BtNodeState::SUCCESS;
}

ALWAYS_INLINE bool IBtNode::IsFailure() const
{
    return _nodeData._state == BtNodeState::FAILURE;
}

ALWAYS_INLINE void IBtNode::SetBt(IBt *bt)
{
    _bt = bt;
}

ALWAYS_INLINE const IBt *IBtNode::GetBt() const
{
    return _bt;
}

ALWAYS_INLINE IBt *IBtNode::GetBt()
{
    return _bt;
}

ALWAYS_INLINE void IBtNode::SetParent(IBtNode *parent)
{
    _parent = parent;
}

ALWAYS_INLINE const IBtNode *IBtNode::GetParent() const
{
    return _parent;
}

ALWAYS_INLINE IBtNode *IBtNode::GetParent()
{
    return _parent;
}

ALWAYS_INLINE void IBtNode::SetNodeId(UInt64 nodeId)
{
    _nodeData._id = nodeId;
}

ALWAYS_INLINE UInt64 IBtNode::GetNodeId() const
{
    return _nodeData._id;
}

ALWAYS_INLINE bool IBtNode::AddNode(IBtNode *node)
{
    if(UNLIKELY(IsFinished()))
        return false;

    node->SetBt(_bt);
    node->SetParent(this);
    _suns.push_back(node);
    node->SetNodeId(static_cast<UInt64>(_suns.size()));

    return true;
}

ALWAYS_INLINE void IBtNode::SetState(Int32 state)
{
    _nodeData._state = state;
}

ALWAYS_INLINE Int32 IBtNode::GetState() const
{
    return _nodeData._state;
}

ALWAYS_INLINE const std::vector<IBtNode *> &IBtNode::GetSuns() const
{
    return _suns;
}

ALWAYS_INLINE std::vector<IBtNode *> &IBtNode::GetSuns()
{
    return _suns;
}

KERNEL_END

#endif
