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
#include <service_common/ServiceCommon.h>
#include <service/common/common.h>
#include <service/LogicService/Common/ServiceCommon.h>

#include <Comps/Test/Impl/TestMgrFactory.h>

#include <Comps/Test/Defs/TestDefs.h>
#include <service/common/BaseComps/SysLogic/SysLogic.h>

#include <Comps/Test/Impl/TestMgr.h>
#include <Comps/Test/Impl/TestMgrStorage.h>
#include <Comps/Test/Impl/TestMgrStorageFactory.h>

#include <protocols/cplusplus/comp_test.pb.h>
#include <protocols/cplusplus/test_opcode.pb.h>

#include "kernel/comp/Event/EventManager.h"
#include "kernel/comp/Event/LibEvent.h"
#include "protocols/Opcodes.h"

SERVICE_BEGIN

TestMgr::TestMgr()
:ITestMgr(KERNEL_NS::RttiUtil::GetTypeId<TestMgr>())
,_sessionConnected(INVALID_LISTENER_STUB)
,_sessionWillDestroy(INVALID_LISTENER_STUB)
,_commonSessionReady(INVALID_LISTENER_STUB)
,_quiteService(INVALID_LISTENER_STUB)
,_enableStartLink(false)
,_isStopTest(false)
,_testSessionCount(0)
,_testConnectIntervalMs(10)
,_testSendMode(0)
,_testSendIntervalMs(0)
,_testSendPackCountOnce(0)
,_testSendPackageBytes(0)
,_testSendPackTimeoutMilliseconds(0)
,_maxId(0)
,_recvId(0)
,_sendId(0)
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

Int32 TestMgr::OnLoaded(Int64 key, const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db)
{
    KERNEL_NS::SmartPtr<SERVICE_COMMON_NS::TestMgrDataOrmData, KERNEL_NS::AutoDelMethods::Release> newData = SERVICE_COMMON_NS::TestMgrDataOrmData::NewThreadLocal_TestMgrDataOrmData();
    
    KERNEL_NS::LibString data;
    const auto len = static_cast<size_t>(db.GetReadableSize());
    if(!newData->FromJsonString(db.GetReadBegin(), len))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("parse pass time data fail key:%llu"), key);
        return Status::ParseFail;
    }
    
    _datas.insert(std::make_pair(key, newData.pop()));

    if(key > _maxId)
        _maxId = key;

    return Status::Success;
}

Int32 TestMgr::OnSave(Int64 key, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) const
{
    auto iter = _datas.find(key);
    if(UNLIKELY(iter == _datas.end()))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("serialize fail key:%llu"), key);
        return Status::SerializeFail;
    }

    KERNEL_NS::LibString data;
    if(!iter->second->ToJsonString(&data))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("serialize fail key:%llu"), key);
        return Status::SerializeFail;
    }
    
    db.Write(data.data(), static_cast<Int64>(data.length()));

    return Status::Success;
}

void TestMgr::OnWillStartup()
{
    g_Log->Info(LOGFMT_OBJ_TAG("test mgr will start up"));
    for(auto iter : _datas)
        g_Log->Info(LOGFMT_OBJ_TAG("test mgr key:%llu, Account:%s"), iter.first, iter.second->account().c_str());

    _MakeNewTestData();
    
    KERNEL_NS::PostCaller([this]()->KERNEL_NS::CoTask<>
    {
        KERNEL_NS::SmartMongoSerializeInfoWrapper wrapper;
        auto ret = co_await GetMongodbProxy()->Query(this, 1, wrapper.Ptr.AsSelf());
        if (ret)
        {
            CLOG_INFO("has data key:1");
            
            auto iter = wrapper.Ptr->find(TestMgrStorage::ValueName);
            if (iter == wrapper.Ptr->end())
            {
                CLOG_WARN("have no value data");
                co_return;
            }
            
            auto &streamInfo = iter->second;
            OnLoaded(1, *streamInfo._stream);
        }
        else
        {
            CLOG_WARN("have no data key:1");
        }
    });
    

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
    GetService()->SubscribeCo(Opcodes::TestOpcodeReq, this, &TestMgr::_OnTestOpcodeReq);
    GetService()->Subscribe(Opcodes::TestRpcReq, this, &TestMgr::_OnTestRpcReq);
    
    // 注册事件
    _quiteService = GetEventMgr()->AddListener(EventEnums::QUIT_SERVICE_EVENT, this, &TestMgr::_OnQuitService);
    
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
    KERNEL_NS::ContainerUtil::DelContainer2(_datas);

    if(_sessionConnected == INVALID_LISTENER_STUB)
        return;

    GetEventMgr()->RemoveListenerX(_sessionConnected);
    GetEventMgr()->RemoveListenerX(_sessionWillDestroy);
    GetEventMgr()->RemoveListenerX(_commonSessionReady);
    GetEventMgr()->RemoveListenerX(_quiteService);
}

KERNEL_NS::CoTask<> TestMgr::_OnTestOpcodeReq(KERNEL_NS::LibPacket *&packet)
{
    if(_isStopTest)
        co_return;

    auto req = packet->GetCoder<TestOpcodeReq>();
    ++_recvId;
    ++_sendId;

    TestOpcodeRes res;
    res.set_content(req->content());
    res.set_testid(req->testid());
    Send(packet->GetSessionId(), Opcodes::TestOpcodeRes, res, packet->GetPacketId());

    co_return;
}

void TestMgr::_OnTestRpcReq(KERNEL_NS::LibPacket *&packet)
{
    auto req = packet->GetCoder<TestRpcReq>();

    g_Log->Info(LOGFMT_OBJ_TAG("_OnTestRpcReq req:%s"), req->ToJsonString().c_str());

    TestRpcRes res;
    res.set_content(req->content());
    Send(packet->GetSessionId(), Opcodes::TestRpcRes, res, packet->GetPacketId());
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

void TestMgr::_MakeNewTestData()
{
    const auto id = ++_maxId;
    auto newData = SERVICE_COMMON_NS::TestMgrDataOrmData::NewThreadLocal_TestMgrDataOrmData();
    newData->set_account("ni hao");
    newData->set_testid(id);
    newData->SetMaskDirtyCallback([this, id](SERVICE_COMMON_NS::IOrmData *)
    {
        MaskNumberKeyModifyDirty(id);
    });

    _datas.insert(std::make_pair(id, newData));
    MaskNumberKeyAddDirty(id);

    CLOG_INFO("make new test data max id:%lld", id);
}


SERVICE_END


