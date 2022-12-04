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
 * Date: 2022-09-15 15:52:50
 * Author: Eric Yonng
 * Description: 
*/

#include<pch.h>
#include<testsuit/testinst/TestMemoryAssist.h>

struct TestBuffer
{
    Byte8 _buffer[128] = {0};
};

class TestBuffer2
{
public:
    TestBuffer2(const Byte8 *str, UInt64 len)
    {
        ::memcpy(_buffer, str, sizeof(_buffer) > len ? len: sizeof(_buffer));
    }

    Byte8 _buffer[128] = {0};
};

void TestMemoryAssist::Run()
{
    auto buffer = OBJ_POOL_NEW(TestBuffer, KERNEL_NS::_Build::TL);
    auto buffer2 = OBJ_POOL_NEW(TestBuffer2, KERNEL_NS::_Build::TL, "hello pool .", 12);

    getchar();
    
    OBJ_POOL_DEL(TestBuffer, KERNEL_NS::_Build::TL, buffer);
    OBJ_POOL_DEL(TestBuffer2, KERNEL_NS::_Build::TL, buffer2);
    
}