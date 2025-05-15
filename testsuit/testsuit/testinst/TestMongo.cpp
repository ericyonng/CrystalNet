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
#include <iostream>

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
        auto key = bsoncxx::builder::stream::document{} << "name" << "eric" << bsoncxx::builder::stream::finalize;
        auto result = fruit.find_one(key.view());
        if(result)
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "find data:%s"), bsoncxx::to_json(*result).c_str());

        }
        else
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMongo, "find fail key:%s"), key.view().data());

        }
    }
    catch (const mongocxx::exception &e)
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(TestMongo, "An exception occurred:%s"), e.what());
    }
}