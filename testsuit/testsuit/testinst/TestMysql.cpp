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
 * Date: 2023-05-22 23:40:28
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestMysql.h>
#include <mysql.h>

void TestMysql::Run()
{
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMysql, "mysql test start."));

    // 程序资源初始化 只初始化一次在整个程序过程只初始化一次 线程不安全
    mysql_library_init(0, 0, 0);

    // 库的初始化, 多线程的初始化，上下文的初始化，线程不安全 因为内部会去调用mysql_library_init（一个程序只能执行一次mysql_init, mysql_library_end会销毁资源）
    MYSQL *mysql = mysql_init(NULL);

    KERNEL_NS::LibString host = "127.0.0.1";
    KERNEL_NS::LibString user = "root";
    KERNEL_NS::LibString pwd = "123456";
    KERNEL_NS::LibString db = "rpg";

    // 连接登录数据库
    mysql_options(mysql, MYSQL_SET_CHARSET_NAME, "utf8mb4");
    if(!mysql_real_connect(mysql, host.c_str(), user.c_str(), pwd.c_str(), NULL, 3306, 0, 0))
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(TestMysql, "Mysql connect fail host:%s, error:%s"), host.c_str(), mysql_error(mysql));
    }
    else
    {
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMysql, "mysql connect success host:%s."), host.c_str());
    }

    {// check if exists db
        bool is_exists = false;
        auto res = mysql_list_dbs(mysql, db.c_str());
        if(res == NULL)
        {
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(TestMysql, "db:%s, not create err:%s"), db.c_str(), mysql_error(mysql));
        }
        else
        {
            is_exists = mysql_num_rows(res) > 0;
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMysql, "DB:%s, is_exists:%d"), db.c_str(), is_exists);
            mysql_free_result(res);
        }

        if(!is_exists)
        {// 不存在就创建数据库
            KERNEL_NS::LibString sql;
            sql.AppendFormat("CREATE DATABASE `%s` DEFAULT CHARACTER SET utf8mb4 DEFAULT COLLATE utf8mb4_bin", db.c_str());
            auto ret = mysql_real_query(mysql, sql.c_str(), static_cast<ULong>(sql.length()));
            if(ret == 0)
            {
                g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMysql, "DB:%s create success."), db.c_str());
            }
            else
            {
                g_Log->Warn(LOGFMT_NON_OBJ_TAG(TestMysql, "db:%s, create fail err:%s"), db.c_str(), mysql_error(mysql));
            }
        }

        // 选择工作的数据库
        auto ret = mysql_select_db(mysql, db.c_str());
        if(ret != 0)
        {
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(TestMysql, "select db:%s fail err:%s"), db.c_str(), mysql_error(mysql));
        }
    }
    
    // 断开mysql连接
    mysql_close(mysql);

    // 释放程序资源, 只做一次
    mysql_library_end();

    // mysql_close可以断开mysql连接
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMysql, "mysql test end."));
}