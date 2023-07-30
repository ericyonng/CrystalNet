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
 * Date: 2022-12-02 22:31:00
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <service/Client/ServiceCompHeader.h>

SERVICE_BEGIN

struct TestAnalyzeInfo
{
    POOL_CREATE_OBJ_DEFAULT(TestAnalyzeInfo);

    TestAnalyzeInfo(Int64 packetId)
        :_packetId(packetId)
        ,_expireTimer(NULL)
    {

    }

    ~TestAnalyzeInfo();
    void Release();

    KERNEL_NS::LibCpuCounter &UpdateCounter()
    {
        return _counter.Update();
    }

    Int64 _packetId;
    KERNEL_NS::LibCpuCounter _counter;
    KERNEL_NS::LibTimer *_expireTimer;
};

struct SessionAnalyzeInfo
{
    POOL_CREATE_OBJ_DEFAULT(SessionAnalyzeInfo);

    SessionAnalyzeInfo(UInt64 sessionId)
        :_sessionId(sessionId)
    {

    }

    ~SessionAnalyzeInfo();

    void Release();

    UInt64 _sessionId;
    std::map<Int64, TestAnalyzeInfo *> _packetIdRefAnalyzeInfo;
};

SERVICE_END