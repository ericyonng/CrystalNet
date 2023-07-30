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
    };
}