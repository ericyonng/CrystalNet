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

   virtual Int32 OnLoaded(UInt64 key, const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) override;
   virtual Int32 OnSave(UInt64 key, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) const override;
    virtual void OnWillStartup() override;
    virtual void OnStartup() override;

    virtual void OnRegisterComps() override;

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

    void _MakeNewTestData();

    Int32 _ReadTestConfigs();

    KERNEL_NS::ListenerStub _sessionConnected;
    KERNEL_NS::ListenerStub _sessionWillDestroy;
    KERNEL_NS::ListenerStub _commonSessionReady;
    KERNEL_NS::ListenerStub _quiteService;

    // 是否开始测试
    bool _enableStartLink;
    bool _isStopTest;

    // 配置
    const ServiceConfig *_serviceConfig;
    Int32 _testSessionCount;        // 测试的会话数量
    Int32 _testConnectIntervalMs;   // 连接的时间间隔
    AddrConfig *_targetAddrConfig;  // 测试的目标地址
    Int32 _testSendMode;            // 1等待响应包发送 其他是间隔发送
    Int32 _testSendIntervalMs;      // 发送时间间隔
    Int32 _testSendPackCountOnce;   // 一次发送多少个包
    Int32 _testSendPackageBytes;    // 一次发送的包内容至少多少个字节
    Int64 _testSendPackTimeoutMilliseconds; // 发包超时时间

    // 需要发送数据包的会话
    std::unordered_map<UInt64, SessionAnalyzeInfo *> _sessionIdRefAnalyzeInfo;
    TestOpcodeReq req;

    std::unordered_map<UInt64, TestMgrData *> _datas;
    UInt64 _maxId;
};

SERVICE_END
