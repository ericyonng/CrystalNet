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
 * Date: 2020-11-22 21:28:54
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestMemoryPool.h>

#define  TEST_MEMORY_POOL_SIZE 32768
#define  TEST_ALLOC_NUM 300000

// void TestMemoryPool::Run()
// {
//     KERNEL_NS::MemoryPool *pool = new KERNEL_NS::MemoryPool;

//     pool->Init();
//     std::cout << pool->ToString().c_str() << std::endl;

//     std::vector<void *> *ptrs = new std::vector<void *>;
//     for (UInt64 i = 16; i <= TEST_MEMORY_POOL_SIZE; i *= 2)
//     {
//         for (UInt64 j = 0; j < TEST_ALLOC_NUM; ++j)
//         {
//             Byte8 *mem = reinterpret_cast<Byte8 *>(pool->Alloc(i));
//             ::memset(mem, 0, i);
//             strcpy(mem, "hello world");
//             ptrs->push_back(mem);
//         }
//         //pool->Free(mem);
//     }

//     std::cout << pool->ToString().c_str() << std::endl;
    
//     getchar();
//     for (auto &ptr : *ptrs)
//         pool->Free(ptr);

//     std::cout << "pool info:" << std::endl;
//     std::cout << pool->ToString().c_str() << std::endl;
//     getchar();
//     pool->Destroy();
//     std::cout << pool->ToString() << std::endl;
//     getchar();
// }

#undef TEST_ALLOC_UNIT_BYTES
#define TEST_ALLOC_UNIT_BYTES 256

#define TEST_ALLOC_TO_ALLOC 1024

#define  TEST_ALLOC_LOOP   10240
#define  TEST_BLOCK_NUM_INIT   1024
#define  TEST_BLOCK_NUM_LIMIT   102400

void TestMemoryPool::Run()
{
    auto memoryPool = new KERNEL_NS::MemoryPool(KERNEL_NS::InitMemoryPoolInfo(1));
    memoryPool->Init();

    // {// ?????????????????? ?????????????????????????????????????????????1.41???(??????????????????memory buffer)
    //     const Int32 testLoopCount = 100000;
    //     const Int32 testBufferSize = TEST_ALLOC_UNIT_BYTES;
    //     KERNEL_NS::LibCpuCounter start;
    //     KERNEL_NS::LibCpuCounter endCount;
    //     start.Update();
    //     auto startPoolTime = KERNEL_NS::LibTime::Now();
    //     for(Int32 idx = 0; idx < testLoopCount; ++idx)
    //         memoryPool->AllocThreadLocal(testBufferSize);
    //     auto endPoolTime = KERNEL_NS::LibTime::Now();
    //     endCount.Update();
    //     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMemoryPool, "TEST speed blockSize:%d, test count:%llu, pool total cost:%lld, unit cost:%lld")
    //                 , testBufferSize, testLoopCount, (endPoolTime - startPoolTime).GetTotalMicroSeconds(), (endPoolTime - startPoolTime).GetTotalMicroSeconds() / static_cast<Int64>(testLoopCount));

    //     KERNEL_NS::LibCpuCounter startSystem;
    //     KERNEL_NS::LibCpuCounter endCountSystem;
    //     startSystem.Update();
    //     auto startSysTime = KERNEL_NS::LibTime::Now();
    //     for(Int32 idx = 0; idx < testLoopCount; ++idx)
    //         new Byte8[testBufferSize];
    //     auto endSysTime = KERNEL_NS::LibTime::Now();
    //     endCountSystem.Update();

    //     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMemoryPool, "TEST speed blockSize:%d, test count:%llu, system malloc total cost:%lld, unit cost:%llu")
    //                 , testBufferSize, testLoopCount, (endSysTime - startSysTime).GetTotalMicroSeconds(), (endSysTime - startSysTime).GetTotalMicroSeconds() / static_cast<Int64>(testLoopCount));

    //     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMemoryPool, "memory pool info:%s"), memoryPool->ToString().c_str());
    // }

    // {// ?????????????????? pool ????????? system ???1.25???
    //     const Int32 testLoopCount = 100000;
    //     const Int32 testBufferSize = TEST_ALLOC_UNIT_BYTES;
    //     std::vector<void *> poolPtrs;
    //     for(Int32 idx = 0; idx < testLoopCount; ++idx)
    //         poolPtrs.push_back(memoryPool->AllocThreadLocal(testBufferSize));

    //     auto poolStart = KERNEL_NS::LibTime::Now();
    //     for(Int32 idx = 0; idx < testLoopCount; ++idx)
    //         memoryPool->FreeThreadLocal(poolPtrs[idx]);

    //     auto poolEnd = KERNEL_NS::LibTime::Now();
        
    //     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMemoryPool, "TEST pool free speed blockSize:%d, test count:%d, pool total cost:%lld, unit cost:%lld")
    //                 , testBufferSize, testLoopCount, (poolEnd - poolStart).GetTotalMicroSeconds(), (poolEnd - poolStart).GetTotalMicroSeconds() / static_cast<Int64>(testLoopCount));

    //     std::vector<Byte8 *> sysPtrs;
    //     for(Int32 idx = 0; idx < testLoopCount; ++idx)
    //         sysPtrs.push_back(new Byte8[testBufferSize]);

    //     auto sysStart = KERNEL_NS::LibTime::Now();
    //     for(Int32 idx = 0; idx < testLoopCount; ++idx)
    //         delete [] sysPtrs[idx];
    //     auto sysEnd = KERNEL_NS::LibTime::Now();

    //     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMemoryPool, "TEST system delete speed blockSize:%d, test count:%d, system malloc total cost:%lld, unit cost:%llu")
    //                 , testBufferSize, testLoopCount, (sysEnd - sysStart).GetTotalMicroSeconds(), (sysEnd - sysStart).GetTotalMicroSeconds() / static_cast<Int64>(testLoopCount));
    
    // }

    // {// ??????????????????????????? pool ??? system???4?????????
    //     const UInt64 testLoopCount = 100000;
    //     const Int32 testBufferSize = TEST_ALLOC_UNIT_BYTES;

    //     auto poolStart = KERNEL_NS::LibTime::Now();
    //     for(Int32 idx = 0; idx < testLoopCount; ++idx)
    //         memoryPool->FreeThreadLocal(memoryPool->AllocThreadLocal(testBufferSize));

    //     auto poolEnd = KERNEL_NS::LibTime::Now();
        
    //     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMemoryPool, "TEST pool free speed blockSize:%d, test count:%llu, pool total cost:%lld, unit cost:%lld")
    //                 , testBufferSize, testLoopCount, (poolEnd - poolStart).GetTotalMicroSeconds(), (poolEnd - poolStart).GetTotalMicroSeconds() / static_cast<Int64>(testLoopCount));

    //     auto sysStart = KERNEL_NS::LibTime::Now();
    //     for(Int32 idx = 0; idx < testLoopCount; ++idx)
    //         delete [] (new Byte8[testBufferSize]);
    //     auto sysEnd = KERNEL_NS::LibTime::Now();

    //     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMemoryPool, "TEST system delete speed blockSize:%d, test count:%llu, system malloc total cost:%lld, unit cost:%llu")
    //                 , testBufferSize, testLoopCount, (sysEnd - sysStart).GetTotalMicroSeconds(), (sysEnd - sysStart).GetTotalMicroSeconds() / static_cast<Int64>(testLoopCount));
    
    // }

//    {// ????????????????????? ?????????pool???1.2????????????????????????NewBuffer?????????????????????
//         const Int32 testLoopCount = 100000;
//         const Int32 testBufferSize = TEST_ALLOC_UNIT_BYTES;

//         auto poolStart = KERNEL_NS::LibTime::Now();
//         for(Int32 idx = 0; idx < testLoopCount; ++idx)
//             memoryPool->Alloc(testBufferSize);

//         auto poolEnd = KERNEL_NS::LibTime::Now();
        
//         g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMemoryPool, "TEST pool free speed blockSize:%d, test count:%llu, pool total cost:%lld, unit cost:%lld")
//                     , testBufferSize, testLoopCount, (poolEnd - poolStart).GetTotalMicroSeconds(), (poolEnd - poolStart).GetTotalMicroSeconds() / static_cast<Int64>(testLoopCount));

//         auto sysStart = KERNEL_NS::LibTime::Now();
//         for(Int32 idx = 0; idx < testLoopCount; ++idx)
//             new Byte8[testBufferSize];
//         auto sysEnd = KERNEL_NS::LibTime::Now();

//         g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMemoryPool, "TEST system delete speed blockSize:%d, test count:%llu, system malloc total cost:%lld, unit cost:%llu")
//                     , testBufferSize, testLoopCount, (sysEnd - sysStart).GetTotalMicroSeconds(), (sysEnd - sysStart).GetTotalMicroSeconds() / static_cast<Int64>(testLoopCount));
//     }

    // {// ????????????????????? linux ??? pool ??? free???????????? 1.5?????????
    //     const Int32 testLoopCount = 100000;
    //     const Int32 testBufferSize = TEST_ALLOC_UNIT_BYTES;
    //     std::vector<void *> poolPtrs;
    //     for(Int32 idx = 0; idx < testLoopCount; ++idx)
    //         poolPtrs.push_back(memoryPool->Alloc(testBufferSize));

    //     auto poolStart = KERNEL_NS::LibTime::Now();
    //     for(Int32 idx = 0; idx < testLoopCount; ++idx)
    //         memoryPool->Free(poolPtrs[idx]);

    //     auto poolEnd = KERNEL_NS::LibTime::Now();
        
    //     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMemoryPool, "TEST pool free speed blockSize:%d, test count:%llu, pool total cost:%lld, unit cost:%lld")
    //                 , testBufferSize, testLoopCount, (poolEnd - poolStart).GetTotalMicroSeconds(), (poolEnd - poolStart).GetTotalMicroSeconds() / static_cast<Int64>(testLoopCount));

    //     std::vector<void *> sysPtrs;
    //     for(Int32 idx = 0; idx < testLoopCount; ++idx)
    //         sysPtrs.push_back(new Byte8[testBufferSize]);

    //     auto sysStart = KERNEL_NS::LibTime::Now();
    //     for(Int32 idx = 0; idx < testLoopCount; ++idx)
    //         delete [] sysPtrs[idx];
    //     auto sysEnd = KERNEL_NS::LibTime::Now();

    //     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMemoryPool, "TEST system delete speed blockSize:%d, test count:%llu, system malloc total cost:%lld, unit cost:%llu")
    //                 , testBufferSize, testLoopCount, (sysEnd - sysStart).GetTotalMicroSeconds(), (sysEnd - sysStart).GetTotalMicroSeconds() / static_cast<Int64>(testLoopCount));
    // }

    {// ???????????????????????????????????? linux ??? pool ???????????? system??? 4.9????????? ????????????????????????
        const Int32 testLoopCount = 100000;
        const Int32 testBufferSize = TEST_ALLOC_UNIT_BYTES;

        auto poolStart = KERNEL_NS::LibTime::Now();
        for(Int32 idx = 0; idx < testLoopCount; ++idx)
            memoryPool->Free(memoryPool->Alloc(testBufferSize));

        auto poolEnd = KERNEL_NS::LibTime::Now();
        
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMemoryPool, "TEST pool free speed blockSize:%d, test count:%d, pool total cost:%lld, unit cost:%lld")
                    , testBufferSize, testLoopCount, (poolEnd - poolStart).GetTotalMicroSeconds(), (poolEnd - poolStart).GetTotalMicroSeconds() / static_cast<Int64>(testLoopCount));

        auto sysStart = KERNEL_NS::LibTime::Now();
        for(Int32 idx = 0; idx < testLoopCount; ++idx)
            delete [] (new Byte8[testBufferSize]);
        auto sysEnd = KERNEL_NS::LibTime::Now();

        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestMemoryPool, "TEST system delete speed blockSize:%d, test count:%d, system malloc total cost:%lld, unit cost:%llu")
                    , testBufferSize, testLoopCount, (sysEnd - sysStart).GetTotalMicroSeconds(), (sysEnd - sysStart).GetTotalMicroSeconds() / static_cast<Int64>(testLoopCount));
    }

    memoryPool->Destroy();
}
