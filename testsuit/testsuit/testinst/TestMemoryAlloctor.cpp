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
 * Date: 2020-11-15 16:01:16
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestMemoryAlloctor.h>


#undef TEST_ALLOC_UNIT_BYTES
#define TEST_ALLOC_UNIT_BYTES 256

#define TEST_ALLOC_TO_ALLOC 1024

#define  TEST_ALLOC_LOOP   10240
#define  TEST_BLOCK_NUM_INIT   1024
#define  TEST_BLOCK_NUM_LIMIT   102400


void TestMemoryAlloctor::Run()
{
    // 默认情况
//     KERNEL_NS::MemoryAlloctorConfig cfg(TEST_ALLOC_UNIT_BYTES);
//     KERNEL_NS::MemoryAlloctor alloctor(cfg);
//     alloctor.Init();

    // 保证不处罚new buffer情况
    KERNEL_NS::MemoryAlloctorConfig cfg(TEST_ALLOC_UNIT_BYTES, 36);
    KERNEL_NS::MemoryAlloctor alloctor(cfg);
    alloctor.Init(true, 40960);

    // 测试gc
    // KERNEL_NS::MemoryAlloctorConfig cfg(TEST_ALLOC_UNIT_BYTES, 1);
    // KERNEL_NS::MemoryAlloctor alloctor(cfg);
    // alloctor.Init(4);

    // {// 测试接口
    //     std::vector<void *> ptrlist;
    //     const Int32 testCount = 16;
    //     for(Int32 idx = 0; idx < testCount; ++idx)
    //         ptrlist.push_back(alloctor.Alloc(TEST_ALLOC_UNIT_BYTES));

    //     for(Int32 idx = 0; idx < testCount; ++idx)
    //     {
    //         alloctor.Free(ptrlist[idx]);
    //     }
    // }

    // 测试memorybuffer
    // {
    //     const UInt64 testLoopCount = 100000;
    //     const auto blockSize = __MEMORY_ALIGN_BLOCK_SIZE__(TEST_ALLOC_UNIT_BYTES);
    //     KERNEL_NS::MemoryBuffer *buffer = new KERNEL_NS::MemoryBuffer(blockSize, testLoopCount, TEST_ALLOC_UNIT_BYTES, NULL);

    //     const Int32 testBufferSize = TEST_ALLOC_UNIT_BYTES;
    //     KERNEL_NS::LibCpuCounter::InitFrequancy();

    //     KERNEL_NS::LibCpuCounter start;
    //     KERNEL_NS::LibCpuCounter endCount;
    //     start.Update();
    //     for(Int32 idx = 0; idx < testLoopCount; ++idx)
    //         buffer->AllocNewBlock();
    //     endCount.Update();
        
    //     printf("TEST speed blockSize:%d, test count:%llu, pool total cost:%llu\n"
    //                 , testBufferSize, testLoopCount, endCount.ElapseMicroseconds(start));

    //     KERNEL_NS::LibCpuCounter startSystem;
    //     KERNEL_NS::LibCpuCounter endCountSystem;
    //     startSystem.Update();
    //     for(Int32 idx = 0; idx < testLoopCount; ++idx)
    //         new Byte8[testBufferSize];
    //     endCountSystem.Update();

    //    printf("TEST speed blockSize:%d, test count:%llu, system malloc total cost:%llu"
    //                 , testBufferSize, testLoopCount, endCountSystem.ElapseMicroseconds(startSystem));

    // }


    // {// 无锁条件下分配 linux 下 MemoryAlloctor在不触发创建New MemoryBuffer情况下是系统的5倍左右, 如果是默认参数情况下，且频繁的触发NewBuffer则是系统分配的1.3倍左右
    //     const Int32 testLoopCount = 100000;
    //     const Int32 testBufferSize = TEST_ALLOC_UNIT_BYTES;
    //     KERNEL_NS::LibCpuCounter start;
    //     KERNEL_NS::LibCpuCounter endCount;
    //     start.Update();
    //     auto startPoolTime = KERNEL_NS::LibTime::Now();
    //     for(Int32 idx = 0; idx < testLoopCount; ++idx)
    //         alloctor.Alloc(testBufferSize);
    //     auto endPoolTime = KERNEL_NS::LibTime::Now();
    //     endCount.Update();
    //     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMemoryAlloctor, "TEST speed blockSize:%d, test count:%d, pool total cost:%lld, unit cost:%lld")
    //                 , testBufferSize, testLoopCount, (endPoolTime - startPoolTime).GetTotalMicroSeconds(), (endPoolTime - startPoolTime).GetTotalMicroSeconds() / static_cast<Int64>(testLoopCount));

    //     KERNEL_NS::LibCpuCounter startSystem;
    //     KERNEL_NS::LibCpuCounter endCountSystem;
    //     startSystem.Update();
    //     auto startSysTime = KERNEL_NS::LibTime::Now();
    //     for(Int32 idx = 0; idx < testLoopCount; ++idx)
    //         new Byte8[testBufferSize];
    //     auto endSysTime = KERNEL_NS::LibTime::Now();
    //     endCountSystem.Update();

    //     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMemoryAlloctor, "TEST speed blockSize:%d, test count:%d, system malloc total cost:%lld, unit cost:%llu")
    //                 , testBufferSize, testLoopCount, (endSysTime - startSysTime).GetTotalMicroSeconds(), (endSysTime - startSysTime).GetTotalMicroSeconds() / static_cast<Int64>(testLoopCount));
    // }

    // {// 无锁条件下释放 linux 下 pool 的 free是系统的 1.5倍左右
    //     const Int32 testLoopCount = 100000;
    //     const Int32 testBufferSize = TEST_ALLOC_UNIT_BYTES;
    //     std::vector<void *> poolPtrs;
    //     for(Int32 idx = 0; idx < testLoopCount; ++idx)
    //         poolPtrs.push_back(alloctor.Alloc(testBufferSize));

    //     auto poolStart = KERNEL_NS::LibTime::Now();
    //     for(Int32 idx = 0; idx < testLoopCount; ++idx)
    //         alloctor.Free(poolPtrs[idx]);

    //     auto poolEnd = KERNEL_NS::LibTime::Now();
        
    //     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMemoryAlloctor, "TEST pool free speed blockSize:%d, test count:%d, pool total cost:%lld, unit cost:%lld")
    //                 , testBufferSize, testLoopCount, (poolEnd - poolStart).GetTotalMicroSeconds(), (poolEnd - poolStart).GetTotalMicroSeconds() / static_cast<Int64>(testLoopCount));

    //     std::vector<Byte8 *> sysPtrs;
    //     for(Int32 idx = 0; idx < testLoopCount; ++idx)
    //         sysPtrs.push_back(new Byte8[testBufferSize]);

    //     auto sysStart = KERNEL_NS::LibTime::Now();
    //     for(Int32 idx = 0; idx < testLoopCount; ++idx)
    //         delete [] sysPtrs[idx];
    //     auto sysEnd = KERNEL_NS::LibTime::Now();

    //     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMemoryAlloctor, "TEST system delete speed blockSize:%d, test count:%d, system malloc total cost:%lld, unit cost:%llu")
    //                 , testBufferSize, testLoopCount, (sysEnd - sysStart).GetTotalMicroSeconds(), (sysEnd - sysStart).GetTotalMicroSeconds() / static_cast<Int64>(testLoopCount));
    // }

    // {// 无锁条件下测试gc效果 linux 下 pool 的 free是系统的 1.5倍左右
    //     const Int32 testLoopCount = 100000;
    //     const Int32 testBufferSize = TEST_ALLOC_UNIT_BYTES;

    //     for(Int32 loop = 0; loop < 64; ++loop)
    //     {
    //         std::vector<void *> poolPtrs;
    //         for(Int32 idx = 0; idx < 4; ++idx)
    //             poolPtrs.push_back(alloctor.Alloc(testBufferSize));

    //         for(Int32 idx = 0; idx < 4; ++idx)
    //             alloctor.Free(poolPtrs[idx]);
    //     }
    // }

    {// 无锁条件下同时分配与释放 linux 下 pool 的性能是 system的 5倍左右 这是大部分的情形
        const Int32 testLoopCount = 1000000;
        const Int32 testBufferSize = TEST_ALLOC_UNIT_BYTES;

        auto poolStart = KERNEL_NS::LibTime::Now();
        for(Int32 idx = 0; idx < testLoopCount; ++idx)
            alloctor.Free(alloctor.Alloc(testBufferSize));

        auto poolEnd = KERNEL_NS::LibTime::Now();
        
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMemoryAlloctor, "TEST pool free speed blockSize:%d, test count:%d, pool total cost:%lld, unit cost:%lld")
                    , testBufferSize, testLoopCount, (poolEnd - poolStart).GetTotalMicroSeconds(), (poolEnd - poolStart).GetTotalMicroSeconds() / static_cast<Int64>(testLoopCount));

        auto sysStart = KERNEL_NS::LibTime::Now();
        for(Int32 idx = 0; idx < testLoopCount; ++idx)
            delete [] (new Byte8[testBufferSize]);
        auto sysEnd = KERNEL_NS::LibTime::Now();

        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMemoryAlloctor, "TEST system delete speed blockSize:%d, test count:%d, system malloc total cost:%lld, unit cost:%llu")
                    , testBufferSize, testLoopCount, (sysEnd - sysStart).GetTotalMicroSeconds(), (sysEnd - sysStart).GetTotalMicroSeconds() / static_cast<Int64>(testLoopCount));
    }


    // {// 加锁条件下分配 linux 下 pool 在频繁触发NewBuffer情况下系统性能是pool的2.2倍,在不触发NewBuffer情况下pool的性能是系统的5.5倍左右
    //     const UInt64 testLoopCount = 100000;
    //     const Int32 testBufferSize = TEST_ALLOC_UNIT_BYTES;

    //     auto poolStart = KERNEL_NS::LibTime::Now();
    //     for(Int32 idx = 0; idx < testLoopCount; ++idx)
    //     {
    //         alloctor.Lock();
    //         alloctor.Alloc(testBufferSize);
    //         alloctor.Unlock();
    //     }

    //     auto poolEnd = KERNEL_NS::LibTime::Now();
        
    //     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMemoryAlloctor, "TEST pool free speed blockSize:%d, test count:%llu, pool total cost:%lld, unit cost:%lld")
    //                 , testBufferSize, testLoopCount, (poolEnd - poolStart).GetTotalMicroSeconds(), (poolEnd - poolStart).GetTotalMicroSeconds() / static_cast<Int64>(testLoopCount));

    //     auto sysStart = KERNEL_NS::LibTime::Now();
    //     for(Int32 idx = 0; idx < testLoopCount; ++idx)
    //         new Byte8[testBufferSize];
    //     auto sysEnd = KERNEL_NS::LibTime::Now();

    //     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMemoryAlloctor, "TEST system delete speed blockSize:%d, test count:%llu, system malloc total cost:%lld, unit cost:%llu")
    //                 , testBufferSize, testLoopCount, (sysEnd - sysStart).GetTotalMicroSeconds(), (sysEnd - sysStart).GetTotalMicroSeconds() / static_cast<Int64>(testLoopCount));
    // }

    // {// 加锁条件下释放 linux 下 pool 的 free是系统的 2倍左右
    //     const UInt64 testLoopCount = 100000;
    //     const Int32 testBufferSize = TEST_ALLOC_UNIT_BYTES;
    //     std::vector<void *> poolPtrs;
    //     for(Int32 idx = 0; idx < testLoopCount; ++idx)
    //         poolPtrs.push_back(alloctor.Alloc(testBufferSize));

    //     auto poolStart = KERNEL_NS::LibTime::Now();
    //     for(Int32 idx = 0; idx < testLoopCount; ++idx)
    //     {
    //         alloctor.Lock();
    //         alloctor.Free(poolPtrs[idx]);
    //         alloctor.Unlock();
    //     }

    //     auto poolEnd = KERNEL_NS::LibTime::Now();
        
    //     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMemoryAlloctor, "TEST pool free speed blockSize:%d, test count:%llu, pool total cost:%lld, unit cost:%lld")
    //                 , testBufferSize, testLoopCount, (poolEnd - poolStart).GetTotalMicroSeconds(), (poolEnd - poolStart).GetTotalMicroSeconds() / static_cast<Int64>(testLoopCount));

    //     std::vector<void *> sysPtrs;
    //     for(Int32 idx = 0; idx < testLoopCount; ++idx)
    //         sysPtrs.push_back(new Byte8[testBufferSize]);

    //     auto sysStart = KERNEL_NS::LibTime::Now();
    //     for(Int32 idx = 0; idx < testLoopCount; ++idx)
    //         delete [] sysPtrs[idx];
    //     auto sysEnd = KERNEL_NS::LibTime::Now();

    //     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMemoryAlloctor, "TEST system delete speed blockSize:%d, test count:%llu, system malloc total cost:%lld, unit cost:%llu")
    //                 , testBufferSize, testLoopCount, (sysEnd - sysStart).GetTotalMicroSeconds(), (sysEnd - sysStart).GetTotalMicroSeconds() / static_cast<Int64>(testLoopCount));
    // }

    // {// 加锁条件下同时分配与释放 linux 下 pool 的性能是 system的 3倍左右 这是大部分的情形
    //     const UInt64 testLoopCount = 100000;
    //     const Int32 testBufferSize = TEST_ALLOC_UNIT_BYTES;

    //     auto poolStart = KERNEL_NS::LibTime::Now();
    //     for(Int32 idx = 0; idx < testLoopCount; ++idx)
    //     {
    //         alloctor.Lock();
    //         alloctor.Free(alloctor.Alloc(testBufferSize));
    //         alloctor.Unlock();
    //     }

    //     auto poolEnd = KERNEL_NS::LibTime::Now();
        
    //     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMemoryAlloctor, "TEST pool free speed blockSize:%d, test count:%llu, pool total cost:%lld, unit cost:%lld")
    //                 , testBufferSize, testLoopCount, (poolEnd - poolStart).GetTotalMicroSeconds(), (poolEnd - poolStart).GetTotalMicroSeconds() / static_cast<Int64>(testLoopCount));

    //     auto sysStart = KERNEL_NS::LibTime::Now();
    //     for(Int32 idx = 0; idx < testLoopCount; ++idx)
    //         delete [] (new Byte8[testBufferSize]);
    //     auto sysEnd = KERNEL_NS::LibTime::Now();

    //     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMemoryAlloctor, "TEST system delete speed blockSize:%d, test count:%llu, system malloc total cost:%lld, unit cost:%llu")
    //                 , testBufferSize, testLoopCount, (sysEnd - sysStart).GetTotalMicroSeconds(), (sysEnd - sysStart).GetTotalMicroSeconds() / static_cast<Int64>(testLoopCount));
    // }


// void TestMemoryAlloctor::Run()
// {
//     KERNEL_NS::MemoryAlloctorConfig cfg(TEST_ALLOC_UNIT_BYTES);
//     KERNEL_NS::MemoryAlloctor alloctor(cfg);
    
//     alloctor.Init(TEST_BLOCK_NUM_INIT);

//     std::vector<char *> *ptrs = new std::vector<char *>;

//     Int64 timeBegin = NowTime();
//     for (int i = 0; i < 10240; ++i)
//     {
//         alloctor.Lock();
//         ptrs->push_back((char *)alloctor.Alloc(TEST_ALLOC_TO_ALLOC));
//         alloctor.Unlock();
//     }

//     UInt64 eleCount = ptrs->size();
//     for (UInt64 i = eleCount - 1; i >= eleCount - 5120; --i)
//     {
//         alloctor.Lock();
//         alloctor.Free((*ptrs)[i]);
//         alloctor.Unlock();
//         ptrs->erase(ptrs->begin() + i);
//     }

//     for (UInt64 i = 0; i < 10240; ++i)
//     {
//         ptrs->push_back((char *)alloctor.Alloc(TEST_ALLOC_TO_ALLOC));
//     }

//     eleCount = ptrs->size();
//     for (UInt64 i = eleCount - 1; i >= eleCount - 51200; --i)
//     {
//         alloctor.Lock();
//         alloctor.Free((*ptrs)[i]);
//         alloctor.Unlock();
//         ptrs->erase(ptrs->begin() + i);
//     }
        
//     for (UInt64 i = 0; i < 10240; ++i)
//     {   
//         alloctor.Lock();
//         ptrs->push_back((char *)alloctor.Alloc(TEST_ALLOC_TO_ALLOC));
//         alloctor.Unlock();
//     }

//     eleCount = ptrs->size();
//     for (UInt64 i = eleCount - 1; i >= 5120; --i)
//     {
//         alloctor.Lock();
//         alloctor.Free((*ptrs)[i]);
//         alloctor.Unlock();
//         ptrs->erase(ptrs->begin() + i);
//     }

//     for (UInt64 i = 0; i < 10240; ++i)
//     {
//         alloctor.Lock();
//         ptrs->push_back((char *)alloctor.Alloc(TEST_ALLOC_TO_ALLOC));
//         alloctor.Unlock();
//     }

//     eleCount = ptrs->size();
//     for (Int64 i = eleCount - 1; i >= 0; --i)
//     {
//         alloctor.Lock();
//         alloctor.Free((*ptrs)[i]);
//         alloctor.Unlock();
//         ptrs->erase(ptrs->begin() + i);
//     }
//     Int64 timeEnd = NowTime();

//     printf("memory alloc ==== interval = %lld us\n", timeEnd - timeBegin);

//     ptrs->clear();
//     timeBegin = NowTime();
//     for (UInt64 i = 0; i < 10240; ++i)
//     {
//         ptrs->push_back((char *)::malloc(TEST_ALLOC_TO_ALLOC));
//     }

//     eleCount = ptrs->size();
//     for (UInt64 i = eleCount - 1; i >= eleCount - 5120; --i)
//     {
//         ::free((*ptrs)[i]);
//         ptrs->erase(ptrs->begin() + i);
//     }

//     for (UInt64 i = 0; i < 102400; ++i)
//     {
//         ptrs->push_back((char *)::malloc(TEST_ALLOC_TO_ALLOC));
//     }

//     eleCount = ptrs->size();
//     for (UInt64 i = eleCount - 1; i >= eleCount - 51200; --i)
//     {
//         ::free((*ptrs)[i]);
//         ptrs->erase(ptrs->begin() + i);
//     }

//     for (UInt64 i = 0; i < 1024000; ++i)
//     {
//         ptrs->push_back((char *)::malloc(TEST_ALLOC_TO_ALLOC));
//     }

//     eleCount = ptrs->size();
//     for (UInt64 i = eleCount - 1; i >= 512000; --i)
//     {
//         ::free((*ptrs)[i]);
//         ptrs->erase(ptrs->begin() + i);
//     }

//     for (UInt64 i = 0; i < 10240000; ++i)
//     {
//         ptrs->push_back((char *)::malloc(TEST_ALLOC_TO_ALLOC));
//     }

//     eleCount = ptrs->size();
//     for (Int64 i = eleCount - 1; i >= 0; --i)
//     {
//         ::free((*ptrs)[i]);
//         ptrs->erase(ptrs->begin() + i);
//     }
// //     for (int i = 0; i < TEST_ALLOC_LOOP; ++i)
// //         ::free(::malloc(TEST_ALLOC_TO_ALLOC));

// //     for (int i = 0; i < TEST_ALLOC_LOOP; ++i)
// //         ::free((*ptrs)[i]);
//     timeEnd = NowTime();

//     printf("system malloc ==== interval = %lld us \n", timeEnd - timeBegin);
// //     for (int i = 0; i < TEST_ALLOC_LOOP; ++i)
// //     {
// //         ptrs.push_back(reinterpret_cast<char *>(alloctor.Alloc(TEST_ALLOC_TO_ALLOC)));
// //     }
// // 
// //     for (int i = 0; i < TEST_ALLOC_LOOP; ++i)
// //         alloctor.Free(ptrs[i]);

// //     char *ptr = reinterpret_cast<char *>(alloctor.Alloc(TEST_ALLOC_TO_ALLOC));
// //     alloctor.Free(ptr);
// //     ptr = reinterpret_cast<char *>(alloctor.Alloc(TEST_ALLOC_TO_ALLOC));
// //     ptr = reinterpret_cast<char *>(alloctor.Alloc(TEST_ALLOC_TO_ALLOC));
// //     ptr = reinterpret_cast<char *>(alloctor.Alloc(TEST_ALLOC_TO_ALLOC));
// //     ptr = reinterpret_cast<char *>(alloctor.Alloc(TEST_ALLOC_TO_ALLOC));
// //     alloctor.Free(ptr);

//     std::cout << alloctor.ToString().c_str() << std::endl;
//     getchar();
//     alloctor.Destroy();
//     getchar();

}