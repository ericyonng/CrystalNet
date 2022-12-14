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
 * Date: 2022-11-23 22:23:00
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <service/TestService/ServiceCompHeader.h>
#include <service/TestService/Comps/Test/Interface/ITestMgr.h>
#include <protocols/protocols.h>

SERVICE_BEGIN

struct SessionAnalyzeInfo;

class TestMgr : public ITestMgr
{
    POOL_CREATE_OBJ_DEFAULT_P1(ITestMgr, TestMgr);

public:
    TestMgr();
    ~TestMgr();

    void Release() override;

protected:
   Int32 _OnGlobalSysInit() override;
   Int32 _OnHostStart() override;
   void _OnGlobalSysClose() override;

private:
    void _Clear();

    void _OnTestOpcodeReq(KERNEL_NS::LibPacket *&packet);
    void _OnTestOpcodeRes(KERNEL_NS::LibPacket *&packet);

    void _OnSessionCreated(KERNEL_NS::LibEvent *ev);
    void _OnWillSessionDestroy(KERNEL_NS::LibEvent *ev);
    void _OnCommonSessionReady(KERNEL_NS::LibEvent *ev);
    void _OnQuitService(KERNEL_NS::LibEvent *ev);

    Int32 _ReadTestConfigs();

    KERNEL_NS::ListenerStub _sessionConnected;
    KERNEL_NS::ListenerStub _sessionWillDestroy;
    KERNEL_NS::ListenerStub _commonSessionReady;
    KERNEL_NS::ListenerStub _quiteService;

    // ??????????????????
    bool _enableStartLink;
    bool _isStopTest;

    // ??????
    const ServiceConfig *_serviceConfig;
    Int32 _testSessionCount;        // ?????????????????????
    Int32 _testConnectIntervalMs;   // ?????????????????????
    AddrConfig *_targetAddrConfig;  // ?????????????????????
    Int32 _testSendMode;            // 1????????????????????? ?????????????????????
    Int32 _testSendIntervalMs;      // ??????????????????
    Int32 _testSendPackCountOnce;   // ????????????????????????
    Int32 _testSendPackageBytes;    // ?????????????????????????????????????????????

    // ??????????????????????????????
    std::unordered_map<UInt64, SessionAnalyzeInfo *> _sessionIdRefAnalyzeInfo;
    TestOpcodeReq req;
};

SERVICE_END
