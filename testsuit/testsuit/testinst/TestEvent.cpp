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
 * Date: 2021-03-21 18:04:34
 * Author: Eric Yonng
 * Description: 
 *              1.测试在IsFiring 状态下AddListener并FireEvent,需要保证Fire也能在异步下触发
 *              2.AddListener后RemoveEvent再触发FireEvent必须保证不会触发
 *              3.FireEvent 之后再listener则listen也在异步队列中
 *              4.FireEvent若在IsFiring状况下addlisten并在统一帧下Fire的话事件将在之后的某个合适时机触发
*/

#include <pch.h>
#include <testsuit/testinst/TestEvent.h>

class EventIds
{
public:

    enum
    {
        TEST_EVENT_TASK1 = 1,
        TEST_EVENT_TASK2 = 2,
        TEST_EVENT_TASK3 = 3,
    };
};

class EventParams
{
public:
    enum
    {
        SESSION_ID = 1,
        NAME = 2,
        EVENT_MGR = 3,
        EVENT_TASK2 = 4,
    };
};

class TestEventTask3
{
public:
    void OnEvent(KERNEL_NS::LibEvent *ev)
    {
        CRYSTAL_TRACE("hell test event task 3");

        g_Log->Custom("test event task3 session id[%lld], name[%s]"
            , ev->GetParam(EventParams::SESSION_ID).AsInt64(), ev->GetParam(EventParams::NAME).AsStr().c_str());

    }
};

class TestEventTask2
{
public:
    void OnEvent(KERNEL_NS::LibEvent *ev)
    {
        CRYSTAL_TRACE("hell test event task 2");

        auto eventMgr = ev->GetParam(EventParams::EVENT_MGR).AsPtr<KERNEL_NS::EventManager>();

        TestEventTask3 task3;
        eventMgr->AddListener(EventIds::TEST_EVENT_TASK3, &task3, &TestEventTask3::OnEvent);
        
        ev = KERNEL_NS::LibEvent::New_LibEvent(EventIds::TEST_EVENT_TASK3);
        ev->SetParam(EventParams::SESSION_ID, 10);
        ev->SetParam(EventParams::NAME, "test event");
        eventMgr->FireEvent(ev);

        g_Log->Custom("test event task2 session id[%lld], name[%s]"
            , ev->GetParam(EventParams::SESSION_ID).AsInt64(), ev->GetParam(EventParams::NAME).AsStr().c_str());

    }
};

class TestEventTask1
{
public:
    void OnEvent(KERNEL_NS::LibEvent *ev)
    {
        CRYSTAL_TRACE("hell test event task 1");

        g_Log->Custom("test event task1 session id[%lld], name[%s]"
        ,ev->GetParam(EventParams::SESSION_ID).AsInt64(), ev->GetParam(EventParams::NAME).AsStr().c_str() );
        
        auto eventMgr = ev->GetParam(EventParams::EVENT_MGR).AsPtr<KERNEL_NS::EventManager>();
        auto task2 = ev->GetParam(EventParams::EVENT_TASK2).AsPtr<TestEventTask2>();

        // task2 TODO:不是好的设计,因为如果对事件顺序有要求的话延迟触发事件会导致逻辑问题,但若不加入延迟队列又会丢失事件,所以这个应该在设计层面规避
        eventMgr->AddListener(EventIds::TEST_EVENT_TASK2, task2, &TestEventTask2::OnEvent);
        ev = KERNEL_NS::LibEvent::New_LibEvent(EventIds::TEST_EVENT_TASK2);
        ev->SetParam(EventParams::SESSION_ID, 10);
        ev->SetParam(EventParams::NAME, "test event");
        ev->SetParam(EventParams::EVENT_MGR, eventMgr);
        eventMgr->FireEvent(ev);
    }
};


void TestEvent::Run() 
{
    TestEventTask1 task1;
    TestEventTask2 task2;
    KERNEL_NS::EventManager eventMgr;

    auto stub1 = eventMgr.AddListener(EventIds::TEST_EVENT_TASK1, &task1, &TestEventTask1::OnEvent);

    // task1
    KERNEL_NS::LibEvent *ev = KERNEL_NS::LibEvent::New_LibEvent(EventIds::TEST_EVENT_TASK1);
    ev->SetParam(EventParams::SESSION_ID, 10);
    ev->SetParam(EventParams::NAME, "test event");
    ev->SetParam(EventParams::EVENT_MGR, &eventMgr);
    ev->SetParam(EventParams::EVENT_TASK2, &task2);
    eventMgr.FireEvent(ev);

    eventMgr.RemoveListenerX(stub1);
}
