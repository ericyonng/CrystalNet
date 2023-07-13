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

// void TestSql::Run()
// {
//     KERNEL_NS::SnowflakeInfo snowFlakeInfo;
//     KERNEL_NS::GuidUtil::InitSnowFlake(snowFlakeInfo, 1, static_cast<UInt64>(KERNEL_NS::LibTime::NowTimestamp()));

//     KERNEL_NS::SmartPtr<KERNEL_NS::MysqlConnect, KERNEL_NS::AutoDelMethods::CustomDelete> mysqlConnection = KERNEL_NS::MysqlConnect::New_MysqlConnect(KERNEL_NS::GuidUtil::Snowflake(snowFlakeInfo));
//     mysqlConnection.SetClosureDelegate([](void *p){
//         auto ptr = KERNEL_NS::KernelCastTo<KERNEL_NS::MysqlConnect>(p);
//         KERNEL_NS::MysqlConnect::Delete_MysqlConnect(ptr);
//     });

//     KERNEL_NS::MysqlConfig config;
//     config._host = "127.0.0.1";
//     config._user = "root";
//     config._pwd = "123456";
//     config._dbName = "rpg";
//     config._port = 3306;

//     // 利用 MYSQL_OPT_BIND 选项绑定多张网卡中的一张
//     config._bindIp = "127.0.0.1";
//     mysqlConnection->SetConfig(config);

//     auto err = mysqlConnection->Init();
//     if(err != Status::Success)
//     {
//         g_Log->Error(LOGFMT_NON_OBJ_TAG(TestSql, "mysql init fail."));
//         return;
//     }

//     err = mysqlConnection->Start();
//     if(err != Status::Success)
//     {
//         g_Log->Error(LOGFMT_NON_OBJ_TAG(TestSql, "mysql start fail."));
//         return;
//     }

//     {// 未实现的builder类型 执行sql时候会报错
//         // KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::KB> builder;

//         // mysqlConnection->ExcuteSql(builder);
//     }

//     {// 删除db
//         KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::DROP_DB> builder;
//         builder.DB("rpg2");

//         mysqlConnection->ExcuteSql(builder);
//     }

//     {// 创建db
//         KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::CREATE_DB> builder;
//         builder.DB("rpg2").Charset("utf8mb4").Collate("utf8mb4_bin");

//         mysqlConnection->ExcuteSql(builder);
//     }

//     {// 删除表
//         KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::DROP_TABLE> builder;
//         builder.Table("tbl_role");

//         mysqlConnection->ExcuteSql(builder);
//     }

//     {// 创建表
//         KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::CREATE_TABLE> builder;
//         builder.Table("tbl_role")
//         .Field("Id BIGINT NOT NULL AUTO_INCREMENT COMMENT \"id\"")
//         // .Field("Id BIGINT NOT NULL DEFAULT 0 COMMENT \"id\"")
//         // .Field("Id2 BIGINT NOT NULL AUTO_INCREMENT COMMENT \"Id2\"")
//         .Field("RoleId INT NOT NULL COMMENT \"角色id\"")
//         .Field("UserId VARCHAR(32) NOT NULL COMMENT \"账号id\"")
//         .Field("Name VARCHAR(32) NOT NULL COMMENT \"名字\"")
//         .Field("LoginTime INT DEFAULT 0 COMMENT \"登录时间\"")
//         .PrimaryKey("Id")
//         // .Unique("UserRoleId", {"UserId", "RoleId"})
//         .Unique("UserName", {"UserId", "Name"})
//         .Index("RoleName", {"RoleId", "Name"})
//         .Index("Role", {"RoleId"})
//         .Comment("role table")
//         ;
//         mysqlConnection->ExcuteSql(builder);
//     }

//     {// 插入数据
//         KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::INSERT> builder;
//         builder.Table("tbl_role")
//         .Fields({"Id", "RoleId", "UserId", "Name"})
//         .Values({"1001", "100101", "\"Eric\"", "\"Yonng\""});

//         mysqlConnection->ExcuteSql(builder);

//         g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "last insert id:%lld"), mysqlConnection->GetLastInsertIdOfAutoIncField());
//     }

//     {// 插入数据
//         KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::INSERT> builder;
//         builder.Table("tbl_role")
//         .Fields({"RoleId", "UserId", "Name"})
//         .Values({"100102", "\"Eric2\"", "\"Yonng2\""});

//         mysqlConnection->ExcuteSql(builder);
//         g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "last insert id:%lld"), mysqlConnection->GetLastInsertIdOfAutoIncField());
//     }

//     {// replace into数据
//         KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::REPLACE_INTO> builder;
//         builder.Table("tbl_role")
//         .Fields({"Id", "RoleId", "UserId", "Name"})
//         .Values({"1003", "100103", "\"Eric3\"", "\"Yonng3\""});

//         mysqlConnection->ExcuteSql(builder);
//         g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "last insert id:%lld"), mysqlConnection->GetLastInsertIdOfAutoIncField());
//     }

//     {// 插入数据
//         KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::INSERT> builder;
//         builder.Table("tbl_role")
//         .Fields({"Id", "RoleId", "UserId", "Name"})
//         .Values({"1005", "100105", "\"God2\"", "\"God2\""});

//         mysqlConnection->ExcuteSql(builder);
//         g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "last insert id:%lld"), mysqlConnection->GetLastInsertIdOfAutoIncField());
//     }

//     {// 插入数据
//         KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::INSERT> builder;
//         builder.Table("tbl_role")
//         .Fields({"Id", "RoleId", "UserId", "Name"})
//         .Values({"900", "1001088", "\"God55\"", "\"God554\""});

//         mysqlConnection->ExcuteSql(builder);
//         g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "last insert id:%lld"), mysqlConnection->GetLastInsertIdOfAutoIncField());
//     }

//     {// replace into数据
//         KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::REPLACE_INTO> builder;
//         builder.Table("tbl_role")
//         .Fields({"Id", "RoleId", "UserId", "Name"})
//         .Values({"1003", "100103", "\"Eric3\"", "\"Yonng3\""});

//         mysqlConnection->ExcuteSql(builder);
//         g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "last insert id:%lld"), mysqlConnection->GetLastInsertIdOfAutoIncField());
//     }

//     {// ALTER TABLE
//         KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::ALTER_TABLE> builder;

//         // 添加字段
//         builder.Table("tbl_role").Add("TestMgr", "INT NOT NULL DEFAULT 0").Add("TestMgr2", "INT NOT NULL DEFAULT 0");
//         mysqlConnection->ExcuteSql(builder);

//         // 添加字段
//         builder.Clear();
//         builder.Table("tbl_role").Add("TestFulltextIndex1", "VARCHAR(32) NOT NULL DEFAULT \"测试全文索引1\"").Add("TestFulltextIndex2", "VARCHAR(32) NOT NULL DEFAULT \"测试全文索引2\"");
//         mysqlConnection->ExcuteSql(builder);

//         // 重命名
//         builder.Clear();
//         builder.Table("tbl_role").Rename("TestMgr", "TestMgrRename").Rename("TestMgr2", "TestMgr2Rename");
//         mysqlConnection->ExcuteSql(builder);

//         g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "last insert id:%lld"), mysqlConnection->GetLastInsertIdOfAutoIncField());

//         // 修改
//         builder.Clear();
//         builder.Table("tbl_role").Modify("TestMgrRename", "BIGINT NOT NULL DEFAULT 0 COMMENT '测试'").Modify("TestMgr2Rename", "BIGINT NOT NULL DEFAULT 0 COMMENT '测试2'");
//         mysqlConnection->ExcuteSql(builder);

//         // 建立索引
//         builder.Clear();
//         builder.Table("tbl_role").AddIndex("idx_test", {"TestMgrRename", "TestMgr2Rename"}, "using btree", "测试索引");
//         mysqlConnection->ExcuteSql(builder);

//         // 建立唯一索引
//         builder.Clear();
//         builder.Table("tbl_role").AddUniqueIndex("idx_UserRoleId", {"UserId", "RoleId"}, "using btree", "测试唯一索引");
//         mysqlConnection->ExcuteSql(builder);

//         // 全文索引
//         builder.Clear();
//         builder.Table("tbl_role").AddIndex("idx_fulltext1", {"TestFulltextIndex1"}, "using btree", "测试全文索引", true, KERNEL_NS::FullTextParser::WITH_PARSER_NGRAM);
//         mysqlConnection->ExcuteSql(builder);
        
//         // 删除索引
//         builder.Clear();
//         builder.Table("tbl_role").DropIndex("idx_test").DropIndex("idx_UserRoleId");
//         mysqlConnection->ExcuteSql(builder);

//         // 修改
//         builder.Clear();
//         builder.Table("tbl_role").Modify("Id", "BIGINT NOT NULL COMMENT 'id'");
//         mysqlConnection->ExcuteSql(builder);

//         // 删除主键
//         builder.Clear();
//         builder.Table("tbl_role").DropPrimaryKey();
//         mysqlConnection->ExcuteSql(builder);

//         // 添加主键
//         builder.Clear();
//         builder.Table("tbl_role").AddPrimaryKey({"Id", "RoleId"});
//         mysqlConnection->ExcuteSql(builder);

//         // 删
//         builder.Clear();
//         builder.Table("tbl_role").Drop("TestMgrRename").Drop("TestMgr2Rename");
//         mysqlConnection->ExcuteSql(builder);
//     }

//     {// 查索引
//         KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::SHOW_INDEX> builder;
//         builder.Table("tbl_role");
//         mysqlConnection->ExcuteSql(builder);

//         mysqlConnection->UseResult([](const KERNEL_NS::MysqlConnect *, MYSQL_RES *){});
//     }

//     {// update数据
//         KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::UPDATE> builder;
//         builder.Table("tbl_role")
//         .Set("Name", "\"Yonng4\"")
//         .Set("UserId", "\"Eric4\"")
//         .Where("`Id`=1003")
//         ;

//         mysqlConnection->ExcuteSql(builder);
//     }

//     {// 查询
//         KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::SELECT> builder;
//         builder.From("tbl_role")
//         .OrderBy({"Id desc", "RoleId asc"}).Limit(5).Where("Name LIKE \"Yonng%\"");

//         mysqlConnection->ExcuteSql(builder);

//         mysqlConnection->StoreResult([](const KERNEL_NS::MysqlConnect *connect, MYSQL_RES *res){
            
//         });

//         g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "last insert id:%lld"), mysqlConnection->GetLastInsertIdOfAutoIncField());
//     }

//     {// delete数据
//         KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::DELETE_RECORD> builder;
//         builder.Table("tbl_role")
//         .Where("Id=1003");
//         ;

//         mysqlConnection->ExcuteSql(builder);
//     }

//     {// delete数据
//         KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::DELETE_RECORD> builder;
//         builder.Table("tbl_role")
//         .Where("Id>0");
//         ;

//         mysqlConnection->ExcuteSql(builder);
//     }

//     {// optimize table
//         KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::OPTIMIZE_TABLE> builder;
//         builder.Table("tbl_role")
//         ;

//         mysqlConnection->ExcuteSql(builder);

//         mysqlConnection->StoreResult([](const KERNEL_NS::MysqlConnect *conn, MYSQL_RES *res){});
//     }

//     {// 清空数据
//         KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::TRUNCATE_TABLE> builder;
//         builder.Table("tbl_role")
//         ;

//         mysqlConnection->ExcuteSql(builder);
//     }
// }

// 测试增删改查, 多条sql等
// void TestSql::Run()
// {
//     KERNEL_NS::SnowflakeInfo snowFlakeInfo;
//     KERNEL_NS::GuidUtil::InitSnowFlake(snowFlakeInfo, 1, static_cast<UInt64>(KERNEL_NS::LibTime::NowTimestamp()));

//     KERNEL_NS::SmartPtr<KERNEL_NS::MysqlConnect, KERNEL_NS::AutoDelMethods::CustomDelete> mysqlConnection = KERNEL_NS::MysqlConnect::New_MysqlConnect(KERNEL_NS::GuidUtil::Snowflake(snowFlakeInfo));
//     mysqlConnection.SetClosureDelegate([](void *p){
//         auto ptr = KERNEL_NS::KernelCastTo<KERNEL_NS::MysqlConnect>(p);
//         KERNEL_NS::MysqlConnect::Delete_MysqlConnect(ptr);
//     });

//     KERNEL_NS::MysqlConfig config;
//     config._host = "127.0.0.1";
//     config._user = "root";
//     config._pwd = "123456";
//     config._dbName = "rpg";
//     config._port = 3306;
//     // config._maxPacketSize = 2 * 1024LLU * 1024LLU * 1024LLU;

//     // 利用 MYSQL_OPT_BIND 选项绑定多张网卡中的一张
//     config._bindIp = "127.0.0.1";
//     mysqlConnection->SetConfig(config);

//     auto err = mysqlConnection->Init();
//     if(err != Status::Success)
//     {
//         g_Log->Error(LOGFMT_NON_OBJ_TAG(TestSql, "mysql init fail."));
//         return;
//     }

//     err = mysqlConnection->Start();
//     if(err != Status::Success)
//     {
//         g_Log->Error(LOGFMT_NON_OBJ_TAG(TestSql, "mysql start fail."));
//         return;
//     }

//     {// 未实现的builder类型 执行sql时候会报错
//         // KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::KB> builder;

//         // mysqlConnection->ExcuteSql(builder);
//     }

//     {// 删除表
//         KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::DROP_TABLE> builder;
//         builder.Table("tbl_role");

//         mysqlConnection->ExcuteSql(builder);
//     }

//     {// 创建表
//         KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::CREATE_TABLE> builder;
//         builder.Table("tbl_role")
//         .Field("Id BIGINT NOT NULL AUTO_INCREMENT COMMENT 'id'")
//         // .Field("Id BIGINT NOT NULL DEFAULT 0 COMMENT \"id\"")
//         // .Field("Id2 BIGINT NOT NULL AUTO_INCREMENT COMMENT \"Id2\"")
//         .Field("RoleId INT NOT NULL COMMENT '角色id'")
//         .Field("UserId VARCHAR(4096) NOT NULL DEFAULT '' COMMENT '账号id'")
//         .Field("Name VARCHAR(4096) NOT NULL DEFAULT '' COMMENT '名字'")
//         .Field("TestText TEXT CHARACTER SET utf8mb4 COLLATE utf8mb4_bin COMMENT 'test text'")
//         .Field("LoginTime INT DEFAULT 0 COMMENT '登录时间'")
//         .PrimaryKey("Id")
//         .Comment("role table")
//         ;
//         mysqlConnection->ExcuteSql(builder);
//     }

//     {// 改表引擎
//         KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::ALTER_TABLE> builder;
//         builder.Table("tbl_role").ChangeEngine("MyISAM");
//         mysqlConnection->ExcuteSql(builder);

//         builder.Clear().Table("tbl_role").ChangeEngine("InnoDB");
//         mysqlConnection->ExcuteSql(builder);
//     }

//     KERNEL_NS::LibString data2k;
//     const Int32 dataBytes = 2048;
//     data2k.AppendData("'");
//     for(Int32 idx = 0; idx < dataBytes; ++idx)
//         data2k.AppendData("a");
//     data2k.AppendData("'");

//     {// 插入数据
//         const auto cpucount = KERNEL_NS::LibCpuCounter::Current();
//         const Int32 count = 5;

//         for(Int32 idx = 0; idx < count; ++idx)
//         {
//             KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::INSERT> builder;
//             builder.Table("tbl_role")
//             .Fields({"RoleId", "UserId", "Name"})
//             .Values({"100101", data2k, data2k});

//             mysqlConnection->ExcuteSql(builder);

//             auto insertId = mysqlConnection->GetLastInsertIdOfAutoIncField();
//             insertId = mysqlConnection->GetLastInsertIdOfAutoIncField();

//             auto affectedRow = mysqlConnection->GetLastAffectedRow();
//             affectedRow = mysqlConnection->GetLastAffectedRow();
//         }

//         auto insertId = mysqlConnection->GetLastInsertIdOfAutoIncField();
//         auto affectedRow = mysqlConnection->GetLastAffectedRow();

//         auto elapseMs = KERNEL_NS::LibCpuCounter::Current().ElapseMilliseconds(cpucount);
//         g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "insert %d record cost:%llu ms"), count, elapseMs);
//     }

//     // {// 查询 UseResult
//     //     KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::SELECT> builder;
//     //     builder.From("tbl_role")
//     //     ;

//     //     mysqlConnection->ExcuteSql(builder);

//     //     // 对比use result / store result 在断线重连情况下的情况
//     //     mysqlConnection->UseResult([](KERNEL_NS::MysqlConnect *connect, MYSQL_RES *res){

//     //             // 没有结果
//     //             if(!res)
//     //                 return;

//     //             Int64 totalCount = 0;
//     //             connect->FetchRow(res, [&totalCount](KERNEL_NS::MysqlConnect *conn, bool hasRecord, KERNEL_NS::SmartPtr<KERNEL_NS::Record, KERNEL_NS::AutoDelMethods::CustomDelete> &record){
//     //                 // g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "get record:%s, totalRows:%lld"), record->ToString().c_str());
//     //                 ++totalCount;

//     //                 // 取走数据:
//     //                 auto r = record.pop();
//     //                 KERNEL_NS::Record::DeleteThreadLocal_Record(r);

//     //                 // if(totalCount < 120)
//     //                 //     KERNEL_NS::SystemUtil::ThreadSleep(1000);
//     //             });

//     //             auto rowNum = connect->GetCurrentResultRows(res);
//     //             g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "get record count:%lld, current res row num:%lld"), totalCount, rowNum);
//     //     });
//     // }

//     // {// 查询 StoreResult
//     //     KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::SELECT> builder;
//     //     builder.From("tbl_role")
//     //     ;

//     //     mysqlConnection->ExcuteSql(builder);

//     //     // 对比use result / store result 在断线重连情况下的情况
//     //     mysqlConnection->StoreResult([](KERNEL_NS::MysqlConnect *connect, MYSQL_RES *res){

//     //             // 没有结果
//     //             if(!res)
//     //                 return;

//     //             Int64 totalCount = 0;
//     //             connect->FetchRow(res, [&totalCount](KERNEL_NS::MysqlConnect *conn, bool hasRecord, KERNEL_NS::SmartPtr<KERNEL_NS::Record, KERNEL_NS::AutoDelMethods::CustomDelete> &record){
//     //                 // g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "get record:%s, totalRows:%lld"), record->ToString().c_str());
//     //                 ++totalCount;

//     //                 // 取走数据:
//     //                 auto r = record.pop();
//     //                 KERNEL_NS::Record::DeleteThreadLocal_Record(r);

//     //                 // if(totalCount < 120)
//     //                 //     KERNEL_NS::SystemUtil::ThreadSleep(1000);
//     //             });

//     //             auto rowNum = connect->GetCurrentResultRows(res);
//     //             g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "get record count:%lld, current res row num:%lld"), totalCount, rowNum);
//     //     });
//     // }

//     {// 分页查询
//         Int64 maxId = 0;
//         bool isFinished = false;

//         do
//         {
//             KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::SELECT> builder;
//             builder.From("tbl_role")
//             .OrderBy("Id asc")
//             .Where(KERNEL_NS::LibString().AppendFormat("`Id` > %lld", maxId))
//             .Limit(1000)
//             ;

//             mysqlConnection->ExcuteSql(builder);

//             // 对比use result / store result 在断线重连情况下的情况
//             mysqlConnection->StoreResult([&maxId, &isFinished](KERNEL_NS::MysqlConnect *connect, MYSQL_RES *res){

//                     // 没有结果
//                     if(!res || (connect->GetCurrentResultRows(res) <= 0))
//                     {
//                         isFinished = true;
//                         return;
//                     }

//                     Int64 totalCount = 0;
//                     connect->FetchRow(res, [&maxId, &totalCount](KERNEL_NS::MysqlConnect *conn, bool hasRecord, KERNEL_NS::SmartPtr<KERNEL_NS::Record, KERNEL_NS::AutoDelMethods::CustomDelete> &record){
//                         // g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "get record:%s, totalRows:%lld"), record->ToString().c_str());
//                         if(!hasRecord)
//                         {
//                             g_Log->Warn(LOGFMT_NON_OBJ_TAG(TestSql, "have no records, conn info:%s"), conn->ToString().c_str());
//                             return;
//                         }

//                         ++totalCount;

//                         auto field = record->GetField("Id");

//                         // field数据都是字符串, 此时需要从字符串解析成Id
//                         KERNEL_NS::LibString dataStr;
//                         dataStr.AppendData(field->GetData()->GetReadBegin(), field->GetData()->GetReadableSize());
//                         auto curId = KERNEL_NS::StringUtil::StringToInt64(dataStr.c_str());
//                         if(maxId < curId)
//                             maxId = curId;

//                         // 取走数据:
//                         auto r = record.pop();
//                         KERNEL_NS::Record::DeleteThreadLocal_Record(r);
//                     });

//                     auto rowNum = connect->GetCurrentResultRows(res);
//                     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "get record count:%lld, current res row num:%lld"), totalCount, rowNum);
//             });

//             if(isFinished)
//                 break;

//         } while (true);

//         g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "maxId:%lld"), maxId);
//     }

//     {// 简单查询 FetchRows
//         Int64 maxId = 0;
//         bool isFinished = false;

//         do
//         {
//             KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::SELECT> builder;
//             builder.From("tbl_role")
//             .OrderBy("Id asc")
//             .Where(KERNEL_NS::LibString().AppendFormat("`Id` > %lld", maxId))
//             .Limit(1000)
//             ;

//             mysqlConnection->ExcuteSql(builder);

//             // 对比use result / store result 在断线重连情况下的情况
//             mysqlConnection->StoreResult([&maxId, &isFinished](KERNEL_NS::MysqlConnect *connect, MYSQL_RES *res){

//                     // 没有结果
//                     if(!res || (connect->GetCurrentResultRows(res) <= 0))
//                     {
//                         isFinished = true;
//                         return;
//                     }

//                     Int64 totalCount = 0;
//                     connect->FetchRows(res, [&maxId, &totalCount](KERNEL_NS::MysqlConnect *conn, bool hasRecord, std::vector<KERNEL_NS::SmartPtr<KERNEL_NS::Record, KERNEL_NS::AutoDelMethods::CustomDelete>> &records){
//                         // g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "get record:%s, totalRows:%lld"), record->ToString().c_str());
//                         if(!hasRecord)
//                         {
//                             g_Log->Warn(LOGFMT_NON_OBJ_TAG(TestSql, "have no records, conn info:%s"), conn->ToString().c_str());
//                             return;
//                         }

//                         totalCount = static_cast<Int64>(records.size());

//                         for(auto &record : records)
//                         {
//                             auto field = record->GetField("Id");

//                             // field数据都是字符串, 此时需要从字符串解析成Id
//                             KERNEL_NS::LibString dataStr;
//                             dataStr.AppendData(field->GetData()->GetReadBegin(), field->GetData()->GetReadableSize());
//                             auto curId = KERNEL_NS::StringUtil::StringToInt64(dataStr.c_str());
//                             if(maxId < curId)
//                                 maxId = curId;

//                             // 取走数据:
//                             auto r = record.pop();
//                             KERNEL_NS::Record::DeleteThreadLocal_Record(r);
//                         }
//                     });

//                     auto rowNum = connect->GetCurrentResultRows(res);
//                     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "get record count:%lld, current res row num:%lld"), totalCount, rowNum);
//             });

//             if(isFinished)
//                 break;

//         } while (true);

//         g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "maxId:%lld"), maxId);
//     }

//     {// 执行多条sql
//         std::vector<KERNEL_NS::LibString> sqls;

//         // 1.插入一条数据
//         {
//             KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::INSERT> builder;
//             builder.Table("tbl_role")
//             .Fields({"RoleId", "UserId", "Name"})
//             .Values({"100101", data2k, data2k});
//             sqls.push_back(builder.ToSql());
//         }

//         {// 修改数据
//             KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::UPDATE> builder;
//             builder.Table("tbl_role")
//             .Set("RoleId", "1")
//             .Where("Id = 1");
//             sqls.push_back(builder.ToSql());
//         }

//         {// 查询数据
//             KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::SELECT> builder;
//             builder.From("tbl_role")
//             .OrderBy("Id asc")
//             .Where(KERNEL_NS::LibString().AppendFormat("`Id` > %lld", 0))
//             .Limit(1000)
//             ;
//             sqls.push_back(builder.ToSql());
//         }

//         const auto &multiSql = KERNEL_NS::StringUtil::ToString(sqls, ";");
//         mysqlConnection->ExcuteSql(multiSql);

//         Int64 countRow = 0;
//         Int32 resCount = 0;
//         mysqlConnection->StoreResult([&countRow, &resCount](KERNEL_NS::MysqlConnect *connect, MYSQL_RES *res)
//         {
//             ++resCount;
//             // 没有结果(create/update/delete/insert 是没有结果的)
//             if(!res || (connect->GetCurrentResultRows(res) <= 0))
//             {
//                 return;
//             }

//             Int64 totalCount = 0;
//             connect->FetchRow(res, [&countRow](KERNEL_NS::MysqlConnect *conn, bool hasRecord, KERNEL_NS::SmartPtr<KERNEL_NS::Record, KERNEL_NS::AutoDelMethods::CustomDelete> &record){
//                 // g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "get record:%s, totalRows:%lld"), record->ToString().c_str());
//                 if(!hasRecord)
//                 {
//                     g_Log->Warn(LOGFMT_NON_OBJ_TAG(TestSql, "have no records, conn info:%s"), conn->ToString().c_str());
//                     return;
//                 }

//                 ++countRow;

//                 auto idField = record->GetField("Id");
//                 auto roleIdField = record->GetField("RoleId");

//                 // field数据都是字符串, 此时需要从字符串解析成Id
//                 KERNEL_NS::LibString dataStr;
//                 dataStr.AppendData(idField->GetData()->GetReadBegin(), idField->GetData()->GetReadableSize());
                
//                 auto id = KERNEL_NS::StringUtil::StringToInt64(dataStr.c_str());
//                 dataStr.clear();
//                 dataStr.AppendData(roleIdField->GetData()->GetReadBegin(), roleIdField->GetData()->GetReadableSize());
//                 auto roleid = KERNEL_NS::StringUtil::StringToInt32(dataStr.c_str());

//                 g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "tbl:%s, %s:%lld, %s:%d"), idField->GetTableName().c_str(), idField->GetName().c_str(), id, roleIdField->GetName().c_str(), roleid);

//                 // 取走数据:
//                 auto r = record.pop();
//                 KERNEL_NS::Record::DeleteThreadLocal_Record(r);
//             });

//             auto rowNum = connect->GetCurrentResultRows(res);
//             g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "get record count:%lld, current res row num:%lld"), totalCount, rowNum);
//         });

//         g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "countRow:%lld, resCount:%d"), countRow, resCount);
//     }

//     // delete数据后看 optimize的结果
//     {// delete数据
//         KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::DELETE_RECORD> builder;
//         builder.Table("tbl_role")
//         ;

//         mysqlConnection->ExcuteSql(builder);
//     }

//     {// optimize table
//         KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::OPTIMIZE_TABLE> builder;
//         builder.Table("tbl_role")
//         ;

//         mysqlConnection->ExcuteSql(builder);

//         mysqlConnection->StoreResult([](KERNEL_NS::MysqlConnect *conn, MYSQL_RES *res){});
//     }

//     {// 清空数据
//         KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::TRUNCATE_TABLE> builder;
//         builder.Table("tbl_role")
//         ;

//         mysqlConnection->ExcuteSql(builder);
//     }
// }

// 测试事务
// 结论: 
//     1.开启事务与没开启事务性能对比:开启事务是没有开启的2倍多(100000条数据插入, 没开启事务:59014ms, 开启事务:22226ms)
//     2.多条sql合并成一条sql后执行mysql_real_query(100000条数据插入，没开启事务用时1919ms,开启事务用时:811ms)
//     3.开启事务流程:START_TRANSACTION -> set autocommit=0 => 要执行的sql => 调用ExcuteSql-> 获取AffectedRows和InsertId -> 可选:ROLLBACK(回滚数据) -> Commit -> set autocommit=1 ->调用ExcuteSql
//     4.直接使用UseTransActionExcuteSql执行开启事务的Sql(只需在回调中处理相关查询的结果的逻辑, 内部已经封装好事务)
//     5.直接使用ExcuteSql 执行不带事务的Sql
//     6.一次性执行多条sql, 10w条数据, 带事务的整个处理完花费:14060ms(7142 qps), 不带事务的花费:49775ms (2040 qps)
// void TestSql::Run()
// {
//     KERNEL_NS::SnowflakeInfo snowFlakeInfo;
//     KERNEL_NS::GuidUtil::InitSnowFlake(snowFlakeInfo, 1, static_cast<UInt64>(KERNEL_NS::LibTime::NowTimestamp()));

//     KERNEL_NS::SmartPtr<KERNEL_NS::MysqlConnect, KERNEL_NS::AutoDelMethods::CustomDelete> mysqlConnection = KERNEL_NS::MysqlConnect::New_MysqlConnect(KERNEL_NS::GuidUtil::Snowflake(snowFlakeInfo));
//     mysqlConnection.SetClosureDelegate([](void *p){
//         auto ptr = KERNEL_NS::KernelCastTo<KERNEL_NS::MysqlConnect>(p);
//         KERNEL_NS::MysqlConnect::Delete_MysqlConnect(ptr);
//     });

//     KERNEL_NS::MysqlConfig config;
//     config._host = "127.0.0.1";
//     config._user = "root";
//     config._pwd = "123456";
//     config._dbName = "rpg";
//     config._port = 3306;
//     // config._maxPacketSize = 2 * 1024LLU * 1024LLU * 1024LLU;

//     // 利用 MYSQL_OPT_BIND 选项绑定多张网卡中的一张
//     config._bindIp = "127.0.0.1";
//     mysqlConnection->SetConfig(config);

//     auto err = mysqlConnection->Init();
//     if(err != Status::Success)
//     {
//         g_Log->Error(LOGFMT_NON_OBJ_TAG(TestSql, "mysql init fail."));
//         return;
//     }

//     err = mysqlConnection->Start();
//     if(err != Status::Success)
//     {
//         g_Log->Error(LOGFMT_NON_OBJ_TAG(TestSql, "mysql start fail."));
//         return;
//     }

//     {// 删除表
//         KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::DROP_TABLE> builder;
//         builder.Table("tbl_role");
//         mysqlConnection->ExcuteSql(builder, KERNEL_NS::MysqlConnect::FUNC_NULL);
//         builder.Table("tbl_role2");
//         mysqlConnection->ExcuteSql(builder, KERNEL_NS::MysqlConnect::FUNC_NULL);
//     }

//     {// 创建表
//         KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::CREATE_TABLE> builder;
//         builder.Table("tbl_role")
//         .Field("Id BIGINT NOT NULL AUTO_INCREMENT COMMENT 'id'")
//         // .Field("Id BIGINT NOT NULL DEFAULT 0 COMMENT \"id\"")
//         // .Field("Id2 BIGINT NOT NULL AUTO_INCREMENT COMMENT \"Id2\"")
//         .Field("RoleId INT NOT NULL COMMENT '角色id'")
//         .Field("UserId VARCHAR(4096) NOT NULL DEFAULT '' COMMENT '账号id'")
//         .Field("Name VARCHAR(4096) NOT NULL DEFAULT '' COMMENT '名字'")
//         .Field("TestText TEXT CHARACTER SET utf8mb4 COLLATE utf8mb4_bin COMMENT 'test text'")
//         .Field("LoginTime INT DEFAULT 0 COMMENT '登录时间'")
//         .PrimaryKey("Id")
//         .Comment("role table")
//         ;
//         mysqlConnection->ExcuteSql(builder, KERNEL_NS::MysqlConnect::DELG_NULL);
//     }

//     {// 创建表2
//         KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::CREATE_TABLE> builder;
//         builder.Table("tbl_role2")
//         .Field("Id BIGINT NOT NULL AUTO_INCREMENT COMMENT 'id'")
//         // .Field("Id BIGINT NOT NULL DEFAULT 0 COMMENT \"id\"")
//         // .Field("Id2 BIGINT NOT NULL AUTO_INCREMENT COMMENT \"Id2\"")
//         .Field("RoleId INT NOT NULL COMMENT '角色id'")
//         .Field("UserId VARCHAR(4096) NOT NULL DEFAULT '' COMMENT '账号id'")
//         .Field("Name VARCHAR(4096) NOT NULL DEFAULT '' COMMENT '名字'")
//         .Field("TestText TEXT CHARACTER SET utf8mb4 COLLATE utf8mb4_bin COMMENT 'test text'")
//         .Field("LoginTime INT DEFAULT 0 COMMENT '登录时间'")
//         .PrimaryKey("Id")
//         .Comment("role table2")
//         ;
//         mysqlConnection->ExcuteSql(builder, KERNEL_NS::MysqlConnect::DELG_NULL);
//     }

//     // {// 改表引擎
//     //     KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::ALTER_TABLE> builder;
//     //     builder.Table("tbl_role").ChangeEngine("MyISAM");
//     //     mysqlConnection->ExcuteSql(builder);

//     //     builder.Clear().Table("tbl_role").ChangeEngine("InnoDB");
//     //     mysqlConnection->ExcuteSql(builder);
//     // }

//     KERNEL_NS::LibString data2k;
//     const Int32 dataBytes = 2048;
//     data2k.AppendData("'");
//     for(Int32 idx = 0; idx < dataBytes; ++idx)
//         data2k.AppendData("a");
//     data2k.AppendData("'");

//     const Int64 count = 100000;
//     {// 插入数据

//         std::vector<KERNEL_NS::LibString> multiSql;
//         for(Int64 idx = 0; idx < count; ++idx)
//         {
//             KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::INSERT> builder;
//             builder.Table("tbl_role")
//             .Fields({"RoleId", "UserId", "Name"})
//             .Values({"100101", data2k, data2k});

//             multiSql.push_back(builder.ToSql());
//         }

//         const auto &sql = KERNEL_NS::StringUtil::ToString(multiSql, ";");

//         Int64 incId = 0;
//         Int32 resCount = 0;
//         Int64 affectedRowCount = 0;

//         const auto cpucount = KERNEL_NS::LibCpuCounter::Current();
//         mysqlConnection->ExcuteSql(sql, [&incId, &resCount, &affectedRowCount](KERNEL_NS::MysqlConnect *conn, bool isSucSendToMysql, MYSQL_RES *res, bool isFinished){
//             if(!isSucSendToMysql)
//             {
//                 g_Log->Warn(LOGFMT_NON_OBJ_TAG(TestSql, "send to mysql fail"));
//                 return;
//             }

//             if(isFinished)
//             {
//                 g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "insert all records resCount:%d, affectedRowCount:%lld"), resCount, affectedRowCount);
//                 return;
//             }

//             ++resCount;
//             const auto insertId = conn->GetLastInsertIdOfAutoIncField();
//             if(insertId > incId)
//                 incId = insertId;

//             affectedRowCount += conn->GetLastAffectedRow();

//         }, false);
//         auto elapseMs = KERNEL_NS::LibCpuCounter::Current().ElapseMilliseconds(cpucount);

//         g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "insert %lld record cost:%llu ms, last incId:%lld, resCount:%d, affectedRowCount:%lld"), count, elapseMs, incId, resCount, affectedRowCount);
//     }

//     {
//         // 开启事务插入数据
//         Int64 incId = 0;
//         std::vector<KERNEL_NS::LibString> multiSql;
//         for(Int64 idx = 0; idx < count; ++idx)
//         {
//             KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::INSERT> builder;
//             builder.Table("tbl_role2")
//             .Fields({"RoleId", "UserId", "Name"})
//             .Values({"100101", data2k, data2k});

//             multiSql.push_back(builder.ToSql());
//         }

//         const auto &sql = KERNEL_NS::StringUtil::ToString(multiSql, ";");

//         // 使用事务执行sql
//         Int32 resCount = 0;
//         Int64 affectedRowCount = 0;

//         const auto cpucount = KERNEL_NS::LibCpuCounter::Current();
//         mysqlConnection->UseTransActionExcuteSql(sql, [&incId, &resCount, &affectedRowCount, count](KERNEL_NS::MysqlConnect *conn,  bool isSucSendToMysql, MYSQL_RES *res, bool isFinished, bool &needRollback){
//             if(!isSucSendToMysql)
//             {
//                 g_Log->Warn(LOGFMT_NON_OBJ_TAG(TestSql, "send to mysql fail"));
//                 return;
//             }

//             if(isFinished)
//             {
//                 needRollback = count != affectedRowCount;
//                 g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "insert all records resCount:%d, affectedRowCount:%lld, needRollback:%d"), resCount, affectedRowCount, needRollback);
//                 return;
//             }

//             ++resCount;
//             Int64 insertId = conn->GetLastInsertIdOfAutoIncField();
//             if(incId <  insertId)
//                 incId = insertId;

//             affectedRowCount += conn->GetLastAffectedRow();

//         }, false);
//         auto elapseMs = KERNEL_NS::LibCpuCounter::Current().ElapseMilliseconds(cpucount);

//         g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "insert %lld record cost:%llu ms, last inc id:%lld, resCount:%d, affectedRowCount:%lld"), count, elapseMs, incId, resCount, affectedRowCount);
//     }

//     {// 清空数据
//         // 清理result
//         mysqlConnection->StoreResult([](KERNEL_NS::MysqlConnect *conn, MYSQL_RES *res){

//         });
        
//         KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::TRUNCATE_TABLE> builder;
//         builder.Table("tbl_role")
//         ;
//         mysqlConnection->ExcuteSql(builder, KERNEL_NS::MysqlConnect::FUNC_NULL);
//         builder.Table("tbl_role2");
//         mysqlConnection->ExcuteSql(builder, KERNEL_NS::MysqlConnect::DELG_NULL);

//         // 文件流
//         // std::fstream in("ini.ini", std::ios::in | std::ios::binary);
//     }
// }

void TestSql::Run()
{
    KERNEL_NS::SnowflakeInfo snowFlakeInfo;
    KERNEL_NS::GuidUtil::InitSnowFlake(snowFlakeInfo, 1, static_cast<UInt64>(KERNEL_NS::LibTime::NowTimestamp()));

    KERNEL_NS::SmartPtr<KERNEL_NS::MysqlConnect, KERNEL_NS::AutoDelMethods::CustomDelete> mysqlConnection = KERNEL_NS::MysqlConnect::New_MysqlConnect(KERNEL_NS::GuidUtil::Snowflake(snowFlakeInfo));
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
    // config._maxPacketSize = 2 * 1024LLU * 1024LLU * 1024LLU;

    // 利用 MYSQL_OPT_BIND 选项绑定多张网卡中的一张
    config._bindIp = "127.0.0.1";
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

    {// 删除表
        KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::DROP_TABLE> builder;
        builder.Table("tbl_role");
        mysqlConnection->ExecuteSql(builder, 1, KERNEL_NS::MysqlConnect::FUNC_NULL);
    }

    {// 创建表
        KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::CREATE_TABLE> builder;
        builder.Table("tbl_role")
        .Field("Id BIGINT NOT NULL AUTO_INCREMENT COMMENT 'id'")
        // .Field("Id BIGINT NOT NULL DEFAULT 0 COMMENT \"id\"")
        // .Field("Id2 BIGINT NOT NULL AUTO_INCREMENT COMMENT \"Id2\"")
        .Field("RoleId INT NOT NULL COMMENT '角色id'")
        .Field("UserId VARCHAR(4096) NOT NULL DEFAULT '' COMMENT '账号id'")
        .Field("Name VARCHAR(4096) NOT NULL DEFAULT '' COMMENT '名字'")
        .Field("TestText TEXT CHARACTER SET utf8mb4 COLLATE utf8mb4_bin COMMENT 'test text'")
        .Field("LoginTime INT DEFAULT 0 COMMENT '登录时间'")
        .PrimaryKey("Id")
        .Comment("role table")
        ;
        mysqlConnection->ExecuteSql(builder, 1, KERNEL_NS::MysqlConnect::DELG_NULL);
    }

    KERNEL_NS::LibString data2k;
    const Int32 dataBytes = 2048;
    data2k.AppendData("'");
    for(Int32 idx = 0; idx < dataBytes; ++idx)
        data2k.AppendData("a");
    data2k.AppendData("'");

    const Int64 count = 100000;
    Int64 incId = 0;
    {// 插入数据
        std::vector<KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::INSERT>> multiSql;
        for(Int64 idx = 0; idx < count; ++idx)
        {
            KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::INSERT> builder;
            builder.Table("tbl_role")
            .Fields({"RoleId", "UserId", "Name"})
            .Values({"100101", data2k, data2k});

            multiSql.push_back(builder);
        }

        Int32 resCount = 0;
        Int64 affectedRowCount = 0;

        const auto cpucount = KERNEL_NS::LibCpuCounter::Current();
        mysqlConnection->ExecuteSqlUsingTransAction(multiSql, 1, [&incId, &resCount, &affectedRowCount] (KERNEL_NS::MysqlConnect *conn, UInt64 seqId, Int32 errCode, UInt32 mysqlErrno, bool isSendToMysql, Int64 insertId, Int64 affectedRows, std::vector<KERNEL_NS::SmartPtr<KERNEL_NS::Record, KERNEL_NS::AutoDelMethods::CustomDelete>> &records){
            
            if(errCode != Status::Success)
            {
                g_Log->Warn(LOGFMT_NON_OBJ_TAG(TestSql, "insert fail errCode:%d, mysqlErrno:%u connection:%s"), errCode, mysqlErrno, conn->ToString().c_str());
                return;
            }

            if(incId < insertId)
                incId = insertId;

            affectedRowCount += affectedRows;
            ++resCount;

            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "inert seqId:%llu,ast incId:%lld, resCount:%d, affectedRowCount:%lld"), seqId, incId, resCount, affectedRowCount);
        });
        auto elapseMs = KERNEL_NS::LibCpuCounter::Current().ElapseMilliseconds(cpucount);

        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "insert %lld record cost:%llu ms, last incId:%lld, resCount:%d, affectedRowCount:%lld"), count, elapseMs, incId, resCount, affectedRowCount);
    }

    // 查询数据
    {
        const Int32 selectCount = 10;
        KERNEL_NS::SqlBuilder<KERNEL_NS::SqlBuilderType::SELECT> builder;
        builder.From("tbl_role")
        .OrderBy("Id asc")
        .Where(KERNEL_NS::LibString().AppendFormat("`Id` > %d", 0))
        .Limit(selectCount)
        ;

        const auto cpucount = KERNEL_NS::LibCpuCounter::Current();
        mysqlConnection->ExecuteSql(builder, 1, [](KERNEL_NS::MysqlConnect *conn, UInt64 seqId, Int32 errCode, UInt32 mysqlErrno, bool isSendToMysql, Int64 insertId, Int64 affectedRows, std::vector<KERNEL_NS::SmartPtr<KERNEL_NS::Record, KERNEL_NS::AutoDelMethods::CustomDelete>> &records){

            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "select seqId:%llu,ast insertId:%lld, affectedRowCount:%lld"), seqId, insertId, affectedRows);
        });
        auto elapseMs = KERNEL_NS::LibCpuCounter::Current().ElapseMilliseconds(cpucount);
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "select %d record cost:%llu ms"), selectCount, elapseMs);
    }

    // 返回多值属于结构化绑定 至少需要c++17 
    // const auto &[err, errStr] = mysqlConnection->TestMulti();
    // g_Log->Info(LOGFMT_NON_OBJ_TAG(TestSql, "err:%d, errStr:%s"), err, errStr.c_str());
}
