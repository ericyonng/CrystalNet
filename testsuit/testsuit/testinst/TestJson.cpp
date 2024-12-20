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
 * Date: 2024-12-17 15:39:00
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestJson.h>

// json整合只需要json.hpp即可
#include <nlohmann/json.hpp>

void TestJson::Run()
{
    // json 字符串
    std::string raw = "{\"key1\":11, \"key2\":\"hello json\", \"key3\":[12, 13], \"key4\":{\"subkey1\":true}}";

    // 反序列化
    nlohmann::json &&jsonObject = nlohmann::json::parse(raw.c_str(), NULL, false);
    assert(jsonObject.is_object());

    // 序列化
    std::string &&dumpData = jsonObject.dump();

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestJson, "dumpData:%s"), dumpData.c_str());

    // 设置数值
    jsonObject["key1"] = 500;
    // 设置字符串
    jsonObject["key2"] = "hello test json";
    // 设置数组
    jsonObject["key3"] = {55, 16};
    // 设置对象
    nlohmann::json subObj;
    subObj["subKey1"] = "hello sub obj";
    jsonObject["key5"] = subObj;
    
    // 获取值
    std::string &&key2Value = jsonObject["key2"].get<std::string>();
    Int32 key1Value = jsonObject["key1"].get<Int32>();

    // 设置一个str，用Int32拿
    jsonObject["strKey"] = "test set str get int";
    jsonObject["strKey2"] = 1000;
    auto &&tempJson = jsonObject["strKey"];
    if(tempJson.is_number())
        Int32 num = jsonObject["strKey"].get<Int32>();
    auto &&numJson = jsonObject["strKey2"];
    if(numJson.is_number())
        Int32 num = numJson.get<Int32>();

    // 遍历数组
    auto &&arr = jsonObject["key3"];
    assert(arr.is_array());
    Int32 count = static_cast<Int32>(arr.size());
    for(Int32 idx = 0; idx < count; ++idx)
        auto elem = arr[idx].get<Int32>();

    // 获取对象
    auto &&key4Object = jsonObject["key4"];
    assert(key4Object.is_object());
    auto subkey1Value = key4Object["subkey1"];
    assert(subkey1Value.is_boolean());

    // 删除元素
    jsonObject.erase("key1");
}