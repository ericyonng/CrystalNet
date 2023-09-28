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

#include <pch.h>
#include <OptionComp/BehaviorTree/impl/BtNode/SelectNode.h>


KERNEL_BEGIN
POOL_CREATE_OBJ_DEFAULT_IMPL(SelectNode);

SelectNode::SelectNode(IBtNode *parent)
:IBtNode(BtNodeType::SELECT, parent)
{

}

SelectNode::~SelectNode()
{

}

void SelectNode::_OnTick(const KERNEL_NS::LibTime &nowTime)
{
    const Int32 count = static_cast<Int32>(_suns.size());
    Int32 failureCount = 0;
    for(Int32 idx = 0; idx < count; ++idx)
    {
        auto node = _suns[idx];
        if(LIKELY(!node->IsFinished()))
        {
            node->Tick(nowTime);
            if(node->IsSuccess())
            {
                SetState(BtNodeState::SUCCESS);
                return;
            }
        }
        
        if(node->IsFailure())
            ++failureCount;
    }

    if(UNLIKELY(failureCount == count))
    {
        SetState(BtNodeState::FAILURE);
        return;
    }
}

KERNEL_END