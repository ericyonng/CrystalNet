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

#include<pch.h>
#include <OptionComp/BehaviorTree/impl/BtNode/LoopNode.h>

KERNEL_BEGIN
POOL_CREATE_OBJ_DEFAULT_IMPL(LoopNode);

LoopNode::LoopNode(IBtNode *parent)
:IBtNode(BtNodeType::LOOP, parent)
,_condition(NULL)
,_body(NULL)
{

}

LoopNode::~LoopNode()
{
    CRYSTAL_RELEASE_SAFE(_condition);
    CRYSTAL_RELEASE_SAFE(_body);
}

void LoopNode::_OnTick(const KERNEL_NS::LibTime &nowTime)
{
    if(UNLIKELY(!_condition))
    {
        SetState(BtNodeState::FAILURE);
        g_Log->Warn(LOGFMT_OBJ_TAG("have no condition tick fail nowTime:%lld, node:%s"), nowTime.GetMilliTimestamp(), ToString().c_str());
        return;
    }

    if(LIKELY(_body))
        _body->Invoke();

    if(UNLIKELY(!_condition->Invoke()))
        SetState(BtNodeState::SUCCESS);
}

KERNEL_END
