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
 * Date: 2021-03-16 23:13:58
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestVariant.h>

void TestVariant::Run() 
{
//     KERNEL_NS::Variant var;
// //    KERNEL_NS::Variant *var3 = KERNEL_NS::Variant::New_Variant();

//     var["id"] = 1000;
//     var["10"] = 30;
//     KERNEL_NS::LibStream<KernelBuildMT> stream;
//     stream.Init(1);
//     var.Serialize(stream);
//     g_Log->Custom("var [\"10\"] = %d", var["10"].AsInt32());
//     g_Log->Custom("var [\"id\"] = %d", var["id"].AsInt32());
//     g_Log->Custom("var string = %s", var.ToString().c_str());

//     KERNEL_NS::Variant var2;
//     var2.DeSerialize(stream);

//     g_Log->Custom("var2 [\"10\"] = %d", var2["10"].AsInt32());
//     g_Log->Custom("var2 [\"id\"] = %d", var2["id"].AsInt32());
//     g_Log->Custom("var2 string = %s", var2.ToString().c_str());

    {// std::map
        // 1.迭代器
        // 2.插入
        // 3.删除
        std::map<Int32, Int32> dict;
        dict.insert(std::make_pair<Int32, Int32>(1, 2));
        dict.insert(std::make_pair<Int32, Int32>(2, 2));
        dict.insert(std::make_pair<Int32, Int32>(3, 5));
        dict.insert(std::make_pair<Int32, Int32>(4, 2));

        KERNEL_NS::Variant *var = KERNEL_NS::Variant::New_Variant(dict);
        var->InsertDict(5, 2);
        
        var->EraseDict(2);
        (*var)[10] = 11;
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "compare dict[1] > dict[3]?:%d"), (*var)[1] > (*var)[3]);
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "type :%s"),  var->TypeToString().c_str());
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "value :%s"),  var->ValueToString().c_str());

        for(auto iter = var->BeginDict(); iter != var->EndDict(); ++iter)
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "var value:%s"), iter->second.ToString().c_str());
        }
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "var:%s"), var->ToString().c_str());

        KERNEL_NS::LibStreamTL *stream = KERNEL_NS::LibStreamTL::NewThreadLocal_LibStream();
        stream->Init(1);
        var->Serialize(*stream);
        KERNEL_NS::Variant recover;
        recover.DeSerialize(*stream);
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "recover:%s"), recover.ToString().c_str());

        KERNEL_NS::Variant *newVar = KERNEL_NS::Variant::New_Variant();
        *newVar = dict;
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "new var:%s"), newVar->ToString().c_str());
        std::map<Int32, Int32> newDict;
        newDict = *newVar;

        for(auto iter : newDict)
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "new dict key:%d, value:%d"), iter.first, iter.second);
    }

    {// std::unordered_map
        std::unordered_map<KERNEL_NS::LibString, Int32> dict;
        dict.insert(std::make_pair<KERNEL_NS::LibString, Int32>("test code", 1));
        dict.insert(std::make_pair<KERNEL_NS::LibString, Int32>("session", 1));

        auto var = KERNEL_NS::Variant::New_Variant(dict);
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "var:%s"), var->ToString().c_str());

        var->InsertDict("test var", 12);
        (*var)["test code"] = 55;
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "var:%s"), var->ToString().c_str());

        for(auto iter = var->BeginDict(); iter != var->EndDict(); ++iter)
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "var value:%s"), iter->second.ToString().c_str());
        }

        KERNEL_NS::Variant *newVar = KERNEL_NS::Variant::New_Variant();
        *newVar = dict;
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "new var:%s"), newVar->ToString().c_str());
        std::unordered_map<Int32, Int32> newDict;
        newDict = *newVar;

        for(auto iter : newDict)
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "new dict key:%d, value:%d"), iter.first, iter.second);
        // 1.迭代器
        // 2.插入
        // 3.删除
    }
    {// std::set
        std::set<Int32> dict;
        dict.insert(1);
        dict.insert(2);
        dict.insert(3);
        dict.insert(4);
        
        auto var = KERNEL_NS::Variant::New_Variant(dict);
        for(auto iter = var->SeqBegin(); iter != var->SeqEnd(); ++iter)
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "var value:%s"), iter->ToString().c_str());
        }

        KERNEL_NS::Variant *newVar = KERNEL_NS::Variant::New_Variant();
        *newVar = dict;
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "new var:%s"), newVar->ToString().c_str());
        std::set<Int32> newDict;
        newDict = *newVar;

        for(auto iter : newDict)
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "new dict value:%d"), iter);
    }

    {// std::unordered_set
        std::unordered_set<KERNEL_NS::LibString> dict;
        dict.insert("code");
        dict.insert("code2");
        dict.insert("bigger");
        auto var = KERNEL_NS::Variant::New_Variant(dict);
        for(auto iter = var->SeqBegin(); iter != var->SeqEnd(); ++iter)
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "value:%s"), iter->ToString().c_str());
        }

        KERNEL_NS::Variant *newVar = KERNEL_NS::Variant::New_Variant();
        *newVar = dict;
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "new var:%s"), newVar->ToString().c_str());
        std::unordered_set<KERNEL_NS::LibString> newDict;
        newDict = *newVar;

        for(auto iter : newDict)
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "new dict value:%s"), iter.c_str());
    }

    {// std::vector
        std::vector<KERNEL_NS::LibString> dict;
        dict.push_back("code");
        dict.push_back("code2");
        dict.push_back("bigger");
        auto var = KERNEL_NS::Variant::New_Variant(dict);
        for(auto iter = var->SeqBegin(); iter != var->SeqEnd(); ++iter)
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "value:%s"), iter->ToString().c_str());
        }

        KERNEL_NS::Variant *newVar = KERNEL_NS::Variant::New_Variant();
        *newVar = dict;
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "new var:%s"), newVar->ToString().c_str());
        std::vector<KERNEL_NS::LibString> newDict;
        newDict = *newVar;

        for(auto iter : newDict)
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "new dict value:%s"), iter.c_str());

        // 1.迭代器
        // 2.插入
        // 3.删除
    }
    {// std::list
        std::list<KERNEL_NS::LibString> dict;
        dict.push_back("code");
        dict.push_back("code2");
        dict.push_back("bigger");
        auto var = KERNEL_NS::Variant::New_Variant(dict);
        for(auto iter = var->SeqBegin(); iter != var->SeqEnd(); ++iter)
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "value:%s"), iter->ToString().c_str());
        }

        KERNEL_NS::Variant *newVar = KERNEL_NS::Variant::New_Variant();
        *newVar = dict;
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "new var:%s"), newVar->ToString().c_str());
        std::list<KERNEL_NS::LibString> newDict;
        newDict = *newVar;

        for(auto iter : newDict)
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "new dict value:%s"), iter.c_str());
    }
    {// std::deque
        std::deque<KERNEL_NS::LibString> dict;
        dict.push_back("code");
        dict.push_back("code2");
        dict.push_back("bigger");
        auto var = KERNEL_NS::Variant::New_Variant(dict);
        for(auto iter = var->SeqBegin(); iter != var->SeqEnd(); ++iter)
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "value:%s"), iter->ToString().c_str());
        }

        KERNEL_NS::Variant *newVar = KERNEL_NS::Variant::New_Variant();
        *newVar = dict;
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "new var:%s"), newVar->ToString().c_str());
        std::deque<KERNEL_NS::LibString> newDict;
        newDict = *newVar;

        for(auto iter : newDict)
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "new dict value:%s"), iter.c_str());
    }
    {// std::queue
        std::queue<KERNEL_NS::LibString> dict;
        dict.push("code");
        dict.push("code2");
        dict.push("bigger");
        auto var = KERNEL_NS::Variant::New_Variant(dict);
        for(auto iter = var->SeqBegin(); iter != var->SeqEnd(); ++iter)
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "value:%s"), iter->ToString().c_str());
        }

        KERNEL_NS::Variant *newVar = KERNEL_NS::Variant::New_Variant();
        *newVar = dict;
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "new var:%s"), newVar->ToString().c_str());
        std::queue<KERNEL_NS::LibString> newDict;
        newDict = *newVar;
        
        while(!newDict.empty())
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "new dict value:%s"), newDict.front().c_str());
            newDict.pop();
        }
    }

    {// 普通类型
        {// 简单类型
            KERNEL_NS::Variant *var = KERNEL_NS::Variant::New_Variant();
            KERNEL_NS::Variant *var2 = KERNEL_NS::Variant::New_Variant();
            var->BecomeInt64() = 64LL;
            var2->BecomeInt64() = 56LL;

            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "var type:%s"), var->TypeToString().c_str());
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "var value:%s"), var->ValueToString().c_str());
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "var:%s"), var->ToString().c_str());

            auto stream = KERNEL_NS::LibStreamTL::New_LibStream();
            stream->Init(1);
            var->Serialize(*stream);
            KERNEL_NS::Variant recover;
            recover.DeSerialize(*stream);
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "var equal recover:%d"), *var == recover);

            std::string str;
            KERNEL_NS::LibString libstr;
            std::cout << *var << std::endl;
            str << *var;
            libstr << *var;

            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "string:%s, libstring:%s"), str.c_str(), libstr.c_str());

            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "compare var with var2 result:%d"), *var > *var2);
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "compare var with var2 result:%d"), *var == *var2);
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "compare var with var2 result:%d"), *var != *var2);
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "compare var with var2 result:%d"), *var < *var2);
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "compare var with var2 result:%d"), *var >= *var2);
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "compare var with var2 result:%d"), *var < *var2);
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "compare var with var2 result:%d"), *var <= *var2);

            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "compare var + var2 result:%s"), (*var + *var2).ToString().c_str());
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "compare var - var2 result:%s"), (*var - *var2).ToString().c_str());
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "compare var * var2 result:%s"), (*var * *var2).ToString().c_str());
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "compare var / var2 result:%s"), (*var / *var2).ToString().c_str());
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "compare var += var2 result:%s"), (*var += *var2).ToString().c_str());
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "compare var -= var2 result:%s"), (*var -= *var2).ToString().c_str());
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "compare var *= var2 result:%s"), (*var *= *var2).ToString().c_str());
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "compare var /= var2 result:%s"), (*var /= *var2).ToString().c_str());

        }

        {// 指针类型
            KERNEL_NS::Variant *var = KERNEL_NS::Variant::New_Variant();
            KERNEL_NS::Variant *var2 = KERNEL_NS::Variant::New_Variant();
            *var2 = 64;
            var->BecomePtr() = var2;

            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "var type:%s"), var->TypeToString().c_str());
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "var value:%s"), var->ValueToString().c_str());
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "var:%s"), var->ToString().c_str());
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "var2:%s"), var2->ToString().c_str());
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "var ptr:%s"), var->AsPtr<KERNEL_NS::Variant>()->ToString().c_str());

            auto stream = KERNEL_NS::LibStreamTL::New_LibStream();
            stream->Init(1);
            var->Serialize(*stream);
            KERNEL_NS::Variant recover;
            recover.DeSerialize(*stream);
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "var equal recover:%d"), *var == recover);

            std::string str;
            KERNEL_NS::LibString libstr;
            std::cout << *var << std::endl;
            str << *var;
            libstr << *var;

            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "string:%s, libstring:%s"), str.c_str(), libstr.c_str());
        }

        {// 字符串类型
            KERNEL_NS::Variant *var = KERNEL_NS::Variant::New_Variant();
            var->BecomeStr() = "hello variant";

            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "var type:%s"), var->TypeToString().c_str());
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "var value:%s"), var->ValueToString().c_str());
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "var:%s"), var->ToString().c_str());

            auto stream = KERNEL_NS::LibStreamTL::New_LibStream();
            stream->Init(1);
            var->Serialize(*stream);
            KERNEL_NS::Variant recover;
            recover.DeSerialize(*stream);
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "var equal recover:%d, recover:%s"), *var == recover, recover.ToString().c_str());

            std::string str;
            KERNEL_NS::LibString libstr;
            std::cout << *var << std::endl;
            str << *var;
            libstr << *var;

            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "string:%s, libstring:%s"), str.c_str(), libstr.c_str());
        }

        {// 字典类型
            KERNEL_NS::Variant *var = KERNEL_NS::Variant::New_Variant();
            var->BecomeDict();
            var->InsertDict(1, 2);
            var->InsertDict(2, 2);
            var->InsertDict(3, 2);
            var->InsertDict(4, 2);

            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "var type:%s"), var->TypeToString().c_str());
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "var value:%s"), var->ValueToString().c_str());
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "var:%s"), var->ToString().c_str());

            auto stream = KERNEL_NS::LibStreamTL::New_LibStream();
            stream->Init(1);
            var->Serialize(*stream);
            KERNEL_NS::Variant recover;
            recover.DeSerialize(*stream);
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "var equal recover:%d, recover:%s"), *var == recover, recover.ToString().c_str());

            std::string str;
            KERNEL_NS::LibString libstr;
            std::cout << *var << std::endl;
            str << *var;
            libstr << *var;

            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "string:%s, libstring:%s"), str.c_str(), libstr.c_str());
        }

        {// 序列类型
            KERNEL_NS::Variant *var = KERNEL_NS::Variant::New_Variant();
            var->BecomeSeq();
            var->SeqPushBack(1);
            var->SeqPushBack(5);
            var->SeqPushBack(8);
            var->SeqPushBack(11);

            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "var type:%s"), var->TypeToString().c_str());
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "var value:%s"), var->ValueToString().c_str());
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "var:%s"), var->ToString().c_str());

            auto stream = KERNEL_NS::LibStreamTL::New_LibStream();
            stream->Init(1);
            var->Serialize(*stream);
            KERNEL_NS::Variant recover;
            recover.DeSerialize(*stream);
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "var equal recover:%d, recover:%s"), *var == recover, recover.ToString().c_str());

            std::string str;
            KERNEL_NS::LibString libstr;
            std::cout << *var << std::endl;
            str << *var;
            libstr << *var;

            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestVariant, "string:%s, libstring:%s"), str.c_str(), libstr.c_str());
        }

        // 1.整型
        // 2.指针类型
        // 3.字符串类型
        // 4.字典类型
        // 5.序列类型

        // 6.比较大小
        // 7.赋值
        // 8.TypeToString
        // 9.ValueToString
        // 10.ToString
        // 11. 序列化反序列化
        // 12. std::string << 
        // 13 LibString << 
        // 14. std::ostream<<
        // 15.四则运算
    }
    getchar();
    
}
