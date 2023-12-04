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
 * Date: 2022-01-08 06:00:23
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_CPU_CPU_FEATURE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_CPU_CPU_FEATURE_H__

#pragma once

#include <bitset>
#include <array>
#include <vector>
#include <string>
#include <string.h>

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>

KERNEL_BEGIN

class KERNEL_EXPORT ExtRegisterType
{
public:
    enum Type
    {
        EAX = 0,
        EBX = 1,
        ECX = 2,
        EDX = 3,
    };
};

class KERNEL_EXPORT CpuFeature
{
public:
    CpuFeature()
        : _ids{ 0 },
        _exIds{ 0 },
        _isIntel{ false },
        _isAMD{ false },
        _f_1_ECX{ 0 },
        _f_1_EDX{ 0 },
        _f_7_EBX{ 0 },
        _f_7_ECX{ 0 },
        _f_81_ECX{ 0 },
        _f_81_EDX{ 0 },
        _f_87_ECX{ 0 },
        _f_87_EDX{ 0 },
        _data{},
        _extdata{}
    {
    }

    static CpuFeature *GetInstance();

    void Init();

    // cpu厂商
    const std::string &GetVendor() const;
    // cpu商标
    const std::string &GetBrand() const;

    bool SSE3(void);
    bool PCLMULQDQ(void);
    bool MONITOR(void);
    bool SSSE3(void);
    bool FMA(void);
    bool CMPXCHG16B(void);
    bool SSE41(void);
    bool SSE42(void);
    bool MOVBE(void);
    bool POPCNT(void);
    bool AES(void);
    bool XSAVE(void);
    bool OSXSAVE(void);
    bool AVX(void);
    bool F16C(void);
    bool RDRAND(void);

    bool MSR(void);
    bool CX8(void);
    bool SEP(void);
    bool CMOV(void);
    bool CLFSH(void);
    bool MMX(void);
    bool FXSR(void);
    bool SSE(void);
    bool SSE2(void);

    bool FSGSBASE(void);
    bool BMI1(void);
    bool HLE(void);
    bool AVX2(void);
    bool BMI2(void);
    bool ERMS(void);
    bool INVPCID(void);
    bool RTM(void);
    bool AVX512F(void);
    bool RDSEED(void);
    bool ADX(void);
    bool AVX512PF(void);
    bool AVX512ER(void);
    bool AVX512CD(void);
    bool SHA(void);

    bool PREFETCHWT1(void);

    bool LAHF(void);
    bool LZCNT(void);
    bool ABM(void);
    bool SSE4a(void);
    bool XOP(void);
    bool TBM(void);

    bool SYSCALL(void);
    bool MMXEXT(void);
    // 高精度cpu时间支持(rdtscp可以保证前面代码执行完成后再执行rdtscp)
    bool RDTSCP(void);
    bool ConstantTscRate();
    bool _3DNOWEXT(void);
    bool _3DNOW(void);

private:
    void _GetCpuId(int *info, Int32 functionId);
    void _GetCpuIdEx(int *info, Int32 functionId, Int32 subFunctionId);

private:
    Int32 _ids;
    Int32 _exIds;
    std::string _vendor;
    std::string _brand;
    bool _isIntel;
    bool _isAMD;
    
    // 0x00000001
    std::bitset<32> _f_1_ECX;
    std::bitset<32> _f_1_EDX;

    // 0x00000007
    std::bitset<32> _f_7_EBX;
    std::bitset<32> _f_7_ECX;

    // 0x80000001
    std::bitset<32> _f_81_ECX;
    std::bitset<32> _f_81_EDX;

    // 0x80000007
    std::bitset<32> _f_87_ECX;
    std::bitset<32> _f_87_EDX;

    std::vector<std::array<Int32, 4>> _data;
    std::vector<std::array<Int32, 4>> _extdata;
};


ALWAYS_INLINE bool CpuFeature::SSE3(void) 
{ 
    return _f_1_ECX[0]; 
}

ALWAYS_INLINE bool CpuFeature::PCLMULQDQ(void) 
{ 
    return _f_1_ECX[1]; 
}

ALWAYS_INLINE bool CpuFeature::MONITOR(void) 
{ return _f_1_ECX[3]; }

ALWAYS_INLINE bool CpuFeature::SSSE3(void) 
{ return _f_1_ECX[9]; }

ALWAYS_INLINE bool CpuFeature::FMA(void) 
{ return _f_1_ECX[12]; }

ALWAYS_INLINE bool CpuFeature::CMPXCHG16B(void) 
{ return _f_1_ECX[13]; }

ALWAYS_INLINE bool CpuFeature::SSE41(void) 
{ return _f_1_ECX[19]; }

ALWAYS_INLINE bool CpuFeature::SSE42(void) 
{ return _f_1_ECX[20]; }

ALWAYS_INLINE bool CpuFeature::MOVBE(void) 
{ return _f_1_ECX[22]; }

ALWAYS_INLINE bool CpuFeature::POPCNT(void) 
{ return _f_1_ECX[23]; }

ALWAYS_INLINE bool CpuFeature::AES(void) 
{ return _f_1_ECX[25]; }

ALWAYS_INLINE bool CpuFeature::XSAVE(void) 
{ return _f_1_ECX[26]; }

ALWAYS_INLINE bool CpuFeature::OSXSAVE(void) 
{ return _f_1_ECX[27]; }

ALWAYS_INLINE bool CpuFeature::AVX(void) 
{ return _f_1_ECX[28]; }

ALWAYS_INLINE bool CpuFeature::F16C(void) 
{ return _f_1_ECX[29]; }

ALWAYS_INLINE bool CpuFeature::RDRAND(void) 
{ return _f_1_ECX[30]; }

ALWAYS_INLINE bool CpuFeature::MSR(void) 
{ return _f_1_EDX[5]; }

ALWAYS_INLINE bool CpuFeature::CX8(void) 
{ return _f_1_EDX[8]; }

ALWAYS_INLINE bool CpuFeature::SEP(void) 
{ return _f_1_EDX[11]; }

ALWAYS_INLINE bool CpuFeature::CMOV(void) 
{ return _f_1_EDX[15]; }

ALWAYS_INLINE bool CpuFeature::CLFSH(void) 
{ return _f_1_EDX[19]; }

ALWAYS_INLINE bool CpuFeature::MMX(void) 
{ return _f_1_EDX[23]; }

ALWAYS_INLINE bool CpuFeature::FXSR(void) 
{ return _f_1_EDX[24]; }

ALWAYS_INLINE bool CpuFeature::SSE(void) 
{ return _f_1_EDX[25]; }

ALWAYS_INLINE bool CpuFeature::SSE2(void) 
{ return _f_1_EDX[26]; }

ALWAYS_INLINE bool CpuFeature::FSGSBASE(void) 
{ return _f_7_EBX[0]; }

ALWAYS_INLINE bool CpuFeature::BMI1(void) 
{ return _f_7_EBX[3]; }

ALWAYS_INLINE bool CpuFeature::HLE(void) 
{ return _isIntel && _f_7_EBX[4]; }

ALWAYS_INLINE bool CpuFeature::AVX2(void) 
{ return _f_7_EBX[5]; }

ALWAYS_INLINE bool CpuFeature::BMI2(void) 
{ return _f_7_EBX[8]; }

ALWAYS_INLINE bool CpuFeature::ERMS(void) 
{ return _f_7_EBX[9]; }

ALWAYS_INLINE bool CpuFeature::INVPCID(void) 
{ return _f_7_EBX[10]; }

ALWAYS_INLINE bool CpuFeature::RTM(void) 
{ return _isIntel && _f_7_EBX[11]; }

ALWAYS_INLINE bool CpuFeature::AVX512F(void) 
{ return _f_7_EBX[16]; }

ALWAYS_INLINE bool CpuFeature::RDSEED(void) 
{ return _f_7_EBX[18]; }

ALWAYS_INLINE bool CpuFeature::ADX(void) 
{ return _f_7_EBX[19]; }

ALWAYS_INLINE bool CpuFeature::AVX512PF(void) 
{ return _f_7_EBX[26]; }

ALWAYS_INLINE bool CpuFeature::AVX512ER(void) 
{ return _f_7_EBX[27]; }

ALWAYS_INLINE bool CpuFeature::AVX512CD(void) 
{ return _f_7_EBX[28]; }

ALWAYS_INLINE bool CpuFeature::SHA(void) 
{ return _f_7_EBX[29]; }

ALWAYS_INLINE bool CpuFeature::PREFETCHWT1(void) 
{ return _f_7_ECX[0]; }

ALWAYS_INLINE bool CpuFeature::LAHF(void) 
{ return _f_81_ECX[0]; }

ALWAYS_INLINE bool CpuFeature::LZCNT(void) 
{ return _isIntel && _f_81_ECX[5]; }

ALWAYS_INLINE bool CpuFeature::ABM(void) 
{ return _isAMD && _f_81_ECX[5]; }

ALWAYS_INLINE bool CpuFeature::SSE4a(void) 
{ return _isAMD && _f_81_ECX[6]; }

ALWAYS_INLINE bool CpuFeature::XOP(void) 
{ return _isAMD && _f_81_ECX[11]; }

ALWAYS_INLINE bool CpuFeature::TBM(void) 
{ return _isAMD && _f_81_ECX[21]; }

ALWAYS_INLINE bool CpuFeature::SYSCALL(void) 
{ return _isIntel && _f_81_EDX[11]; }

ALWAYS_INLINE bool CpuFeature::MMXEXT(void) 
{ return _isAMD && _f_81_EDX[22]; }

ALWAYS_INLINE bool CpuFeature::RDTSCP(void) 
{ return _isIntel && _f_81_EDX[27]; }

ALWAYS_INLINE bool CpuFeature::ConstantTscRate() 
{ return _f_87_ECX[8]; }

ALWAYS_INLINE const std::string &CpuFeature::GetVendor() const
{
    return _vendor;
}

ALWAYS_INLINE const std::string &CpuFeature::GetBrand() const
{
    return _brand;
}

ALWAYS_INLINE bool CpuFeature::_3DNOWEXT(void) 
{ return _isAMD && _f_81_EDX[30]; }

ALWAYS_INLINE bool CpuFeature::_3DNOW(void) 
{ return _isAMD && _f_81_EDX[31]; }

KERNEL_END

#endif
