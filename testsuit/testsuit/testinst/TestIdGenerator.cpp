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
* Date: 2025-01-05 20:01:43
* Author: Eric Yonng
* Description:
*/

#include "pch.h"
#include "TestIdGenerator.h"

static constexpr  Int32 s_maxLoop = 1000000;
void TestIdGenerator::Run()
{
    bool isGenerated = false;
    KERNEL_FINALLY_BEGIN(arg)
    {
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestIdGenerator, "isGenerated:%d"), isGenerated);
    }
    KERNEL_FINALLY_END();

    auto idGen = KERNEL_NS::TlsUtil::GetIdGenerator();
    auto id = idGen->NewId();

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestIdGenerator, "id:%llu"), id);

    auto startTime = KERNEL_NS::LibTime::Now();
    for(Int32 idx = 0; idx < s_maxLoop;++idx)
        idGen->NewId();

    auto endTime = KERNEL_NS::LibTime::Now();

    auto elapsed = endTime - startTime;
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestIdGenerator, "diff:%llu, one cost:%llu"), elapsed.GetTotalNanoSeconds(), elapsed.GetTotalNanoSeconds()/s_maxLoop);
    getchar();
}
