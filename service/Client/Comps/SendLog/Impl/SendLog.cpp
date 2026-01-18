#include <pch.h>
#include <service/Client/Comps/SendLog/Impl/SendLog.h>
#include <service/Client/Comps/SendLog/Impl/SendLogFactory.h>
#include <protocols/protocols.h>
#include <Comps/User/user.h>

SERVICE_BEGIN

bool DataSourceInfoCompare::operator()(const DataSourceInfo *left, const DataSourceInfo *right) const
{
    if(left == right)
        return false;
    
    if(left->mstime() == right->mstime())
    {
        return left->requestid() < right->requestid();
    }

    return left->mstime() < right->mstime();
}


POOL_CREATE_OBJ_DEFAULT_IMPL(ISendLog);
POOL_CREATE_OBJ_DEFAULT_IMPL(SendLog);

SendLog::SendLog()
    :ISendLog(KERNEL_NS::RttiUtil::GetTypeId<SendLog>())
,_waitConfirmId(0)
,_reSendTimer(NULL)
,_reSendInterval(KERNEL_NS::TimeSlice::FromSeconds(10))
{
    
}

SendLog::~SendLog()
{
    _Clear();
}

void SendLog::Release()
{
    SendLog::DeleteByAdapter_SendLog(SendLogFactory::_buildType.V, this);
}

void SendLog::SendData(SERVICE_NS::DataSourceInfo *data)
{
    // 1.直接广播给每个User,
    auto userMgr = GetGlobalSys<IClientUserMgr>();
    auto &allUsers = userMgr->GetAllUsers();

    // 先放队列中
    _dataSortedByTime.insert(data);

    // 没有待确认的包, 则直接发
    if(_waitConfirmId == 0)
    {
        for(auto d : _dataSortedByTime)
        {
            if(_waitConfirmId == 0)
            {
                _waitConfirmId = d->requestid();
                _requestWaitConfirm.set_packetid(_waitConfirmId);
            }
            *_requestWaitConfirm.add_datalist() = *d;
            d->Release();
        }
        _dataSortedByTime.clear();

        for(auto iter : allUsers)
        {
            auto user = iter.second;
            UNUSED(user->Send(Opcodes::OpcodeConst::OPCODE_SendDataRequest, _requestWaitConfirm));
        }

        // 10秒没确认, 则重传
        _reSendTimer->Schedule(_reSendInterval);

        if(g_Log->IsEnable(KERNEL_NS::LogLevel::Debug))
            g_Log->Debug(LOGFMT_OBJ_TAG("SendData req id:%lld"), _waitConfirmId);
    }
    
    // 2.发起定时, 如果15秒内没收到返回, 则重传, 直到收到返回包为止(移除data)
    // 3. 断线重登后, 重新发没收到Response的数据, 所以要监听User登录完成事件
}

Int32 SendLog::_OnGlobalSysInit()
{
    GetService()->Subscribe(Opcodes::OpcodeConst::OPCODE_SendDataResponse, this, &SendLog::_OnSendDataResponse);

    _reSendTimer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
    _reSendTimer->SetTimeOutHandler(this, &SendLog::_OnResendTimer);

    return Status::Success;
}

void SendLog::_OnGlobalSysClose()
{
    _Clear();
}


void SendLog::_OnSendDataResponse(KERNEL_NS::LibPacket *&packet)
{
    auto res = packet->GetCoder<SendDataResponse>();
    if(res->packetid() != _waitConfirmId)
    {
        if(g_Log->IsEnable(KERNEL_NS::LogLevel::Debug))
            g_Log->Debug(LOGFMT_OBJ_TAG("confirm response not waiting packetId:%lld, res packet id:%lld"), _waitConfirmId, res->packetid());
        return;
    }
    
    _requestWaitConfirm.clear_datalist();
    _requestWaitConfirm.set_packetid(0);

    if(g_Log->IsEnable(KERNEL_NS::LogLevel::Debug))
        g_Log->Debug(LOGFMT_OBJ_TAG("confirm response, packetId:%lld"), _waitConfirmId);

    _waitConfirmId = 0;

    // 继续发包
    for(auto data :_dataSortedByTime)
    {
        if(_waitConfirmId == 0)
        {
            _waitConfirmId = data->requestid();
            _requestWaitConfirm.set_packetid(_waitConfirmId);
        }

        *_requestWaitConfirm.add_datalist() = *data;
        data->Release();
    }
    _dataSortedByTime.clear();

    // 发送
    if (_waitConfirmId)
    {
        auto userMgr = GetGlobalSys<IClientUserMgr>();
        auto &allUsers = userMgr->GetAllUsers();
        for(auto iter : allUsers)
        {
            auto user = iter.second;
            UNUSED(user->Send(Opcodes::OpcodeConst::OPCODE_SendDataRequest, _requestWaitConfirm));
        }

        _reSendTimer->Schedule(_reSendInterval);

        if(g_Log->IsEnable(KERNEL_NS::LogLevel::Debug))
            g_Log->Debug(LOGFMT_OBJ_TAG("_OnSendDataResponse and SendData req id:%lld"), _waitConfirmId);
    }
}

void SendLog::_OnResendTimer(KERNEL_NS::LibTimer *timer)
{
    // 已确认
    if(_waitConfirmId == 0)
    {
        timer->Cancel();
        return;
    }

    // 超时重传
    auto userMgr = GetGlobalSys<IClientUserMgr>();
    auto &allUsers = userMgr->GetAllUsers();
    for(auto iter : allUsers)
    {
        auto user = iter.second;
        UNUSED(user->Send(Opcodes::OpcodeConst::OPCODE_SendDataRequest, _requestWaitConfirm));
    }

    if(g_Log->IsEnable(KERNEL_NS::LogLevel::Debug))
        g_Log->Debug(LOGFMT_OBJ_TAG("_OnResendTimer and SendData req id:%lld"), _waitConfirmId);
}


void SendLog::_Clear()
{
    KERNEL_NS::ContainerUtil::DelSetContainer<decltype(_dataSortedByTime), KERNEL_NS::AutoDelMethods::NoSafeDelete>(_dataSortedByTime);

    if(_reSendTimer)
    {
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(_reSendTimer);
    }
    _reSendTimer = NULL;
}

SERVICE_END