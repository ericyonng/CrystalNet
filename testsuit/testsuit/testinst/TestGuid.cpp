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
 * Date: 2021-01-08 00:57:50
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestGuid.h>


void TestGuid::Run() 
{
    std::cout << KERNEL_NS::GuidUtil::GenStr() << std::endl;

    // // 测试雪花算法,并测试性能 制造序号满
    // KERNEL_NS::SnowflakeInfo snowflakeInfo;
    // Int64 nowTime = KERNEL_NS::LibTime::NowMilliTimestamp();
    // KERNEL_NS::GuidUtil::InitSnowFlake(snowflakeInfo, 1, nowTime, 0, 0);

    // UInt64 uid = 0;
    // while (true)
    // {
    //     uid = KERNEL_NS::GuidUtil::Snowflake(snowflakeInfo);
    //     if(snowflakeInfo._lastSeq >= static_cast<UInt64>(KERNEL_NS::UIDMask::SEQ_BITS_LIMIT_MASK))
    //         break;
    // }
    
    // std::cout << "uid seq full." << ", uid=" << uid << std::endl;

    // uid = KERNEL_NS::GuidUtil::Snowflake(snowflakeInfo);

    // 10秒id生成不重复
//     std::set<UInt64> *uids = new std::set<UInt64>;
//     Int64 endTime = KERNEL_NS::LibTime::NowTimestamp() + 10;
//     while ( KERNEL_NS::LibTime::NowTimestamp() <= endTime )
//     {
//         uid = KERNEL_NS::GuidUtil::Snowflake(snowflakeInfo);
//         if(uids->find(uid) != uids->end())
//         {
//             std::cout << "duplicate uid =" << uid << std::endl;
//             break;
//         }
// 
//         uids->insert(uid);
//     }
//     std::cout << "generate uid count = " << uids->size() << std::endl;
//     std::cout << "final uid = " << uid << std::endl;

    // 1亿个id的重复性测试
//     std::set<UInt64> *duplicateTest = new std::set<UInt64>;
//     Int64 testLoop = 100000000;
//     for(;--testLoop >= 0;)
//     {
//         uid = KERNEL_NS::GuidUtil::Snowflake(snowflakeInfo);
//         if(duplicateTest->find(uid) != duplicateTest->end())
//         {
//             std::cout << "duplicate uid = " << uid << ", has generate count = " << duplicateTest->size() << std::endl;
//             break;
//         }
// 
//         duplicateTest->insert(uid);
//     }

    // 1亿个id测试性能
//     std::list<UInt64> *snowflakeIds = new std::list<UInt64>;
//     Int64 loopCount = 10000000;
//     Int64 beginTime = KERNEL_NS::LibTime::NowMilliTimestamp();
//     for (; --loopCount >= 0;)
//         snowflakeIds->push_back(KERNEL_NS::GuidUtil::Snowflake(snowflakeInfo));
//     Int64 endTime = KERNEL_NS::LibTime::NowMilliTimestamp();
// 
//     std::cout << " generate uid count = " << snowflakeIds->size() << std::endl;
//     std::cout << " use time = " << endTime - beginTime << " ms" << std::endl;
//     std::cout << " speed =" << snowflakeIds->size() * 1000 / (endTime - beginTime) << " per second" << std::endl;
// 
//     UInt64 finalUid = *(--snowflakeIds->end());
//     std::cout << " final uid = " << finalUid << std::endl;
// 
//     UInt64 idTime = 0;
//     UInt64 instanceId = 0;
//     UInt64 seq = 0;
//    KERNEL_NS::GuidUtil::GetSnowflakeIdPart(finalUid, idTime, instanceId,  seq);
// 
//     std::cout << "final id time = " << idTime << ", final id instanceId ="<< instanceId << ", final id seq="<< seq << std::endl;


    // // 基于时间的雪花算法测试
    // KERNEL_NS::SnowflakeInfo baseSystime;
    // KERNEL_NS::GuidUtil::InitSnowFlake(baseSystime, 1, KERNEL_NS::LibTime::NowMilliTimestamp(), 0, 0);

    // KERNEL_NS::GuidUtil::SnowflakeBaseSysTime(baseSystime);

    // UInt64 idTime = 0;
    // UInt64 instanceId = 0;
    // UInt64 seq = 0;
    // KERNEL_NS::GuidUtil::GetSnowflakeIdPart(baseSystime._maskInfo, uid, idTime, instanceId,  seq);

    // std::cout << "id time = " << idTime << ", id instanceId ="<< instanceId << ", id seq="<< seq << std::endl;


    // // 1亿个重复性测试
    // std::set<UInt64> *duplicateTest = new std::set<UInt64>;
    // Int64 testLoop = 1000000;
    // for(;--testLoop >= 0;)
    // {
    //     uid = KERNEL_NS::GuidUtil::SnowflakeBaseSysTime(snowflakeInfo);
    //     if(!uid)
    //     {
    //         std::cout << "generate id fail" << "uid =" << uid << ", testLoop=" << testLoop << std::endl;
    //         break;
    //     }

    //     if(duplicateTest->find(uid) != duplicateTest->end())
    //     {
    //         std::cout << "duplicate uid = " << uid << ", has generate count = " << duplicateTest->size() << std::endl;
    //         break;
    //     }

    //     duplicateTest->insert(uid);
    // }

    // // 1亿个id测试性能
    //     std::list<UInt64> *snowflakeIds = new std::list<UInt64>;
    //     Int64 loopCount = 10000000;
    //     Int64 beginTime = KERNEL_NS::LibTime::NowMilliTimestamp();
    //     for (; --loopCount >= 0;)
    //     {
    //         snowflakeIds->push_back(KERNEL_NS::GuidUtil::SnowflakeBaseSysTime(snowflakeInfo));
    //     }
    //     Int64 endTime = KERNEL_NS::LibTime::NowMilliTimestamp();

    //     std::cout << " generate uid count = " << snowflakeIds->size() << std::endl;
    //     std::cout << " use time = " << endTime - beginTime << " ms" << std::endl;
    //     std::cout << " speed =" << snowflakeIds->size() * 1000 / (endTime - beginTime) << " per second" << std::endl;

    //     UInt64 finalUid = *(--snowflakeIds->end());
    //     std::cout << " final uid = " << finalUid << std::endl;

    //     idTime = 0;
    //     instanceId = 0;
    //     seq = 0;
    //     KERNEL_NS::GuidUtil::GetSnowflakeIdPart(snowflakeInfo._maskInfo, finalUid, idTime, instanceId,  seq);

    //     std::cout << "final id time = " << idTime << ", final id instanceId ="<< instanceId << ", final id seq="<< seq << std::endl;

    {
        const Int32 loop = 100000000;
        KERNEL_NS::SnowflakeInfo snowFlakeInfo;
        KERNEL_NS::GuidUtil::InitSnowFlake(snowFlakeInfo, 1, static_cast<UInt64>(KERNEL_NS::LibTime::NowTimestamp()));
        auto startTime = KERNEL_NS::LibTime::Now();
        UInt64 uid = 0;
        for(Int32 idx = 0; idx < loop; ++idx)
        {
            uid = KERNEL_NS::GuidUtil::Snowflake(snowFlakeInfo);
        }
        auto endTime = KERNEL_NS::LibTime::Now();
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestGuid, "snow flake test count:%d, cost:%lld(micro seconds) last uid:%llu.")
                    , loop, (endTime - startTime).GetTotalMicroSeconds(), uid);
    }

    getchar();
    
}
