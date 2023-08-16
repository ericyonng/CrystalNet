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
 * Date: 2023-07-31 23:47:14
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <Comps/User/impl/User.h>
#include <Comps/User/interface/IUserSys.h>
#include <Comps/User/interface/IUserMgr.h>
#include <Comps/UserSys/UserSys.h>
#include <Comps/DB/db.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(PendingUser);

PendingUser::PendingUser()
:_status(UserStatus::USER_PENDING)
,_loginInfo(NULL)
,_byUserId(0)
,_stub(0)
,_cb(NULL)
,_dbOperatorId(0)
,_sessionId(0)
{
}

PendingUser::~PendingUser()
{
    CRYSTAL_RELEASE_SAFE(_cb);
}

KERNEL_NS::LibString PendingUser::ToString() const
{
    KERNEL_NS::LibString info;
    info.AppendFormat("status:%d, login mode:%d, account name:%s, login token:%s, login phone imei:%s, appid:%s, versionId:%llu, _dbOperatorId:%d, byAccountName:%s, byUserId:%llu, _sessionId:%llu cb owner:%s, cb handler:%s"
    , _status, _loginInfo ? _loginInfo->loginmode() : -1, _loginInfo ? _loginInfo->accountname().c_str() : ""
    , _loginInfo ? _loginInfo->logintoken().c_str() : ""
    , _loginInfo ? _loginInfo->loginphoneimei().c_str() : "", _loginInfo ? _loginInfo->appid().c_str() : "", _loginInfo ? _loginInfo->versionid() : 0
    , _dbOperatorId, _byAccountName.c_str()
    , _byUserId, _sessionId, _cb ? _cb->GetOwnerRtti() : "NONE", _cb ? _cb->GetCallbackRtti() : "NONE");

    return info;
}


POOL_CREATE_OBJ_DEFAULT_IMPL(IUser);
POOL_CREATE_OBJ_DEFAULT_IMPL(User);

User::User(IUserMgr *userMgr)
:_userMgr(userMgr)
,_status(UserStatus::USER_CREATED)
,_userBaseInfo(CRYSTAL_NEW(UserBaseInfo))
,_activedSessionId(0)
,_lruTime(KERNEL_NS::LibTime::NowMilliTimestamp())
,_heatbeatTime(KERNEL_NS::LibTime::NowMilliTimestamp())
,_curMaxPacketId(0)
{
    AddFlag(LogicSysFlagsType::DISABLE_FOCUS_BY_SERVICE_FLAG);
}

User::~User()
{
    _Clear();
}

void User::Release()
{
    User::DeleteThreadLocal_User(this);
}

void User::OnRegisterComps()
{
    RegisterComp<LoginMgrFactory>();
}

Int32 User::OnLoaded(UInt64 key, const std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> &fieldRefdb)
{
    _userBaseInfo->set_userid(key);

    // 基础信息的先
    auto descriptor = UserBaseInfo::descriptor();
    {
        const auto &fieldName = descriptor->FindFieldByNumber(UserBaseInfo::kAccountNameFieldNumber)->name();
        auto iter = fieldRefdb.find(fieldName);
        if(iter == fieldRefdb.end())
        {
            g_Log->Error(LOGFMT_OBJ_TAG("account name field data not found fieldName:%s, key:%llu"), fieldName.c_str(), key);
            return Status::NotFound;
        }

        auto data = iter->second;
        if(data->GetReadableSize())
           _userBaseInfo->set_accountname(std::string(data->GetReadBegin(), data->GetReadableSize()));
    }

    {
        const auto &fieldName = descriptor->FindFieldByNumber(UserBaseInfo::kNameFieldNumber)->name();
        auto iter = fieldRefdb.find(fieldName);
        if(iter == fieldRefdb.end())
        {
            g_Log->Error(LOGFMT_OBJ_TAG("name field data not found fieldName:%s, key:%llu"), fieldName.c_str(), key);
            return Status::NotFound;
        }

        auto data = iter->second;
        if(data->GetReadableSize())
           _userBaseInfo->set_name(std::string(data->GetReadBegin(), data->GetReadableSize()));
    }

    {
        const auto &fieldName = descriptor->FindFieldByNumber(UserBaseInfo::kNicknameFieldNumber)->name();
        auto iter = fieldRefdb.find(fieldName);
        if(iter == fieldRefdb.end())
        {
            g_Log->Error(LOGFMT_OBJ_TAG("nickname field data not found fieldName:%s, key:%llu"), fieldName.c_str(), key);
            return Status::NotFound;
        }

        auto data = iter->second;
        if(data->GetReadableSize())
            _userBaseInfo->set_nickname(std::string(data->GetReadBegin(), data->GetReadableSize()));
    }

    {
        const auto &fieldName = descriptor->FindFieldByNumber(UserBaseInfo::kPwdFieldNumber)->name();
        auto iter = fieldRefdb.find(fieldName);
        if(iter == fieldRefdb.end())
        {
            g_Log->Error(LOGFMT_OBJ_TAG("pwd field data not found fieldName:%s, key:%llu"), fieldName.c_str(), key);
            return Status::NotFound;
        }

        auto data = iter->second;
        if(data->GetReadableSize())
            _userBaseInfo->set_pwd(std::string(data->GetReadBegin(), data->GetReadableSize()));
    }

    {
        const auto &fieldName = descriptor->FindFieldByNumber(UserBaseInfo::kBindPhoneFieldNumber)->name();
        auto iter = fieldRefdb.find(fieldName);
        if(iter == fieldRefdb.end())
        {
            g_Log->Error(LOGFMT_OBJ_TAG("bindphone field data not found fieldName:%s, key:%llu"), fieldName.c_str(), key);
            return Status::NotFound;
        }

        auto data = iter->second;
        UInt64 value = 0;
        if(data->GetReadableSize())
            data->Read(&value, data->GetReadableSize());
        _userBaseInfo->set_bindphone(value);
    }

    {
        const auto &fieldName = descriptor->FindFieldByNumber(UserBaseInfo::kLastLoginTimeFieldNumber)->name();
        auto iter = fieldRefdb.find(fieldName);
        if(iter == fieldRefdb.end())
        {
            g_Log->Error(LOGFMT_OBJ_TAG("last login time field data not found fieldName:%s, key:%llu"), fieldName.c_str(), key);
            return Status::NotFound;
        }

        auto data = iter->second;
        UInt64 value = 0;
        if(data->GetReadableSize())
            data->Read(&value, data->GetReadableSize());
        _userBaseInfo->set_lastlogintime(value);
    }

    {
        const auto &fieldName = descriptor->FindFieldByNumber(UserBaseInfo::kLastLoginIpFieldNumber)->name();
        auto iter = fieldRefdb.find(fieldName);
        if(iter == fieldRefdb.end())
        {
            g_Log->Error(LOGFMT_OBJ_TAG("last login ip field data not found fieldName:%s, key:%llu"), fieldName.c_str(), key);
            return Status::NotFound;
        }

        auto data = iter->second;
        if(data->GetReadableSize())
            _userBaseInfo->set_lastloginip(std::string(data->GetReadBegin(), data->GetReadableSize()));
    }

    {
        const auto &fieldName = descriptor->FindFieldByNumber(UserBaseInfo::kLastLoginPhoneImeiFieldNumber)->name();
        auto iter = fieldRefdb.find(fieldName);
        if(iter == fieldRefdb.end())
        {
            g_Log->Error(LOGFMT_OBJ_TAG("last login phone imei field data not found fieldName:%s, key:%llu"), fieldName.c_str(), key);
            return Status::NotFound;
        }

        auto data = iter->second;
        if(data->GetReadableSize())
            _userBaseInfo->set_lastloginphoneimei(std::string(data->GetReadBegin(), data->GetReadableSize()));
    }

    {
        const auto &fieldName = descriptor->FindFieldByNumber(UserBaseInfo::kCreateIpFieldNumber)->name();
        auto iter = fieldRefdb.find(fieldName);
        if(iter == fieldRefdb.end())
        {
            g_Log->Error(LOGFMT_OBJ_TAG("create ip field data not found fieldName:%s, key:%llu"), fieldName.c_str(), key);
            return Status::NotFound;
        }

        auto data = iter->second;
        if(data->GetReadableSize())
            _userBaseInfo->set_createip(std::string(data->GetReadBegin(), data->GetReadableSize()));
    }

    {
        const auto &fieldName = descriptor->FindFieldByNumber(UserBaseInfo::kCreateTimeFieldNumber)->name();
        auto iter = fieldRefdb.find(fieldName);
        if(iter == fieldRefdb.end())
        {
            g_Log->Error(LOGFMT_OBJ_TAG("create time field data not found fieldName:%s, key:%llu"), fieldName.c_str(), key);
            return Status::NotFound;
        }

        auto data = iter->second;
        UInt64 value = 0;
        if(data->GetReadableSize())
            data->Read(&value, data->GetReadableSize());
        _userBaseInfo->set_createtime(value);
    }

    {
        const auto &fieldName = descriptor->FindFieldByNumber(UserBaseInfo::kCreatePhoneImeiFieldNumber)->name();
        auto iter = fieldRefdb.find(fieldName);
        if(iter == fieldRefdb.end())
        {
            g_Log->Error(LOGFMT_OBJ_TAG("create phone imei field data not found fieldName:%s, key:%llu"), fieldName.c_str(), key);
            return Status::NotFound;
        }

        auto data = iter->second;
        if(data->GetReadableSize())
            _userBaseInfo->set_createphoneimei(std::string(data->GetReadBegin(), data->GetReadableSize()));
    }

    {
        const auto &fieldName = descriptor->FindFieldByNumber(UserBaseInfo::kBindMailAddrFieldNumber)->name();
        auto iter = fieldRefdb.find(fieldName);
        if(iter == fieldRefdb.end())
        {
            g_Log->Error(LOGFMT_OBJ_TAG("bind mail addr field data not found fieldName:%s, key:%llu"), fieldName.c_str(), key);
            return Status::NotFound;
        }

        auto data = iter->second;
        if(data->GetReadableSize())
            _userBaseInfo->set_bindmailaddr(std::string(data->GetReadBegin(), data->GetReadableSize()));
    }

   {
        const auto &fieldName = descriptor->FindFieldByNumber(UserBaseInfo::kPwdSaltFieldNumber)->name();
        auto iter = fieldRefdb.find(fieldName);
        if(iter == fieldRefdb.end())
        {
            g_Log->Error(LOGFMT_OBJ_TAG("pwd salt field data not found fieldName:%s, key:%llu"), fieldName.c_str(), key);
            return Status::NotFound;
        }

        auto data = iter->second;
        if(data->GetReadableSize())
            _userBaseInfo->set_pwdsalt(std::string(data->GetReadBegin(), data->GetReadableSize()));
    }

    for(auto iter : _fieldNameRefUserSys)
    {
        auto &fieldName = iter.first;
        auto userSys = iter.second;
        auto iterData = fieldRefdb.find(fieldName);
        if(iterData == fieldRefdb.end())
        {
            g_Log->Error(LOGFMT_OBJ_TAG("user sys have no data key:%llu field name:%s, system name:%s"), key, fieldName.c_str(), userSys->GetObjName().c_str());
            return Status::NotFound;
        }

        auto data = iterData->second;
        auto err = userSys->OnLoaded(*data);
        if(err != Status::Success)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("user sys load fail err:%d, key:%llu, field name:%s, system name:%s"), err, key, fieldName.c_str(), userSys->GetObjName().c_str());
            return err;
        }
        g_Log->Info(LOGFMT_OBJ_TAG("[User Sys Loaded]user id:%llu, field name:%s, system name:%s, data loaded."), key, fieldName.c_str(), userSys->GetObjName().c_str());
    }

    return Status::Success;
}

Int32 User::OnSave(UInt64 key, std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> &fieldRefdb) const
{
    // 基本信息全部持久化
    auto descriptor = UserBaseInfo::descriptor();
    {
        auto data = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
        data->Init(sizeof(UInt64));
        data->WriteUInt64(_userBaseInfo->userid());

        const auto &fieldName = descriptor->FindFieldByNumber(UserBaseInfo::kUserIdFieldNumber)->name();
        fieldRefdb.insert(std::make_pair(fieldName, data));
    }
    {
        auto data = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
        data->Init(static_cast<Int64>(_userBaseInfo->accountname().size()));
        data->Write(_userBaseInfo->accountname().data(), static_cast<Int64>(_userBaseInfo->accountname().size()));

        const auto &fieldName = descriptor->FindFieldByNumber(UserBaseInfo::kAccountNameFieldNumber)->name();
        fieldRefdb.insert(std::make_pair(fieldName, data));
    }
    {
        auto data = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
        data->Init(static_cast<Int64>(_userBaseInfo->name().size()));
        data->Write(_userBaseInfo->name().data(), static_cast<Int64>(_userBaseInfo->name().size()));

        const auto &fieldName = descriptor->FindFieldByNumber(UserBaseInfo::kNameFieldNumber)->name();
        fieldRefdb.insert(std::make_pair(fieldName, data));
    }
    {
        auto data = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
        data->Init(static_cast<Int64>(_userBaseInfo->nickname().size()));
        data->Write(_userBaseInfo->nickname().data(), static_cast<Int64>(_userBaseInfo->nickname().size()));

        const auto &fieldName = descriptor->FindFieldByNumber(UserBaseInfo::kNicknameFieldNumber)->name();
        fieldRefdb.insert(std::make_pair(fieldName, data));
    }
    {
        auto data = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
        data->Init(static_cast<Int64>(_userBaseInfo->pwd().size()));
        data->Write(_userBaseInfo->pwd().data(), static_cast<Int64>(_userBaseInfo->pwd().size()));

        const auto &fieldName = descriptor->FindFieldByNumber(UserBaseInfo::kPwdFieldNumber)->name();
        fieldRefdb.insert(std::make_pair(fieldName, data));
    }
    {
        auto data = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
        data->Init(static_cast<Int64>(sizeof(UInt64)));
        data->WriteUInt64(_userBaseInfo->bindphone());

        const auto &fieldName = descriptor->FindFieldByNumber(UserBaseInfo::kBindPhoneFieldNumber)->name();
        fieldRefdb.insert(std::make_pair(fieldName, data));
    }
    {
        auto data = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
        data->Init(static_cast<Int64>(sizeof(UInt64)));
        data->WriteUInt64(_userBaseInfo->lastlogintime());

        const auto &fieldName = descriptor->FindFieldByNumber(UserBaseInfo::kLastLoginTimeFieldNumber)->name();
        fieldRefdb.insert(std::make_pair(fieldName, data));
    }
    {
        auto data = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
        data->Init(static_cast<Int64>(_userBaseInfo->lastloginip().size()));
        data->Write(_userBaseInfo->lastloginip().data(), static_cast<Int64>(_userBaseInfo->lastloginip().size()));

        const auto &fieldName = descriptor->FindFieldByNumber(UserBaseInfo::kLastLoginIpFieldNumber)->name();
        fieldRefdb.insert(std::make_pair(fieldName, data));
    }
    {
        auto data = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
        data->Init(static_cast<Int64>(_userBaseInfo->lastloginphoneimei().size()));
        data->Write(_userBaseInfo->lastloginphoneimei().data(), static_cast<Int64>(_userBaseInfo->lastloginphoneimei().size()));

        const auto &fieldName = descriptor->FindFieldByNumber(UserBaseInfo::kLastLoginPhoneImeiFieldNumber)->name();
        fieldRefdb.insert(std::make_pair(fieldName, data));
    }
    {
        auto data = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
        data->Init(static_cast<Int64>(_userBaseInfo->createip().size()));
        data->Write(_userBaseInfo->createip().data(), static_cast<Int64>(_userBaseInfo->createip().size()));

        const auto &fieldName = descriptor->FindFieldByNumber(UserBaseInfo::kCreateIpFieldNumber)->name();
        fieldRefdb.insert(std::make_pair(fieldName, data));
    }
    {
        auto data = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
        data->Init(static_cast<Int64>(sizeof(UInt64)));
        data->WriteUInt64(_userBaseInfo->createtime());

        const auto &fieldName = descriptor->FindFieldByNumber(UserBaseInfo::kCreateTimeFieldNumber)->name();
        fieldRefdb.insert(std::make_pair(fieldName, data));
    }
    {
        auto data = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
        data->Init(static_cast<Int64>(_userBaseInfo->createphoneimei().size()));
        data->Write(_userBaseInfo->createphoneimei().data(), static_cast<Int64>(_userBaseInfo->createphoneimei().size()));

        const auto &fieldName = descriptor->FindFieldByNumber(UserBaseInfo::kCreatePhoneImeiFieldNumber)->name();
        fieldRefdb.insert(std::make_pair(fieldName, data));
    }
    {
        auto data = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
        data->Init(static_cast<Int64>(_userBaseInfo->bindmailaddr().size()));
        data->Write(_userBaseInfo->bindmailaddr().data(), static_cast<Int64>(_userBaseInfo->bindmailaddr().size()));

        const auto &fieldName = descriptor->FindFieldByNumber(UserBaseInfo::kBindMailAddrFieldNumber)->name();
        fieldRefdb.insert(std::make_pair(fieldName, data));
    }

    {
        auto data = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
        data->Init(static_cast<Int64>(_userBaseInfo->pwdsalt().size()));
        data->Write(_userBaseInfo->pwdsalt().data(), static_cast<Int64>(_userBaseInfo->pwdsalt().size()));

        const auto &fieldName = descriptor->FindFieldByNumber(UserBaseInfo::kPwdSaltFieldNumber)->name();
        fieldRefdb.insert(std::make_pair(fieldName, data));
    }

    for(auto iter = _dirtySys.begin(); iter != _dirtySys.end();)
    {
        auto logic = *iter;
        auto data = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
        auto storageInfo = _GetStorageInfoBy(logic);
        data->Init(static_cast<Int64>(storageInfo->GetCapacitySize()));
        auto err = logic->OnSave(*data);
        if(err != Status::Success)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("logic on save fail err:%d logic:%s, field name:%s")
                    , logic->GetObjName().c_str(), storageInfo->GetFieldName().c_str());

            KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(data);
            return err;
        }

        fieldRefdb.insert(std::make_pair(storageInfo->GetFieldName(), data));

        iter = _dirtySys.erase(iter);
    }

    return Status::Success;
}

IUserMgr *User::GetUserMgr()
{
    return _userMgr;
}

const IUserMgr *User::GetUserMgr() const
{
    return _userMgr;
}

Int64 User::Send(KERNEL_NS::LibPacket *packet) const
{
    // TODO:需要rpc
    if(UNLIKELY(_activedSessionId == 0))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("no session cant send"));
        packet->ReleaseUsingPool();
        return -1;
    }

    return _userMgr->Send(_activedSessionId, packet);
}

void User::Send(const std::list<KERNEL_NS::LibPacket *> &packets) const
{
    // TODO:需要rpc
    if(UNLIKELY(_activedSessionId == 0))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("no session cant send"));
        for(auto iter : packets)
            iter->ReleaseUsingPool();
        return;
    }

    _userMgr->Send(_activedSessionId, packets);
}

Int64 User::Send(Int32 opcode, const KERNEL_NS::ICoder &coder, Int64 packetId) const 
{
    // TODO:需要rpc
    if(UNLIKELY(_activedSessionId == 0))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("no session cant send"));
        return -1;
    }

    return _userMgr->Send(_activedSessionId, opcode, coder, packetId > 0 ? packetId : NewPacketId());
}

void User::OnLogin()
{
    auto &userSyss = GetCompsByType(ServiceCompType::USER_SYS_COMP);
    for(auto userSys : userSyss)
        userSys->CastTo<IUserSys>()->OnLogin();

    // TODO:
    g_Log->Info(LOGFMT_OBJ_TAG("login user:%s"), ToString().c_str());
}

void User::OnLoginFinish()
{
    auto &userSyss = GetCompsByType(ServiceCompType::USER_SYS_COMP);
    for(auto userSys : userSyss)
        userSys->CastTo<IUserSys>()->OnLoginFinish();

    SetUserStatus(UserStatus::USER_LOGINED);

    g_Log->Info(LOGFMT_OBJ_TAG("OnLoginFinish user:%s"), ToString().c_str());
}

void User::OnLogout()
{
    auto &userSyss = GetCompsByType(ServiceCompType::USER_SYS_COMP);
    for(auto userSys : userSyss)
        userSys->CastTo<IUserSys>()->OnLogout();

    g_Log->Info(LOGFMT_OBJ_TAG("OnLogout user:%s"), ToString().c_str());
}

void User::OnUserCreated()
{
    auto &userSyss = GetCompsByType(ServiceCompType::USER_SYS_COMP);
    for(auto userSys : userSyss)
        userSys->CastTo<IUserSys>()->OnUserCreated();

    g_Log->Info(LOGFMT_OBJ_TAG("OnUserCreated user:%s"), ToString().c_str());
}

Int32 User::GetUserStatus() const
{
    return _status;
}

void User::SetUserStatus(Int32 status) 
{
    _status = status;
}

void User::MaskDirty()
{
    _userMgr->MaskNumberKeyModifyDirty(_userBaseInfo->userid());
}

void User::MaskDirty(IUserSys *userSys)
{
    _dirtySys.insert(userSys);
    _userMgr->MaskNumberKeyModifyDirty(_userBaseInfo->userid());
}

void User::OnMaskAddDirty()
{
    for(auto logic : _needStorageSys)
        _dirtySys.insert(logic);
}

void User::MaskDirtyAll()
{
    for(auto logic : _needStorageSys)
        _dirtySys.insert(logic);

    MaskDirty();
}

void User::Purge()
{
    _userMgr->Purge();
}

void User::PurgeAndWaitComplete()
{
    _userMgr->PurgeAndWait();
}

void User::PurgeEndWith(KERNEL_NS::IDelegate<void, Int32> *handler)
{
    auto dbMgr = _userMgr->GetGlobalSys<IMysqlMgr>();
    dbMgr->PurgeEndWith(handler);
}

UserBaseInfo *User::GetUserBaseInfo()
{
    return _userBaseInfo;
}

const UserBaseInfo *User::GetUserBaseInfo() const 
{
    return _userBaseInfo;
}

UInt64 User::GetUserId() const 
{ 
    return _userBaseInfo->userid();
}

void User::BindSession(UInt64 sessionId)
{
    _activedSessionId = sessionId;

    _userMgr->AddUserBySessionId(sessionId, this);
}

UInt64 User::GetSessionId() const
{
    return _activedSessionId;
}

void User::Logout(Int32 logoutReason)
{
    // 设置状态
    SetUserStatus(UserStatus::USER_LOGOUTING);

    // 事件
    auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::USER_WILL_LOGOUT);
    ev->SetParam(Params::USER_OBJ, this);
    _userMgr->GetEventMgr()->FireEvent(ev);

    // 执行OnLogout
    OnLogout();

    // 会话踢下线
    if(_activedSessionId)
    {
        g_Log->Debug(LOGFMT_OBJ_TAG("user offline %s"), ToString().c_str());
        
        LogoutNty nty;
        nty.set_logoutreason(logoutReason);
        Send(Opcodes::OpcodeConst::OPCODE_LogoutNty, nty);

        // 5秒后关闭
        _userMgr->CloseSession(_activedSessionId, 5000, true, true);
        _userMgr->RemoveUserBySessionId(_activedSessionId);
        _activedSessionId = 0;
    }

    SetUserStatus(UserStatus::USER_LOGOUTED);

    // 所有组件标脏
    MaskDirtyAll();

    // 存库
    PurgeAndWaitComplete();

    // 清空包id
    _curMaxPacketId = 0;
}

bool User::IsLogined() const
{
    return _status == UserStatus::USER_LOGINED;
}

bool User::IsLogout() const
{
    return (_status == UserStatus::USER_LOGOUTING) || (_status == UserStatus::USER_LOGOUTED);
}

const KERNEL_NS::BriefSockAddr *User::GetUserAddr() const
{
    auto session = _userMgr->GetGlobalSys<ISessionMgr>()->GetSession(_activedSessionId);
    if(UNLIKELY(!session))
    {
        return NULL;
    }

    return &(session->GetSessionInfo()->_remoteAddr);
}

KERNEL_NS::LibString User::ToString() const
{
    KERNEL_NS::LibString info;
    info.AppendFormat("user id:%llu, account name:%s, status:%d, max packet id:%lld"
    , _userBaseInfo->userid(), _userBaseInfo->accountname().c_str(), _status, _curMaxPacketId);

    info.AppendFormat(", session infos:");
    auto sessionMgr = _userMgr->GetService()->GetComp<ISessionMgr>();
    auto session = sessionMgr->GetSession(_activedSessionId);
    if(session)
    {
        auto sessionInfo = session->GetSessionInfo();
        info.AppendFormat("[ session id:%llu, session type:%d, remote addr:%s:%hu ], "
            , _activedSessionId, sessionInfo->_sessionType
            ,  sessionInfo->_remoteAddr._ip.c_str()
            , sessionInfo->_remoteAddr._port);
    }

    return info;
}

Int64 User::GetLruTime() const
{
    return _lruTime;
}

void User::UpdateLrtTime()
{
    _lruTime = KERNEL_NS::LibTime::NowMilliTimestamp();
}

Int64 User::GetHeartbeatExpireTime() const
{
    return _heatbeatTime;
}

void User::UpdateHeartbeatExpireTime(Int64 spanTimeInMs)
{
    _heatbeatTime = KERNEL_NS::LibTime::NowMilliTimestamp() + spanTimeInMs;
}

Int64 User::NewPacketId() const
{
    return ++_curMaxPacketId;
}

Int32 User::_OnSysInit()
{   
    // 创建事件管理器
    auto eventMgr = KERNEL_NS::EventManager::New_EventManager();
    SetEventMgr(eventMgr);

    _RegisterEvents();
    return Status::Success;
}

Int32 User::_OnSysCompsCreated()
{
    // user需要存储的子系统
    auto &allSubStorages = _userMgr->GetStorageInfo()->GetSubStorageInfos();
    for(auto subStorageInfo : allSubStorages)
    {
        if(!subStorageInfo->IsSystemDataStorage())
            continue;

        auto comp = GetComp(subStorageInfo->GetSystemName());
        if(UNLIKELY(!comp))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("user sys have storage info but have no user sys obj please check system name:%s"), subStorageInfo->GetSystemName().c_str());
            return Status::NotFound;
        }

        auto userSys = comp->CastTo<IUserSys>();
        _needStorageSys.push_back(userSys);
        _fieldNameRefUserSys.insert(std::make_pair(subStorageInfo->GetFieldName(), userSys));
        _userSysRefStorageInfo.insert(std::make_pair(userSys, subStorageInfo));
    }

    return Status::Success;
}

void User::_OnSysClose()
{
    {
        auto comps = GetCompsByType(ServiceCompType::USER_SYS_COMP);
        for(auto comp : comps)
            comp->CastTo<ILogicSys>()->SetEventMgr(NULL);
    }

    {
        auto comps = GetCompsByType(ServiceCompType::LOGIC_SYS);
        for(auto comp : comps)
            comp->CastTo<ILogicSys>()->SetEventMgr(NULL);
    }
}

void User::_Clear()
{
   CRYSTAL_RELEASE_SAFE(_userBaseInfo);

   auto eventMgr = GetEventMgr();
   KERNEL_NS::EventManager::Delete_EventManager(eventMgr);
   SetEventMgr(NULL);
}

void User::_RegisterEvents()
{
}

SERVICE_END
