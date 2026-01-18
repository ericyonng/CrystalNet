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
 * Date: 2023-07-31 23:47:22
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <service_common/ServiceCommon.h>
#include <service/common/common.h>
#include <service/TestService/Common/ServiceCommon.h>

#include <Comps/User/impl/UserMgr.h>
#include <Comps/User/impl/UserMgrFactory.h>
#include <Comps/User/impl/UserMgrStorage.h>
#include <Comps/User/impl/UserMgrStorageFactory.h>
#include <Comps/User/impl/User.h>
#include <Comps/StubHandle/StubHandle.h>
#include <OptionComp/storage/mysql/mysqlcomp.h>
#include <Comps/DB/db.h>
#include <Comps/UserSys/UserSys.h>
#include <Comps/config/config.h>
#include <Comps/User/impl/UserSessionMgr.h>
#include <Comps/User/impl/UserSessionMgrFactory.h>
#include <Comps/NickName/nickname.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(IUserMgr);
POOL_CREATE_OBJ_DEFAULT_IMPL(UserMgr);

UserMgr::UserMgr()
:IUserMgr(KERNEL_NS::RttiUtil::GetTypeId<UserMgr>())
,_lruCapacityLimit(1000)
{

}

UserMgr::~UserMgr()
{
    _Clear();
}

void UserMgr::Release()
{
    UserMgr::DeleteByAdapter_UserMgr(UserMgrFactory::_buildType.V, this);
}

void UserMgr::OnRegisterComps()
{
    RegisterComp<UserMgrStorageFactory>();

    // 会话管理
    RegisterComp<UserSessionMgrFactory>();
}

void UserMgr::OnWillStartup()
{
    g_Log->Info(LOGFMT_OBJ_TAG("will startup"));
}

void UserMgr::OnStartup()
{
    g_Log->Info(LOGFMT_OBJ_TAG("startup"));

    // TODO:需不需要抛事件?
    // for(auto userId : _pendingLoginEventOnStartup)
    // {
    //     auto user = GetUser(userId);
    //     if(UNLIKELY(!user))
    //         continue;

    //     auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::USER_LOGIN);
    //     ev->SetParam(Params::USER_OBJ, user);
    //     GetEventMgr()->FireEvent(ev);
    // }
    _pendingLoginEventOnStartup.clear();

    _LruPopUser();

    // // 用户登录
    // KERNEL_NS::LibString randAccount;
    // auto &random = KERNEL_NS::LibInt64Random<KERNEL_NS::_Build::TL>::GetInstance();
    // randAccount.AppendFormat("test_eric_%lld", random.Gen(0, 127));
    
    // KERNEL_NS::SmartPtr<LoginInfo, KERNEL_NS::AutoDelMethods::Release> loginInfo = CRYSTAL_NEW(LoginInfo);
    // loginInfo->set_loginmode(LoginMode::REGISTER);
    // loginInfo->set_accountname(randAccount.GetRaw());
    
    // KERNEL_NS::LibString pwd;
    // KERNEL_NS::LibString pwdText = "12345678";
    // _rsa.PubKeyEncrypt(pwdText, pwd);
    // auto newPwd = KERNEL_NS::LibBase64::Encode(pwd);
    // auto tempPwd = KERNEL_NS::LibBase64::Decode(newPwd);
    
    // if(tempPwd.size() < 128)
    //     tempPwd.resize(128);
    // KERNEL_NS::LibString coverPwd;
    // _rsa.PrivateKeyDecrypt(tempPwd, coverPwd);

    // loginInfo->set_pwd(newPwd.GetRaw());

    // loginInfo->set_logintoken("123456");
    // loginInfo->set_loginphoneimei("123456");
    // loginInfo->set_appid("123456");

    // KERNEL_NS::LibString randText = "123456";
    // KERNEL_NS::LibString cypherText;
    // _rsa.PubKeyEncrypt(randText, cypherText);
    // auto newCypherText = KERNEL_NS::LibBase64::Encode(cypherText);
    // auto cypherTextTemp = KERNEL_NS::LibBase64::Decode(newCypherText);
    // if(cypherTextTemp.size() < 128)
    //     cypherTextTemp.resize(128);

    // KERNEL_NS::LibString coverText;
    // _rsa.PrivateKeyDecrypt(cypherTextTemp, coverText);

    // loginInfo->set_cyphertext(newCypherText.GetRaw());
    // loginInfo->set_origintext(randText.GetRaw());
    // loginInfo->set_versionid(10101);
    // auto registerInfo = loginInfo->mutable_userregisterinfo();
    // registerInfo->set_accountname(randAccount.GetRaw());
    // registerInfo->set_pwd(newPwd.GetRaw());
    // registerInfo->set_nickname("123456");
    // registerInfo->set_createphoneimei("123456");
    // auto err = LoginBy(0, loginInfo, [this](Int32 errCode, PendingUser *pending, IUser *user, KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> &var){
    //     user->GetUserBaseInfo()->set_nickname("aakkkkkkkkkkkkkkkkkkkkkkkkkkkkkdalkaldfaskldfaskdfasklfjaslkdfaslkdfalskdfaskdfjalksdjfaksfalskdfjaskdjfalksdjfaksdfjalskdfjaksldjfaklsjdfkasjdfkasjdfkasjdflkasjflaksdjflsakfjaskdjf");
    //     user->MaskDirty();
    //     user->PurgeAndWaitComplete();

    //     g_Log->Info(LOGFMT_OBJ_TAG("user login:%s"), user->ToString().c_str());
    // });

    // if(err != Status::Success)
    // {
    //     g_Log->Warn(LOGFMT_OBJ_TAG("login fail err:%d, user acount name:%s"), err, loginInfo->accountname().c_str());
    // }
}

Int32 UserMgr::OnLoaded(UInt64 key, const std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> &fieldRefdb)
{
    KERNEL_NS::SmartPtr<User, KERNEL_NS::AutoDelMethods::CustomDelete> user = User::NewThreadLocal_User(this);
    user.SetClosureDelegate([](void *p){
        auto ptr = reinterpret_cast<User *>(p);
        ptr->WillClose();
        ptr->Close();
        User::DeleteThreadLocal_User(ptr);
    });

    user->SetUserStatus(UserStatus::USER_INITING);
    auto err = user->Init();
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("user init fail err:%d, key:%llu"), err, key);
        return err;
    }
    user->SetUserStatus(UserStatus::USER_INITED);

    user->SetUserStatus(UserStatus::USER_STARTING);
    err = user->Start();
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("user start fail err:%d, key:%llu"), err, key);
        return err;
    }
    user->SetUserStatus(UserStatus::USER_STARTED);

    user->SetUserStatus(UserStatus::USER_ONLOADING);
    err = user->OnLoaded(key, fieldRefdb);
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("user on loaded fail err:%d, key:%llu"), err, key);
        return err;
    }
    user->SetUserStatus(UserStatus::USER_ONLOADED);

    _AddUser(user.AsSelf());
    _pendingLoginEventOnStartup.push_back(user->GetUserId());

    if(g_Log->IsEnable(KERNEL_NS::LogLevel::Info))
        g_Log->Info(LOGFMT_OBJ_TAG("[user loaded]: %s"), user->ToString().c_str());

    user.pop();
    return Status::Success;
}

Int32 UserMgr::OnSave(UInt64 key, std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> &fieldRefdb) const
{
    auto user = GetUser(key);
    if(UNLIKELY(!user))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("user not found key:%llu"), key);
        return Status::NotFound;
    }

    auto err = user->OnSave(key, fieldRefdb);
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("user on save fail err:%d, key:%llu"), err, key);
        return err;
    }

    return Status::Success;
}

IUser *UserMgr::GetUser(UInt64 userId)
{
    auto iter = _userIdRefUser.find(userId);
    return iter == _userIdRefUser.end() ? NULL : iter->second;
}

const IUser *UserMgr::GetUser(UInt64 userId) const
{
    auto iter = _userIdRefUser.find(userId);
    return iter == _userIdRefUser.end() ? NULL : iter->second;
}

IUser *UserMgr::GetUser(const KERNEL_NS::LibString &accountName)
{
    auto iter = _accountNameRefUser.find(accountName);
    return iter == _accountNameRefUser.end() ? NULL : iter->second;
}

const IUser *UserMgr::GetUser(const KERNEL_NS::LibString &accountName) const
{
    auto iter = _accountNameRefUser.find(accountName);
    return iter == _accountNameRefUser.end() ? NULL : iter->second;
}

const IUser *UserMgr::GetUserBySessionId(UInt64 sessionId) const
{
    auto iter = _sessionIdRefUser.find(sessionId);
    return iter == _sessionIdRefUser.end() ? NULL : iter->second;
}

IUser *UserMgr::GetUserBySessionId(UInt64 sessionId)
{
    auto iter = _sessionIdRefUser.find(sessionId);
    return iter == _sessionIdRefUser.end() ? NULL : iter->second;
}

IUser *UserMgr::GetLoginedUserBySessionId(UInt64 sessionId)
{
    auto user = GetUserBySessionId(sessionId);
    if(!user || !user->IsLogined())
        return NULL;

    return user;
}

std::map<KERNEL_NS::LibString, IUser *> &UserMgr::GetAllUsers()
{
    return _accountNameRefUser;
}

void UserMgr::MaskNumberKeyAddDirty(UInt64 key)
{
    auto user = GetUser(key);
    if(UNLIKELY(!user))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("have no user key:%llu"), key);
        return;
    }

    user->OnMaskAddDirty();

    ILogicSys::MaskNumberKeyAddDirty(key);
}

Int32 UserMgr::Login(UInt64 sessionId, KERNEL_NS::SmartPtr<LoginInfo, KERNEL_NS::AutoDelMethods::Release> &loginInfo
    , KERNEL_NS::IDelegate<void, Int32, PendingUser *, IUser *, KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> &> *cb
    , KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> var)
{
    KERNEL_NS::SmartPtr<PendingUser, KERNEL_NS::AutoDelMethods::CustomDelete> pendingInfo = PendingUser::NewThreadLocal_PendingUser();
    pendingInfo.SetClosureDelegate([](void *p){
        auto ptr = reinterpret_cast<PendingUser *>(p);
        PendingUser::DeleteThreadLocal_PendingUser(ptr);
    });
    pendingInfo->_cb = cb;
    pendingInfo->_var = var;
    pendingInfo->_byAccountName = loginInfo->accountname();
    pendingInfo->_loginInfo = loginInfo;
    pendingInfo->_sessionId = sessionId;

    // 参数校验
    {
        if(loginInfo->loginmode() == LoginMode::REGISTER)
        {
            if(loginInfo->accountname() != loginInfo->userregisterinfo().accountname())
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("account name not same between login info and register info session id:%llu, account name:%s, register account name:%s"), sessionId, loginInfo->accountname().c_str(), loginInfo->userregisterinfo().accountname().c_str());
                if(LIKELY(cb))
                    cb->Invoke(Status::RepeateLogin, pendingInfo, NULL, var);
                return Status::CheckAccountFail;
            }
        }
    }

    {// 相同会话都在login拒绝掉
        if(_loginPendingSessions.find(sessionId) != _loginPendingSessions.end())
        {
            if(g_Log->IsEnable(KERNEL_NS::LogLevel::Info))
                g_Log->Info(LOGFMT_OBJ_TAG("repeate login in same session and pending info:%s"), pendingInfo->ToString().c_str());
            if(LIKELY(cb))
                cb->Invoke(Status::RepeateLogin, pendingInfo, NULL, var);
            return Status::RepeateLogin;
        }
    }

    {// 相同session可能登录不同账号
        auto user = GetUserBySessionId(sessionId);
        if(UNLIKELY(user))
        {
            if(g_Log->IsEnable(KERNEL_NS::LogLevel::Info))
                g_Log->Info(LOGFMT_OBJ_TAG("login in same session and will logout exists account pending info:%s, user:%s"), pendingInfo->ToString().c_str(), user->ToString().c_str());
            user->Logout(LogoutReason::LOG_IN_OTHER_ACCOUNT, false, sessionId);
        }
    }

    // 是否禁止无权限的人登录
    if(GetService()->GetComp<ConfigLoader>()->GetComp<CommonConfigMgr>()->GetConfigById(CommonConfigIdEnums::DISABLE_NO_AUTH_LOGIN)->_value != 0)
    {
        auto authRoleConfig = CetAuthRoleConfig(pendingInfo->_byAccountName);
        if(authRoleConfig == NULL)
        {
            if(g_Log->IsEnable(KERNEL_NS::LogLevel::Info))
                g_Log->Info(LOGFMT_OBJ_TAG("DISABLE_NO_AUTH_LOGIN and not auth role pending info:%s"), pendingInfo->ToString().c_str());
            if(LIKELY(cb))
                cb->Invoke(Status::NoAuth, pendingInfo, NULL, var);
            return Status::NoAuth;
        }
    }

    // TODO:获取分布式锁（LogicProxy做）
    // 是否已经存在User, 
    auto user = GetUser(pendingInfo->_byAccountName);
    if(!user)
    {
        auto authRoleConfig = CetAuthRoleConfig(pendingInfo->_byAccountName);
        if (authRoleConfig)
        {
            return AuthRoleLogin(*authRoleConfig, pendingInfo);
        }
        
        // 查库
        return LoadUser(pendingInfo->_byAccountName, pendingInfo);
    }

    auto stubMgr = GetGlobalSys<IStubHandleMgr>();
    pendingInfo->_stub = stubMgr->NewStub();

    Int32 err = Status::Success;
    if(LIKELY(pendingInfo->_sessionId))
    {
        const auto &curTime = KERNEL_NS::LibTime::Now();

        auto authRoleConfig = CetAuthRoleConfig(pendingInfo->_byAccountName);
        if (authRoleConfig)
        {
            return AuthRoleLogin(*authRoleConfig, user, pendingInfo);
        }

        // 校验登录
        auto loginMgr = user->GetComp<ILoginMgr>();
        err = loginMgr->CheckLogin(pendingInfo);
        if(err != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("check login fail err:%d, pending info:%s, userId:%llu"), err, pendingInfo->ToString().c_str(), user->GetUserId());
            if(LIKELY(cb))
                cb->Invoke(err, pendingInfo, user, var);

            return err;
        }

        if(pendingInfo->_sessionId == user->GetSessionId())
            g_Log->Warn(LOGFMT_OBJ_TAG("user repeate login in same session user:%s"), user->ToString().c_str());

        // 不同会话顶号
        if(user->IsLogined())
            user->Logout(LogoutReason::LOGIN_OTHER_PLACE, true, sessionId);

        user->BindSession(pendingInfo->_sessionId);
        user->OnLogin();
        user->OnLoginFinish();

        if(LIKELY(cb))
            cb->Invoke(Status::Success, pendingInfo, user, var);

        auto loginInfo = pendingInfo->_loginInfo;
        auto baseInfo = user->GetUserBaseInfo();
        auto addr = user->GetUserAddr();
        baseInfo->set_lastlogintime(curTime.GetMilliTimestamp());
        baseInfo->set_lastloginphoneimei(loginInfo->loginphoneimei());
        baseInfo->set_lastloginip(addr->_ip.GetRaw());

        user->MaskDirty();
    }

    _RemoveFromLru(user);
    user->UpdateLrtTime();
    _AddToLru(user);

    // if(LIKELY(cb))
    //     cb->Invoke(Status::Success, pendingInfo, user, var);

    if(g_Log->IsEnable(KERNEL_NS::LogLevel::Info))
        g_Log->Info(LOGFMT_OBJ_TAG("user already online user :%s, pending:%s"), user->ToString().c_str(), pendingInfo->ToString().c_str());

    return Status::Success;
}

Int32 UserMgr::LoadUser(const KERNEL_NS::LibString &accountName
    , KERNEL_NS::IDelegate<void, Int32, PendingUser *, IUser *, KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> &> *cb
    , KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> var)
{
    KERNEL_NS::SmartPtr<PendingUser, KERNEL_NS::AutoDelMethods::CustomDelete> pendingInfo = PendingUser::NewThreadLocal_PendingUser();
    pendingInfo.SetClosureDelegate([](void *p){
        auto ptr = reinterpret_cast<PendingUser *>(p);
        PendingUser::DeleteThreadLocal_PendingUser(ptr);
    });
    pendingInfo->_cb = cb;
    pendingInfo->_var = var;

    return LoadUser(accountName, pendingInfo);
}

Int32 UserMgr::LoadUser(UInt64 userId
    , KERNEL_NS::IDelegate<void, Int32, PendingUser *, IUser *, KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> &> *cb
    , KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> var)
{
    KERNEL_NS::SmartPtr<PendingUser, KERNEL_NS::AutoDelMethods::CustomDelete> newPendingInfo = PendingUser::NewThreadLocal_PendingUser();
    newPendingInfo.SetClosureDelegate([](void *p){
        auto ptr = reinterpret_cast<PendingUser *>(p);
        PendingUser::DeleteThreadLocal_PendingUser(ptr);
    });
    newPendingInfo->_cb = cb;
    newPendingInfo->_var = var;

    // 已经Load过
    auto existsPendingInfo = _GetPendingByUserId(userId);
    if(UNLIKELY(existsPendingInfo))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user pending is already exists, userId:%llu, exists pending:%s, new pending:%s")
            , userId, existsPendingInfo->ToString().c_str(), newPendingInfo->ToString().c_str());

        return Status::Repeat;
    }

    // 补全信息
    newPendingInfo->_byUserId = userId;
    auto mysqlMgr = GetGlobalSys<IMysqlMgr>();
    auto dbOid = mysqlMgr->NewDbOperatorId();
    newPendingInfo->_dbOperatorId = dbOid;
    auto stubMgr = GetGlobalSys<IStubHandleMgr>();
    newPendingInfo->_stub = stubMgr->NewStub();

    // 添加到pending字典
    _AddUserPendingInfo(userId, newPendingInfo);

    // 查询数据
    auto storageInfo = GetStorageInfo();
    KERNEL_NS::SelectSqlBuilder *selectBuilder = KERNEL_NS::SelectSqlBuilder::NewThreadLocal_SelectSqlBuilder();
    auto descriptor = UserBaseInfo::descriptor();
    const KERNEL_NS::LibString &userIdName = descriptor->FindFieldByNumber(UserBaseInfo::kUserIdFieldNumber)->name();
    selectBuilder->DB(mysqlMgr->GetCurrentServiceDbName()).From(storageInfo->GetTableName())
    .Where(KERNEL_NS::LibString().AppendFormat("`%s` = ?", userIdName.c_str()));
    std::vector<KERNEL_NS::Field *> fields;
    fields.resize(1);
    KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName(), userIdName, MYSQL_TYPE_LONGLONG, 0);
    v->Write(&userId, static_cast<Int64>(sizeof(userId)));
    fields[0] = v;

    std::vector<KERNEL_NS::MysqlSqlBuilderInfo *> builders;
    auto newMysqlBuilder = KERNEL_NS::MysqlSqlBuilderInfo::Create();
    newMysqlBuilder->_builder = selectBuilder;
    newMysqlBuilder->_fields = fields;
    builders.push_back(newMysqlBuilder);
    auto err = mysqlMgr->NewRequestBy(newPendingInfo->_stub, mysqlMgr->GetCurrentServiceDbName(), dbOid, builders, this, &UserMgr::_OnDbUserLoaded);
    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d, userId:%llu"), err, userId);
        _RemovePendingInfo(userId);
        return err;
    }

    return Status::Success;
}

Int32 UserMgr::LoadUser(const KERNEL_NS::LibString &accountName, KERNEL_NS::SmartPtr<PendingUser, KERNEL_NS::AutoDelMethods::CustomDelete> &pendingUser)
{
    // 已经Load过
    auto pendingInfo = _GetPendingByAccount(accountName);
    if(UNLIKELY(pendingInfo))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user pending is already exists, accountName:%s, exists pending:%s, new pending:%s")
            , accountName.c_str(), pendingInfo->ToString().c_str(), pendingUser->ToString().c_str());

        return Status::Repeat;
    }

    // 补全信息
    pendingUser->_byAccountName = accountName;
    auto mysqlMgr = GetGlobalSys<IMysqlMgr>();
    auto dbOid = mysqlMgr->NewDbOperatorId();
    pendingUser->_dbOperatorId = dbOid;
    auto stubMgr = GetGlobalSys<IStubHandleMgr>();
    pendingUser->_stub = stubMgr->NewStub();

    // 添加到pending字典
    _AddUserPendingInfo(accountName, pendingUser);

    // 查询数据
    auto storageInfo = GetStorageInfo();
    KERNEL_NS::SelectSqlBuilder *selectBuilder = KERNEL_NS::SelectSqlBuilder::NewThreadLocal_SelectSqlBuilder();
    auto descriptor = UserBaseInfo::descriptor();
    const KERNEL_NS::LibString &accountNameName = descriptor->FindFieldByNumber(UserBaseInfo::kAccountNameFieldNumber)->name();
    selectBuilder->DB(mysqlMgr->GetCurrentServiceDbName()).From(storageInfo->GetTableName())
    .Where(KERNEL_NS::LibString().AppendFormat("`%s` like ?", accountNameName.c_str()));
    std::vector<KERNEL_NS::Field *> fields;
    fields.resize(1);
    KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName(), accountNameName, MYSQL_TYPE_STRING, 0);
    v->Write(accountName.data(), static_cast<Int64>(accountName.size()));
    fields[0] = v;

    std::vector<KERNEL_NS::MysqlSqlBuilderInfo *> builders;
    auto newMysqlBuilder = KERNEL_NS::MysqlSqlBuilderInfo::Create();
    newMysqlBuilder->_builder = selectBuilder;
    newMysqlBuilder->_fields = fields;
    builders.push_back(newMysqlBuilder);
    auto err = mysqlMgr->NewRequestBy(pendingUser->_stub, mysqlMgr->GetCurrentServiceDbName(), dbOid, builders, this, &UserMgr::_OnDbUserLoaded);
    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d, accountName:%s"), err, accountName.c_str());
        _RemovePendingInfo(accountName);
        return err;
    }

    return Status::Success;
}

Int32 UserMgr::CheckAuthRole(const RoleAuthConfig &config, const PendingUser *pendingUser) const
{
    // 注册登录: pwd校验, 账号校验, 登录设备校验, 密文校验
    if(!pendingUser->_loginInfo)
    {// TODO:需要外部校验
        g_Log->Warn(LOGFMT_OBJ_TAG("have no login info user:%s, accountName:%s"), pendingUser->ToString().c_str(), config._id.c_str());
        return Status::ParamError;
    }

    auto &rsa = GetRsa();
    auto &loginInfo = *pendingUser->_loginInfo;
    if(loginInfo.cyphertext().empty())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("cypher test is empty user:%s, accountName:%s"), pendingUser->ToString().c_str(), config._id.c_str());
        return Status::Failed;
    }

    const auto &cypherRaw = KERNEL_NS::LibBase64::Decode(loginInfo.cyphertext());
    if(cypherRaw.empty())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("cypher test is empty user:%s, accountName:%s"), pendingUser->ToString().c_str(), config._id.c_str());
        return Status::Failed;
    }
    KERNEL_NS::LibString textRaw;
    rsa.PrivateKeyDecrypt(cypherRaw, textRaw);
    if(textRaw.empty())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("client not no author user:%s, accountName:%s"), pendingUser->ToString().c_str(), config._id.c_str());
        return Status::Failed;
    }
    if(textRaw.GetRaw() != loginInfo.origintext())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("client not no author user:%s, accountName:%s"), pendingUser->ToString().c_str(), config._id.c_str());
        return Status::Failed;
    }

    // TODO:
    const auto &nowTime = KERNEL_NS::LibTime::Now();
    {// 校验密码
        if(pendingUser->_loginInfo->pwd().empty())
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("pwd invalid user:%s, accountName:%s"), pendingUser->ToString().c_str(), config._id.c_str());
            return Status::InvalidPwd;
        }

        // 还原密码
        const auto cypherPwd = KERNEL_NS::LibBase64::Decode(pendingUser->_loginInfo->pwd());
        KERNEL_NS::LibString pwd;
        rsa.PrivateKeyDecrypt(cypherPwd, pwd);
        if(pwd.empty())
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("pwd empty user:%s, accountName:%s"), pendingUser->ToString().c_str(), config._id.c_str());
            return Status::InvalidPwd;
        }

        if (config._pwd != pwd)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("pwd invalid user:%s, accountName:%s"), pendingUser->ToString().c_str(), config._id.c_str());
            return Status::InvalidPwd;
        }
    }

    return Status::Success;
}

Int32 UserMgr::AuthRoleLogin(const RoleAuthConfig &config, IUser *user, KERNEL_NS::SmartPtr<PendingUser, KERNEL_NS::AutoDelMethods::CustomDelete> &pendingUser)
{
    auto st = CheckAuthRole(config, pendingUser);
    if (st != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("CheckAuthRole fail, accountName:%s, new pending:%s")
    , config._id.c_str(), pendingUser->ToString().c_str());

        if (pendingUser->_cb)
        {
            pendingUser->_cb->Invoke(st, pendingUser, user, pendingUser->_var);
        }
        return st;
    }

    if(pendingUser->_sessionId == user->GetSessionId())
        g_Log->Warn(LOGFMT_OBJ_TAG("user repeate login in same session user:%s"), user->ToString().c_str());

    // 不同会话顶号
    if(user->IsLogined())
        user->Logout(LogoutReason::LOGIN_OTHER_PLACE, true, pendingUser->_sessionId);

    user->BindSession(pendingUser->_sessionId);
    user->OnLogin();
    user->OnLoginFinish();

    if (pendingUser->_cb)
    {
        pendingUser->_cb->Invoke(Status::Success, pendingUser, user, pendingUser->_var);
    }

    auto loginInfo = pendingUser->_loginInfo;
    auto baseInfo = user->GetUserBaseInfo();
    auto addr = user->GetUserAddr();
    baseInfo->set_lastlogintime(KERNEL_NS::LibTime::Now().GetMilliTimestamp());
    baseInfo->set_lastloginphoneimei(loginInfo->loginphoneimei());
    baseInfo->set_lastloginip(addr->_ip.GetRaw());

    _RemoveFromLru(user);
    user->UpdateLrtTime();
    _AddToLru(user);

    if(g_Log->IsEnable(KERNEL_NS::LogLevel::Info))
        g_Log->Info(LOGFMT_OBJ_TAG("AuthRoleLogin user already online user :%s, pending:%s"), user->ToString().c_str(), pendingUser->ToString().c_str());

    return Status::Success;
}

Int32 UserMgr::AuthRoleLogin(const RoleAuthConfig &config, KERNEL_NS::SmartPtr<PendingUser, KERNEL_NS::AutoDelMethods::CustomDelete> &pendingUser)
{
    // 补全信息
    pendingUser->_byAccountName = config._id;
    auto stubMgr = GetGlobalSys<IStubHandleMgr>();
    pendingUser->_stub = stubMgr->NewStub();

    auto st = CheckAuthRole(config, pendingUser);
    if (st != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("CheckAuthRole fail, accountName:%s, exists pending:%s, new pending:%s")
    , config._id.c_str(), pendingUser->ToString().c_str(), pendingUser->ToString().c_str());

        if(LIKELY(pendingUser->_cb))
        {
            pendingUser->_cb->Invoke(st, pendingUser, NULL, pendingUser->_var);
        }
        return st;
    }

    IUser *user = NULL;
    Int32 err = Status::Success;
     do
     {
         KERNEL_NS::SmartPtr<User, KERNEL_NS::AutoDelMethods::CustomDelete> u = User::NewThreadLocal_User(this);
         u.SetClosureDelegate([](void *p)
         {
             auto ptr = reinterpret_cast<User *>(p);

             ptr->WillClose();
             ptr->Close();

             User::DeleteThreadLocal_User(ptr);
         });

         u->SetUserStatus(UserStatus::USER_INITING);
         err = u->Init();
         if(err != Status::Success)
         {
             g_Log->Warn(LOGFMT_OBJ_TAG("user init fail err:%d, pending info:%s"), err, pendingUser->ToString().c_str());
             break;
         }
         u->SetUserStatus(UserStatus::USER_INITED);

         u->SetUserStatus(UserStatus::USER_STARTING);
         err = u->Start();
         if(err != Status::Success)
         {
             g_Log->Warn(LOGFMT_OBJ_TAG("user init fail err:%d, pending info:%s"), err, pendingUser->ToString().c_str());
             break;
         }
         u->SetUserStatus(UserStatus::USER_STARTED);

         u->SetUserStatus(UserStatus::USER_ONLOADING);
         KERNEL_NS::SmartPtr<UserBaseInfoFactory, KERNEL_NS::AutoDelMethods::Release> userBaseInfoFactory = UserBaseInfoFactory::CreateFactory();
         auto userInfo = dynamic_cast<UserBaseInfo *>(userBaseInfoFactory->Create());
         userInfo->set_userid(KERNEL_NS::TlsUtil::GetIdGenerator()->NewId());
         userInfo->set_accountname(config._id.GetRaw());
         userInfo->set_name(config._id.GetRaw());
         userInfo->set_nickname(config._id.GetRaw());
         u->SetUserBaseInfo(userInfo);
         u->SetUserStatus(UserStatus::USER_ONLOADED);

         _AddUser(u.AsSelf());
         user = u.pop();
     }
     while (false);

    if(LIKELY(user))
    {
        auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::USER_OBJ_CREATED);
        ev->SetParam(Params::USER_OBJ, user);
        GetEventMgr()->FireEvent(ev);
        
        _RemoveFromLru(user);
        user->UpdateLrtTime();
        _AddToLru(user);
    }

    if((!user) || (err != Status::Success))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user fail err:%d, pending info:%s"), err, pendingUser->ToString().c_str());

        if(LIKELY(pendingUser->_cb))
        {
            pendingUser->_cb->Invoke(err, pendingUser, NULL, pendingUser->_var);
        }

        _RemovePendingInfo(pendingUser->_byAccountName);
        _RemovePendingInfo(pendingUser->_byUserId);

        return err;
    }

    if(pendingUser->_sessionId)
        user->BindSession(pendingUser->_sessionId);

    // 创建对象时调用
    user->OnUserObjCreated();

    // 登录
    if(pendingUser->_sessionId)
    {
        user->OnLogin();
        user->OnLoginFinish();
    }

    auto baseInfo = user->GetUserBaseInfo();

    // 最后登录信息
    if(pendingUser->_sessionId)
    {
        auto loginInfo = pendingUser->_loginInfo;
        auto addr = user->GetUserAddr();
        const auto &nowTime = KERNEL_NS::LibTime::Now();
        baseInfo->set_lastlogintime(nowTime.GetMilliTimestamp());
        baseInfo->set_lastloginphoneimei(loginInfo->loginphoneimei());
        baseInfo->set_lastloginip(addr->_ip.GetRaw());
        baseInfo->set_createip(addr->_ip.GetRaw());
        baseInfo->set_createtime(nowTime.GetMilliTimestamp());
    }

    // 登录事件
    if(pendingUser->_sessionId)
    {
        auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::USER_LOGIN);
        ev->SetParam(Params::USER_OBJ, user);
        GetEventMgr()->FireEvent(ev);
    }

    // 处理回调
    if(LIKELY(pendingUser->_cb))
    {
        pendingUser->_cb->Invoke(Status::Success, pendingUser, user, pendingUser->_var);
    }

    _RemovePendingInfo(pendingUser->_byAccountName);
    _RemovePendingInfo(pendingUser->_byUserId);

    // lru操作
    _LruPopUser();

    if(g_Log->IsEnable(KERNEL_NS::LogLevel::Info))
        g_Log->Info(LOGFMT_OBJ_TAG("AuthRoleLogin user login success user :%s, pending:%s"), user->ToString().c_str(), pendingUser->ToString().c_str());
    
    return Status::Success;
}

const RoleAuthConfig *UserMgr::CetAuthRoleConfig(const KERNEL_NS::LibString &accountName) const
{
    auto configLoader = GetService()->GetComp<ConfigLoader>();
    auto roleAuthMgr = configLoader->GetComp<RoleAuthConfigMgr>();
    return roleAuthMgr->GetConfigById(accountName);  
}


bool UserMgr::IsPhoneNumberBinded(const IUser *operateUser, UInt64 phoneNubmer, const std::set<UInt64> &excludeUserIds, bool &hasBindPhone) const
{
    hasBindPhone = false;
    for(auto &iter : _userIdRefUser)
    {
        auto user = iter.second;
        if(excludeUserIds.find(user->GetUserId()) != excludeUserIds.end())
            continue;

        if(iter.second->GetUserBaseInfo()->bindphone() == phoneNubmer)
        {
            hasBindPhone = true;
            return true;
        }
    }

    // 查询用户
    auto userStorage = GetComp<UserMgrStorage>();
    auto service = ILogicSys::GetCurrentService();
    auto mysqlMgr = service->GetComp<IMysqlMgr>();
    auto descriptor = UserBaseInfo::descriptor();
    const auto &bindPhnoneName = descriptor->FindFieldByNumber(UserBaseInfo::kBindPhoneFieldNumber)->name();
    const auto &userIdName = descriptor->FindFieldByNumber(UserBaseInfo::kUserIdFieldNumber)->name();
    const auto &excludeUserIdsStr = KERNEL_NS::StringUtil::ToString(excludeUserIds, ",");

    KERNEL_NS::SelectSqlBuilder *builder = KERNEL_NS::SelectSqlBuilder::NewThreadLocal_SelectSqlBuilder();
    builder->DB(mysqlMgr->GetCurrentServiceDbName()).From(userStorage->GetTableName())
    .Where(KERNEL_NS::LibString().AppendFormat("`%s` = %llu and `%s` NOT IN (%s)"
    , bindPhnoneName.c_str(), phoneNubmer, userIdName.c_str(), excludeUserIdsStr.c_str()));

    UInt64 stub = 0;
    Int32 err = Status::Success;
    Int32 opId = operateUser ? operateUser->GetStorageOperatorId() : mysqlMgr->GetStorageOperatorId();
    std::vector<KERNEL_NS::MysqlSqlBuilderInfo *> builders;
    auto newMysqlBuilder = KERNEL_NS::MysqlSqlBuilderInfo::Create();
    newMysqlBuilder->_builder = builder;
    builders.push_back(newMysqlBuilder);
    err = mysqlMgr->NewRequestAndWaitResponseBy2(stub, mysqlMgr->GetCurrentServiceDbName(), opId, builders,
    [phoneNubmer, &err, &hasBindPhone](KERNEL_NS::MysqlResponse *res){
        if(res->_errCode != Status::Success)
        {
            g_Log->Error(LOGFMT_NON_OBJ_TAG(UserMgr, "NewRequestAndWaitResponseBy2 fail db name:%s res seqId:%llu, mysqlError:%u, bind phone:%llu")
                    , res->_dbName.c_str(), res->_seqId, res->_mysqlErrno, phoneNubmer);
            err = res->_errCode;
            return;
        }

        hasBindPhone = !res->_datas.empty();
    });

    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(UserMgr,"NewRequestBy fail err:%d"), err);
        return false;
    }

    return true;
}

bool UserMgr::IsBindedPhone(UInt64 userId) const
{
    auto user = GetUser(userId);
    if(user)
    {
        return user->HasBindPhone();
    }

    // 查询用户
    auto userStorage = GetComp<UserMgrStorage>();
    auto service = ILogicSys::GetCurrentService();
    auto mysqlMgr = service->GetComp<IMysqlMgr>();
    auto descriptor = UserBaseInfo::descriptor();
    const auto &bindPhnoneName = descriptor->FindFieldByNumber(UserBaseInfo::kBindPhoneFieldNumber)->name();
    const auto &userIdName = descriptor->FindFieldByNumber(UserBaseInfo::kUserIdFieldNumber)->name();

    KERNEL_NS::SelectSqlBuilder *builder = KERNEL_NS::SelectSqlBuilder::NewThreadLocal_SelectSqlBuilder();
    builder->DB(mysqlMgr->GetCurrentServiceDbName()).From(userStorage->GetTableName())
    .Where(KERNEL_NS::LibString().AppendFormat("`%s` = ? and `%s` != ?"
    , userIdName.c_str(), bindPhnoneName.c_str()));

    std::vector<KERNEL_NS::Field *> fields;
    fields.resize(2);
    {
        KERNEL_NS::Field *v = KERNEL_NS::Field::Create(userStorage->GetTableName(), userIdName, MYSQL_TYPE_LONGLONG, 0);
        v->Write(&userId, static_cast<Int64>(sizeof(userId)));
        fields[0] = v;
    }
    {
        KERNEL_NS::Field *v = KERNEL_NS::Field::Create(userStorage->GetTableName(), bindPhnoneName, MYSQL_TYPE_LONGLONG, 0);
        UInt64 value = 0;
        v->Write(&value, static_cast<Int64>(sizeof(value)));
        fields[1] = v;
    }

    UInt64 stub = 0;
    Int32 err = Status::Success;
    Int32 opId = mysqlMgr->GetStorageOperatorId();
    std::vector<KERNEL_NS::MysqlSqlBuilderInfo *> builders;
    auto newMysqlBuilder = KERNEL_NS::MysqlSqlBuilderInfo::Create();
    newMysqlBuilder->_builder = builder;
    newMysqlBuilder->_fields = fields;
    builders.push_back(newMysqlBuilder);
    bool hasBindPhone = false;
    err = mysqlMgr->NewRequestAndWaitResponseBy2(stub, mysqlMgr->GetCurrentServiceDbName(), opId, builders,
    [&err, &hasBindPhone](KERNEL_NS::MysqlResponse *res){
        if(res->_errCode != Status::Success)
        {
            g_Log->Error(LOGFMT_NON_OBJ_TAG(UserMgr, "NewRequestAndWaitResponseBy2 fail db name:%s res seqId:%llu, mysqlError:%u")
                    , res->_dbName.c_str(), res->_seqId, res->_mysqlErrno);
            err = res->_errCode;
            return;
        }

        hasBindPhone = !res->_datas.empty();
    });

    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d"), err);
        return false;
    }

    return hasBindPhone;
}

void UserMgr::Purge()
{
    auto dbMgr = GetGlobalSys<IMysqlMgr>();
    if(dbMgr)
        dbMgr->Purge(this);
}

void UserMgr::PurgeAndWait()
{
    auto dbMgr = GetGlobalSys<IMysqlMgr>();
    if(dbMgr)
        dbMgr->PurgeAndWaitComplete(this);
}

void UserMgr::PurgeEndWith(KERNEL_NS::IDelegate<void, Int32> *handler)
{
    auto dbMgr = GetGlobalSys<IMysqlMgr>();
    if(dbMgr)
        dbMgr->PurgeEndWith(handler);
}

void UserMgr::RemoveUserBySessionId(UInt64 sessionId)
{
    _sessionIdRefUser.erase(sessionId);
}

void UserMgr::AddUserBySessionId(UInt64 sessionId, IUser *user)
{
    auto iter = _sessionIdRefUser.find(sessionId);
    if(UNLIKELY(iter != _sessionIdRefUser.end()))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("duplicate session id:%llu with user, exists user:%s, new user:%s"), sessionId, iter->second->ToString().c_str(), user->ToString().c_str());
        iter->second = user;
        return;
    }

    _sessionIdRefUser.insert(std::make_pair(sessionId, user));
}

KERNEL_NS::LibRsa &UserMgr::GetRsa() const
{
    return _rsa;
}

void UserMgr::OnPassDay(const KERNEL_NS::LibTime &nowTime)
{
    for(auto iter : _userIdRefUser)
        iter.second->OnPassDay(nowTime);
}

void UserMgr::OnPassWeek(const KERNEL_NS::LibTime &nowTime)
{
    for(auto iter : _userIdRefUser)
        iter.second->OnPassWeek(nowTime);
}

void UserMgr::OnPassMonth(const KERNEL_NS::LibTime &nowTime)
{
    for(auto iter : _userIdRefUser)
        iter.second->OnPassMonth(nowTime);
}

void UserMgr::OnPassYear(const KERNEL_NS::LibTime &nowTime)
{
    for(auto iter : _userIdRefUser)
        iter.second->OnPassYear(nowTime);
}

void UserMgr::OnPassTimeEnd(const KERNEL_NS::LibTime &nowTime)
{
    for(auto iter : _userIdRefUser)
        iter.second->OnPassTimeEnd(nowTime);
}

Int32 UserMgr::_OnGlobalSysInit()
{
    auto ini = GetApp()->GetIni();
    if(!ini->CheckReadNumber(GetService()->GetServiceName().c_str(), "UserLruCapacityLimit", _lruCapacityLimit))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("lack of %s:UserLruCapacityLimit config in ini:%s"), GetService()->GetServiceName().c_str(), ini->GetPath().c_str());
        return Status::Failed;
    }

    if(!ini->ReadStr(GetService()->GetServiceName().c_str(), "RsaPrivateKey", _rsaPrivateKey))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("lack of %s:RsaPrivateKey config in ini:%s"), GetService()->GetServiceName().c_str(), ini->GetPath().c_str());
        return Status::Failed;
    }
    if(!ini->ReadStr(GetService()->GetServiceName().c_str(), "RsaPublicKey", _rsaPubKey))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("lack of %s:RsaPublicKey config in ini:%s"), GetService()->GetServiceName().c_str(), ini->GetPath().c_str());
        return Status::Failed;
    }
    _rsaPrivateKey.strip();
    _rsaPubKey.strip();

    _rsaPrivateKeyRaw = KERNEL_NS::LibBase64::Decode(_rsaPrivateKey);
    _rsaPubKeyRaw = KERNEL_NS::LibBase64::Decode(_rsaPubKey);
    if(!_rsa.ImportKey(&_rsaPubKeyRaw, &_rsaPrivateKeyRaw, KERNEL_NS::LibRsa::PUB_PKC8_FLAG))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("ImportKey fail _rsaPrivateKey:%s, _rsaPubKey:%s")
        , _rsaPrivateKey.c_str(), _rsaPubKey.c_str());
        return Status::Failed;
    }

    auto commonMgr = GetService()->GetComp<ConfigLoader>()->GetComp<CommonConfigMgr>();
    _lruCapacityLimit = commonMgr->GetConfigById(CommonConfigIdEnums::USER_LRU_CAPACITY_LIMIT)->_value;

    GetService()->Subscribe(Opcodes::OpcodeConst::OPCODE_LoginReq, this, &UserMgr::_OnClientLoginReq);
    GetService()->Subscribe(Opcodes::OpcodeConst::OPCODE_LogoutReq, this, &UserMgr::_OnClientLogoutReq);
    GetService()->Subscribe(Opcodes::OpcodeConst::OPCODE_LoginFinishReq, this, &UserMgr::_OnLoginFinishReq);

    return Status::Success;
}

Int32 UserMgr::_OnGlobalSysCompsCreated()
{
    // 用户不需要起服加载
    auto storageInfo = GetStorageInfo();
    if(LIKELY(storageInfo))
        storageInfo->ClearFlags(StorageFlagType::LOAD_DATA_ON_STARTUP_FLAG);
        
    return Status::Success;
}

void UserMgr::_OnHostWillClose()
{
    auto users = _lru;
    for(auto user : users)
        user->WillClose();
}

void UserMgr::_OnGlobalSysClose()
{
    auto users = _lru;
    for(auto user : users)
        user->Close();

    _Clear();
}

void UserMgr::_OnQuitServiceEventDefault(KERNEL_NS::LibEvent *ev)
{
    auto allUsers = _lru;
    ev->SetDontDelAfterFire(true);
    
    for(auto user : allUsers)
        user->Logout(LogoutReason::CLOSE_SERVER);

    ev->SetDontDelAfterFire(false);
    IGlobalSys::_OnQuitServiceEventDefault(ev);
}


void UserMgr::_AddUserPendingInfo(UInt64 userId, KERNEL_NS::SmartPtr<PendingUser, KERNEL_NS::AutoDelMethods::CustomDelete> &pendingInfo)
{
    _userIdRefPendingUser.insert(std::make_pair(userId, pendingInfo));
    _stubRefPendingUser.insert(std::make_pair(pendingInfo->_stub, pendingInfo));
    if(pendingInfo->_sessionId)
        _loginPendingSessions.insert(pendingInfo->_sessionId);
}

void UserMgr::_AddUserPendingInfo(const KERNEL_NS::LibString &accountName, KERNEL_NS::SmartPtr<PendingUser, KERNEL_NS::AutoDelMethods::CustomDelete> &pendingInfo)
{
    _accountNameRefPendingUser.insert(std::make_pair(accountName, pendingInfo));
    _stubRefPendingUser.insert(std::make_pair(pendingInfo->_stub, pendingInfo));
    if(pendingInfo->_sessionId)
        _loginPendingSessions.insert(pendingInfo->_sessionId);
}

void UserMgr::_RemovePendingInfo(const KERNEL_NS::LibString &accountName)
{
    auto iter = _accountNameRefPendingUser.find(accountName);
    if(UNLIKELY(iter == _accountNameRefPendingUser.end()))
        return;

    if(iter->second->_sessionId)
        _loginPendingSessions.erase(iter->second->_sessionId);
    _stubRefPendingUser.erase(iter->second->_stub);
    _accountNameRefPendingUser.erase(iter);
}

void UserMgr::_RemovePendingInfo(UInt64 userId)
{
    auto iter = _userIdRefPendingUser.find(userId);
    if(iter == _userIdRefPendingUser.end())
        return;

    if(iter->second->_sessionId)
        _loginPendingSessions.erase(iter->second->_sessionId);

    _stubRefPendingUser.erase(iter->second->_stub);
    _userIdRefPendingUser.erase(iter);
}

void UserMgr::_AddUser(User *user)
{
    _userIdRefUser.insert(std::make_pair(user->GetUserId(), user));
    _accountNameRefUser.insert(std::make_pair(user->GetUserBaseInfo()->accountname(), user));
}

void UserMgr::_RemoveUser(User *user)
{
    _userIdRefUser.erase(user->GetUserId());
    _accountNameRefUser.erase(user->GetUserBaseInfo()->accountname());
    _RemoveFromLru(user);
}

void UserMgr::_RemoveFromLru(IUser *user)
{
    _lru.erase(user);
}

void UserMgr::_AddToLru(IUser *user)
{
    _lru.insert(user);
}

void UserMgr::_LruPopUser()
{
    for(;static_cast<Int32>(_lru.size()) > _lruCapacityLimit;)
    {
        auto firstNode = _lru.begin();
        auto expiredlUser = (*firstNode)->CastTo<User>();
        _lru.erase(firstNode);

        // 踢掉
        if(expiredlUser->IsLogined())
            expiredlUser->Logout(LogoutReason::USER_IDLE);

        auto mysqlMgr = GetGlobalSys<IMysqlMgr>();
        if(mysqlMgr)
            mysqlMgr->PurgeAndWaitComplete(this);

        // 即将移除user
        auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::USER_WILL_REMOVE);
        ev->SetParam(Params::USER_ID, expiredlUser->GetUserId());
        GetEventMgr()->FireEvent(ev);

        _RemoveUser(expiredlUser);

        expiredlUser->WillClose();
        expiredlUser->Close();
        User::DeleteThreadLocal_User(expiredlUser);
    }
    
}

void UserMgr::_OnDbUserLoaded(KERNEL_NS::MysqlResponse *res)
{
    if(g_Log->IsEnable(KERNEL_NS::LogLevel::Info))
        g_Log->Info(LOGFMT_OBJ_TAG("mysql res:%s"), res->ToString().c_str());
    const auto &curTime = KERNEL_NS::LibTime::Now(); 

    // 可能之前有load过user导致pengding 一起处理
    auto pendingUser = _GetPendingByStub(res->_stub);
    if(!pendingUser)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("pending user is handled before get result stub:%llu, err:%d"), res->_stub, res->_errCode);
        return;
    }

    // 判断session是否存在
    if(GetGlobalSys<ISessionMgr>()->GetSession(pendingUser->_sessionId) == NULL)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("session is disconnected before user loaded session id:%llu, pending user:%s")
        , pendingUser->_sessionId, pendingUser->ToString().c_str());
        pendingUser->_sessionId = 0;
    }

    // 再次确认是否有User
    IUser *user = NULL;
    if(pendingUser->_byAccountName.empty())
    {
        user = GetUser(pendingUser->_byAccountName);
    }
    else
    {
        user = GetUser(pendingUser->_byUserId);
    }

    // user已经被提前load回来
    if(user)
    {
        if(g_Log->IsEnable(KERNEL_NS::LogLevel::Info))
            g_Log->Info(LOGFMT_OBJ_TAG("user is already exists user:%s, pendingUser:%s"), user->ToString().c_str(), pendingUser->ToString().c_str());
        
        _RemoveFromLru(user);
        user->UpdateLrtTime();
        _AddToLru(user);

        Int32 err = Status::Success;
        if(pendingUser->_sessionId)
        {
            // 校验登录
            auto loginMgr = user->GetComp<ILoginMgr>();
            err = loginMgr->CheckLogin(pendingUser);
            if(err != Status::Success)
            {// 登录失败
                g_Log->Warn(LOGFMT_OBJ_TAG("check login fail err:%d, pending info:%s, userId:%llu"), err, pendingUser->ToString().c_str(), user->GetUserId());
                if(LIKELY(pendingUser->_cb))
                    pendingUser->_cb->Invoke(err, pendingUser, user, pendingUser->_var);
            }
            else
            {
                // 顶号
                if(user->IsLogined())
                    user->Logout(LogoutReason::LOGIN_OTHER_PLACE, pendingUser->_sessionId != user->GetSessionId(), pendingUser->_sessionId);

                user->BindSession(pendingUser->_sessionId);
                user->OnLogin();
                user->OnLoginFinish();

                if(LIKELY(pendingUser->_cb))
                    pendingUser->_cb->Invoke(err, pendingUser, user, pendingUser->_var);

                auto loginInfo = pendingUser->_loginInfo;
                auto baseInfo = user->GetUserBaseInfo();
                auto addr = user->GetUserAddr();
                baseInfo->set_lastlogintime(curTime.GetMilliTimestamp());
                baseInfo->set_lastloginphoneimei(loginInfo->loginphoneimei());
                baseInfo->set_lastloginip(addr->_ip.GetRaw());
                user->MaskDirty();
            }
        }
        else
        {
            if(LIKELY(pendingUser->_cb))
                pendingUser->_cb->Invoke(err, pendingUser, user, pendingUser->_var);
        }

        if(!pendingUser->_byAccountName.empty())
        {
            _RemovePendingInfo(pendingUser->_byAccountName);
        }
        else
        {
            _RemovePendingInfo(pendingUser->_byUserId);
        }

        // 处理其他回调
        {
            auto pendingByAccount = _GetPendingByAccount(user->GetUserBaseInfo()->accountname());
            if(UNLIKELY(pendingByAccount))
            {
                if(pendingByAccount->_cb)
                    pendingByAccount->_cb->Invoke(Status::Success, pendingByAccount, user, pendingByAccount->_var);
            
                _RemovePendingInfo(user->GetUserBaseInfo()->accountname());
            }
        }
        {
            auto pendingByAccount = _GetPendingByUserId(user->GetUserId());
            if(UNLIKELY(pendingByAccount))
            {
                if(pendingByAccount->_cb)
                    pendingByAccount->_cb->Invoke(Status::Success, pendingByAccount, user, pendingByAccount->_var);
            
                _RemovePendingInfo(user->GetUserId());
            }
        }

        return;
    }

    // load失败
    if(res->_errCode != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("load user fail pendingUser:%s"), pendingUser->ToString().c_str());
        if(LIKELY(pendingUser->_cb))
        {
            pendingUser->_cb->Invoke(res->_errCode, pendingUser, NULL, pendingUser->_var);
        }

        if(!pendingUser->_byAccountName.empty())
        {
            _RemovePendingInfo(pendingUser->_byAccountName);
        }
        else
        {
            _RemovePendingInfo(pendingUser->_byUserId);
        }

        return;
    }

    // 没有user, 判断是否是注册
    Int32 err = Status::Success;
    bool isAlreadyExists = false;
    do
    {
        if(res->_datas.empty())
        {
            // 是否有登录信息
            if(!pendingUser->_loginInfo)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("user is not created before pending info:%s"), pendingUser->ToString().c_str());
                err = Status::UserNotCreatedBefore;
                break;
            }

            // 不是注册则返回错误
            if(pendingUser->_loginInfo->loginmode() != LoginMode::REGISTER)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("user is not created before pending info:%s"), pendingUser->ToString().c_str());
                err = Status::UserNotCreatedBefore;
                break;
            }

            // 没有注册信息报错
            if(!pendingUser->_loginInfo->has_userregisterinfo())
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("user have no register info pending info:%s"), pendingUser->ToString().c_str());
                err = Status::HaveNoRegisterInfo;
                break;
            }

            // 有昵称就urldecode
            // if(!pendingUser->_loginInfo->userregisterinfo().nickname().empty())
            // {
            //     KERNEL_NS::LibString decodedName;
            //     if(!KERNEL_NS::UrlCoder::Decode(pendingUser->_loginInfo->userregisterinfo().nickname(), decodedName))
            //     {
            //         g_Log->Warn(LOGFMT_OBJ_TAG("urldecoded fail login info:%s, pending user:%s"), pendingUser->_loginInfo->ToJsonString().c_str(), pendingUser->ToString().c_str());
            //         err = Status::InvalidNickname;
            //         break;
            //     }
            //     pendingUser->_loginInfo->mutable_userregisterinfo()->set_nickname(decodedName.GetRaw());
            // }

            // 校验注册信息
            err = _CheckRegisterInfo(*(pendingUser->_loginInfo));
            if(err != Status::Success)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("check regiseter info fail err:%d pending info:%s"), err, pendingUser->ToString().c_str());
                break;
            }

            // 自动生成昵称
            auto nicknameGlobal = GetGlobalSys<INicknameGlobal>();
            if(pendingUser->_loginInfo->userregisterinfo().nickname().empty())
            {
                KERNEL_NS::LibString nickname;
                nicknameGlobal->GenRandNickname(nickname);
                pendingUser->_loginInfo->mutable_userregisterinfo()->set_nickname(nickname.GetRaw());
            }
            else
            {
                nicknameGlobal->AddUsedNickname(pendingUser->_loginInfo->userregisterinfo().nickname());
            }
            
            // 是注册则创建user账号
            KERNEL_NS::SmartPtr<User, KERNEL_NS::AutoDelMethods::CustomDelete> u = User::NewThreadLocal_User(this);
            u.SetClosureDelegate([](void *p)
            {
                auto ptr = reinterpret_cast<User *>(p);

                ptr->WillClose();
                ptr->Close();

                User::DeleteThreadLocal_User(ptr);
            });

            u->SetStorageOperatorId(pendingUser->_dbOperatorId);

            // TODO:全球唯一id
            auto &registerInfo = pendingUser->_loginInfo->userregisterinfo();
            auto baseInfo = u->GetUserBaseInfo();
            baseInfo->set_accountname(registerInfo.accountname());
            baseInfo->set_nickname(registerInfo.nickname());
            _BuildPwd(baseInfo, registerInfo.pwd());
            baseInfo->set_createtime(curTime.GetMilliTimestamp());
            baseInfo->set_createphoneimei(registerInfo.createphoneimei());

            auto globalUidMgr = GetService()->GetComp<IGlobalUidMgr>();
            baseInfo->set_userid(globalUidMgr->NewGuid());

            // token = 收集imei + ip + userId
            // baseInfo->set_logintoken()

            // TODO:
            // baseInfo->set_userid();
            // baseInfo->set_lastloginip()
            // baseInfo->set_createip()

            u->SetUserStatus(UserStatus::USER_INITING);
            err = u->Init();
            if(err != Status::Success)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("user init fail err:%d, pending info:%s"), err, pendingUser->ToString().c_str());
                break;
            }
            u->SetUserStatus(UserStatus::USER_INITED);

            u->SetUserStatus(UserStatus::USER_STARTING);
            err = u->Start();
            if(err != Status::Success)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("user init fail err:%d, pending info:%s"), err, pendingUser->ToString().c_str());
                break;
            }
            u->SetUserStatus(UserStatus::USER_STARTED);

            _AddUser(u.AsSelf());
            user = u.pop();
            MaskNumberKeyAddDirty(user->GetUserId());

            user->SetUserStatus(UserStatus::USER_ONLOADING);
            auto dbMgr = GetGlobalSys<IMysqlMgr>();
            if(dbMgr)
                dbMgr->PurgeAndWaitComplete(this);

            user->SetUserStatus(UserStatus::USER_ONLOADED);
        }
        else
        {
            isAlreadyExists = true;
            KERNEL_NS::SmartPtr<User, KERNEL_NS::AutoDelMethods::CustomDelete> u = User::NewThreadLocal_User(this);
            u.SetClosureDelegate([](void *p)
            {
                auto ptr = reinterpret_cast<User *>(p);

                ptr->WillClose();
                ptr->Close();

                User::DeleteThreadLocal_User(ptr);
            });

            u->SetStorageOperatorId(pendingUser->_dbOperatorId);

            u->SetUserStatus(UserStatus::USER_INITING);
            err = u->Init();
            if(err != Status::Success)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("user init fail err:%d, pending info:%s"), err, pendingUser->ToString().c_str());
                break;
            }
            u->SetUserStatus(UserStatus::USER_INITED);

            u->SetUserStatus(UserStatus::USER_STARTING);
            err = u->Start();
            if(err != Status::Success)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("user init fail err:%d, pending info:%s"), err, pendingUser->ToString().c_str());
                break;
            }
            u->SetUserStatus(UserStatus::USER_STARTED);

            auto &record = res->_datas[0];
            auto userId = record->GetPrimaryKey()->GetUInt64();

            u->SetUserStatus(UserStatus::USER_ONLOADING);
            err =  u->OnLoaded(userId, record->GetFieldDatas());
            if(err != Status::Success)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("user onloaded fail err:%d, pending info:%s, userId:%llu"), err, pendingUser->ToString().c_str(), userId);
                break;
            }
            u->SetUserStatus(UserStatus::USER_ONLOADED);

            _AddUser(u.AsSelf());
            user = u.pop();

            if(pendingUser->_sessionId)
            {
                // 校验登录
                auto loginMgr = user->GetComp<ILoginMgr>();
                err = loginMgr->CheckLogin(pendingUser);
                if(err != Status::Success)
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("check login fail err:%d, pending info:%s, userId:%llu"), err, pendingUser->ToString().c_str(), userId);
                    break;
                }
            }

            // token = 收集imei + ip + userId
            // baseInfo->set_logintoken()

            // TODO:
            // baseInfo->set_userid();
            // baseInfo->set_lastloginip()
            // baseInfo->set_createip()

        }
    } while (false);

    if(LIKELY(user))
    {
        auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::USER_OBJ_CREATED);
        ev->SetParam(Params::USER_OBJ, user);
        GetEventMgr()->FireEvent(ev);
        
        _RemoveFromLru(user);
        user->UpdateLrtTime();
        _AddToLru(user);
    }

    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user fail err:%d, pending info:%s"), err, pendingUser->ToString().c_str());

        if(LIKELY(pendingUser->_cb))
        {
            pendingUser->_cb->Invoke(err, pendingUser, NULL, pendingUser->_var);
        }

        if(!pendingUser->_byAccountName.empty())
        {
            _RemovePendingInfo(pendingUser->_byAccountName);
        }
        else
        {
            _RemovePendingInfo(pendingUser->_byUserId);
        }

        return;
    }

    if(pendingUser->_sessionId)
        user->BindSession(pendingUser->_sessionId);

    // 创建user
    if((!isAlreadyExists) && pendingUser->_loginInfo && (pendingUser->_loginInfo->loginmode() == LoginMode::REGISTER))
    {
        user->OnUserCreated();
    }

    // 创建对象时调用
    user->OnUserObjCreated();

    // 登录
    if(pendingUser->_sessionId)
    {
        user->OnLogin();
        user->OnLoginFinish();

        // 处理回调
        if(LIKELY(pendingUser->_cb))
        {
            if(isAlreadyExists && pendingUser->_loginInfo->loginmode() == LoginMode::REGISTER)
            {
                pendingUser->_cb->Invoke(Status::UserAllReadyExistsCantRegisterAgain, pendingUser, user, pendingUser->_var);
            }
            else
            {
                pendingUser->_cb->Invoke(Status::Success, pendingUser, user, pendingUser->_var);
            }
        }
    }

    auto baseInfo = user->GetUserBaseInfo();

    // 最后登录信息
    if(pendingUser->_sessionId)
    {
        auto loginInfo = pendingUser->_loginInfo;
        auto addr = user->GetUserAddr();
        baseInfo->set_lastlogintime(curTime.GetMilliTimestamp());
        baseInfo->set_lastloginphoneimei(loginInfo->loginphoneimei());
        baseInfo->set_lastloginip(addr->_ip.GetRaw());
    }
    
    user->MaskDirty();

    // 注册事件
    if((!isAlreadyExists) && pendingUser->_loginInfo && (pendingUser->_loginInfo->loginmode() == LoginMode::REGISTER))
    {
        auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::USER_CREATED);
        ev->SetParam(Params::USER_OBJ, user);
        GetEventMgr()->FireEvent(ev);
    }

    // 登录事件
    if(pendingUser->_sessionId)
    {
        auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::USER_LOGIN);
        ev->SetParam(Params::USER_OBJ, user);
        GetEventMgr()->FireEvent(ev);
    }
    else
    {
        // 处理回调
        if(LIKELY(pendingUser->_cb))
        {
            if(isAlreadyExists && pendingUser->_loginInfo && (pendingUser->_loginInfo->loginmode() == LoginMode::REGISTER))
            {
                pendingUser->_cb->Invoke(Status::UserAllReadyExistsCantRegisterAgain, pendingUser, user, pendingUser->_var);
            }
            else
            {
                pendingUser->_cb->Invoke(Status::Success, pendingUser, user, pendingUser->_var);
            }
        }
    }

    if(!pendingUser->_byAccountName.empty())
    {
        _RemovePendingInfo(pendingUser->_byAccountName);
    }
    else
    {
        _RemovePendingInfo(pendingUser->_byUserId);
    }

    // 处理其他回调
    {
        auto pendingByAccount = _GetPendingByAccount(user->GetUserBaseInfo()->accountname());
        if(UNLIKELY(pendingByAccount))
        {
            if(pendingByAccount->_cb)
                pendingByAccount->_cb->Invoke(Status::Success, pendingByAccount, user, pendingByAccount->_var);
        
            _RemovePendingInfo(user->GetUserBaseInfo()->accountname());
        }
    }
    {
        auto pendingByAccount = _GetPendingByUserId(user->GetUserId());
        if(UNLIKELY(pendingByAccount))
        {
            if(pendingByAccount->_cb)
                pendingByAccount->_cb->Invoke(Status::Success, pendingByAccount, user, pendingByAccount->_var);
        
            _RemovePendingInfo(user->GetUserId());
        }
    }

    // lru操作
    _LruPopUser();
}

Int32 UserMgr::_CheckRegisterInfo(const LoginInfo &loginInfo) const
{
    auto &regiseterInfo = loginInfo.userregisterinfo();
    if(!KERNEL_NS::StringUtil::IsUtf8String(regiseterInfo.accountname()))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("account name not utf8:%s"), regiseterInfo.accountname().c_str());
        return Status::InvalidChar;
    }
    if((!regiseterInfo.nickname().empty()))
    {
        if(!KERNEL_NS::StringUtil::IsUtf8String(regiseterInfo.nickname()))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("nick name not utf8:%s"), regiseterInfo.nickname().c_str());
            return Status::InvalidChar;
        }

        auto nicknameGlobal = GetGlobalSys<INicknameGlobal>();
        if(!nicknameGlobal->CheckNickname(regiseterInfo.nickname()))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("nick name not utf8:%s"), regiseterInfo.nickname().c_str());
            return Status::InvalidNickname;
        }
    }
    if(regiseterInfo.pwd().empty())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("pwd empty"));
        return Status::Failed;
    }
    if(regiseterInfo.createphoneimei().empty())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("imei empty"));
        return Status::Failed;
    }

    {// 只能英文数字 + _
       KERNEL_NS::LibString str = regiseterInfo.accountname();
        const Int32 len = static_cast<Int32>(str.size());
        for(Int32 idx = 0; idx < len; ++idx)
        {
            auto &ch = str[idx];
            if((!KERNEL_NS::LibString::isalpha(ch)) && 
                (!KERNEL_NS::LibString::isdigit(ch)) && 
                (ch != '_'))
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("account name invalid char :%s")
                , regiseterInfo.accountname().c_str());
                return Status::InvalidChar;
            }
        }
    }

    // 密码需要解成明文, 检查格式再加密成sha1
    auto pwdRaw = KERNEL_NS::LibBase64::Decode(regiseterInfo.pwd());
    if(pwdRaw.empty())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("invalid pwd empty"));
        return Status::Failed;
    }

    KERNEL_NS::LibString plainPwdText;
    _rsa.PrivateKeyDecrypt(pwdRaw, plainPwdText);
    if(UNLIKELY(plainPwdText.empty()))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("pwd empty"));
        return Status::Failed;
    }

    auto commonConfig = GetService()->GetComp<ConfigLoader>()->GetComp<CommonConfigMgr>()->GetConfigById(CommonConfigIdEnums::USER_LOGIN_KEY_CHAR_COUNT);
    auto maxKenLenCommonConfig = GetService()->GetComp<ConfigLoader>()->GetComp<CommonConfigMgr>()->GetConfigById(CommonConfigIdEnums::USER_LOGIN_KEY_CHAR_COUNT_MAX_LEN);
    if(static_cast<Int32>(plainPwdText.length()) < commonConfig->_value)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("pwd less than :%d, size:%d plainPwdText:%s")
                , commonConfig->_value, static_cast<Int32>(plainPwdText.length())
                , plainPwdText.c_str());

        return Status::Failed;
    }
    if(static_cast<Int32>(plainPwdText.length()) > maxKenLenCommonConfig->_value)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("pwd more than :%d, size:%d plainPwdText:%s")
                , maxKenLenCommonConfig->_value, static_cast<Int32>(plainPwdText.length())
                , plainPwdText.c_str());

        return Status::Failed;
    }

    if(loginInfo.cyphertext().empty())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("cypher test is empty"));
        return Status::Failed;
    }

    const auto &cypherRaw = KERNEL_NS::LibBase64::Decode(loginInfo.cyphertext());
    if(cypherRaw.empty())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("cypher test is empty"));
        return Status::Failed;
    }
    KERNEL_NS::LibString textRaw;
    _rsa.PrivateKeyDecrypt(cypherRaw, textRaw);
    if(textRaw.empty())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("client not no author"));
        return Status::Failed;
    }
    if(textRaw.GetRaw() != loginInfo.origintext())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("client not no author"));
        return Status::Failed;
    }

    auto nameMaxLenConfig = GetService()->GetComp<ConfigLoader>()->GetComp<CommonConfigMgr>()->GetConfigById(CommonConfigIdEnums::NAME_MAX_LEN);
    KERNEL_NS::LibString accountname = regiseterInfo.accountname();
    if(static_cast<Int32>(accountname.length_with_utf8()) > nameMaxLenConfig->_value)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("name len too long account name:%s"), regiseterInfo.accountname().c_str());
        return Status::NameTooLong;
    }

    // TODO: 有没有被封禁ip, 有没有被封禁设备id,得添加一个封禁模块(BanMgr)
    // 密码是否合法:至少8个字符
    // 设备imei
    return Status::Success;
}

void UserMgr::_BuildPwd(UserBaseInfo *baseInfo, const std::string &pwd)
{
    auto pwdRaw = KERNEL_NS::LibBase64::Decode(pwd);
    KERNEL_NS::LibString plainPwdText;
    _rsa.PrivateKeyDecrypt(pwdRaw, plainPwdText);

    // pwd = base64(sha256(pwd + salt))
    KERNEL_NS::LibString saltKey;
    KERNEL_NS::CypherGeneratorUtil::Gen<KERNEL_NS::_Build::TL>(saltKey, 8);
    auto encodeKey = KERNEL_NS::LibBase64::Encode(saltKey);
    baseInfo->set_pwdsalt(encodeKey.GetRaw());

    plainPwdText.GetRaw().insert(plainPwdText.size()/2, saltKey.GetRaw());
    KERNEL_NS::LibString finalPwd = KERNEL_NS::LibDigest::MakeSha256(plainPwdText);
    finalPwd = KERNEL_NS::LibBase64::Encode(finalPwd);
    baseInfo->set_pwd(finalPwd.GetRaw());
}

void UserMgr::_OnClientLoginReq(KERNEL_NS::LibPacket *&packet)
{
    auto req = packet->GetCoder<LoginReq>();
    auto sessionId = packet->GetSessionId();
    const auto packetId = packet->GetPacketId();

    KERNEL_NS::SmartPtr<LoginInfo, KERNEL_NS::AutoDelMethods::Release> loginInfo = req->mutable_loginuserinfo();
    req->unsafe_arena_release_loginuserinfo();

    g_Log->Info(LOGFMT_OBJ_TAG("user will login account name:%s, sessionId:%llu"), loginInfo->accountname().c_str(), sessionId);

    auto err = LoginBy(sessionId, loginInfo, [this, sessionId, packetId](Int32 errCode, PendingUser *pending, IUser *user, KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> &var){

        LoginRes res;
        res.set_errcode(errCode);

        if(user)
        {
            res.set_userid(user->GetUserId());
            res.set_servertime(KERNEL_NS::LibTime::NowMilliTimestamp());
        }

        Send(sessionId, Opcodes::OpcodeConst::OPCODE_LoginRes, res, packetId);
    });

    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("login fail err:%d, sessionId:%llu"), err, sessionId);
    }
}

void UserMgr::_OnLoginFinishReq(KERNEL_NS::LibPacket *&packet)
{
    auto sessionId = packet->GetSessionId();
    auto user = GetUserBySessionId(sessionId);
    if(UNLIKELY(!user))
    {
        return;
    }

    Int32 errCode = Status::Success;
    do
    {
        auto status = user->GetUserStatus();
        if(status != UserStatus::USER_LOGINED)
        {
            errCode = Status::LoginStatusError;
            g_Log->Warn(LOGFMT_OBJ_TAG("user status error user:%s, status:%d,%s")
            , user->ToString().c_str(), status, ClientUserStatus::ENUMS_Name(status).c_str());

            break;
        }

        user->SetUserStatus(UserStatus::CLIENT_LOGIN_ENDING);
        user->OnClientLoginFinish();
        
        auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::CLIENT_USER_LOGIN_FINISH);
        ev->SetParam(Params::USER_OBJ, user);
        user->FireEvent(ev);

        if(g_Log->IsEnable(KERNEL_NS::LogLevel::Info))
            g_Log->Info(LOGFMT_OBJ_TAG("client user login finished user:%s"), user->ToString().c_str());
        
    } while (false);
    
    LoginFinishRes res;
    res.set_errcode(errCode);
    user->Send(Opcodes::OpcodeConst::OPCODE_LoginFinishRes, res, packet->GetPacketId());
}

void UserMgr::_OnClientLogoutReq(KERNEL_NS::LibPacket *&packet)
{
    auto sessionId = packet->GetSessionId();
    auto user = GetLoginedUserBySessionId(sessionId);
    if(UNLIKELY(!user))
    {
        return;
    }

    user->Logout(LogoutReason::USER_LOGOUT);
}

void UserMgr::_Clear()
{
    _stubRefPendingUser.clear();
    _accountNameRefPendingUser.clear();
    _userIdRefPendingUser.clear();

    KERNEL_NS::ContainerUtil::DelContainer2(_userIdRefUser);
    _accountNameRefUser.clear();
    _lru.clear();
}


SERVICE_END
