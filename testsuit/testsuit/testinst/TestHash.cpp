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
 * Date: 2026-06-18 20:53:42
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include "TestHash.h"

#include <unordered_set>
#include <vector>

#include <kernel/comp/Utils/TimeUtil.h>

// 引入官方 xxHash 用于对比 (header-only, 单TU内联)
// 注意: 必须在 pch.h 之后, 且仅在此 TU 定义 XXH_INLINE_ALL
#include <xxhash.h>

void TestHash::Run()
{
    CLOG_INFO_GLOBAL(TestHash, "========== TestHash Start ==========");

    TestAgainstOfficial();
    TestKnownVectors();
    TestCollisionRate();
    TestPerformance();
    TestDeterminism();

    CLOG_INFO_GLOBAL(TestHash, "========== TestHash End ==========");
    getchar();
}

void TestHash::TestKnownVectors()
{
    CLOG_INFO_GLOBAL(TestHash, "---- TestKnownVectors ----");

    // 空输入: XXH64("", 0, seed=0) = 0xEF46DB3751D8E999 (官方已知向量)
    {
        UInt64 h = KERNEL_NS::HashUtil::Hash64("", 0);
        bool ok = (h == 0xEF46DB3751D8E999ULL);
        CLOG_INFO_GLOBAL(TestHash, "XxHash64(\"\") = 0x%016llX, expect 0xEF46DB3751D8E999, %s",
                         h, ok ? "PASS" : "FAIL");
    }

    // "hello world" 打印供人工核对
    {
        KERNEL_NS::LibString info = "hello world";
        UInt64 hXx = KERNEL_NS::HashUtil::Hash64(info.c_str(), info.length());
        UInt64 h64 = KERNEL_NS::HashUtil::Hash64(info.c_str(), info.length());
        UInt32 h32 = KERNEL_NS::HashUtil::Hash(info.c_str(), info.length());
        auto h128 = KERNEL_NS::HashUtil::Hash128(info.c_str(), info.length());
        CLOG_INFO_GLOBAL(TestHash, "info:%s, hash32:%u, Hash64(Murmur3Hi):%llu, XxHash64:%llu, hash128[Hi:%llu Lo:%llu]",
                         info.c_str(), h32, h64, hXx, h128.Hi, h128.Lo);
    }

    // 单字节输入
    {
        const char *s = "a";
        UInt64 h = KERNEL_NS::HashUtil::Hash64(s, 1);
        CLOG_INFO_GLOBAL(TestHash, "XxHash64(\"a\") = 0x%016llX", h);
    }
}

void TestHash::TestCollisionRate()
{
    CLOG_INFO_GLOBAL(TestHash, "---- TestCollisionRate ----");

    const Int32 N = 1000000;
    std::unordered_set<UInt64> seen;
    seen.reserve(static_cast<size_t>(N) * 2);

    Int32 collisions = 0;
    for (Int32 i = 0; i < N; ++i)
    {
        KERNEL_NS::LibString key;
        key.AppendFormat("key_%d", i);
        UInt64 h = KERNEL_NS::HashUtil::Hash64(key.c_str(), key.length());
        if (!seen.insert(h).second)
            ++collisions;
    }

    double rate = static_cast<double>(collisions) / N;
    CLOG_INFO_GLOBAL(TestHash, "XxHash64: keys=%d, collisions=%d, rate=%.8f", N, collisions, rate);

    // 同样测试 Hash64(Murmur3 Hi) 对比
    std::unordered_set<UInt64> seen2;
    seen2.reserve(static_cast<size_t>(N) * 2);
    Int32 collisions2 = 0;
    for (Int32 i = 0; i < N; ++i)
    {
        KERNEL_NS::LibString key;
        key.AppendFormat("key_%d", i);
        UInt64 h = KERNEL_NS::HashUtil::Hash64(key.c_str(), key.length());
        if (!seen2.insert(h).second)
            ++collisions2;
    }
    double rate2 = static_cast<double>(collisions2) / N;
    CLOG_INFO_GLOBAL(TestHash, "Hash64(Murmur3Hi): keys=%d, collisions=%d, rate=%.8f", N, collisions2, rate2);
}

void TestHash::TestPerformance()
{
    CLOG_INFO_GLOBAL(TestHash, "---- TestPerformance ----");

    const size_t bufSize = 4 * 1024 * 1024; // 4MB
    std::vector<U8> buf(bufSize);
    for (size_t i = 0; i < bufSize; ++i)
        buf[i] = static_cast<U8>(i & 0xFF);

    const Int32 iterations = 200;

    // XxHash64
    {
        Int64 t0 = KERNEL_NS::TimeUtil::GetFastMicroTimestamp();
        UInt64 acc = 0;
        for (Int32 i = 0; i < iterations; ++i)
            acc += KERNEL_NS::HashUtil::Hash64(buf.data(), bufSize);
        Int64 t1 = KERNEL_NS::TimeUtil::GetFastMicroTimestamp();
        Int64 elapsedUs = t1 - t0;
        double totalBytes = static_cast<double>(bufSize) * iterations;
        double gbPerSec = elapsedUs > 0 ? (totalBytes / 1024.0 / 1024.0 / 1024.0) / (elapsedUs / 1000000.0) : 0.0;
        CLOG_INFO_GLOBAL(TestHash, "XxHash64: iters=%d, size=%lluMB, elapsed=%lldus, throughput=%.2f GB/s, acc=%llu",
                         iterations, static_cast<UInt64>(bufSize / (1024 * 1024)), elapsedUs, gbPerSec, acc);
    }

    // Hash64 (Murmur3 128 取 Hi)
    {
        Int64 t0 = KERNEL_NS::TimeUtil::GetFastMicroTimestamp();
        UInt64 acc = 0;
        for (Int32 i = 0; i < iterations; ++i)
            acc += KERNEL_NS::HashUtil::Hash64(buf.data(), bufSize);
        Int64 t1 = KERNEL_NS::TimeUtil::GetFastMicroTimestamp();
        Int64 elapsedUs = t1 - t0;
        double totalBytes = static_cast<double>(bufSize) * iterations;
        double gbPerSec = elapsedUs > 0 ? (totalBytes / 1024.0 / 1024.0 / 1024.0) / (elapsedUs / 1000000.0) : 0.0;
        CLOG_INFO_GLOBAL(TestHash, "Hash64(Murmur3Hi): iters=%d, size=%lluMB, elapsed=%lldus, throughput=%.2f GB/s, acc=%llu",
                         iterations, static_cast<UInt64>(bufSize / (1024 * 1024)), elapsedUs, gbPerSec, acc);
    }

    // 官方 XXH64 (seed=0) 作为基准对照
    {
        Int64 t0 = KERNEL_NS::TimeUtil::GetFastMicroTimestamp();
        UInt64 acc = 0;
        for (Int32 i = 0; i < iterations; ++i)
            acc += static_cast<UInt64>(XXH64(buf.data(), bufSize, 0));
        Int64 t1 = KERNEL_NS::TimeUtil::GetFastMicroTimestamp();
        Int64 elapsedUs = t1 - t0;
        double totalBytes = static_cast<double>(bufSize) * iterations;
        double gbPerSec = elapsedUs > 0 ? (totalBytes / 1024.0 / 1024.0 / 1024.0) / (elapsedUs / 1000000.0) : 0.0;
        CLOG_INFO_GLOBAL(TestHash, "Official XXH64(seed=0): iters=%d, size=%lluMB, elapsed=%lldus, throughput=%.2f GB/s, acc=%llu",
                         iterations, static_cast<UInt64>(bufSize / (1024 * 1024)), elapsedUs, gbPerSec, acc);
    }
}

void TestHash::TestDeterminism()
{
    CLOG_INFO_GLOBAL(TestHash, "---- TestDeterminism ----");

    KERNEL_NS::LibString info = "determinism check 确定性测试";
    UInt64 h1 = KERNEL_NS::HashUtil::Hash64(info.c_str(), info.length());
    UInt64 h2 = KERNEL_NS::HashUtil::Hash64(info.c_str(), info.length());
    bool ok = (h1 == h2);
    CLOG_INFO_GLOBAL(TestHash, "XxHash64 determinism: h1=%llu, h2=%llu, %s",
                     h1, h2, ok ? "PASS" : "FAIL");
}

void TestHash::TestAgainstOfficial()
{
    CLOG_INFO_GLOBAL(TestHash, "---- TestAgainstOfficial (mine XxHash64 vs official XXH64 seed=0) ----");

    Int32 passCnt = 0;
    Int32 failCnt = 0;
    // UInt64 testNull = KERNEL_NS::HashUtil::Hash64(NULL, 0);

    auto checkOne = [&](const char *name, const void *data, size_t len)
    {
        UInt64 mine = KERNEL_NS::HashUtil::Hash64(data, len);
        UInt64 offi = static_cast<UInt64>(XXH64(data, len, 0));
        bool ok = (mine == offi);
        if (ok) ++passCnt; else ++failCnt;
        CLOG_INFO_GLOBAL(TestHash, "[%s] %-28s len=%4llu mine=0x%016llX offi=0x%016llX",
                         ok ? "PASS" : "FAIL", name, static_cast<UInt64>(len), mine, offi);
    };

    // 边界长度: 覆盖 0/1/2/3/4/5/7/8/9/15/16/17/31/32/33 等关键点
    std::vector<U8> inc;
    const size_t lens[] = {0,1,2,3,4,5,7,8,9,15,16,17,31,32,33,39,40,41,43,44,45,
                           63,64,65,127,128,255,256,1000,4095,4096,4097,65535,65536};
    for (size_t len : lens)
    {
        inc.assign(len, 0);
        for (size_t i = 0; i < len; ++i)
            inc[i] = static_cast<U8>(i & 0xFF);
        KERNEL_NS::LibString nm;
        nm.AppendFormat("incremental[%llu]", static_cast<UInt64>(len));
        checkOne(nm.c_str(), inc.data(), len);
    }

    // 特殊模式
    {
        std::vector<U8> z(100, 0);          checkOne("all-zero[100]", z.data(), z.size());
        std::vector<U8> f(100, 0xFF);       checkOne("all-0xFF[100]", f.data(), f.size());
        std::vector<U8> alt(100);
        for (size_t i = 0; i < alt.size(); ++i)
            alt[i] = static_cast<U8>((i & 1) ? 0x55 : 0xAA);
        checkOne("alternating[100]", alt.data(), alt.size());
    }

    // 字符串
    checkOne("\"\"", "", 0);
    checkOne("\"a\"", "a", 1);
    checkOne("\"abc\"", "abc", 3);
    checkOne("\"hello world\"", "hello world", 11);
    checkOne("\"The quick brown fox\"", "The quick brown fox", 19);
    checkOne("36chars", "abcdefghijklmnopqrstuvwxyz0123456789", 36);
    checkOne("32bytes-string", "01234567890123456789012345678901", 32);
    checkOne("64bytes-string",
             "0123456789012345678901234567890101234567890123456789012345678901", 64);

    // 官方已知向量
    {
        UInt64 mine = KERNEL_NS::HashUtil::Hash64("", 0);
        UInt64 offi = static_cast<UInt64>(XXH64("", 0, 0));
        bool ok = (mine == 0xEF46DB3751D8E999ULL && offi == 0xEF46DB3751D8E999ULL);
        if (ok) ++passCnt; else ++failCnt;
        CLOG_INFO_GLOBAL(TestHash, "[%s] known-vector empty -> 0xEF46DB3751D8E999  mine=0x%016llX offi=0x%016llX",
                         ok ? "PASS" : "FAIL", mine, offi);
    }

    // 大块数据 4MB (TestPerformance 场景)
    {
        std::vector<U8> big(4 * 1024 * 1024);
        for (size_t i = 0; i < big.size(); ++i)
            big[i] = static_cast<U8>(i & 0xFF);
        checkOne("4MB-incremental", big.data(), big.size());
    }

    CLOG_INFO_GLOBAL(TestHash, "==== TestAgainstOfficial Summary: PASS=%d FAIL=%d ====",
                     passCnt, failCnt);
}
