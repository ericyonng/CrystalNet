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
 * Date: 2022-09-28 11:26:13
 * Author: Eric Yonng
 * Description: 
*/
#include <pch.h>
#include <testsuit/testinst/TestBt.h>
#include <OptionComp/BehaviorTree/BehaviorTree.h>

class TestBtType
{
public:
    enum ENUMS
    {
        BEGIN = KERNEL_NS::BtNodeType::MAX_TYPE,
        KILL_NODE,
        COUNT_NODE,
    };
};

class TestBehaviorTree : public KERNEL_NS::IBt
{
    POOL_CREATE_OBJ_DEFAULT_P1(IBt, TestBehaviorTree);

public:
    TestBehaviorTree(UInt64 id = 1)
    :IBt(id)
    {
        
    }

    ~TestBehaviorTree()
    {

    }

    void Release()
    {
        TestBehaviorTree::DeleteThreadLocal_TestBehaviorTree(this);
    }
};

class KillNode : public KERNEL_NS::IBtNode
{
    POOL_CREATE_OBJ_DEFAULT_P1(IBtNode, KillNode);

public:
    KillNode()
    :KERNEL_NS::IBtNode(TestBtType::KILL_NODE, NULL)
    ,_killCount(0)
    ,_targetCount(0)
    {

    }

    ~KillNode() override
    {

    }

    void Release() override
    {
        KillNode::DeleteThreadLocal_KillNode(this);
    }

    void _OnTick(const KERNEL_NS::LibTime &nowTime) override
    {
        ++_killCount;

        if(_killCount >= _targetCount)
            SetState(KERNEL_NS::BtNodeState::SUCCESS);
    }

    void _OnSuccess() override
    {
        g_Log->Info(LOGFMT_OBJ_TAG("kill node success _killCount:%d, _targetCount:%d node:%s.")
        , _killCount, _targetCount, ToString().c_str());
    }

    void SetTargetCount(Int32 count)
    {
        _targetCount = count;
    }
    
private:
    Int32 _killCount;
    Int32 _targetCount;
};


void TestBt::Run()
{
    KERNEL_NS::SmartPtr<KERNEL_NS::TimerMgr, KERNEL_NS::AutoDelMethods::CustomDelete> timerMgr = KERNEL_NS::TimerMgr::New_TimerMgr();
    timerMgr.SetClosureDelegate([](void *p){
        KERNEL_NS::TimerMgr::Delete_TimerMgr(KERNEL_NS::KernelCastTo<KERNEL_NS::TimerMgr>(p));
    });
    timerMgr->Launch(NULL);

    // 创建bt
    KERNEL_NS::SmartPtr<TestBehaviorTree, KERNEL_NS::AutoDelMethods::CustomDelete> bt = TestBehaviorTree::NewThreadLocal_TestBehaviorTree();
    bt.SetClosureDelegate([](void *p){
        TestBehaviorTree::DeleteThreadLocal_TestBehaviorTree(KERNEL_NS::KernelCastTo<TestBehaviorTree>(p));
    });

    bt->SetTimerMgr(timerMgr);
    bt->SetIntervalMs(1000);

    // 添加根节点
    auto root = KERNEL_NS::ParallelNode::Create(NULL);
    bt->SetRootNode(root);

    // 循环三次打印hello world
    auto loopNode = KERNEL_NS::LoopNode::Create(NULL);
    Int32 loopCount = 3;
    auto &&bodyLam = [&loopCount](){
        g_Log->Custom("hello world loopCount:%d", loopCount);
    };
    auto &&quitLoop = [&loopCount]() -> bool 
    {
        --loopCount;
        return loopCount > 0;
    };
    loopNode->SetBody(KERNEL_CREATE_CLOSURE_DELEGATE(bodyLam, void));
    loopNode->SetQuitLoopCondition(KERNEL_CREATE_CLOSURE_DELEGATE(quitLoop, bool));
    root->AddNode(loopNode);

    // 顺序节点
    auto sequenceNode = KERNEL_NS::SequenceNode::Create(NULL);
    root->AddNode(sequenceNode);
    auto killNode = KillNode::NewThreadLocal_KillNode();
    killNode->SetTargetCount(10);
    sequenceNode->AddNode(killNode);
    killNode = KillNode::NewThreadLocal_KillNode();
    killNode->SetTargetCount(20);
    sequenceNode->AddNode(killNode);

    // 选择节点
    auto selector = KERNEL_NS::SelectNode::Create(NULL);
    root->AddNode(selector);
    auto newLoop = KERNEL_NS::LoopNode::Create(NULL);
    loopCount = 5;
    newLoop->SetBody(KERNEL_CREATE_CLOSURE_DELEGATE(bodyLam, void));
    newLoop->SetQuitLoopCondition(KERNEL_CREATE_CLOSURE_DELEGATE(quitLoop, bool));
    killNode = KillNode::NewThreadLocal_KillNode();
    killNode->SetTargetCount(15);
    selector->AddNode(killNode);
    selector->AddNode(newLoop);

    auto err = bt->Init();
    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(TestBt, "bt init fail bt:%s"), bt->ToString().c_str());
        return;
    }

    Int32 looped = 0;
    while (true)
    {
        KERNEL_NS::SystemUtil::ThreadSleep(1000);
        timerMgr->Drive();

        if(++looped > 60)
            break;
    }

    g_Log->Custom("bt test ending.");
    getchar();

    bt->Close();
}