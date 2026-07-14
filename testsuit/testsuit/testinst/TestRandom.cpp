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
 * Date: 2021-01-24 00:12:19
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestRandom.h>

#define TEST_RANDOM_TIMES 10000000

void TestRandom::Run() 
{
    // KERNEL_NS::LibInt64Random<KERNEL_NS::_Build::MT> &randomEngine = KERNEL_NS::LibInt64Random<KERNEL_NS::_Build::MT>::GetInstance();
    // randomEngine.ResetSeed();
    // for(Int32 i = 0; i<10; ++i )
    // {
    //     //KERNEL_NS::LibInt64Random::ResetSeed();
    //     std::cout<< randomEngine.Gen() << std::endl;
    // }
    //
    // for (Int32 i = 0; i < 100; ++i)
    //     std::cout << randomEngine.Gen(-10000, 10000) << std::endl;
    //
    // auto beginTime = KERNEL_NS::TimeUtil::GetMicroTimestamp();
    // for(Int32 i = 0; i<TEST_RANDOM_TIMES; ++i )
    //     randomEngine.Gen(1, 10000);
    //
    // auto endTime = KERNEL_NS::TimeUtil::GetMicroTimestamp();
    //
    // std::cout << "use time micro seconds=" << (endTime - beginTime) << std::endl;
    //
    // KERNEL_NS::LibInt64Random<KERNEL_NS::_Build::TL> &randomEngineTL = KERNEL_NS::LibInt64Random<KERNEL_NS::_Build::TL>::GetInstance<1, 127>();
    // randomEngineTL.ResetSeed();
    // for(Int32 idx = 0; idx < 5; ++idx)
    // {
    //     std::cout << randomEngineTL.Gen() << std::endl;
    // }

    for(Int32 idx = 0; idx < TEST_RANDOM_TIMES; idx++)
    {
        auto r1 = KERNEL_NS::RandomUtil::RandomInt32();
        KERNEL_NS::SystemUtil::ThreadSleep(1000);
        auto r2 = KERNEL_NS::RandomUtil::RandomInt32(1000, 10000);
        KERNEL_NS::SystemUtil::ThreadSleep(1000);
        auto r3 = KERNEL_NS::RandomUtil::RandomInt32(100, 200);
        KERNEL_NS::SystemUtil::ThreadSleep(1000);
        auto r4 = KERNEL_NS::RandomUtil::RandomInt32(-100, 0);
        KERNEL_NS::SystemUtil::ThreadSleep(1000);
        auto r5 = KERNEL_NS::RandomUtil::RandomUInt32();
        KERNEL_NS::SystemUtil::ThreadSleep(1000);
        auto r6 = KERNEL_NS::RandomUtil::RandomUInt32(1000, 10000);
        KERNEL_NS::SystemUtil::ThreadSleep(1000);
        auto r7 = KERNEL_NS::RandomUtil::RandomUInt32(100, 200);
        KERNEL_NS::SystemUtil::ThreadSleep(1000);

        auto r8 = KERNEL_NS::RandomUtil::RandomInt64();
        KERNEL_NS::SystemUtil::ThreadSleep(1000);

        auto r9 = KERNEL_NS::RandomUtil::RandomInt64(10000000, 1000000000);
        KERNEL_NS::SystemUtil::ThreadSleep(1000);

        auto r10 = KERNEL_NS::RandomUtil::RandomInt64(-500, 5);
        KERNEL_NS::SystemUtil::ThreadSleep(1000);

        auto r11 = KERNEL_NS::RandomUtil::RandomInt64(-100, 0);
        KERNEL_NS::SystemUtil::ThreadSleep(1000);

        auto r12 = KERNEL_NS::RandomUtil::RandomUInt64();
        KERNEL_NS::SystemUtil::ThreadSleep(1000);

        auto r13 = KERNEL_NS::RandomUtil::RandomUInt64(999, 1999);
        KERNEL_NS::SystemUtil::ThreadSleep(1000);

        auto r14 = KERNEL_NS::RandomUtil::RandomUInt64(0, 99999);

        CLOG_INFO_GLOBAL(TestRandom, "R1:%d, r2:%d r3:%d r4:%d r5:%u r6:%u r7:%u r8:%lld r9:%lld r10:%lld r11:%lld r12:%llu r13:%llu r14:%llu"
            , r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14);
    }

}
