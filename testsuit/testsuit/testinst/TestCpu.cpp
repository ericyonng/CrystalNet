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
 * Date: 2022-09-07 00:20:05
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestCpu.h>

void TestCpu::Run()
{
    KERNEL_NS::LibCpuCounter counter;
    KERNEL_NS::CpuFeature cpuFeature;
    cpuFeature.Init();

    counter.Update();
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCpu, "vendor:%s, brand:%s, rdtscp:%d, ")
            , cpuFeature.GetVendor().c_str(), cpuFeature.GetBrand().c_str(), cpuFeature.RDTSCP());

    const auto &endCounter = KERNEL_NS::LibCpuCounter().Update();
    const auto &slice = endCounter - counter;
    auto endCounter2 = endCounter;
    endCounter2 -= slice;

    const auto elapseMicroseconds = endCounter.ElapseMicroseconds(counter);
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCpu, "elapseMicroseconds:%llu, slice:%llu, counter start:%llu, counter end:%llu, endCounter2 == counter:%d")
        , elapseMicroseconds, slice.GetTotalMicroseconds(), counter.GetCurCount(), endCounter.GetCurCount(), endCounter2 == counter);

}
