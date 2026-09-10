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
 * Date: 2023-08-02 13:23:59
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <ServiceCompHeader.h>
#include <service/common/BaseComps/Storage/storage.h>

SERVICE_BEGIN

class UserMgrStorage : public IStorageInfo
{
    POOL_CREATE_OBJ_DEFAULT_P1(IStorageInfo, UserMgrStorage);

    // 字段定义
    // 平时使用账号名 + 密码登录
    // 密码存成sha1加密的16进制字符串 通过邮箱修改
    // 必须
public:
    // static constexpr const Byte8 *USER_ID = "UserId";
    // static constexpr const Byte8 *ACCOUNT_NAME = "AccountName";
    // static constexpr const Byte8 *NAME = "Name";
    // static constexpr const Byte8 *NICKNAME = "Nickname";
    // static constexpr const Byte8 *PWD = "Pwd";
    // static constexpr const Byte8 *BIND_PHONE = "BindPhone";
    // static constexpr const Byte8 *LAST_LOGIN_TIME = "LastLoginTime";
    // static constexpr const Byte8 *LAST_LOGIN_IP = "LastLoginIp";
    // static constexpr const Byte8 *LAST_LOGIN_PHONE_IMEI = "LastLoginPhoneImei";
    // static constexpr const Byte8 *CREATE_IP = "CreateIp";
    // static constexpr const Byte8 *CREATE_TIME = "CreateTime";
    // static constexpr const Byte8 *CREATE_PHONE_IMEI = "CreatePhoneImei";
    // static constexpr const Byte8 *BIND_MAIL_ADDR = "BindMailAddr";

    // 角色系统信息（管理员, 读者）
    // static constexpr const Byte8 *ROLE_SYSTEM_INFO = "CreatePhoneImei";
    // 二维码生成模块
    // static constexpr const Byte8 *QR_CODE = "QrCode";
    // 登录令牌生成模块 logintoken = sha1(userId + loginip + 登录的imei + 生成的key + sessionId) key登录时候验证过期后重新生成 key是有过期时间的, 过期后重新生成(通过全球唯一id生成, 避免了重复),当使用令牌登录时候需要验证是否匹配, 不匹配只能通过密码登录，且会提示需要重登, 有效期内重登会推送新的登录令牌, sessionId是维持会话, 有效避免攻击
    // static constexpr const Byte8 *LOGIN_TOKEN_INFO = "LoginTokenInfo";
    
public:
    UserMgrStorage();
    ~UserMgrStorage();

    void Release() override;

    virtual bool RegisterStorages() override;
};

SERVICE_END