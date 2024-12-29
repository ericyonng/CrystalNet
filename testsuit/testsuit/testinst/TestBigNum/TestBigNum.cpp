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
 * Date: 2024.08.02 14:38:00
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include "TestBigNum.h"

void TestBigNum::Run()
{
    KERNEL_NS::BigNum bigNum = KERNEL_NS::BigNum::ZeroBigNum;
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBigNum, "bigNum:%s"), bigNum.ToString().c_str());
    KERNEL_NS::BigNum bigNum2 = 555555LLU;
    KERNEL_NS::BigNum bigNum3 = KERNEL_NS::BigNum(555, 10000);
    
    // 加减
    auto bigNum4 = bigNum3 + bigNum3;
    auto bigNum5 = bigNum4 - bigNum2;
    
    // 位运算
    KERNEL_NS::BigNum big6 = bigNum5 & bigNum4;
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBigNum, "big6:%s"), big6.ToString().c_str());

    KERNEL_NS::BigNum &&big7 = bigNum5 | bigNum4;
    KERNEL_NS::BigNum big8 = ~bigNum5;
    KERNEL_NS::BigNum big9 = bigNum5 ^ bigNum4;
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBigNum, "big9:%s"), big9.ToString().c_str());

    bigNum4 += bigNum5;
    bigNum4 -= bigNum5;

    bigNum4 |= bigNum5;
    bigNum4 &= bigNum5;
    bigNum4 ^= bigNum5;

    if(big7 && big8)
    {
        KERNEL_NS::LibString str = big7.ToString();
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBigNum, "big7:%s, big8:%s"), str.c_str(), big8.ToString().c_str());
    }

    if(big7 || big8)
    {
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBigNum, "big7:%s, big8:%s"), big7.ToString().c_str(), big8.ToHexString().c_str());
    }

    if(!big7)
    {
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBigNum, "big7:%s, big8:%s"), big7.ToString().c_str(), big8.ToString().c_str());
    }

    UInt64 jk = 54654;
    auto bigNum14 = bigNum3 + jk;
    auto bigNum15 = bigNum3 - jk;
    bigNum15 += jk;
    bigNum15 -= jk;
    bigNum15 = bigNum14 & jk;
    bigNum15 = bigNum14 | jk;
    bigNum15 = jk;
    bigNum15 = bigNum14 ^ jk;
    bigNum15 &= jk;
    bigNum15 |= jk;
    bigNum15 ^= jk;

    if(bigNum15 && jk)
    {
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBigNum, "big7:%s, big8:%s"), bigNum15.ToString().c_str(), big8.ToString().c_str());
    }

    if(bigNum15 || jk)
    {
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBigNum, "big7:%s, big8:%s"), bigNum15.ToString().c_str(), big8.ToString().c_str());
    }

    if(jk && bigNum15)
    {
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBigNum, "big7:%s, big8:%s"), bigNum15.ToString().c_str(), big8.ToString().c_str());
    }

    if(jk || bigNum15)
    {
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBigNum, "big7:%s, big8:%s"), bigNum15.ToString().c_str(), big8.ToString().c_str());
    }

    // auto ret = jk < bigNum14;
    // auto ret2 = jk > bigNum14;
    // auto ret3 = jk == bigNum14;
    /// auto ret4 = jk <= bigNum14;
    // auto ret5 = jk >= bigNum14;
    // auto ret6 = jk != bigNum14;

    // auto bigNum30 = KERNEL_NS::BigNum(0xFFFF, 0xFF000000FF00FF00);
    // auto bigNumCopy = bigNum30;
    // auto bigNum31 = bigNum30 >> 8;
    // auto bigNum33 = bigNum30 >> 32;
    // auto bigNum34 = bigNum30 >> 72;
    // auto bigNum35 = bigNum30 << 8; 
    // auto bigNum36 = bigNum30 << 40; 
    // auto bigNum37 = bigNum30 << 56; 
    // auto bigNum38 = bigNum30 << 72; 

    // bigNum30 >>= 8;
    // auto ret41 = bigNum30 == bigNum31;
    // bigNum30 >>= 24;
    // auto ret42 = bigNum30 == bigNum33;
    // bigNum30 >>= 40;
    // auto ret43 = bigNum30 == bigNum34;
    // bigNumCopy <<= 8;
    // auto ret44 = bigNumCopy == bigNum35;
    // bigNumCopy <<= 32;
    // auto ret45 = bigNumCopy == bigNum36;
    // bigNumCopy <<= 16;
    // auto ret46 = bigNumCopy == bigNum37;
    // bigNumCopy <<= 16;
    // auto ret47 = bigNumCopy == bigNum38;

}