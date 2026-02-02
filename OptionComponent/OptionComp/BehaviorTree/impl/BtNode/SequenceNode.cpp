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
 * Date: 2023-09-28 00:57:00
 * Author: Eric Yonng
 * Description:
*/


#include <pch.h>
#include <OptionComp/BehaviorTree/impl/BtNode/SequenceNode.h>


KERNEL_BEGIN


SequenceNode::SequenceNode(IBtNode *parent)
:IBtNode(BtNodeType::SEQUENCE, parent)
{

}

SequenceNode::~SequenceNode()
{

}

void SequenceNode::_OnTick(const KERNEL_NS::LibTime &nowTime)
{
    // 顺序执行每个节点, 执行的节点状态是running则暂停执行, 如果是成功则继续执行下个节点, 如果是失败则停止执行后续节点, 并返回FAILURE
    const Int32 count = static_cast<Int32>(_suns.size());
    Int32 successCount = 0;
    for(Int32 idx = 0; idx < count; ++idx)
    {
        auto node = _suns[idx];
        if(UNLIKELY(node->IsSuccess()))
        {
            ++successCount;
            continue;
        }

        node->Tick(nowTime);
        if(node->IsFailure())
        {
            SetState(BtNodeState::FAILURE);
            return;
        }

        if(node->IsRunning())
            break;

        if(LIKELY(node->IsSuccess()))
            ++successCount;
    }

    if(successCount == count)
        SetState(BtNodeState::SUCCESS);
}

KERNEL_END