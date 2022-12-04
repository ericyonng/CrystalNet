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
 * Date: 2022-08-28 23:18:31
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include "TestConcurrentPriorityQueue.h"

static KERNEL_NS::ConcurrentPriorityQueue<KERNEL_NS::LibString *> *g_queue = NULL;
static const Int32 g_maxLevel = 4;
static void Thread1(KERNEL_NS::LibThread *t, KERNEL_NS::Variant *var)
{
    Int32 idx = var->AsInt32();
    while (!t->IsDestroy())
    {
        KERNEL_NS::SystemUtil::ThreadSleep(500);
        g_queue->PushQueue(idx, &((new KERNEL_NS::LibString())->AppendFormat("hello idx:%d", idx)));
    }
}

static void Thread2(KERNEL_NS::LibThread *t, KERNEL_NS::Variant *var)
{
    Int32 idx = var->AsInt32();
    while (!t->IsDestroy())
    {
        KERNEL_NS::SystemUtil::ThreadSleep(500);
        g_queue->PushQueue(idx, &((new KERNEL_NS::LibString())->AppendFormat("hello idx:%d", idx)));
    }
}

static void Thread3(KERNEL_NS::LibThread *t)
{
    std::vector<KERNEL_NS::LibList<KERNEL_NS::LibString *, KERNEL_NS::_Build::MT> *> cache;
    for(Int32 idx = 0; idx <= g_maxLevel; ++idx)
        cache.push_back(KERNEL_NS::LibList<KERNEL_NS::LibString *, KERNEL_NS::_Build::MT>::New_LibList());

    while (!t->IsDestroy())
    {
        g_queue->SwapAll(cache);

        for(auto list:cache)
        {
            if(!list || list->IsEmpty())
                continue;

            for(auto node = list->Begin(); node;)
            {
                auto data = node->_data;

                g_Log->Info(LOGFMT_NON_OBJ_TAG(TestConcurrentPriorityQueue, "recv a message :%s"), data->c_str());
                CRYSTAL_DELETE(data);
                node = list->Erase(node);
            }
        }
    }
}

void TestConcurrentPriorityQueue::Run()
{
    KERNEL_NS::LibThread *t1, *t2, *t3;
    t1 = new KERNEL_NS::LibThread;
    t2 = new KERNEL_NS::LibThread;
    t3 = new KERNEL_NS::LibThread;
    g_queue = KERNEL_NS::ConcurrentPriorityQueue<KERNEL_NS::LibString *>::New_ConcurrentPriorityQueue();
    g_queue->SetMaxLevel(g_maxLevel);
    g_queue->Init();

    {
        auto delg1 = KERNEL_NS::DelegateFactory::Create(&Thread1);
        auto var = KERNEL_NS::Variant::New_Variant();
        *var = 1;
        t1->AddTask2(delg1, var);
    }
    {
        auto delg1 = KERNEL_NS::DelegateFactory::Create(&Thread2);
        auto var = KERNEL_NS::Variant::New_Variant();
        *var = 2;
        t2->AddTask2(delg1, var);
    }
    {
        auto delg1 = KERNEL_NS::DelegateFactory::Create(&Thread3);
        t3->AddTask(delg1);
    }

    t1->Start();
    t2->Start();
    t3->Start();

    getchar();

    t1->HalfClose();
    t2->HalfClose();
    t3->HalfClose();

    t1->FinishClose();
    t2->FinishClose();
    t3->FinishClose();

    g_queue->Destroy();
    KERNEL_NS::ConcurrentPriorityQueue<KERNEL_NS::LibString *>::Delete_ConcurrentPriorityQueue(g_queue);

    delete t1;
    delete t2;
    delete t3;
}