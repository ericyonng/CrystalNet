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
 * Date: 2026-07-03 11:45:36
 * Author: Eric Yonng
 * Description: 
*/
#pragma once

#include <OptionComp/storage/MongoDB/Impl/IMongodbStorageInfo.h>
#include <service/common/macro.h>
#include <kernel/comp/LibString.h>

SERVICE_BEGIN

class UserMgrMongoStorage : public KERNEL_NS::IMongodbStorageInfo
{
    POOL_CREATE_OBJ_DEFAULT_P1(IMongodbStorageInfo, UserMgrMongoStorage);
    
public:
      UserMgrMongoStorage();
      ~UserMgrMongoStorage() override;

    virtual void Release() override;
    virtual Int32 _OnHostInit() override;
    virtual void OnRegisterComps() override;  

    /** 字段名 **/
    // 存储时唯一索引名(user_id)
    static KERNEL_NS::LibString GetKeyName();
    // 账号名(唯一索引)
    static KERNEL_NS::LibString GetAccountName();
    // 昵称名
    static KERNEL_NS::LibString GetNickNameName();
    // 最后登录时间
    static KERNEL_NS::LibString GetLastLoginTimeName();
    // 最后登录ip
    static KERNEL_NS::LibString GetLastLoginIpName();
    // 建号ip
    static KERNEL_NS::LibString GetCreateIpName();
    // 建号时间
    static KERNEL_NS::LibString GetCreateTimeName();
    // 最后跨天时间
    static KERNEL_NS::LibString GetLastPassDayTimeName();
};


SERVICE_END