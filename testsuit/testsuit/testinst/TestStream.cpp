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
 * Author: Eric Yonng
 * Date: 2021-03-08 14:40:40
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestStream.h>

#define TEST_STREAM_BUFFER_SIZE 1024LLU

struct TestStreamData
{
    Int32 _number;
    KERNEL_NS::LibString _str;
};

struct TestSeriDesirialize
{
    UInt64 _number;
    Float _float;
    KERNEL_NS::LibString _str;

    void SerializeTo(KERNEL_NS::LibString &str)
    {
        KERNEL_NS::LibStream<KernelBuildMT> buffer;
        UInt64 totalSize = sizeof(_number) + sizeof(_float) + sizeof(UInt64) + _str.size();
        str.resize(totalSize);
        //buffer.Init(totalSize);
        buffer.Attach(const_cast<Byte8 *>(str.data()), totalSize, 0, 0);

        buffer.Write(_number);
        buffer.Write(_float);
        UInt64 len = _str.size();
        buffer.Write(len);
        buffer.Write(_str.data(), len);

        //buffer.SerializeTo(str);
    }

    void DeserializeFrom(const KERNEL_NS::LibString &str)
    {
        KERNEL_NS::LibStream<KernelBuildMT> buffer;
        buffer.Attach(const_cast<Byte8 *>(str.data()), str.size(), 0, 0);
        //buffer.DeserializeFrom(str);

        buffer.Read(_number);
        buffer.Read(_float);
        UInt64 len = 0;
        buffer.Read(len);
        _str.resize(len);
        buffer.Read(const_cast<Byte8 *>(_str.data()), len);
    }
};

struct TestSerializeObj
{
    UInt64 _number;
    Float _float;
    KERNEL_NS::LibString _str;

    bool Serialize(KERNEL_NS::LibStream<KernelBuildMT> &stream) const
    {
        stream.Write(_number);
        stream.Write(_float);
        stream.Write(_str);

        return true;
    }

    bool DeSerialize(KERNEL_NS::LibStream<KernelBuildMT> &stream)
    {
        stream.Read(_number);
        stream.Read(_float);
        stream.Read(_str);

        return true;
    }
};

struct TestSerializeObj2
{
    UInt64 _number;
    Float _float;
    KERNEL_NS::LibString _str;
    TestSerializeObj _obj;

    bool Serialize(KERNEL_NS::LibStream<KernelBuildMT> &stream)
    {
        stream.Write(_number);
        stream.Write(_float);
        stream.Write(_str);
        stream.Write(_obj);

        return true;
    }

    bool DeSerialize(KERNEL_NS::LibStream<KernelBuildMT> &stream)
    {
        stream.Read(_number);
        stream.Read(_float);
        stream.Read(_str);
        stream.Read(_obj);

        return true;
    }
};

void TestStream::Run() 
{
    // 测试attach模式
    {
        KERNEL_NS::LibStream<KernelBuildMT> buffer;
        Byte8 *mem = g_MemoryPool->Alloc<Byte8>(TEST_STREAM_BUFFER_SIZE);
        buffer.Attach(mem, TEST_STREAM_BUFFER_SIZE, 0, 0);

        TestStreamData testData;
        testData._number = 150;
        testData._str = "hello test stream!";

        buffer.Write(testData._number);
        // buffer.Write(testData._str);
        buffer.Write(testData._str.length());
        buffer.Write(testData._str.data(), testData._str.length());

        Int32 number;
        buffer.Read(number);
        g_Log->Custom("buffer number=[%d]", number);
        UInt64 strLen = 0;
        buffer.Read(strLen);
        KERNEL_NS::LibString str;
        str.resize(strLen);
        buffer.Read(const_cast<Byte8 *>(str.data()), strLen);
        g_Log->Custom("buffer str=%s", str.c_str());

        // 释放资源
        g_MemoryPool->Free(mem);
    }    

    // 测试Init
    {
        KERNEL_NS::LibStream<KernelBuildMT> buffer;
        buffer.Init(TEST_STREAM_BUFFER_SIZE);

        TestStreamData testData;
        testData._number = 150;
        testData._str = "hello test stream init mode!";

        buffer.Write(testData._number);
        // buffer.Write(testData._str);
        buffer.Write(testData._str.length());
        buffer.Write(testData._str.data(), testData._str.length());

        Int32 number;
        buffer.Read(number);
        g_Log->Custom("buffer number=[%d]", number);
        UInt64 strLen = 0;
        buffer.Read(strLen);
        KERNEL_NS::LibString str;
        str.resize(strLen);
        buffer.Read(const_cast<Byte8 *>(str.data()), strLen);
        g_Log->Custom("buffer str=%s", str.c_str());
    }

    // 测试
    {
        KERNEL_NS::LibStream<KernelBuildMT> buffer;
        Byte8 *mem = g_MemoryPool->Alloc<Byte8>(TEST_STREAM_BUFFER_SIZE);
        buffer.Attach(mem, TEST_STREAM_BUFFER_SIZE, 0, 0);

        TestStreamData testData;
        testData._number = 150;
        testData._str = "hello test stream!";

        buffer.Write(testData._number);
        // buffer.Write(testData._str);
        buffer.Write(testData._str.length());
        buffer.Write(testData._str.data(), testData._str.length());

        KERNEL_NS::LibStream<KernelBuildMT> buffer2;

        Int32 number;
        buffer2.Read(number);
        g_Log->Custom("buffer2 number=[%d]", number);
        UInt64 strLen = 0;
        buffer2.Read(strLen);
        KERNEL_NS::LibString str;
        str.resize(strLen);
        buffer2.Read(const_cast<Byte8 *>(str.data()), strLen);
        g_Log->Custom("buffer2 str=%s", str.c_str());
    }

    // 接管
    {
        KERNEL_NS::LibStream<KernelBuildMT> buffer;
        Byte8 *mem = g_MemoryPool->Alloc<Byte8>(TEST_STREAM_BUFFER_SIZE);
        buffer.Attach(mem, TEST_STREAM_BUFFER_SIZE, 0, 0);

        TestStreamData testData;
        testData._number = 150;
        testData._str = "hello test stream!";

        buffer.Write(testData._number);
        // buffer.Write(testData._str);
        buffer.Write(testData._str.length());
        buffer.Write(testData._str.data(), testData._str.length());

        KERNEL_NS::LibStream<KernelBuildMT> buffer2;
        buffer2.TakeOver(buffer);

        Int32 number;
        buffer2.Read(number);
        g_Log->Custom("buffer2 take over number=[%d]", number);
        UInt64 strLen = 0;
        buffer2.Read(strLen);
        KERNEL_NS::LibString str;
        str.resize(strLen);
        buffer2.Read(const_cast<Byte8 *>(str.data()), strLen);
        g_Log->Custom("buffer2 take over str=%s", str.c_str());
    }

    // 序列化反序列化字节流
    {
        KERNEL_NS::LibStream<KernelBuildMT> buffer;
        Byte8 *mem = g_MemoryPool->Alloc<Byte8>(TEST_STREAM_BUFFER_SIZE);
        buffer.Attach(mem, TEST_STREAM_BUFFER_SIZE, 0, 0);

        TestStreamData testData;
        testData._number = 150;
        testData._str = "hello test stream serialize/deserialize!";

        buffer.Write(testData._number);
        // buffer.Write(testData._str);
        buffer.Write(testData._str.length());
        buffer.Write(testData._str.data(), testData._str.length());

        KERNEL_NS::LibString str;
        buffer.SerializeTo(str);

        KERNEL_NS::LibString base64Str;
        KERNEL_NS::LibBase64::Encode(str.c_str(), str.length(), base64Str);
        g_Log->Custom("buffer2 take over number=[%s]", base64Str.c_str());

        buffer.DeserializeFrom(str);

        Int32 number;
        buffer.Read(number);
        g_Log->Custom("test serialize/deserialize number=[%d]", number);
        UInt64 strLen = 0;
        buffer.Read(strLen);
        KERNEL_NS::LibString lastStr;
        lastStr.resize(strLen);
        buffer.Read(const_cast<Byte8 *>(lastStr.data()), strLen);
        g_Log->Custom("test serialize/deserialize str=%s", lastStr.c_str());

        g_MemoryPool->Free(mem);
    }

    // 对象序列化/反序列化
    {
        TestSeriDesirialize testObj;
        testObj._number = 250;
        testObj._float = 58.9f;
        testObj._str = "hello test seridesirialize obj!!!.";

        KERNEL_NS::LibString buffer;
        testObj.SerializeTo(buffer);

        g_Log->Custom("test sirialize obj str size=[%llu]", static_cast<UInt64>(buffer.size()));
        KERNEL_NS::LibString base64Str;
        KERNEL_NS::LibBase64::Encode(buffer.data(), buffer.size(), base64Str);
        g_Log->Custom("test sirialize obj base64 size=[%llu], str=[%s]", static_cast<UInt64>(base64Str.size()), base64Str.c_str());

        TestSeriDesirialize testObj2;
        testObj2.DeserializeFrom(buffer);        
        g_Log->Custom("testObj str=[%s], number=[%llu], float=[%f]", testObj2._str.c_str(), testObj2._number, testObj2._float);
    }

    // 测试libstream序列化反序列化
    {
        TestSerializeObj2 obj;
        obj._number = 150;
        obj._float = 1589.556f;
        obj._str = "hello libstream";
        obj._obj._str = "inter libstream test";
        obj._obj._float = 9968.15f;
        obj._obj._number = 15999;

        KERNEL_NS::LibStream<KernelBuildMT> stream;
        stream.Init(4096);
        obj.Serialize(stream);

        KERNEL_NS::LibString str;
        stream.SerializeTo(str);
        KERNEL_NS::LibString base64Str;
        KERNEL_NS::LibBase64::Encode(str.data(), str.size(), base64Str);
        g_Log->Custom("stream sirialize base64 size=[%llu], str=[%s]", static_cast<UInt64>(base64Str.size()), base64Str.c_str());

        TestSerializeObj2 obj2;
        obj2.DeSerialize(stream);
        g_Log->Custom("TestSerializeObj2 str=[%s]", obj2._str.c_str());
    }
}
