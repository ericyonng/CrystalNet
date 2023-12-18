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
 * Date: 2023-09-27 22:18:00
 * Author: Eric Yonng
 * Description:
*/

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_BEHAVIOR_TREE_IMPL_BTNODE_SELECT_NODE_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_BEHAVIOR_TREE_IMPL_BTNODE_SELECT_NODE_H__

#pragma once

#include <OptionComp/BehaviorTree/impl/IBtNode.h>
#include <kernel/comp/Delegate/LibDelegate.h>

KERNEL_BEGIN

class SelectNode : public IBtNode
{
    POOL_CREATE_OBJ_DEFAULT_P1(IBtNode, SelectNode);

public:
    SelectNode(IBtNode *parent);
    ~SelectNode();
    void Release() override;

    static SelectNode *Create(IBtNode *parent);

protected:
    virtual void _OnTick(const KERNEL_NS::LibTime &nowTime) override;
    virtual Int32 _OnInit() override { return Status::Success; }
    virtual void _OnClose() override { }
};

ALWAYS_INLINE void SelectNode::Release()
{
    SelectNode::DeleteThreadLocal_SelectNode(this);
}

ALWAYS_INLINE SelectNode *SelectNode::Create(IBtNode *parent)
{
    return SelectNode::NewThreadLocal_SelectNode(parent);
}

KERNEL_END

#endif