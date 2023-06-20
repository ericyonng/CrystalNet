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
 * Date: 2023-06-19 11:01:00
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestSql.h>
#include <OptionComp/storage/mysql/mysqlcomp.h>

void TestSql::Run()
{
    KERNEL_NS::SmartPtr<KERNEL_NS::MysqlConnect, KERNEL_NS::AutoDelMethods::CustomDelete> mysqlConnection = KERNEL_NS::MysqlConnect::New_MysqlConnect();
    mysqlConnection.SetClosureDelegate([](void *p){
        auto ptr = KERNEL_NS::KernelCastTo<KERNEL_NS::MysqlConnect>(p);
        KERNEL_NS::MysqlConnect::Delete_MysqlConnect(ptr);
    });

    KERNEL_NS::MysqlConfig config;
    config._host = "127.0.0.1";
    config._user = "root";
    config._pwd = "123456";
    config._dbName = "rpg";
    config._port = 3306;
    mysqlConnection->SetConfig(config);

    auto err = mysqlConnection->Init();
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(TestSql, "mysql init fail."));
        return;
    }

    err = mysqlConnection->Start();
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(TestSql, "mysql start fail."));
        return;
    }

    {// 删除db
        KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::DROP_DB> builder;
        const auto &sql = builder.DB("rpg2").ToSql();
        g_Log->Info2(LOGFMT_NON_OBJ_TAG_NO_FMT(TestSql), sql);

        mysqlConnection->ExcuteSql(builder);
    }

    {// 创建db
        KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::CREATE_DB> builder;
        const auto &sql = builder.DB("rpg2").Charset("utf8mb4").Collate("utf8mb4_bin").ToSql();
        g_Log->Info2(LOGFMT_NON_OBJ_TAG_NO_FMT(TestSql), sql);

        mysqlConnection->ExcuteSql(builder);
    }

    {// 删除表
        KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::DROP_TABLE> builder;
        const auto &sql = builder.Table("tbl_role").ToSql();
        g_Log->Info2(LOGFMT_NON_OBJ_TAG_NO_FMT(TestSql), sql);

        mysqlConnection->ExcuteSql(builder);
    }

    {// 创建表
        KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::CREATE_TABLE> builder;
        builder.Table("tbl_role")
        .Field("Id BIGINT NOT NULL AUTO_INCREMENT COMMENT \"id\"")
        .Field("RoleId INT NOT NULL COMMENT \"角色id\"")
        .Field("UserId VARCHAR(32) NOT NULL COMMENT \"账号id\"")
        .Field("Name VARCHAR(32) NOT NULL COMMENT \"名字\"")
        .Field("LoginTime INT DEFAULT 0 COMMENT \"登录时间\"")
        .PrimaryKey("Id")
        .Unique("UserRoleId", {"UserId", "RoleId"})
        .Unique("UserName", {"UserId", "Name"})
        .Index("RoleName", {"RoleId", "Name"})
        .Index("Role", {"RoleId"})
        .Comment("角色表")
        ;
        const auto &sql = builder.ToSql();
        g_Log->Info2(LOGFMT_NON_OBJ_TAG_NO_FMT(TestSql), sql);

        mysqlConnection->ExcuteSql(builder);
    }

    {// 插入数据
        KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::INSERT> builder;
        builder.Table("tbl_role")
        .Fields({"Id", "RoleId", "UserId", "Name"})
        .Values({"1001", "100101", "\"Eric\"", "\"Yonng\""});

        const auto &sql = builder.ToSql();
        g_Log->Info2(LOGFMT_NON_OBJ_TAG_NO_FMT(TestSql), sql);

        mysqlConnection->ExcuteSql(builder);
    }

    {// 插入数据
        KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::INSERT> builder;
        builder.Table("tbl_role")
        .Fields({"Id", "RoleId", "UserId", "Name"})
        .Values({"1002", "100102", "\"Eric2\"", "\"Yonng2\""});

        const auto &sql = builder.ToSql();
        g_Log->Info2(LOGFMT_NON_OBJ_TAG_NO_FMT(TestSql), sql);

        mysqlConnection->ExcuteSql(builder);
    }

    {// replace into数据
        KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::REPLACE_INTO> builder;
        builder.Table("tbl_role")
        .Fields({"Id", "RoleId", "UserId", "Name"})
        .Values({"1003", "100103", "\"Eric3\"", "\"Yonng3\""});

        const auto &sql = builder.ToSql();
        g_Log->Info2(LOGFMT_NON_OBJ_TAG_NO_FMT(TestSql), sql);

        mysqlConnection->ExcuteSql(builder);
    }

    {// 插入数据
        KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::INSERT> builder;
        builder.Table("tbl_role")
        .Fields({"Id", "RoleId", "UserId", "Name"})
        .Values({"1005", "100105", "\"God2\"", "\"God2\""});

        const auto &sql = builder.ToSql();
        g_Log->Info2(LOGFMT_NON_OBJ_TAG_NO_FMT(TestSql), sql);

        mysqlConnection->ExcuteSql(builder);
    }

    {// update数据
        KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::UPDATE> builder;
        builder.Table("tbl_role")
        .Set("Name", "\"Yonng4\"")
        .Set("UserId", "\"Eric4\"")
        .Where("`Id`=1003")
        ;

        const auto &sql = builder.ToSql();
        g_Log->Info2(LOGFMT_NON_OBJ_TAG_NO_FMT(TestSql), sql);

        mysqlConnection->ExcuteSql(builder);
    }

    {// 查询
        KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::SELECT> builder;
        builder.From("tbl_role")
        .OrderBy({"Id desc", "RoleId asc"}).Limit(5).Where("Name LIKE \"Yonng%\"");

        mysqlConnection->ExcuteSql(builder);

        mysqlConnection->StoreResult([](KERNEL_NS::MysqlConnect *connect, MYSQL_RES *res){
            
        });
    }

    {// delete数据
        KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::DELETE_RECORD> builder;
        builder.Table("tbl_role")
        .Where("Id=1003");
        ;

        mysqlConnection->ExcuteSql(builder);
    }

    {// 清空数据
        KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::TRUNCATE_TABLE> builder;
        builder.Table("tbl_role")
        ;

        mysqlConnection->ExcuteSql(builder);
    }
}