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
 * Date: 2021-01-02 01:56:05
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestTime.h>

#define TEST_TIME_PERFORMANCE_CNT    10000000
void TestTime::Run()
{
    // 测试TimeUtil
    KERNEL_NS::TimeUtil::SetTimeZone();

    std::cout << "cu time zone = " << KERNEL_NS::TimeUtil::GetTimeZone() << std::endl;

    // 测试time util 各个时间
    std::cout << "test time util clock time stamp" << std::endl;

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS

    struct timespec tp, tp2, tp3, tp4;
    ::syscall(SYS_clock_gettime, CLOCK_MONOTONIC_RAW, &tp);
    ::clock_gettime(CLOCK_REALTIME, &tp2);
    ::clock_gettime(CLOCK_REALTIME_COARSE, &tp3);

    std::cout << "SYS_clock_gettime CLOCK_MONOTONIC_RAW :" << " sec part = " << tp.tv_sec << std::endl;
    std::cout << "SYS_clock_gettime CLOCK_MONOTONIC_RAW :" << " tv_nsec part = " << tp.tv_nsec << std::endl;

    std::cout << "clock_gettime CLOCK_REALTIME :" << " sec part = " << tp2.tv_sec << std::endl;
    std::cout << "clock_gettime CLOCK_REALTIME :" << " tv_nsec part = " << tp2.tv_nsec << std::endl;

    std::cout << "clock_gettime CLOCK_REALTIME_COARSE :" << " sec part = " << tp3.tv_sec << std::endl;
    std::cout << "clock_gettime CLOCK_REALTIME_COARSE :" << " tv_nsec part = " << tp3.tv_nsec << std::endl;

    std::cout << "get reps :" << std::endl;
    clock_getres(CLOCK_REALTIME_COARSE, &tp4);
    std::cout << "CLOCK_REALTIME_COARSE res sec part:" << tp4.tv_sec << std::endl;
    std::cout << "CLOCK_REALTIME_COARSE res tv_nsec part:" << tp4.tv_nsec << std::endl;
#endif

    std::cout << "test GetHandwarTime :" << KERNEL_NS::TimeUtil::GetHandwareSysRunTime() << std::endl;
    std::cout << "test GetClockRealTime :" << KERNEL_NS::TimeUtil::GetClockRealTime() << std::endl;
    std::cout << "test GetClockMonotonicTime :" << KERNEL_NS::TimeUtil::GetClockMonotonicSysRunTime() << std::endl;
    std::cout << "test GetMicroTimestamp :" << KERNEL_NS::TimeUtil::GetMicroTimestamp() << std::endl;
    std::cout << "test GetNanoTimestamp :" << KERNEL_NS::TimeUtil::GetNanoTimestamp() << std::endl;
    std::cout << "test GetFastMicroTimestamp :" << KERNEL_NS::TimeUtil::GetFastMicroTimestamp() << std::endl;
    std::cout << "test GetFastNanoTimestamp :" << KERNEL_NS::TimeUtil::GetFastNanoTimestamp() << std::endl;
    std::cout << "test GetChronoMicroTimestamp :" << KERNEL_NS::TimeUtil::GetChronoMicroTimestamp() << std::endl;
    std::cout << "test GetClockRealTimeCoarse :" << KERNEL_NS::TimeUtil::GetClockRealTimeCoarse() << std::endl;

    std::cout << "test performance:" << std::endl;

    Int64 beginTime = KERNEL_NS::LibTime::NowMilliTimestamp();
    for(Int64 i=0; i<TEST_TIME_PERFORMANCE_CNT; ++i)
        KERNEL_NS::TimeUtil::GetMicroTimestamp();

    Int64 endTime = KERNEL_NS::LibTime::NowMilliTimestamp();

    std::cout << " GetMicroTimestamp use time ="<< endTime - beginTime << " ms" << std::endl;

    beginTime = KERNEL_NS::LibTime::NowMilliTimestamp();
    for(Int64 i=0; i<TEST_TIME_PERFORMANCE_CNT; ++i)
        KERNEL_NS::TimeUtil::GetFastMicroTimestamp();

    endTime = KERNEL_NS::LibTime::NowMilliTimestamp();

    std::cout << " GetFastMicroTimestamp use time ="<< endTime - beginTime << " ms" << std::endl;

    beginTime = KERNEL_NS::LibTime::NowMilliTimestamp();
    for(Int64 i=0; i<TEST_TIME_PERFORMANCE_CNT; ++i)
        KERNEL_NS::TimeUtil::GetNanoTimestamp();

    endTime = KERNEL_NS::LibTime::NowMilliTimestamp();

    std::cout << " GetNanoTimestamp use time ="<< endTime - beginTime << " ms" << std::endl;

    beginTime = KERNEL_NS::LibTime::NowMilliTimestamp();
    for(Int64 i=0; i<TEST_TIME_PERFORMANCE_CNT; ++i)
        KERNEL_NS::TimeUtil::GetFastNanoTimestamp();

    endTime = KERNEL_NS::LibTime::NowMilliTimestamp();

    std::cout << " GetFastNanoTimestamp use time ="<< endTime - beginTime << " ms" << std::endl;

    beginTime = KERNEL_NS::LibTime::NowMilliTimestamp();
    for(Int64 i=0; i<TEST_TIME_PERFORMANCE_CNT; ++i)
        KERNEL_NS::TimeUtil::GetChronoMicroTimestamp();

    endTime = KERNEL_NS::LibTime::NowMilliTimestamp();

    std::cout << " GetChronoMicroTimestamp use time ="<< endTime - beginTime << " ms" << std::endl;

    beginTime = KERNEL_NS::LibTime::NowMilliTimestamp();
    for(Int64 i=0; i<TEST_TIME_PERFORMANCE_CNT; ++i)
        KERNEL_NS::TimeUtil::GetClockRealTime();

    endTime = KERNEL_NS::LibTime::NowMilliTimestamp();

    std::cout << " GetClockRealTime use time ="<< endTime - beginTime << " ms" << std::endl;

    beginTime = KERNEL_NS::LibTime::NowMilliTimestamp();
    for(Int64 i=0; i<TEST_TIME_PERFORMANCE_CNT; ++i)
        KERNEL_NS::TimeUtil::GetClockRealTimeCoarse();

    endTime = KERNEL_NS::LibTime::NowMilliTimestamp();

    std::cout << " GetClockRealTimeCoarse use time ="<< endTime - beginTime << " ms" << std::endl;

    beginTime = KERNEL_NS::LibTime::NowMilliTimestamp();
    for(Int64 i=0; i<TEST_TIME_PERFORMANCE_CNT; ++i)
        KERNEL_NS::TimeUtil::GetHandwareSysRunTime();

    endTime = KERNEL_NS::LibTime::NowMilliTimestamp();

    std::cout << " GetHandwarTime use time ="<< endTime - beginTime << " ms" << std::endl;

    beginTime = KERNEL_NS::LibTime::NowMilliTimestamp();
    for(Int64 i=0; i<TEST_TIME_PERFORMANCE_CNT; ++i)
        KERNEL_NS::TimeUtil::GetClockMonotonicSysRunTime();

    endTime = KERNEL_NS::LibTime::NowMilliTimestamp();

    std::cout << " GetClockMonotonicTime use time ="<< endTime - beginTime << " ms" << std::endl;

    // 与高精度的误差
    Int64 minDiff = 0, maxDiff = 0, curDiff = 0;
    std::vector<Int64> *middleArr = new std::vector<Int64>;
    std::map<Int64, Int64> *statistics = new std::map<Int64, Int64>;
    auto __addStatistics = [&statistics] (Int64 level)->void
    {
        auto iter = statistics->find(level);
        if(iter == statistics->end())
            iter = statistics->insert(std::make_pair(level, 0)).first;
        ++iter->second;
    };
    for(Int64 i=0;i<TEST_TIME_PERFORMANCE_CNT;++i)
    {
        curDiff = abs(KERNEL_NS::TimeUtil::GetClockRealTime() - KERNEL_NS::TimeUtil::GetClockRealTimeCoarse());
        middleArr->push_back(curDiff);

        if(!minDiff)
            minDiff = curDiff;
        else if(curDiff && minDiff > curDiff)
        {
            minDiff = curDiff;
        }

        if(maxDiff < curDiff)
          maxDiff = curDiff;

        // 误差100以上统计
        if(curDiff >=100 && curDiff < 500)
        {
            __addStatistics(100);
        }
        else if( curDiff >= 500 && curDiff < 1000)
        {
            __addStatistics(500);
        }
        else if( curDiff >= 1000 && curDiff < 5000)
        {
            __addStatistics(1000);
        }
        else if( curDiff >= 5000 && curDiff < 10000)
        {
            __addStatistics(5000);
        }
        else if( curDiff >= 10000 && curDiff < 15000)
        {
            __addStatistics(10000);
        }
        else if( curDiff >= 15000 && curDiff < 20000)
        {
            __addStatistics(15000);
        }
        else if( curDiff >= 20000)
        {
            __addStatistics(20000);
        }

        // 误差500 以上统计
        // 误差1000以上统计
        // 误差5000以上统计
        // 误差10000以上统计
        // 误差15000以上统计
        // 误差20000以上统计
    }

    // GetClockRealTime GetClockRealTimeCoarse diff min = 161, diff max =22426, middle value = 591 单位微妙
    std::cout << "GetClockRealTime GetClockRealTimeCoarse diff min = " << minDiff << ", diff max =" << maxDiff << ", middle value = " << middleArr->at(middleArr->size()/2) << std::endl;

    // diff level :100, count = 9
    // diff level :500, count = 4229356
    // diff level :1000, count = 5770635 误差在毫秒级别
    for(auto iter:*statistics)
    {
        std::cout << "diff level :" << iter.first << ", count = " << iter.second << std::endl;
    }

    std::cout << "test time util end" << std::endl;
    
    // 测试TimeSlice
    KERNEL_NS::TimeSlice slice(10, 500);
    KERNEL_NS::TimeSlice slice2("00:00:10.500000");

    std::cout << "slice = " << slice.ToString() << std::endl;
    std::cout << "slice2 = " << slice2.ToString() << std::endl;
    
    // 测试LibTime
    KERNEL_NS::LibTime t1 = KERNEL_NS::LibTime::Now();
    KERNEL_NS::SystemUtil::ThreadSleep(10000);
    KERNEL_NS::LibTime t2;
    t2.UpdateTime();

    std::cout << "t2 - t1 =" << (t2 - t1).ToString() << std::endl;

    KERNEL_NS::LibTime t3 =  KERNEL_NS::LibTime::FromFmtString("2023-06-25 00:31:00.100000093");
    std::cout << "t3 = " << t3.ToString() << std::endl;
    std::cout << "t2 - t3 ="<< (t2 - t3).ToString() << std::endl;
    std::cout << "t3 to zero time = " << t3.GetIntervalTo(KERNEL_NS::TimeSlice("00:00:00.100000100")).ToString() << std::endl;

    std::cout << "test lib time get time performance"<< std::endl;
    beginTime = KERNEL_NS::LibTime::NowMilliTimestamp();
    for(Int64 i=0; i<TEST_TIME_PERFORMANCE_CNT; ++i)
        KERNEL_NS::LibTime::NowTimestamp();

    endTime = KERNEL_NS::LibTime::NowMilliTimestamp();

    std::cout << " NowTimestamp use time ="<< endTime - beginTime << " ms" << std::endl;

    beginTime = KERNEL_NS::LibTime::NowMilliTimestamp();
    for(Int64 i=0; i<TEST_TIME_PERFORMANCE_CNT; ++i)
        KERNEL_NS::LibTime::NowMicroTimestamp();

    endTime = KERNEL_NS::LibTime::NowMilliTimestamp();

    std::cout << " NowMicroTimestamp use time ="<< endTime - beginTime << " ms" << std::endl;

    beginTime = KERNEL_NS::LibTime::NowMilliTimestamp();
    for(Int64 i=0; i<TEST_TIME_PERFORMANCE_CNT; ++i)
        KERNEL_NS::LibTime::NowMilliTimestamp();

    endTime = KERNEL_NS::LibTime::NowMilliTimestamp();

    std::cout << " NowMilliTimestamp use time ="<< endTime - beginTime << " ms" << std::endl;
    getchar();
}