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
 * Date: 2025-05-12 23:29:10
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestMongo.h>

#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/exception/exception.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/pool.hpp>
#include <iostream>

#include <protocols/protocols.h>


void TestMongo::Run()
{
    mongocxx::instance instance;

    try
    {
        // Start example code here 密码特殊符号需要使用url编码, 需要把所有的节点域名或者ip列出来避免某个节点不可用
        // 写关注：w=majority, journal=true
        // 读关注：&readConcernLevel=majority
        mongocxx::uri uri("mongodb://testmongo:abc%5E159%40@127.0.0.1:28017,127.0.0.1:28018,127.0.0.1:28019/?authSource=admin&replicaSet=rs0&w=majority&journal=true&readConcernLevel=majority");
        mongocxx::client client(uri);
        // End example code here

        auto test2 = client["test2"];
        auto fruit = test2["fruit"];
        auto key = bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("name", "testmongo"));
        auto result = fruit.find_one(key.view());
        if(result)
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "find data:%s"), bsoncxx::to_json(*result).c_str());

        }
        else
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "find fail key:%s"), key.view().data());
        }

        // 连接池
        mongocxx::pool pool(uri);

        KERNEL_NS::LibThreadPool threadPool;
        threadPool.Init(0, 10);
        threadPool.Start();

        threadPool.AddTask2([&pool](KERNEL_NS::LibThreadPool *threadPool, KERNEL_NS::Variant *param)
        {
            try
            {
                    // 取一个连接
                auto client = pool.acquire();

                // 访问test2 数据库(不存在会在插入)
                auto test2 = client["test2"];
                auto collection = test2["new_collection"];

                // 设置表的大多数写成功, 且journal写完成功才算成功
                mongocxx::write_concern concern;
                // 大多数节点成功后成功
                concern.acknowledge_level(mongocxx::write_concern::level::k_majority);
                // 写操作落盘后成功
                concern.journal(true);
                collection.write_concern(concern);
                
                auto ret = collection.insert_one(bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("name", "testmongo")
                    , bsoncxx::builder::basic::kvp("sex", 1)));


                auto newDb = client->database("new_mongodb");
                auto player = newDb.create_collection("player4");

                // 创建唯一索引
                auto list_index = player.list_indexes();
                auto has_index = [&list_index](const std::string &fieldName)->bool
                {
                    for(auto &index : list_index)
                    {
                        auto key = index["key"];
                        auto bsonStr = bsoncxx::to_json(index);
                        auto keyDoc = key.get_document();
                        auto docStr = bsoncxx::to_json(keyDoc);

                        if(keyDoc.view().find(fieldName) != key.get_document().view().end())
                            return true;
                    }

                    return false;
                };
                
                if(!has_index("PlayerId"))
                {
                    auto key_index = bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("PlayerId", 1));
                    auto options = bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("unique", true), bsoncxx::builder::basic::kvp("name", "PlayerIdIndex"));
                    player.create_index(key_index.view(), options.view());
                }

                // 显示的创建表
                auto member = newDb.create_collection("member");

                auto generator = KERNEL_NS::TlsUtil::GetIdGenerator();

                auto playerId = generator->NewId();
                auto key = bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("PlayerId", static_cast<std::int64_t>(playerId)));

                // 读数据配置
                mongocxx::read_preference rp;
                // // 从主节点上读数据
                // rp.mode(mongocxx::read_preference::read_mode::k_primary);
                // // 只从从节点读
                // rp.mode(mongocxx::read_preference::read_mode::k_secondary);
                // // 优先从从节点读，不可用时到主节点
                // rp.mode(mongocxx::read_preference::read_mode::k_secondary_preferred);
                // // 优先从主节点读，不可用时到从节点
                // rp.mode(mongocxx::read_preference::read_mode::k_primary_preferred);
                // // 从延迟最低的节点读取(就近路由)
                // rp.mode(mongocxx::read_preference::read_mode::k_nearest);
                // // 结合标签过滤节点(region标签为east的节点， tags可以在mongodb的mongod.conf中的replication:tags:region:east, 配置)
                // rp.tags(bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("region", "east")));
                // // 设置读数据的节点
                // player.read_preference(rp);

                // 设置majority, 常用的隔离级别,相当于读已提交级别 解决事务的隔离性问题
                mongocxx::read_concern rc;
                rc.acknowledge_level(mongocxx::read_concern::level::k_majority);
                player.read_concern(rc);

                // 设置查询的最大等待时间, 防止游标因网络问题长时间阻塞：
                mongocxx::options::find opts{};
                opts.max_await_time(std::chrono::seconds(10)); // 等待新批次最多 10 秒

                auto findOne = player.find_one(key.view(), opts);
                if(findOne)
                {
                    // 更新
                    player.update_one(key.view(), bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("$set", bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("name", "bba")))));

                    // 替换
                    player.replace_one(key.view(), bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("PlayerId", (long long)playerId)));

                    // 删除
                    // player.delete_one(key.view());

                }
                else
                {
                    player.insert_one(bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("PlayerId", (long long)playerId)
                    , bsoncxx::builder::basic::kvp("name", "xiaoming")));
                }

                // GridFS存储大文件
                ::CRYSTAL_NET::service::LoginReq *req = new ::CRYSTAL_NET::service::LoginReq;
                auto userInfo = req->mutable_loginuserinfo();
                userInfo->set_loginmode(1);
                userInfo->set_accountname("xiaoming");
                userInfo->set_pwd("xiaoming");
                userInfo->set_logintoken("xxxxxxx4554");
                userInfo->set_port(5555);


                KERNEL_NS::LibString info;
                std::string data;
                req->SerializeToString(&data);
                auto dataJson = req->ToJsonString();

                // 存储二进制数据
                auto binData = bsoncxx::types::b_binary();
                binData.sub_type = bsoncxx::binary_sub_type::k_binary;
                binData.size = data.size();
                binData.bytes = (uint8_t *) data.data();
                auto binDoc = bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("BinData", binData));
                player.update_one(key.view(), bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("$set", bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("BinData", binData)))));
                
                // 创建GridFs存储桶
                auto bucket = newDb.gridfs_bucket();
                auto uploader = bucket.open_upload_stream(KERNEL_NS::LibString().AppendFormat("Player_%llu", playerId).c_str());
                uint8_t const *ptr = (uint8_t const *)data.data();
                uploader.write(ptr, data.size());
                auto result = uploader.close();

                // 更新gridfs 引用id
                auto resultId = result.id();
                player.update_one(key.view(), bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("$set", bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("GridFS_id", resultId)))));
                auto newPlayerData = player.find_one(key.view());
                if(newPlayerData)
                {
                    auto iter = newPlayerData.value().find("BinData");
                    if(iter != newPlayerData.value().end())
                    {
                        auto resultBin = iter->get_binary();
                        KERNEL_NS::LibString newBinBuffer;
                        newBinBuffer.AppendData((Byte8 *)resultBin.bytes, resultBin.size);
                        ::CRYSTAL_NET::service::LoginReq reqResult;
                        reqResult.ParseFromString(newBinBuffer.GetRaw());
                        auto resultJson = reqResult.ToJsonString();
                    }
                }

                auto downloader = bucket.open_download_stream(resultId);
                // auto chunkSize = 16 * 1024 *1024;
                auto buffer = KERNEL_NS::KernelAllocMemory<KERNEL_NS::_Build::TL>(1024);
                KERNEL_NS::LibString parseData;
                while(auto chunk = downloader.read((std::uint8_t*)buffer, 1024))
                {
                    parseData.AppendData((const Byte8 *)buffer, chunk);
                }

                ::CRYSTAL_NET::service::LoginReq *req2 = new ::CRYSTAL_NET::service::LoginReq;
                req2->ParseFromString(parseData.GetRaw());
                auto parseJson = req2->ToJsonString();

                // 遍历数据库表
                auto collections = newDb.list_collections();
                for(auto &doc : collections)
                    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "doc:%s"), bsoncxx::to_json(doc).c_str());

                // 删除集合
                auto test_drop = newDb.create_collection("test_drop_collection");
                test_drop.drop();
            }
            catch (const mongocxx::exception &e)
            {
                g_Log->Error(LOGFMT_NON_OBJ_TAG(TestMongo, "mongodb operation err:%s"), e.what());
                throw e;
            }
            catch (...)
            {
                g_Log->Error(LOGFMT_NON_OBJ_TAG(TestMongo, "mongodb operation err :unknown"));
                throw;
            }
        });

        threadPool.Close();
    }
    catch (const mongocxx::exception &e)
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(TestMongo, "An exception occurred:%s"), e.what());
    }
}
