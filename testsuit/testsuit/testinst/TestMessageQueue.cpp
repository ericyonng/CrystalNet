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
 * Date: 2021-03-13 23:12:40
 * Author: Eric Yonng
 * Description: 
 *             1.8生产者,1消费者 1min测试：中位数:1035948 pps, 平均值: 1048130 pps, 总包数: 62865186
 *             2.8生产者,8消费者 1min测试: 
 *             3.linux 单线程生产极限30w/s
*/

#include <pch.h>
#include <testsuit/testinst/TestMessageQueue.h>

#define TEST_MQ_MAX_CHANNEL 4
#define TEST_MQ_CONSUMER_COUNT 1

static std::atomic<UInt64> g_curMsgCount{0};
static std::atomic<UInt64> g_genTotalMsgCount{ 0 };
static std::atomic<UInt64> g_curGenTotalMsgCount{ 0 };
static std::atomic<UInt64> g_consumerTotalMsgCount{ 0 };

class TestMqBlock : public KERNEL_NS::MessageBlock
{
    POOL_CREATE_OBJ_DEFAULT_P1(MessageBlock, TestMqBlock);

public:
    TestMqBlock(){}
    virtual ~TestMqBlock(){}

    virtual void Release()
    {
        // CRYSTAL_DELETE(this);
        TestMqBlock::Delete_TestMqBlock(this);
    }

public:
    KERNEL_NS::LibString _data;

};

class TestMq
{
public:
    TestMq(UInt64 id)
    {
        _id = id;
        _queue = new KERNEL_NS::MessageQueue;
    }

    void Run(KERNEL_NS::LibThread *pool)
    {
        KERNEL_NS::LibTime lastTime = KERNEL_NS::LibTime::Now();

        g_Log->Info(LOGFMT_OBJ_TAG("start mq id[%llu], thread state[%s] "), _id, pool->ToString().c_str());
        
        // std::vector<Int64> *pushcost = new std::vector<Int64>;
        // std::vector<Int64> *sinalcost = new std::vector<Int64>;
        while (!pool->IsDestroy())
        {
            auto newBlock = TestMqBlock::New_TestMqBlock();
            // newBlock->_data.AppendFormat("hello test mq thread id[%llu] id[%llu]"
            // , threadId, _id);

            ++g_genTotalMsgCount;
            ++g_curGenTotalMsgCount;

            // lastTime1 = KERNEL_NS::TimeUtil::GetMicroTimestamp();
            _queue->Push(newBlock);
            // lastTime2 = KERNEL_NS::TimeUtil::GetMicroTimestamp();

            // 性能消耗大, 每1min导致延迟11秒左右 若没有sinal可提示一倍的性能
            _queue->Sinal();
            // lastTime3 = KERNEL_NS::TimeUtil::GetMicroTimestamp();

            // pushcost->push_back(lastTime2-lastTime1);
            // sinalcost->push_back(lastTime3-lastTime2);
        }

        // std::sort(pushcost->begin(), pushcost->end(), [](Int64 l, Int64 r) ->bool 
        // {
        //     return l < r;
        // });
        // std::sort(sinalcost->begin(), sinalcost->end(), [](Int64 l, Int64 r) ->bool 
        // {
        //     return l < r;
        // });

        // {
        //     Int64 count = static_cast<Int64>(pushcost->size());
        //     Int64 total = 0;
        //     for(auto v:*pushcost)
        //         total += v;
        //     Int64 average = total/count;
        //     Int64 mid = static_cast<Int64>(pushcost->at(count/2));
        //     g_Log->Info(LOGFMT_OBJ_TAG("push op count[%lld] total[%lld] average[%lld] mic sec, mid[%lld] mic sec"), count, total, average, mid);
        // }

        // {
        //     Int64 count = static_cast<Int64>(sinalcost->size());
        //     Int64 total = 0;
        //     for(auto v:*sinalcost)
        //         total += v;
        //     Int64 average= total/ count;
        //     Int64 mid = static_cast<Int64>(sinalcost->at(count/2));
        //     g_Log->Info(LOGFMT_OBJ_TAG("sinalcost op count[%lld] total[%lld] average[%lld] mic sec, mid[%lld] mic sec"), count, total, average, mid);
        // }


        // 唤醒继续消费
        if(_queue->HasMsg())
            _queue->Sinal();
        g_Log->Info(LOGFMT_OBJ_TAG("thread quit id[%llu]"), _id);
    }

    KERNEL_NS::MessageQueue *GetQueue()
    {
        return _queue.AsSelf();
    }

private:
    UInt64 _id;
    KERNEL_NS::SmartPtr<KERNEL_NS::MessageQueue, KERNEL_NS::_Build::MT> _queue;
};


class TestConsumer
{
public:
    TestConsumer(UInt64 id)
    {
        _id = id;
    }

    void Attach(KERNEL_NS::MessageQueue *queue, UInt64 genId)
    {
        _queue.push_back(queue);
        _swapMsgList.resize(_queue.size());
        _attachGenIds.push_back(genId);
    }
    
    void Run(KERNEL_NS::LibThread *pool)
    {
        const UInt64 queueSize = static_cast<UInt64>(_queue.size());
        for (UInt64 idx = 0; idx < queueSize; ++idx)
        {
            _swapMsgList[idx] = CRYSTAL_NEW(std::list<KERNEL_NS::MessageBlock *>);
        }

        g_Log->Info(LOGFMT_OBJ_TAG("start TestConsumer id[%llu], thread state[%s] "), _id, pool->ToString().c_str());
        KERNEL_NS::LibTime lastTime = KERNEL_NS::LibTime::Now();
        while ( !pool->IsDestroy())
        {
            _lck.Lock();
            _lck.Wait();
            _lck.Unlock();

            for (UInt64 idx = 0; idx < queueSize; ++idx)
            {
                auto queue = _queue[idx];
                if (!queue->HasMsg())
                    continue;

                auto &swapMsgList = _swapMsgList[idx];
                queue->PopImmediately(swapMsgList);
                for (auto iterList = swapMsgList->begin(); iterList != swapMsgList->end();)
                {
                    auto msg = (*iterList)->Cast<TestMqBlock>();
                    ++g_curMsgCount;
                    ++g_consumerTotalMsgCount;
                    msg->Release();
                    iterList = swapMsgList->erase(iterList);
                }
            }
        }

        UInt64 leftMsg = 0;
        for (auto &swapMsg : _swapMsgList)
            leftMsg += swapMsg->size();

        for(auto &queue:_queue)
            leftMsg += queue->GetMsgCount();
        
        KERNEL_NS::LibString genString = "attach genids:";
        for (auto genId : _attachGenIds)
            genString.AppendFormat("%llu,", genId);
        
        g_Log->Info(LOGFMT_OBJ_TAG("%s swapListCount[%llu] left msg count[%llu], consumer id[%llu]")
            , genString.c_str(), static_cast<UInt64>(_swapMsgList.size()), leftMsg, _id);

        for (auto &swapMsg : _swapMsgList)
        {
            KERNEL_NS::ContainerUtil::DelContainer<KERNEL_NS::MessageBlock *
                , KERNEL_NS::AutoDelMethods::Release>(*swapMsg);
            CRYSTAL_DELETE_SAFE(swapMsg);
        }
        _swapMsgList.clear();
    }

    KERNEL_NS::ConditionLocker *GetLck()
    {
        return &_lck;
    }

private:
    KERNEL_NS::ConditionLocker _lck;
    UInt64 _id;
    std::vector<KERNEL_NS::MessageQueue *> _queue;
    std::vector<std::list<KERNEL_NS::MessageBlock *> *> _swapMsgList;
    std::vector<UInt64> _attachGenIds;
};

void TestMessageQueue::Run() 
{
    // 初始化
    std::vector<KERNEL_NS::LibThread *> consumerThreads;
    consumerThreads.resize(TEST_MQ_CONSUMER_COUNT);
    std::vector<KERNEL_NS::LibThread *> genThreads;
    genThreads.resize(TEST_MQ_MAX_CHANNEL);

    std::vector<TestConsumer *> vecConsumer;
    vecConsumer.resize(TEST_MQ_CONSUMER_COUNT);
    for (auto idx = 0; idx < TEST_MQ_CONSUMER_COUNT; ++idx)
    {
        vecConsumer[idx] = new TestConsumer(idx);
        consumerThreads[idx] = new KERNEL_NS::LibThread;
        consumerThreads[idx]->AddTask(vecConsumer[idx], &TestConsumer::Run);
    }

    std::vector<TestMq *> vecMq;
    vecMq.resize(TEST_MQ_MAX_CHANNEL);
    for(UInt64 idx = 0; idx < TEST_MQ_MAX_CHANNEL; ++idx)
    {
        UInt64 consumerId = idx % TEST_MQ_CONSUMER_COUNT;

        vecMq[idx] = new TestMq(idx);
        // 消费者绑定消息队列
        vecConsumer[consumerId]->Attach(vecMq[idx]->GetQueue(), idx);
        // 消息队列绑定消费者
        vecMq[idx]->GetQueue()->Attach(vecConsumer[consumerId]->GetLck());
        genThreads[idx] = new KERNEL_NS::LibThread;
        genThreads[idx]->AddTask(vecMq[idx], &TestMq::Run);
    }

    // 启动消息队列
    for (UInt64 idx = 0; idx < TEST_MQ_MAX_CHANNEL; ++idx)
        vecMq[idx]->GetQueue()->Start();

    for(auto &t:consumerThreads)
        t->Start();

    for(auto &t:genThreads)
        t->Start();

    // 1min
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMessageQueue, "start calclate 1min."));
    Int64 seconds = 60;
    std::vector<UInt64> *msgCountsPerSecond = new std::vector<UInt64>;
    std::vector<UInt64> *genMsgCountsPerSecond = new std::vector<UInt64>;
    while (seconds > 0)
    {
        KERNEL_NS::SystemUtil::ThreadSleep(1000);
        UInt64 curCount = g_curMsgCount.exchange(0);
        UInt64 curGen = g_curGenTotalMsgCount.exchange(0);
        msgCountsPerSecond->push_back(curCount);
        genMsgCountsPerSecond->push_back(curGen);
        --seconds;
    }

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMessageQueue, "will quit."));
    for(auto &t:genThreads)
        t->HalfClose();
    
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMessageQueue, "after gen threads half close."));
    for(auto &t:genThreads)
        t->FinishClose();

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMessageQueue, "after gen threads finish close."));
    for(auto &t:consumerThreads)
        t->HalfClose();

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMessageQueue, "after consumer threads half close."));
    for (auto &consumer : vecConsumer)
        consumer->GetLck()->Broadcast();

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMessageQueue, "after consumer threads Broadcast."));
    for(auto &t:consumerThreads)
        t->FinishClose();

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMessageQueue, "after consumer threads FinishClose."));
    
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMessageQueue, "begin calclate midlle and average."));
    // 统计平均值/中位数
    std::sort(msgCountsPerSecond->begin(), msgCountsPerSecond->end(), [](UInt64 l, UInt64 r)->bool{
        
        return l < r;
    });

    // 统计平均值/中位数
    std::sort(genMsgCountsPerSecond->begin(), genMsgCountsPerSecond->end(), [](UInt64 l, UInt64 r)->bool{
        
        return l < r;
    });

    UInt64 consumerMid = 0;
    auto consumerCount = static_cast<UInt64>(msgCountsPerSecond->size());
    if(consumerCount)
        consumerMid = (*msgCountsPerSecond)[consumerCount/2];

    UInt64 genMid = 0;
    auto genCount = static_cast<UInt64>(genMsgCountsPerSecond->size());
    if(genCount)
        genMid = (*genMsgCountsPerSecond)[consumerCount/2];

    UInt64 consumerAvarage = 0;
    if(consumerCount)
    {
        for(auto v:*msgCountsPerSecond)
            consumerAvarage += v;

        consumerAvarage/=consumerCount;
    }

    UInt64 genAvarage = 0;
    if(genCount)
    {
        for(auto v:*genMsgCountsPerSecond)
            genAvarage += v;

        genAvarage/=genCount;
    }

    KERNEL_NS::ContainerUtil::DelContainer(vecMq);
    KERNEL_NS::ContainerUtil::DelContainer(vecConsumer);
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMessageQueue, "consumer quit consumerMid pps[%llu]/s, consumerAvarage pps[%llu]/s, "
    "genMid pps[%llu]/s, genAvarage pps[%llu]/s g_consumerTotalMsgCount[%llu] g_genTotalMsgCount[%llu]")
        , consumerMid, consumerAvarage, genMid, genAvarage, g_consumerTotalMsgCount.load(), g_genTotalMsgCount.load());

}
