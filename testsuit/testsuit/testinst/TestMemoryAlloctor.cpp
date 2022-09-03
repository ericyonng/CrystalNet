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
#define TEST_ALLOC_UNIT_BYTES 1024

#define TEST_ALLOC_TO_ALLOC 1024

#define  TEST_ALLOC_LOOP   10240
#define  TEST_BLOCK_NUM_INIT   1024
#define  TEST_BLOCK_NUM_LIMIT   102400

static Int64 NowTime()
{
    // return std::chrono::system_clock::now().time_since_epoch().count() / 10;
    return KERNEL_NS::TimeUtil::GetMicroTimestamp();
}

void TestMemoryAlloctor::Run()
{
    KERNEL_NS::MemoryAlloctorConfig cfg(TEST_ALLOC_UNIT_BYTES);
    KERNEL_NS::MemoryAlloctor alloctor(cfg);
    
    alloctor.Init(TEST_BLOCK_NUM_INIT);

    std::vector<char *> *ptrs = new std::vector<char *>;

    Int64 timeBegin = NowTime();
    for (int i = 0; i < 10240; ++i)
    {
        alloctor.Lock();
        ptrs->push_back((char *)alloctor.Alloc(TEST_ALLOC_TO_ALLOC));
        alloctor.Unlock();
    }

    UInt64 eleCount = ptrs->size();
    for (UInt64 i = eleCount - 1; i >= eleCount - 5120; --i)
    {
        alloctor.Lock();
        alloctor.Free((*ptrs)[i]);
        alloctor.Unlock();
        ptrs->erase(ptrs->begin() + i);
    }

    for (UInt64 i = 0; i < 10240; ++i)
    {
        ptrs->push_back((char *)alloctor.Alloc(TEST_ALLOC_TO_ALLOC));
    }

    eleCount = ptrs->size();
    for (UInt64 i = eleCount - 1; i >= eleCount - 51200; --i)
    {
        alloctor.Lock();
        alloctor.Free((*ptrs)[i]);
        alloctor.Unlock();
        ptrs->erase(ptrs->begin() + i);
    }
        
    for (UInt64 i = 0; i < 10240; ++i)
    {   
        alloctor.Lock();
        ptrs->push_back((char *)alloctor.Alloc(TEST_ALLOC_TO_ALLOC));
        alloctor.Unlock();
    }

    eleCount = ptrs->size();
    for (UInt64 i = eleCount - 1; i >= 5120; --i)
    {
        alloctor.Lock();
        alloctor.Free((*ptrs)[i]);
        alloctor.Unlock();
        ptrs->erase(ptrs->begin() + i);
    }

    for (UInt64 i = 0; i < 10240; ++i)
    {
        alloctor.Lock();
        ptrs->push_back((char *)alloctor.Alloc(TEST_ALLOC_TO_ALLOC));
        alloctor.Unlock();
    }

    eleCount = ptrs->size();
    for (Int64 i = eleCount - 1; i >= 0; --i)
    {
        alloctor.Lock();
        alloctor.Free((*ptrs)[i]);
        alloctor.Unlock();
        ptrs->erase(ptrs->begin() + i);
    }
    Int64 timeEnd = NowTime();

    printf("memory alloc ==== interval = %lld us\n", timeEnd - timeBegin);

    ptrs->clear();
    timeBegin = NowTime();
    for (UInt64 i = 0; i < 10240; ++i)
    {
        ptrs->push_back((char *)::malloc(TEST_ALLOC_TO_ALLOC));
    }

    eleCount = ptrs->size();
    for (UInt64 i = eleCount - 1; i >= eleCount - 5120; --i)
    {
        ::free((*ptrs)[i]);
        ptrs->erase(ptrs->begin() + i);
    }

    for (UInt64 i = 0; i < 102400; ++i)
    {
        ptrs->push_back((char *)::malloc(TEST_ALLOC_TO_ALLOC));
    }

    eleCount = ptrs->size();
    for (UInt64 i = eleCount - 1; i >= eleCount - 51200; --i)
    {
        ::free((*ptrs)[i]);
        ptrs->erase(ptrs->begin() + i);
    }

    for (UInt64 i = 0; i < 1024000; ++i)
    {
        ptrs->push_back((char *)::malloc(TEST_ALLOC_TO_ALLOC));
    }

    eleCount = ptrs->size();
    for (UInt64 i = eleCount - 1; i >= 512000; --i)
    {
        ::free((*ptrs)[i]);
        ptrs->erase(ptrs->begin() + i);
    }

    for (UInt64 i = 0; i < 10240000; ++i)
    {
        ptrs->push_back((char *)::malloc(TEST_ALLOC_TO_ALLOC));
    }

    eleCount = ptrs->size();
    for (Int64 i = eleCount - 1; i >= 0; --i)
    {
        ::free((*ptrs)[i]);
        ptrs->erase(ptrs->begin() + i);
    }
//     for (int i = 0; i < TEST_ALLOC_LOOP; ++i)
//         ::free(::malloc(TEST_ALLOC_TO_ALLOC));

//     for (int i = 0; i < TEST_ALLOC_LOOP; ++i)
//         ::free((*ptrs)[i]);
    timeEnd = NowTime();

    printf("system malloc ==== interval = %lld us \n", timeEnd - timeBegin);
//     for (int i = 0; i < TEST_ALLOC_LOOP; ++i)
//     {
//         ptrs.push_back(reinterpret_cast<char *>(alloctor.Alloc(TEST_ALLOC_TO_ALLOC)));
//     }
// 
//     for (int i = 0; i < TEST_ALLOC_LOOP; ++i)
//         alloctor.Free(ptrs[i]);

//     char *ptr = reinterpret_cast<char *>(alloctor.Alloc(TEST_ALLOC_TO_ALLOC));
//     alloctor.Free(ptr);
//     ptr = reinterpret_cast<char *>(alloctor.Alloc(TEST_ALLOC_TO_ALLOC));
//     ptr = reinterpret_cast<char *>(alloctor.Alloc(TEST_ALLOC_TO_ALLOC));
//     ptr = reinterpret_cast<char *>(alloctor.Alloc(TEST_ALLOC_TO_ALLOC));
//     ptr = reinterpret_cast<char *>(alloctor.Alloc(TEST_ALLOC_TO_ALLOC));
//     alloctor.Free(ptr);

    std::cout << alloctor.ToString().c_str() << std::endl;
    getchar();
    alloctor.Destroy();
    getchar();

}