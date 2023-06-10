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
 * 1.创建索引越多, 在插入或者update的时候性能开销是比较大的
 * 2.truncate 表会把表的数据清空，自增id归0
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

    // 字符集设置
    mysql_options(mysql, MYSQL_SET_CHARSET_NAME, "utf8mb4");

    // 自动重连
    Int32 op = 1;
    mysql_options(mysql, MYSQL_OPT_RECONNECT, &op);

    // 设置mysql的最大包大小
    ULong maxPacketSize = 1024*1024*1024;
    mysql_options(mysql, MYSQL_OPT_MAX_ALLOWED_PACKET, &maxPacketSize);

    // 表信息打开
    bool isOpenTableInfo = true;
    mysql_options(mysql, MYSQL_OPT_OPTIONAL_RESULTSET_METADATA, &isOpenTableInfo);

    // 连接登录数据库
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
            sql.AppendFormat("CREATE DATABASE IF NOT EXISTS `%s` DEFAULT CHARACTER SET utf8mb4 DEFAULT COLLATE utf8mb4_bin", db.c_str());
            // 执行sql语句如果有结果的必须要获取结果集并清理
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

    {
        KERNEL_NS::LibString sql;
        sql.AppendFormat("select *from tbl_role");
        // 执行sql语句如果有结果的必须要获取结果集并清理
        auto ret = mysql_real_query(mysql, sql.c_str(), static_cast<ULong>(sql.length()));
        if(ret != 0)
        {
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(TestMysql, "query fail sql:%s fail err:%s"), sql.c_str(), mysql_error(mysql));
        }
    }

    {// 获取结果
        // mysql_store_result() 会等待所有结果返回时才执行完成 这个函数需要考虑缓冲,需要设置最大缓冲数据大小 MYSQL_OPT_MAX_ALLOWED_PACKET 默认64MB 可以通过mysql_options来设置大小
        // mysql_use_result只是和mysql通信，告知我要开始读数据，会立即返回 不读取数据
        // auto res = mysql_use_result(mysql);
        auto res = mysql_store_result(mysql);

        if(res)
        {// 获取表字段信息
            // 使用 mysql_use_result 的 mysql_fetch_row 会从mysql远程去取数据, 如果是mysql_store_result 会直接从本地缓存中取数据
            MYSQL_ROW row;
            auto fieldNum = mysql_num_fields(res);
            while (row = mysql_fetch_row(res))
            {
                auto lens = mysql_fetch_lengths(res);
                for(UInt32 idx = 0; idx < fieldNum; ++idx)
                {
                    // 取当前字段信息并打印字段信息与数据
                    auto curField = mysql_fetch_field_direct(res, idx);

                    if(g_Log->IsEnable(KERNEL_NS::LogLevel::Info))
                        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMysql, "field:%s len:%lu, create len:%lu, max len:%lu data:%s"), curField->name, lens[idx], curField->length, curField->max_length, row[idx] ? row[idx] : "NULL");
                }
            }

            // 获取表字段
            MYSQL_FIELD *field;
            while (field = mysql_fetch_field(res))
            {
                g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMysql, "field name:%s, create field len:%lu, table:%s, max len:%lu, type:%d")
                , field->name, field->length, field->table, field->max_length, field->type);
            }
        }
        else
        {
            if(g_Log->IsEnable(KERNEL_NS::LogLevel::Warn))
                g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMysql, "mysql store result fail error:%s"), mysql_error(mysql));
        }


        if(res)
        {// res必须被释放才能继续执行后续sql
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMysql, "get result starting."));
            mysql_free_result(res);
        }
    }

    for(Int32 idx = 0; idx < 1000; ++idx)
    {
        KERNEL_NS::SystemUtil::ThreadSleep(1000);
        auto pingRet = mysql_ping(mysql);
        if(pingRet == 0)
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMysql, "mysql is connected."));
        }
        else
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMysql, "mysql is disconnected err:%s."), mysql_error(mysql));
        }
    }
    
    // 断开mysql连接
    mysql_close(mysql);

    // 释放程序资源, 只做一次
    mysql_library_end();

    // mysql_close可以断开mysql连接
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMysql, "mysql test end."));
}