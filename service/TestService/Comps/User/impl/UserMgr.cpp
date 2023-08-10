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

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(IUserMgr);
POOL_CREATE_OBJ_DEFAULT_IMPL(UserMgr);

UserMgr::UserMgr()
:_lruCapacityLimit(1000)
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

    for(auto userId : _pendingLoginEventOnStartup)
    {
        auto user = GetUser(userId);
        if(UNLIKELY(!user))
            continue;

        auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::USER_LOGIN);
        ev->SetParam(Params::USER_OBJ, user);
        GetEventMgr()->FireEvent(ev);
    }
    _pendingLoginEventOnStartup.clear();

    _LruPopUser();

    // 用户登录
    KERNEL_NS::LibString randAccount;
    auto &random = KERNEL_NS::LibInt64Random<KERNEL_NS::_Build::TL>::GetInstance();
    randAccount.AppendFormat("test_eric_%lld", random.Gen(0, 127));
    
    KERNEL_NS::SmartPtr<LoginInfo, KERNEL_NS::AutoDelMethods::Release> loginInfo = CRYSTAL_NEW(LoginInfo);
    loginInfo->set_loginmode(LoginMode::REGISTER);
    loginInfo->set_accountname(randAccount.GetRaw());
    loginInfo->set_pwd("123456");
    loginInfo->set_logintoken("123456");
    loginInfo->set_loginphoneimei("123456");
    loginInfo->set_appid("123456");
    loginInfo->set_cyphertext("123456");
    loginInfo->set_origintext("123456");
    loginInfo->set_versionid(10101);
    auto registerInfo = loginInfo->mutable_userregisterinfo();
    registerInfo->set_accountname("test_eric");
    registerInfo->set_pwd("123456");
    registerInfo->set_nickname("123456");
    registerInfo->set_createphoneimei("123456");
    auto err = LoginBy(0, loginInfo, [this](Int32 errCode, PendingUser *pending, IUser *user, KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> &var){
        user->GetUserBaseInfo()->set_nickname("aakkkkkkkkkkkkkkkkkkkkkkkkkkkkkdalkaldfaskldfaskdfasklfjaslkdfaslkdfalskdfaskdfjalksdjfaksfalskdfjaskdjfalksdjfaksdfjalskdfjaksldjfaklsjdfkasjdfkasjdfkasjdflkasjflaksdjflsakfjaskdjf");
        user->MaskDirty();
        user->PurgeAndWaitComplete();

        g_Log->Info(LOGFMT_OBJ_TAG("user login:%s"), user->ToString().c_str());
    });

    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("login fail err:%d, user acount name:%s"), err, loginInfo->accountname().c_str());
    }
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

    user->OnLogin();
    user->OnLoginFinish();

    _pendingLoginEventOnStartup.push_back(user->GetUserId());

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

    // TODO:获取分布式锁（LogicProxy做）
    // 是否已经存在User, 
    auto user = GetUser(pendingInfo->_byAccountName);
    if(!user)
    {
        // 查库
        return LoadUser(pendingInfo->_byAccountName, pendingInfo);
    }

    auto stubMgr = GetGlobalSys<IStubHandleMgr>();
    pendingInfo->_stub = stubMgr->NewStub();

    Int32 err = Status::Success;
    if(LIKELY(pendingInfo->_sessionId))
    {
        const auto &curTime = KERNEL_NS::LibTime::Now();

        // 校验登录
        auto loginMgr = user->GetComp<ILoginMgr>();
        err = loginMgr->CheckLogin(pendingInfo);
        if(err != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("check login fail err:%d, pending info:%s, userId:%llu"), err, pendingInfo->ToString().c_str(), user->GetUserId());
        }
        else
        {
            // 顶号
            if(user->IsLogined())
                user->Logout();

            user->OnLogin();
            user->BindSession(pendingInfo->_sessionId);
            user->OnLoginFinish();
        }

        auto loginInfo = pendingInfo->_loginInfo;
        auto baseInfo = user->GetUserBaseInfo();
        auto addr = user->GetUserAddr();
        baseInfo->set_lastlogintime(curTime.GetMilliTimestamp());
        baseInfo->set_lastloginphoneimei(loginInfo->loginphoneimei());
        baseInfo->set_lastloginip(addr->_ip.GetRaw());
    }

    _RemoveFromLru(user);
    user->UpdateLrtTime();
    _AddToLru(user);

    if(LIKELY(cb))
        cb->Invoke(Status::Success, pendingInfo, user, var);

    g_Log->Info(LOGFMT_OBJ_TAG("user already onlin user :%s, pending:%s"), user->ToString().c_str(), pendingInfo->ToString().c_str());

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
    KERNEL_NS::Field *v = KERNEL_NS::Field::Create(storageInfo->GetTableName(), userIdName, MYSQL_TYPE_STRING, 0);
    v->Write(&userId, static_cast<Int64>(sizeof(userId)));
    fields[0] = v;

    std::vector<KERNEL_NS::SqlBuilder *> builders;
    builders.push_back(selectBuilder);
    auto err = mysqlMgr->NewRequestBy(newPendingInfo->_stub, mysqlMgr->GetCurrentServiceDbName(), dbOid, builders, fields, this, &UserMgr::_OnDbUserLoaded);
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

    std::vector<KERNEL_NS::SqlBuilder *> builders;
    builders.push_back(selectBuilder);
    auto err = mysqlMgr->NewRequestBy(pendingUser->_stub, mysqlMgr->GetCurrentServiceDbName(), dbOid, builders, fields, this, &UserMgr::_OnDbUserLoaded);
    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("NewRequestBy fail err:%d, accountName:%s"), err, accountName.c_str());
        _RemovePendingInfo(accountName);
        return err;
    }

    return Status::Success;
}

void UserMgr::Purge()
{
    auto dbMgr = GetGlobalSys<IMysqlMgr>();
    dbMgr->Purge(this);
}

void UserMgr::PurgeAndWait()
{
    auto dbMgr = GetGlobalSys<IMysqlMgr>();
    dbMgr->PurgeAndWaitComplete(this);
}

void UserMgr::PurgeEndWith(KERNEL_NS::IDelegate<void, Int32> *handler)
{
    auto dbMgr = GetGlobalSys<IMysqlMgr>();
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
        return;
    }

    _sessionIdRefUser.insert(std::make_pair(sessionId, user));
}

Int32 UserMgr::_OnGlobalSysInit()
{
    auto ini = GetApp()->GetIni();
    if(!ini->CheckReadNumber(GetService()->GetServiceName().c_str(), "UserLruCapacityLimit", _lruCapacityLimit))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("lack of %s:UserLruCapacityLimit config in ini:%s"), GetService()->GetServiceName().c_str(), ini->GetPath().c_str());
        return Status::Failed;
    }

    auto commonMgr = GetService()->GetComp<ConfigLoader>()->GetComp<CommonConfigMgr>();
    _lruCapacityLimit = commonMgr->GetConfigById(CommonConfigIdEnums::USER_LRU_CAPACITY_LIMIT)->_value;

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
        user->FireEvent(ev);

    ev->SetDontDelAfterFire(false);
    IGlobalSys::_OnQuitServiceEventDefault(ev);
}


void UserMgr::_AddUserPendingInfo(UInt64 userId, KERNEL_NS::SmartPtr<PendingUser, KERNEL_NS::AutoDelMethods::CustomDelete> &pendingInfo)
{
    _userIdRefPendingUser.insert(std::make_pair(userId, pendingInfo));
    _stubRefPendingUser.insert(std::make_pair(pendingInfo->_stub, pendingInfo));
}

void UserMgr::_AddUserPendingInfo(const KERNEL_NS::LibString &accountName, KERNEL_NS::SmartPtr<PendingUser, KERNEL_NS::AutoDelMethods::CustomDelete> &pendingInfo)
{
    _accountNameRefPendingUser.insert(std::make_pair(accountName, pendingInfo));
    _stubRefPendingUser.insert(std::make_pair(pendingInfo->_stub, pendingInfo));
}

void UserMgr::_RemovePendingInfo(const KERNEL_NS::LibString &accountName)
{
    auto iter = _accountNameRefPendingUser.find(accountName);
    if(UNLIKELY(iter == _accountNameRefPendingUser.end()))
        return;

    _stubRefPendingUser.erase(iter->second->_stub);
    _accountNameRefPendingUser.erase(iter);
}

void UserMgr::_RemovePendingInfo(UInt64 userId)
{
    auto iter = _userIdRefPendingUser.find(userId);
    if(iter == _userIdRefPendingUser.end())
        return;

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
    if(LIKELY(static_cast<Int32>(_lru.size()) < _lruCapacityLimit))
        return; 

    auto firstNode = _lru.begin();
    auto expiredlUser = (*firstNode)->CastTo<User>();
    _lru.erase(firstNode);

    // 踢掉
    if(expiredlUser->IsLogined())
        expiredlUser->Logout();

    auto mysqlMgr = GetGlobalSys<IMysqlMgr>();
    mysqlMgr->PurgeAndWaitComplete(this);

    // 即将移除user
    auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::USER_WILL_REMOVE);
    ev->SetParam(Params::USER_ID, expiredlUser->CastTo<IUser>());
    GetEventMgr()->FireEvent(ev);

    _RemoveUser(expiredlUser);

    expiredlUser->WillClose();
    expiredlUser->Close();
    User::DeleteThreadLocal_User(expiredlUser);
}

void UserMgr::_OnDbUserLoaded(KERNEL_NS::MysqlResponse *res)
{
    g_Log->Info(LOGFMT_OBJ_TAG("mysql res:%s"), res->ToString().c_str());
    const auto &curTime = KERNEL_NS::LibTime::Now(); 

    // 可能之前有load过user导致pengding 一起处理
    auto pendingUser = _GetPendingByStub(res->_stub);
    if(!pendingUser)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("pending user is handled before get result stub:%llu, err:%d"), res->_stub, res->_errCode);
        return;
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
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("check login fail err:%d, pending info:%s, userId:%llu"), err, pendingUser->ToString().c_str(), user->GetUserId());
            }
            else
            {
                // 顶号
                if(user->IsLogined())
                    user->Logout();

                user->OnLogin();
                user->BindSession(pendingUser->_sessionId);
                user->OnLoginFinish();
            }

            auto loginInfo = pendingUser->_loginInfo;
            auto baseInfo = user->GetUserBaseInfo();
            auto addr = user->GetUserAddr();
            baseInfo->set_lastlogintime(curTime.GetMilliTimestamp());
            baseInfo->set_lastloginphoneimei(loginInfo->loginphoneimei());
            baseInfo->set_lastloginip(addr->_ip.GetRaw());
        }

        // TODO:记录登录信息

        if(LIKELY(pendingUser->_cb))
            pendingUser->_cb->Invoke(err, pendingUser, user, pendingUser->_var);

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

            // 校验注册信息
            err = _CheckRegisterInfo(pendingUser->_loginInfo->userregisterinfo());
            if(err != Status::Success)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("check regiseter info fail err:%d pending info:%s"), err, pendingUser->ToString().c_str());
                break;
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
            baseInfo->set_pwd(registerInfo.pwd());
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

            err = u->Init();
            if(err != Status::Success)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("user init fail err:%d, pending info:%s"), err, pendingUser->ToString().c_str());
                break;
            }
            err = u->Start();
            if(err != Status::Success)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("user init fail err:%d, pending info:%s"), err, pendingUser->ToString().c_str());
                break;
            }

            _AddUser(u.AsSelf());
            user = u.pop();
            MaskNumberKeyAddDirty(user->GetUserId());

            auto dbMgr = GetGlobalSys<IMysqlMgr>();
            dbMgr->PurgeAndWaitComplete(this);
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

            err = u->Init();
            if(err != Status::Success)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("user init fail err:%d, pending info:%s"), err, pendingUser->ToString().c_str());
                break;
            }
            err = u->Start();
            if(err != Status::Success)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("user init fail err:%d, pending info:%s"), err, pendingUser->ToString().c_str());
                break;
            }

            auto &record = res->_datas[0];
            auto userId = record->GetPrimaryKey()->GetUInt64();

            err =  u->OnLoaded(userId, record->GetFieldDatas());
            if(err != Status::Success)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("user onloaded fail err:%d, pending info:%s, userId:%llu"), err, pendingUser->ToString().c_str(), userId);
                break;
            }

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
        _RemoveFromLru(user);
        user->UpdateLrtTime();
        _AddToLru(user);
    }

    if(err != Status::Success)
    {
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

    // 创建user
    if((!isAlreadyExists) && pendingUser->_loginInfo && (pendingUser->_loginInfo->loginmode() == LoginMode::REGISTER))
    {
        user->OnUserCreated();
    }

    // 登录
    if(pendingUser->_sessionId)
    {
        user->OnLogin();
        user->BindSession(pendingUser->_sessionId);
        user->OnLoginFinish();
    }

    auto loginInfo = pendingUser->_loginInfo;
    auto &registerInfo = pendingUser->_loginInfo->userregisterinfo();
    auto baseInfo = user->GetUserBaseInfo();

    // 最后登录信息
    if(pendingUser->_sessionId)
    {
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

Int32 UserMgr::_CheckRegisterInfo(const RegisterUserInfo &regiseterInfo) const
{
    // TODO: 有没有被封禁ip, 有没有被封禁设备id,得添加一个封禁模块(BanMgr)
    // 密码是否合法:至少8个字符
    // 设备imei
    return Status::Success;
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
