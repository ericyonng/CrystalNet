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
 * Date: 2023-09-27 21:52:00
 * Author: Eric Yonng
 * Description:
*/

#include<pch.h>
#include <OptionComp/BehaviorTree/impl/BtNode/ParallelNode.h>

KERNEL_BEGIN

ParallelNode::ParallelNode(IBtNode *parent)
:IBtNode(BtNodeType::PARALLEL, parent)
{

}

ParallelNode::~ParallelNode()
{

}

void ParallelNode::_OnTick(const KERNEL_NS::LibTime &nowTime)
{
    const Int32 count = static_cast<Int32>(_suns.size());
    bool isFinished = true;
    for(Int32 idx = 0; idx < count; ++idx)
    {
        auto node = _suns[idx];
        if(UNLIKELY(node->IsFinished()))
            continue;

        node->Tick(nowTime);

        if(node->IsFinished())
            continue;

        isFinished = false;        
    }

    if(isFinished)
        SetState(BtNodeState::SUCCESS);
}


KERNEL_END
