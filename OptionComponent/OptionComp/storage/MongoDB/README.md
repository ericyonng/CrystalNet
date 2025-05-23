# 版本

* Driver: mongo-cxx-driver-r4.1.0.tar.gz
* 数据库版本号：mongodb-linux-x86_64-rhel8-8.0.6.tgz

# 官方开发文档

https://www.mongodb.com/zh-cn/docs/drivers/

driver api文档：https://mongocxx.org/api/mongocxx-4.1.0/

# 设计

* 通用数据包装

  * 可以抽象一个文档通用块

    message DocDataBlock

    {

      // 数据不超过 16 MB 存储BinData, 超过的存储GridFS, 并设置GridFsId

      bytes BinData = 1;

      string GridFsId = 2;

    }

    

    通过DocDataBlock 来解析文档数据

  



# 测试用例

测试用例见TestMongo.cpp

```c++

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
        // Start example code here 密码特殊符号需要使用url编码
        mongocxx::uri uri("mongodb://testmongo:abc%5E159%40@127.0.0.1:28017,127.0.0.1:28018,127.0.0.1:28019/?authSource=admin&replicaSet=rs0");
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
                auto key = bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("PlayerId", (long long)playerId));

                auto findOne = player.find_one(key.view());
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
                auto chunkSize = 16 * 1024 *1024;
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
```



# 连接池

* 必须使用连接池来控制连接数, 保证数据库不会过载, 默认连接池最小是0， 最大数量是100

  * ```
    #include <mongocxx/client.hpp>
    #include <mongocxx/instance.hpp>
    #include <mongocxx/pool.hpp>
    
    int main() {
        // 1. 初始化驱动实例（整个进程只需一次）
        mongocxx::instance instance{};
    
        // 2. 创建连接池，配置连接参数
        mongocxx::uri uri("mongodb://localhost:27017/?minPoolSize=10&maxPoolSize=100");
        mongocxx::pool pool{uri};
    
        // 3. 从池中获取连接
        auto client = pool.acquire();  // 返回一个 pool::entry 对象
    
        // 4. 使用连接操作数据库
        auto db = client->database("test");
        auto coll = db.collection("users");
        // 执行查询、插入等操作...
    
        // 5. 连接在 client 对象析构时自动归还到池中
        return 0;
    }
    ```

# 异常处理

mongodb操作触发异常会抛异常, 所以需要catch



# 注意

* mongodb 单文档有大小上限，这是BSON设计所决定的，上限是16MB， 当超过的时候无法正确存储

  * 一个方案是通过mongodb的GridFS，存储 + Player原表中创建GridFS的id引用, 当小于16MB的时候在Player的BinData直接存储，如果大于16MB的时候在GridFS中存，并引用id，不过需要关注一致性问题，因为毕竟存两个地方

  * 还有一个方案是，Player表进行分片存储, Player数据中存分片id数组

    
    
    
    
    
    
    
    
* 写操作必须要开启大多数写成功才算成功， 必须写操作落盘到journal后才算成功：
      

  ```
  // 设置表的大多数写成功, 且journal写完成功才算成功
    mongocxx::write_concern concern;
    
    // 大多数节点成功后成功
    concern.acknowledge_level(mongocxx::write_concern::level::k_majority);
    
    // 写操作落盘后成功
    concern.journal(true);
    collection.write_concern(concern);
  ```

  

* 读操作可以设置从节点读取, nearest读取等
  * ![image-20250518180853553](./img/readPreference配置.png)



* 比较严格的读隔离级别，线性化读
  * ![image-20250518190650496](D:\workplace\Development\CrystalNet\CrystalNet\doc\mongodbimage\线性化读.png)