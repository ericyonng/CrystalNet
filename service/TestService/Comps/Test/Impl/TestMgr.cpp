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
 * Date: 2022-11-23 22:23:06
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <service/TestService/Comps/Test/Impl/TestMgrFactory.h>

#include <service/TestService/MyTestService.h>
#include <service/TestService/Comps/Test/Defs/TestDefs.h>
#include <service/TestService/Comps/SysLogic/SysLogic.h>

#include <service/TestService/Comps/Test/Impl/TestMgr.h>
#include <service/TestService/Comps/Test/Impl/TestMgrStorage.h>
#include <service/TestService/Comps/Test/Impl/TestMgrStorageFactory.h>
#include <service/TestService/Comps/DB/db.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(ITestMgr);

POOL_CREATE_OBJ_DEFAULT_IMPL(TestMgr);

TestMgr::TestMgr()
:_sessionConnected(INVALID_LISTENER_STUB)
,_sessionWillDestroy(INVALID_LISTENER_STUB)
,_commonSessionReady(INVALID_LISTENER_STUB)
,_quiteService(INVALID_LISTENER_STUB)
,_enableStartLink(false)
,_isStopTest(false)
,_serviceConfig(NULL)
,_testSessionCount(0)
,_testConnectIntervalMs(10)
,_targetAddrConfig(AddrConfig::NewThreadLocal_AddrConfig())
,_testSendMode(0)
,_testSendIntervalMs(0)
,_testSendPackCountOnce(0)
,_testSendPackageBytes(0)
,_testSendPackTimeoutMilliseconds(0)
,_maxId(0)
{
}

TestMgr::~TestMgr()
{
    _Clear();
}

void TestMgr::Release()
{
    TestMgr::DeleteByAdapter_TestMgr(TestMgrFactory::_buildType.V, this);
}

Int32 TestMgr::OnLoaded(UInt64 key, const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db)
{
    TestMgrData *newData = new TestMgrData;
    if(!newData->Decode(db))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("parse fail key:%llu"), key);
        newData->Release();
        return Status::ParseFail;
    }

    _datas.insert(std::make_pair(key, newData));

    if(key > _maxId)
        _maxId = key;

    return Status::Success;
}

Int32 TestMgr::OnSave(UInt64 key, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) const
{
    auto iter = _datas.find(key);
    if(UNLIKELY(iter == _datas.end()))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("serialize fail key:%llu"), key);
        return Status::SerializeFail;
    }

    if(!iter->second->Encode(db))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("serialize fail key:%llu"), key);
        return Status::SerializeFail;
    }

    return Status::Success;
}

void TestMgr::OnWillStartup()
{
    g_Log->Info(LOGFMT_OBJ_TAG("test mgr will start up"));
    for(auto iter : _datas)
        g_Log->Info(LOGFMT_OBJ_TAG("test mgr key:%llu, Account:%s"), iter.first, iter.second->account().c_str());

    // auto timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
    // timer->SetTimeOutHandler([this](KERNEL_NS::LibTimer *t){
        
    //     if(GetService()->IsServiceModuleQuit(this))
    //     {
    //         KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
    //         return;
    //     }

    //     for(auto idx = 0; idx < 10; ++idx)
    //          _MakeNewTestData();

    //     auto maxId = _maxId;
    //     GetService()->GetComp<IMysqlMgr>()->PurgeEndWith([maxId, this](Int32 errCode){
    //         g_Log->Info(LOGFMT_OBJ_TAG("purge finished maxId:%llu, errCode:%d"), maxId, errCode);
    //     });

    //     // for(auto idx = 0; idx < 10; ++idx)
    //     //      _MakeNewTestData();

    //     // 同步调用
    //      // GetService()->GetComp<IMysqlMgr>()->PurgeAndWaitComplete(this);
    // });

    // timer->Schedule(1000);
}

void TestMgr::OnStartup()
{
    g_Log->Info(LOGFMT_OBJ_TAG("test mgr start up"));
}

void TestMgr::OnRegisterComps()
{
    RegisterComp<TestMgrStorageFactory>();
}

Int32 TestMgr::_OnGlobalSysInit()
{
    // 注册协议
    GetService()->Subscribe(Opcodes::OpcodeConst::OPCODE_TestOpcodeReq, this, &TestMgr::_OnTestOpcodeReq);
    GetService()->Subscribe(Opcodes::OpcodeConst::OPCODE_TestOpcodeRes, this, &TestMgr::_OnTestOpcodeRes);

    // 注册事件
    _sessionConnected = GetEventMgr()->AddListener(EventEnums::SESSION_CREATED, this, &TestMgr::_OnSessionCreated);
    _sessionWillDestroy = GetEventMgr()->AddListener(EventEnums::SESSION_WILL_DESTROY, this, &TestMgr::_OnWillSessionDestroy);
    _commonSessionReady = GetEventMgr()->AddListener(EventEnums::SERVICE_COMMON_SESSION_READY, this, &TestMgr::_OnCommonSessionReady);
    _quiteService = GetEventMgr()->AddListener(EventEnums::QUIT_SERVICE_EVENT, this, &TestMgr::_OnQuitService);
    
    auto st = _ReadTestConfigs();
    if(st != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("read test configs fail."));
        return st;
    }

    if(_testSendPackageBytes > 0)
    {
        std::string *content = req.mutable_content();
        content->resize(_testSendPackageBytes);
        for (Int32 idx = 0; idx < _testSendPackageBytes; ++idx)
            content->at(idx) = 1;
    }

    return Status::Success;
}

Int32 TestMgr::_OnHostStart()
{
    return Status::Success;
}

void TestMgr::_OnGlobalSysClose()
{
    _Clear();
}

void TestMgr::_Clear()
{
    if(_targetAddrConfig)
    {
        AddrConfig::DeleteThreadLocal_AddrConfig(_targetAddrConfig);
        _targetAddrConfig = NULL;
    }

    KERNEL_NS::ContainerUtil::DelContainer2(_datas);

    KERNEL_NS::ContainerUtil::DelContainer2(_sessionIdRefAnalyzeInfo);

    if(_sessionConnected == INVALID_LISTENER_STUB)
        return;

    GetEventMgr()->RemoveListenerX(_sessionConnected);
    GetEventMgr()->RemoveListenerX(_sessionWillDestroy);
    GetEventMgr()->RemoveListenerX(_commonSessionReady);
    GetEventMgr()->RemoveListenerX(_quiteService);
}

void TestMgr::_OnTestOpcodeReq(KERNEL_NS::LibPacket *&packet)
{
    if(_isStopTest)
        return;

    auto req = packet->GetCoder<TestOpcodeReq>();
    g_Log->Custom("req:%s", req->DebugString().c_str());

    TestOpcodeRes res;
    res.set_content(req->content());
    res.set_testid(req->testid());
    Send(packet->GetSessionId(), Opcodes::OpcodeConst::OPCODE_TestOpcodeRes, res, packet->GetPacketId());
}

void TestMgr::_OnTestOpcodeRes(KERNEL_NS::LibPacket *&packet)
{
    if(_isStopTest)
        return;

    const auto sessionId = packet->GetSessionId();
    auto iter = _sessionIdRefAnalyzeInfo.find(sessionId);
    if(UNLIKELY(iter == _sessionIdRefAnalyzeInfo.end()))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("session is not exists sessionId:%llu"), sessionId);
        return;
    }

    auto analyzeInfo = iter->second;
    auto iterPacketAnalyzeInfo = analyzeInfo->_packetIdRefAnalyzeInfo.find(packet->GetPacketId());
    if(UNLIKELY(iterPacketAnalyzeInfo == analyzeInfo->_packetIdRefAnalyzeInfo.end()))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("session pacekt analyze info not exists sessionId:%llu, packetId:%lld"), sessionId, packet->GetPacketId());
        return;
    }

    auto packetAnalyzeInfo = iterPacketAnalyzeInfo->second;

    // 算时间
    const auto costNs = KERNEL_NS::LibCpuCounter::Current().ElapseNanoseconds(packetAnalyzeInfo->_counter);
    GetApp()->PushResponceNs(costNs);

    auto res = packet->GetCoder<TestOpcodeRes>();
    if(g_Log->IsEnable(KERNEL_NS::LogLevel::Custom))
        g_Log->Custom("packet id:%lld, TestOpcodeRes res size:%d cost %llu (ns).", packet->GetPacketId(), static_cast<Int32>(res->ByteSizeLong()), costNs);

    // 始终使用同一个packetId
    if(_testSendMode == 1)
    {
        TestOpcodeReq req;
        *req.mutable_content() = *res->mutable_content();

        packetAnalyzeInfo->_counter.Update();
        Send(packet->GetSessionId(), Opcodes::OpcodeConst::OPCODE_TestOpcodeReq, req, packet->GetPacketId());

        packetAnalyzeInfo->_expireTimer->Schedule(_testSendPackTimeoutMilliseconds);
    }
    else
    {// 移除分析数据
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(packetAnalyzeInfo->_expireTimer);
        packetAnalyzeInfo->_expireTimer = NULL;
        analyzeInfo->_packetIdRefAnalyzeInfo.erase(packet->GetPacketId());
        packetAnalyzeInfo->Release();
    }
}

void TestMgr::_OnSessionCreated(KERNEL_NS::LibEvent *ev)
{
    if(_isStopTest)
        return;

    if(!_enableStartLink)
        return;

    auto sessionId = ev->GetParam(Params::SESSION_ID).AsUInt64();
    _sessionIdRefAnalyzeInfo.insert(std::make_pair(sessionId, SessionAnalyzeInfo::NewThreadLocal_SessionAnalyzeInfo(sessionId)));

    // 会话数量还没达到指定目标会话数量
    if(static_cast<Int32>(_sessionIdRefAnalyzeInfo.size()) < _testSessionCount)
        return;

    _enableStartLink = false;

    // 下一帧开始测试
    auto nextFrame = [this](KERNEL_NS::LibTimer *t)
    {
        if(_isStopTest)
        {
            KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
            return;
        }

        for(auto &iter : _sessionIdRefAnalyzeInfo)
        {
            auto sessionId = iter.first;
            auto analyzeInfo = iter.second;
            for(Int32 idx = 0; idx < _testSendPackCountOnce; ++idx)
            {
                auto packetAnalyzeInfo = TestAnalyzeInfo::NewThreadLocal_TestAnalyzeInfo(0);
                packetAnalyzeInfo->_counter.Update();
                auto packetId = NewPacketId(analyzeInfo->_sessionId);
                Send(analyzeInfo->_sessionId, Opcodes::OpcodeConst::OPCODE_TestOpcodeReq, req, packetId);
                if(UNLIKELY(packetId < 0))
                {
                    packetAnalyzeInfo->Release();
                    g_Log->Warn(LOGFMT_OBJ_TAG("send fail sessionId:%llu"), analyzeInfo->_sessionId);
                    continue;
                }

                auto packetExpire = [this, sessionId, packetId](KERNEL_NS::LibTimer *t) mutable
                {
                    do
                    {
                        auto iter = _sessionIdRefAnalyzeInfo.find(sessionId);
                        if(iter == _sessionIdRefAnalyzeInfo.end())
                        {
                            break;
                        }

                        auto &packetIdRefInfo = iter->second;
                        auto iterInfo = packetIdRefInfo->_packetIdRefAnalyzeInfo.find(packetId);
                        if(iterInfo == packetIdRefInfo->_packetIdRefAnalyzeInfo.end())
                        {
                            break;
                        }

                        if(_testSendMode == 1)
                        {
                            auto packetAnalyzeInfo = iterInfo->second;
                            packetAnalyzeInfo->_counter.Update();
                            packetId = NewPacketId(sessionId);
                            Send(sessionId, Opcodes::OpcodeConst::OPCODE_TestOpcodeReq, req, packetId);
                            if(UNLIKELY(packetId < 0))
                            {
                                packetAnalyzeInfo->Release();
                                g_Log->Warn(LOGFMT_OBJ_TAG("send fail sessionId:%llu"), sessionId);
                            }
                        }
                        else
                        {
                            TestAnalyzeInfo::DeleteThreadLocal_TestAnalyzeInfo(iterInfo->second);
                            packetIdRefInfo->_packetIdRefAnalyzeInfo.erase(iterInfo);
                        }
       
                    }while(false);

                    g_Log->Debug(LOGFMT_OBJ_TAG("time out sessionId:%llu, packetId:%lld"), sessionId, packetId);
                    
                    if(_testSendMode != 1)
                        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
                };

                packetAnalyzeInfo->_expireTimer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
                packetAnalyzeInfo->_expireTimer->SetTimeOutHandler(KERNEL_CREATE_CLOSURE_DELEGATE(packetExpire, void, KERNEL_NS::LibTimer *));
                packetAnalyzeInfo->_expireTimer->Schedule(_testSendPackTimeoutMilliseconds);

                packetAnalyzeInfo->_packetId = packetId;
                analyzeInfo->_packetIdRefAnalyzeInfo.insert(std::make_pair(packetId, packetAnalyzeInfo));
            }
        }

        if(_testSendMode == 1)
        {// 等待响应包发送则不需要定时发送
            KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
        }
    };

    auto timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
    timer->SetTimeOutHandler(KERNEL_CREATE_CLOSURE_DELEGATE(nextFrame, void, KERNEL_NS::LibTimer *));
    
    if(_testSendMode == 1)
    {// 1等待响应包发送
        timer->Schedule(1);
    }
    else
    {// 其他间隔时间发送
        timer->Schedule(std::max<Int32>(_testSendIntervalMs, 0));
    }
}

void TestMgr::_OnWillSessionDestroy(KERNEL_NS::LibEvent *ev)
{
    auto sessionId = ev->GetParam(Params::SESSION_ID).AsUInt64();
    auto iter = _sessionIdRefAnalyzeInfo.find(sessionId);
    if(iter == _sessionIdRefAnalyzeInfo.end())
        return;

    iter->second->Release();
    _sessionIdRefAnalyzeInfo.erase(iter);

    g_Log->Debug(LOGFMT_OBJ_TAG("test - session destroy sessionId:%llu"), sessionId);
}

void TestMgr::_OnCommonSessionReady(KERNEL_NS::LibEvent *ev)
{
    if(GetService()->GetAppAliasName() != "cli")
        return;

    _enableStartLink = true;

    // 一直连接直到连接数足够
    auto linkTimerOut = [this](KERNEL_NS::LibTimer *t) mutable -> void 
    {
        if(_isStopTest)
        {
            KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
            return;
        }

        if(!_enableStartLink)
        {
            KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
            return;
        }

        UInt64 stub = 0;
        auto st = this->GetGlobalSys<ISysLogicMgr>()->AsynTcpConnect(_targetAddrConfig->_remoteIp
        , _targetAddrConfig->_remotePort
        , stub
        , _targetAddrConfig->_localIp
        , _targetAddrConfig->_localPort
        , NULL
        , 0
        , 0
        ,  _targetAddrConfig->_priorityLevel
        ,  _targetAddrConfig->_sessionType
        , _targetAddrConfig->_af
        , _targetAddrConfig->_remoteProtocolStackType);
        if(st != Status::Success)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("asyn connect fail st:%d, _targetAddrConfig:%s"), st, _targetAddrConfig->ToString().c_str());
        }
    };

    auto timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
    timer->SetTimeOutHandler(KERNEL_CREATE_CLOSURE_DELEGATE(linkTimerOut, void, KERNEL_NS::LibTimer *));
    timer->Schedule(std::max<Int32>(_testConnectIntervalMs, 0));
}

void TestMgr::_OnQuitService(KERNEL_NS::LibEvent *ev)
{
    _isStopTest = true;

    GetService()->MaskServiceModuleQuitFlag(this);
    // 等到poller可以退出的时候方可结束
    // auto timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
    // timer->SetTimeOutHandler([this](KERNEL_NS::LibTimer *t){
    //     auto service = GetService();
    //     if(!service->GetPoller()->CanQuit())
    //         return;

    //     g_Log->Info(LOGFMT_OBJ_TAG("test mgr final end."));

    //     service->MaskServiceModuleQuitFlag(this);
    //     KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
    // });

    // timer->Schedule(1000);
}

Int32 TestMgr::_ReadTestConfigs()
{
    if(GetService()->GetAppAliasName() != "cli")
        return Status::Success;

    auto app = GetService()->GetApp();
    auto ini = app->GetIni();
    const auto &serviceName = GetService()->GetServiceName();

    // 读取配置
    _serviceConfig = GetService()->CastTo<MyTestService>()->GetServiceConfig();

    {// 会话创建总数
        Int64 count = 0;
        if(!ini->CheckReadInt(serviceName.c_str(), "TestSessionCount", count))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("check read TestSessionCount config fail service name:%s"), serviceName.c_str());
            return Status::ConfigError;
        }
        _testSessionCount = static_cast<Int32>(count);
    }

    {// 连接时间间隔
        Int64 value = 0;
        if(!ini->CheckReadInt(serviceName.c_str(), "TestConnectIntervalMs", value))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("check read TestConnectIntervalMs config fail service name:%s"), serviceName.c_str());
            return Status::ConfigError;
        }
        _testConnectIntervalMs = static_cast<Int32>(value);
    }

    {// 目标地址
        KERNEL_NS::LibString cache;
        if(!ini->ReadStr(serviceName.c_str(), "TestTargetAddr", cache))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("check read TestTargetAddr config fail service name:%s"), GetService()->GetServiceName().c_str());
            return Status::ConfigError;
        }
        cache.strip();
        
        if(!_targetAddrConfig->Parse(cache, _serviceConfig->_portRefSessionType))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("check parse TestTargetAddr config fail service name:%s, value:%s")
                    , GetService()->GetServiceName().c_str(), cache.c_str());
            return Status::ConfigError;
        }
    }

    {// 测试发送模式
        Int64 value = 0;
        if(!ini->CheckReadInt(serviceName.c_str(), "TestSendMode", value))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("check read TestSendMode config fail service name:%s"), serviceName.c_str());
            return Status::ConfigError;
        }
        _testSendMode = static_cast<Int32>(value);
    }

    {// 发送时间间隔
        Int64 value = 0;
        if(!ini->CheckReadInt(serviceName.c_str(), "TestSendIntervalMs", value))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("check read TestSendIntervalMs config fail service name:%s"), serviceName.c_str());
            return Status::ConfigError;
        }
        _testSendIntervalMs = static_cast<Int32>(value);
    }

    {// 一次发送多少个包
        Int64 value = 0;
        if(!ini->CheckReadInt(serviceName.c_str(), "TestSendPackCountOnce", value))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("check read TestSendPackCountOnce config fail service name:%s"), serviceName.c_str());
            return Status::ConfigError;
        }
        _testSendPackCountOnce = static_cast<Int32>(value);
    }

    {// 一次发送的包内容至少多少个字节
        Int64 value = 0;
        if(!ini->CheckReadInt(serviceName.c_str(), "TestSendPackageBytes", value))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("check read TestSendPackageBytes config fail service name:%s"), serviceName.c_str());
            return Status::ConfigError;
        }
        _testSendPackageBytes = static_cast<Int32>(value);
    }

    {// 发包超时时间
        Int64 value = 0;
        if(!ini->CheckReadInt(serviceName.c_str(), "TestSendPackageTimeoutMilliseconds", value))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("check read TestSendPackageTimeoutMilliseconds config fail service name:%s"), serviceName.c_str());
            return Status::ConfigError;
        }

        _testSendPackTimeoutMilliseconds = value;
    }

    return Status::Success;
}

void TestMgr::_MakeNewTestData()
{
    auto newData = new TestMgrData;
    newData->set_account("ni hao");
    newData->set_testid(static_cast<Int64>(++_maxId));

    _datas.insert(std::make_pair(_maxId, newData));
    MaskNumberKeyAddDirty(_maxId);

    g_Log->Info(LOGFMT_OBJ_TAG("make new test data max id:%llu"), _maxId);
}

SERVICE_END


