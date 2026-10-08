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
 * Date: 2026-09-29 12:03:00
 * Author: Eric Yonng
 * Description: 
*/

#include "pch.h"
#include <service/common//UnifiedSessionBridge.h>

#include "kernel/comp/memory/ObjPoolWrap.h"
#include <service/common/UnifiedService.h>

#include "kernel/comp/NetEngine/Poller/Defs/PollerEvent.h"
#include <service/common/Params.h>
#include <service/common/BaseComps/Event/Event.h>

#include "kernel/comp/Event/LibEvent.h"
#include "kernel/comp/Log/log.h"
#include "kernel/comp/NetEngine/LibPacket.h"

SERVICE_BEGIN

UnifiedSessionBridge::UnifiedSessionBridge()
 :ISessionBridge( KERNEL_NS::RttiUtil::GetTypeId<UnifiedSessionBridge>())
{
 
}

UnifiedSessionBridge::~UnifiedSessionBridge()
{
 
}

void UnifiedSessionBridge::Release()
{
    UnifiedSessionBridge::DeleteByAdapter_UnifiedSessionBridge(UnifiedSessionBridgeFactory::_buildType.V, this);
}

KERNEL_NS::IProtocolStack* UnifiedSessionBridge::GetProtocolStack(KERNEL_NS::LibSession* session)
{
    return _service->GetProtocolStack(session);
}

void UnifiedSessionBridge::_OnSessionCreated(KERNEL_NS::PollerEvent*& msg)
{
    auto sessionCreatedEv = msg->CastTo<KERNEL_NS::SessionCreatedEvent>();

    // 预创建
    {
        auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::SESSION_WILL_CREATED);
        ev->SetParam(Params::SESSION_ID, sessionCreatedEv->_sessionId);
        ev->SetParam(Params::LOCAL_ADDR, &sessionCreatedEv->_localAddr);
        ev->SetParam(Params::REMOTE_ADDR, &sessionCreatedEv->_targetAddr);
        ev->SetParam(Params::PROTOCOL_TYPE, sessionCreatedEv->_protocolType);
        ev->SetParam(Params::PROTOCOL_STACK, sessionCreatedEv->_protocolStackType);
        ev->SetParam(Params::SESSION_POLLER_ID, sessionCreatedEv->_sessionPollerId);
        ev->SetParam(Params::SERVICE_ID, sessionCreatedEv->_belongServiceId);
        ev->SetParam(Params::STUB, sessionCreatedEv->_stub);
        ev->SetParam(Params::IS_FROM_CONNECT, sessionCreatedEv->_isFromConnect);
        ev->SetParam(Params::IS_LINKER, sessionCreatedEv->_isLinker);
        ev->SetParam(Params::TARGET_ADDR_IP_CONFIG, &sessionCreatedEv->_targetConfig);
        ev->SetParam(Params::TARGET_ADDR_FAILURE_IP_SET, &sessionCreatedEv->_failureIps);
        _eventMgr->FireEvent(ev);
    }

    // 创建完成
    auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::SESSION_CREATED);
    ev->SetParam(Params::SESSION_ID, sessionCreatedEv->_sessionId);
    ev->SetParam(Params::LOCAL_ADDR, &sessionCreatedEv->_localAddr);
    ev->SetParam(Params::REMOTE_ADDR, &sessionCreatedEv->_targetAddr);
    ev->SetParam(Params::PROTOCOL_TYPE, sessionCreatedEv->_protocolType);
    ev->SetParam(Params::SESSION_POLLER_ID, sessionCreatedEv->_sessionPollerId);
    ev->SetParam(Params::SERVICE_ID, sessionCreatedEv->_belongServiceId);
    ev->SetParam(Params::STUB, sessionCreatedEv->_stub);
    ev->SetParam(Params::IS_FROM_CONNECT, sessionCreatedEv->_isFromConnect);
    ev->SetParam(Params::IS_LINKER, sessionCreatedEv->_isLinker);
    ev->SetParam(Params::TARGET_ADDR_IP_CONFIG, &sessionCreatedEv->_targetConfig);
    ev->SetParam(Params::TARGET_ADDR_FAILURE_IP_SET, &sessionCreatedEv->_failureIps);
    _eventMgr->FireEvent(ev);
}

void UnifiedSessionBridge::_OnAsynConnectRes(KERNEL_NS::PollerEvent*& msg)
{
    auto connectRes = msg->CastTo<KERNEL_NS::AsynConnectResEvent>();

    auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::ASYN_CONNECT_RES);
    ev->SetParam(Params::ERROR_CODE, connectRes->_errCode);
    ev->SetParam(Params::LOCAL_ADDR, &connectRes->_localAddr);
    ev->SetParam(Params::REMOTE_ADDR, &connectRes->_targetAddr);
    ev->SetParam(Params::FAMILY, connectRes->_family);
    ev->SetParam(Params::PROTOCOL_TYPE, connectRes->_protocolType);
    ev->SetParam(Params::SESSION_POLLER_ID, connectRes->_sessionPollerId);
    ev->SetParam(Params::SERVICE_ID, connectRes->_fromServiceId);
    ev->SetParam(Params::STUB, connectRes->_stub);
    ev->SetParam(Params::SESSION_ID, connectRes->_sessionId);
    ev->SetParam(Params::TARGET_ADDR_IP_CONFIG, &connectRes->_targetConfig);
    ev->SetParam(Params::TARGET_ADDR_FAILURE_IP_SET, &connectRes->_failureIps);
    ev->SetParam(Params::TARGET_PACKET_OPTIONS, &connectRes->_packetOptions);
    _eventMgr->FireEvent(ev);
}

void UnifiedSessionBridge::_OnAddListenRes(KERNEL_NS::PollerEvent*& msg)
{
    CLOG_DEBUG("add listen res:%s", msg->ToString().c_str());

    KERNEL_NS::AddListenResEvent *addListenEv = msg->CastTo<KERNEL_NS::AddListenResEvent>();
    
    // 抛事件
    auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::ADD_LISTEN_RES);
    ev->SetParam(Params::ERROR_CODE, addListenEv->_errCode);
    ev->SetParam(Params::LOCAL_ADDR, &addListenEv->_localAddr);
    ev->SetParam(Params::FAMILY, addListenEv->_family);
    ev->SetParam(Params::SERVICE_ID, addListenEv->_serviceId);
    ev->SetParam(Params::STUB, addListenEv->_stub);
    ev->SetParam(Params::PROTOCOL_TYPE, addListenEv->_protocolType);
    ev->SetParam(Params::SESSION_ID, addListenEv->_sessionId);
    _eventMgr->FireEvent(ev);
}

void UnifiedSessionBridge::_OnSessionDestroy(KERNEL_NS::PollerEvent*& msg)
{
    KERNEL_NS::SessionDestroyEvent *destroyEv = msg->CastTo<KERNEL_NS::SessionDestroyEvent>();

    // 预创建
    {
        auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::SESSION_WILL_DESTROY);
        ev->SetParam(Params::SESSION_ID, destroyEv->_sessionId);
        ev->SetParam(Params::SESSION_CLOSE_REASON, destroyEv->_closeReason);
        ev->SetParam(Params::SERVICE_ID, destroyEv->_serviceId);
        ev->SetParam(Params::STUB, destroyEv->_stub);

        _eventMgr->FireEvent(ev);
    }

    // 销毁完成
    auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::SESSION_DESTROY);
    ev->SetParam(Params::SESSION_ID, destroyEv->_sessionId);
    ev->SetParam(Params::SESSION_CLOSE_REASON, destroyEv->_closeReason);
    ev->SetParam(Params::SERVICE_ID, destroyEv->_serviceId);
    ev->SetParam(Params::STUB, destroyEv->_stub);

    _eventMgr->FireEvent(ev);
}

void UnifiedSessionBridge::_OnRecvMsg(KERNEL_NS::PollerEvent*& msg)
{
    auto event = msg->CastTo<KERNEL_NS::RecvMsgEvent>();
    auto packets = event->_packets;
    if(UNLIKELY(!packets))
    {
        CLOG_ERROR("have no any packets msg:%s", msg->ToString().c_str());
        return;
    }

    for(auto node = packets->Begin(); node;)
    {
        auto packet = node->_data;
        node = packets->Erase(node);

        if(UNLIKELY(!packet))
        {
            CLOG_ERROR("packet cant be null session id:%llu, service id:%llu"
                , event->_sessionId, event->_serviceId);
            continue;
        }

        const auto opcode = packet->GetOpcode();
        const auto sessionId = packet->GetSessionId();

        // 来消息了
        auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::SERVICE_MSG_RECV);
        ev->SetParam(Params::SESSION_ID, sessionId);
        ev->SetParam(Params::OPCODE, opcode);
        ev->SetParam(Params::PACKET, packet);
        _eventMgr->FireEvent(ev);

        auto handler = _service->GetMsgHandler(opcode);
        if(UNLIKELY(!handler))
        {
            CLOG_WARN("a packet with unknown opcode handler packet:%s", packet->ToString().c_str());
            packet->ReleaseUsingPool();
            continue;
        }

        #ifdef ENABLE_PERFORMANCE_RECORD
            const auto packetId = packet->GetPacketId();
            auto &&outputLogFunc = [sessionId, packetId, opcode](UInt64 costMs){
                const auto opcodeInfo = Opcodes::GetOpcodeInfo(opcode);
                g_Log->Warn(LOGFMT_NON_OBJ_TAG(UnifiedService, "sessionId:%llu, packetid:%lld, opcode:%d,[%s], costMs:%llu ms. "),  sessionId, packetId, opcode, opcodeInfo ? opcodeInfo->_opcodeName.c_str() : "Unknown Opcode.", costMs);
            };
                
            PERFORMANCE_RECORD_DEF(pr, outputLogFunc, 10);
        #endif

        handler->Invoke(packet);
        if(LIKELY(packet))
            packet->ReleaseUsingPool();

        // 消费消息数量统计
        _service->AddConsumePackets(1);
    }

    KERNEL_NS::LibList<KERNEL_NS::LibPacket *>::Delete_LibList(packets);
    event->_packets = NULL;
}

void UnifiedSessionBridge::_OnQuitingServiceEv(KERNEL_NS::PollerEvent*& msg)
{
    _service->OnQuitingServiceEv(msg);
}

Int32 UnifiedSessionBridge::_OnInit()
{
    SetPoller(_service->GetPoller());
    _service = GetOwner()->CastTo<UnifiedService>();
    _eventMgr = _service->GetEventManager();
    
    auto st = ISessionBridge::_OnInit();
    if (st != Status::Success)
    {
        CLOG_ERROR("session bridge init fail st:%d", st);
        return st;
    }
    
    // 订阅退出服务事件
    Subscribe(KERNEL_NS::PollerEventType::QuitServiceEvent, this, &UnifiedSessionBridge::_OnQuitingServiceEv);
    
    return Status::Success;
}
    

KERNEL_NS::CompFactory *UnifiedSessionBridgeFactory::FactoryCreate()
{
  return kernel::ObjPoolWrap<UnifiedSessionBridgeFactory>::NewByAdapter(_buildType.V);
}

void UnifiedSessionBridgeFactory::Release()
{
    KERNEL_NS::ObjPoolWrap<UnifiedSessionBridgeFactory>::DeleteByAdapter(_buildType.V, this);
}

KERNEL_NS::CompObject *UnifiedSessionBridgeFactory::Create() const
{
    auto comp = UnifiedSessionBridge::NewByAdapter_UnifiedSessionBridge(_buildType.V);
    comp->SetInterfaceTypeId(KERNEL_NS::RttiUtil::GetTypeId<KERNEL_NS::ISessionBridge>());
    
    return comp;
}


SERVICE_END
