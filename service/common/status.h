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
 * Date: 2022-09-18 22:55:08
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <kernel/kernel.h>

namespace Status
{
    enum ServiceSatusEnum : Int32
    {
        ServiceStatusBegin = Status::FrameStatusEnd + 1,    // 服务层状态码起始

        CreateNewStubFail = ServiceStatusBegin + 1,         // stub失败

        LoadSystemTableFail = ServiceStatusBegin + 2,       // 拉取系统表失败

        DBTableError = ServiceStatusBegin + 3,              // 数据表错误

        DBAddDataFail = ServiceStatusBegin + 4,             // db添加数据失败
        DBAffectedRowsNotEnough = ServiceStatusBegin + 5,             // db数据变更行数没达到预期行数

        ParseFail = ServiceStatusBegin + 6,                 // 解析失败
        CreateTableInfoInSystemTableFail = ServiceStatusBegin + 7,  // 在系统表创建表结构失败
        MysqlMgrCheckLogicFail = ServiceStatusBegin + 8,    // 检查logic时校验失败(table名不可超过64字节, field name不可超过64字节)
        MysqlMgrFillStorageInfoFail,                        // 填充storageinfo时候失败

        DBLoadDataFail,                                     // db 加载数据失败
        DBCreateTableSqlBuilderFail,                        // 创建db 建表sql builder失败
        DBNewRequestFail,                                   // 创建新的db request失败
        SerializeFail,                                      // 序列化失败
        DBCheckDropTablesFail,                              // 清库失败
        DBError,                                            // db错误
        PurgeFail,                                          // 数据清洗失败
        UserNotCreatedBefore,                               // 账户之前没有创建过
        HaveNoRegisterInfo,                                 // 没有注册信息
        TokenExpired,                                        // token失效
        TokenError,                                         // 错误token
        UserAllReadyExistsCantRegisterAgain,                // 用户已经存在不可重新注册
        InvalidChar,                                        // 非法字符
        InvalidPwd,                                         // 错误密码
        MysqlNetworkError,                                  // mysql网络问题
        RepeateLogin,                                       // 重复登录
        CheckAccountFail,                                   // 账号校验失败
        NameTooLong,                                        // 名字过长
        InvalidNickname,                                    // 无效名
        InvalidInviteCode,                                  // 无效邀请码
        InvalidName,                                        // 无效名
        InvalidContent,                                     // 无效内容
        LibraryNotFound,                                    // 图书馆不存在
        AlreadyMemberOfLibrary,                             // 已经是图书馆成员不可再加入
        NotJoinAnyLibrary,                                  // 没加入任何图书馆
        CantQuitLibrary,                                    // 当前不可退出
        HaveBookBorrowedNotReturnBack,                      // 还有书还没还
        NeedBindPhone,                                      // 必须绑定收集
        InvalidPhoneNubmer,                                 // 手机号不合法
        NotMember,                                          // 不是图书馆成员
        MemberIsLocked,                                     // 被锁定无法操作
        NotLibrarian,                                       // 不是馆长不可操作
        NotManager,                                         // 不是管理员
        AuthNotEnough,                                      // 权限不足
        LoadUserFail,                                       // 加载玩家失败
        LoginStatusError,                                   // 登录状态失败
        ImageTooLarge,                                      // 图片过大
        RepeateBook,                                        // 图书已存在
        BookNotFound,                                       // 图书不存在
        ContentTooLong,                                     // 内容过长
        KeyWordsTooMuch,                                    // 关键词太多
        BookCountOverCapacity,                              // 超过库存
    };
}