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
 * Date: 2023-09-27 20:41:00
 * Author: Eric Yonng
 * Description:
*/

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_BEHAVIOR_TREE_IMPL_BTNODE_LOOP_NODE_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_BEHAVIOR_TREE_IMPL_BTNODE_LOOP_NODE_H__

#pragma once

#include <OptionComp/BehaviorTree/impl/IBtNode.h>
#include <kernel/comp/Delegate/LibDelegate.h>

KERNEL_BEGIN

class LoopNode : public IBtNode
{
    POOL_CREATE_OBJ_DEFAULT_P1(IBtNode, LoopNode);

public:
    LoopNode(IBtNode *parent);
    ~LoopNode();
    void Release();

    static LoopNode *Create(IBtNode *parent);

    // 设置循环跳出条件, 当Condition 为false时候结束Loop
    void SetQuitLoopCondition(IDelegate<bool> *condition);

    // 设置循环体执行代码
    void SetBody(IDelegate<void> *body);
    
protected:
    void _OnTick(const KERNEL_NS::LibTime &nowTime) override;

protected:
    virtual Int32 _OnInit() override { return Status::Success; }
    virtual void _OnClose() override { }
    
    IDelegate<bool> *_condition;
    IDelegate<void> *_body;
};

ALWAYS_INLINE void LoopNode::Release()
{
    LoopNode::DeleteThreadLocal_LoopNode(this);
}

ALWAYS_INLINE LoopNode *LoopNode::Create(IBtNode *parent)
{
    return LoopNode::NewThreadLocal_LoopNode(parent);
}

ALWAYS_INLINE void LoopNode::SetQuitLoopCondition(IDelegate<bool> *condition)
{
    CRYSTAL_RELEASE_SAFE(_condition);
    _condition = condition;
}

ALWAYS_INLINE void LoopNode::SetBody(IDelegate<void> *body)
{
    CRYSTAL_RELEASE_SAFE(_body);
    _body = body;
}



KERNEL_END

#endif
