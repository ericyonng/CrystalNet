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

void TestMemoryPool::Run()
{
    KERNEL_NS::MemoryPool *pool = new KERNEL_NS::MemoryPool;

    pool->Init();
    std::cout << pool->ToString().c_str() << std::endl;

    std::vector<void *> *ptrs = new std::vector<void *>;
    for (UInt64 i = 16; i <= TEST_MEMORY_POOL_SIZE; i *= 2)
    {
        for (UInt64 j = 0; j < TEST_ALLOC_NUM; ++j)
        {
            Byte8 *mem = reinterpret_cast<Byte8 *>(pool->Alloc(i));
            ::memset(mem, 0, i);
            strcpy(mem, "hello world");
            ptrs->push_back(mem);
        }
        //pool->Free(mem);
    }

    std::cout << pool->ToString().c_str() << std::endl;
    
    getchar();
    for (auto &ptr : *ptrs)
        pool->Free(ptr);

    std::cout << "pool info:" << std::endl;
    std::cout << pool->ToString().c_str() << std::endl;
    getchar();
    pool->Destroy();
    std::cout << pool->ToString() << std::endl;
    getchar();
}