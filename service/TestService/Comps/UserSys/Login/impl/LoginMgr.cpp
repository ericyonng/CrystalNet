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
 * Date: 2023-08-06 14:31:00
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <Comps/UserSys/Login/impl/LoginMgr.h>
#include <Comps/UserSys/Login/impl/LoginMgrFactory.h>
#include <Comps/config/config.h>
#include <protocols/protocols.h>

SERVICE_BEGIN
POOL_CREATE_OBJ_DEFAULT_IMPL(ILoginMgr);
POOL_CREATE_OBJ_DEFAULT_IMPL(LoginMgr);


LoginMgr::LoginMgr()
:_loginInfo(CRYSTAL_NEW(UserLoginInfo))
,_updateKey(NULL)
{

}

LoginMgr::~LoginMgr()
{
    _Clear();
}

void LoginMgr::Release()
{
    LoginMgr::DeleteByAdapter_LoginMgr(LoginMgrFactory::_buildType.V, this);
}

Int32 LoginMgr::OnLoaded(const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db)
{
    if(UNLIKELY(!_loginInfo->Decode(db)))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("decode fail user:%s"), _userOwner->ToString().c_str());
        return Status::ParseFail;
    }

    return Status::Success;
}

Int32 LoginMgr::OnSave(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) const
{
    if(UNLIKELY(!_loginInfo->Encode(db)))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("encode fail user:%s"), _userOwner->ToString().c_str());
        return Status::SerializeFail;
    }

    return Status::Success;
}

void LoginMgr::OnLogin()
{
    const auto &nowTime = KERNEL_NS::LibTime::Now();
    if(_loginInfo->keyexpiretime() == 0 || _loginInfo->keyexpiretime() <= nowTime.GetSecTimestamp())
    {
        _Update(false);
    }

    _StartTimer();
}

void LoginMgr::OnLoginFinish()
{
    // 密钥
    _SendInfo();
}

Int32 LoginMgr::CheckLogin(const PendingUser *pendingUser) const
{
    // 注册登录: pwd校验, 账号校验, 登录设备校验, 密文校验
    if(!pendingUser->_loginInfo)
    {// TODO:需要外部校验
        g_Log->Warn(LOGFMT_OBJ_TAG("have no login info user:%s"), _userOwner->ToString().c_str());
        return Status::ParamError;
    }

    if(pendingUser->_loginInfo->loginmode() == LoginMode::REGISTER)
    {// 
        g_Log->Warn(LOGFMT_OBJ_TAG("user already exists user:%s"), _userOwner->ToString().c_str());
        return Status::UserAllReadyExistsCantRegisterAgain;
    }

    auto &rsa = _userMgr->GetRsa();
    auto &loginInfo = *pendingUser->_loginInfo;
    if(loginInfo.cyphertext().empty())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("cypher test is empty user:%s"), _userOwner->ToString().c_str());
        return Status::Failed;
    }

    const auto &cypherRaw = KERNEL_NS::LibBase64::Decode(loginInfo.cyphertext());
    if(cypherRaw.empty())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("cypher test is empty user:%s"), _userOwner->ToString().c_str());
        return Status::Failed;
    }
    KERNEL_NS::LibString textRaw;
    rsa.PrivateKeyDecrypt(cypherRaw, textRaw);
    if(textRaw.empty())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("client not no author user:%s"), _userOwner->ToString().c_str());
        return Status::Failed;
    }
    if(textRaw.GetRaw() != loginInfo.origintext())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("client not no author user:%s"), _userOwner->ToString().c_str());
        return Status::Failed;
    }

    // TODO:
    const auto &nowTime = KERNEL_NS::LibTime::Now();
    if(pendingUser->_loginInfo->loginmode() == LoginMode::USE_LOGIN_TOKEN)
    {
        if(_loginInfo->keyexpiretime() <= nowTime.GetSecTimestamp())
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("key expired user:%s, pendingUser:%s")
            , GetUser()->ToString().c_str(), pendingUser->ToString().c_str());
            return Status::TokenExpired;
        }

        auto userBaseInfo = _userOwner->GetUserBaseInfo();
        const auto keyText = KERNEL_NS::LibBase64::Decode(_loginInfo->key());
        KERNEL_NS::LibString buildToken = userBaseInfo->lastloginphoneimei() + userBaseInfo->lastloginip() + KERNEL_NS::StringUtil::Num2Str(GetUser()->GetUserId()) + keyText;
        buildToken = KERNEL_NS::LibDigest::MakeSha256(buildToken);
        buildToken = KERNEL_NS::LibBase64::Encode(buildToken);
        if(buildToken != pendingUser->_loginInfo->logintoken())
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("token error user:%s, pending:%s")
                , GetUser()->ToString().c_str(), pendingUser->ToString().c_str());
            return Status::TokenError;
        }
    }
    else
    {// 校验密码
        if(pendingUser->_loginInfo->pwd().empty())
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("pwd invalid user:%s"), _userOwner->ToString().c_str());
            return Status::InvalidPwd;
        }

        auto userInfo = _userOwner->GetUserBaseInfo();
        // 还原密码
        const auto cypherPwd = KERNEL_NS::LibBase64::Decode(pendingUser->_loginInfo->pwd());
        KERNEL_NS::LibString pwd;
        rsa.PrivateKeyDecrypt(cypherPwd, pwd);
        if(pwd.empty())
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("pwd empty user:%s"), _userOwner->ToString().c_str());
            return Status::InvalidPwd;
        }

        const auto &salt = KERNEL_NS::LibBase64::Decode(userInfo->pwdsalt());
        pwd.GetRaw().insert(pwd.size() / 2, salt.GetRaw());
        const auto &merge = KERNEL_NS::LibDigest::MakeSha256(pwd);
        const auto &encodePwd = KERNEL_NS::LibBase64::Encode(merge);
        if(userInfo->pwd() != encodePwd.GetRaw())
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("invalid pwd user:%s"), _userOwner->ToString().c_str());
            return Status::InvalidPwd;
        }
    }

    return Status::Success;
}

void LoginMgr::OnRegisterComps()
{

}

Int32 LoginMgr::_OnUserSysInit()
{
    _updateKey = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
    _updateKey->SetTimeOutHandler(this, &LoginMgr::_OnTimerOut);

    return Status::Success;
}

Int32 LoginMgr::_OnHostStart() 
{
    return Status::Success;
}

void LoginMgr::_OnSysClose()
{
    _Clear();
}

void LoginMgr::_Clear()
{
    CRYSTAL_RELEASE_SAFE(_loginInfo);
}

void LoginMgr::_OnTimerOut(KERNEL_NS::LibTimer *t)
{
    t->Cancel();

    _Update();

    _StartTimer();
}

void LoginMgr::_Update(bool isNty)
{
    // TODO:更新token
    // 更新key
    auto conmmonConfigMgr = GetService()->GetComp<ConfigLoader>()->GetComp<CommonConfigMgr>();
    auto config = conmmonConfigMgr->GetConfigById(CommonConfigIdEnums::USER_LOGIN_KEY_CHAR_COUNT);

    // 生成新的key
    KERNEL_NS::LibString keyText;
    for(;;)
    {
        KERNEL_NS::CypherGeneratorUtil::Gen(keyText, config->_value);
        if(keyText.GetRaw() != _loginInfo->key())
            break;
    }
    auto encodeKeyText = KERNEL_NS::LibBase64::Encode(keyText);
    _loginInfo->set_key(encodeKeyText.GetRaw());

    // 过期时间
    auto expireConfig = conmmonConfigMgr->GetConfigById(CommonConfigIdEnums::USER_LOGIN_KEY_EXPIRE_TIME);
    const auto &expireTime = KERNEL_NS::LibTime::Now() + KERNEL_NS::TimeSlice::FromSeconds(static_cast<Int64>(expireConfig->_value));
    _loginInfo->set_keyexpiretime(expireTime.GetSecTimestamp());  

    // TODO:sha256(imei + ip + userid + key)
    auto addr = _userOwner->GetUserAddr();
    const auto &imei = _userOwner->GetUserBaseInfo()->lastloginphoneimei();
    KERNEL_NS::LibString token;
    token.AppendData(imei.c_str(), static_cast<Int64>(imei.size()));
    token.AppendData(addr->_ip)
    .AppendFormat("%llu", _userOwner->GetUserId());
    token.AppendData(keyText);

    const auto &temp = KERNEL_NS::LibDigest::MakeSha256(token);
    const auto &finalToken = KERNEL_NS::LibBase64::Encode(temp);
    _loginInfo->set_token(finalToken.GetRaw());

    MaskDirty();

    if(isNty)
        _SendInfo();
}

void LoginMgr::_StartTimer()
{
    const auto &nowTime = KERNEL_NS::LibTime::Now();
    const auto diffSeconds = _loginInfo->keyexpiretime() > nowTime.GetSecTimestamp() ? (_loginInfo->keyexpiretime() - nowTime.GetSecTimestamp()) : 0;
    _updateKey->Schedule(diffSeconds * 1000);
}

void LoginMgr::_SendInfo()
{
    LoginInfoNty nty;
    nty.set_token(_loginInfo->token());
    nty.set_keyexpiretime(_loginInfo->keyexpiretime());

    Send(Opcodes::OpcodeConst::OPCODE_LoginInfoNty, nty);
}


SERVICE_END