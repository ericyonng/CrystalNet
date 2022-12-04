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

#include <kernel/kernel_inc.h>
#include <kernel/comp/SmartPtr.h>

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

    static CpuFeature *GetInstance()
    {
        SmartPtr<CpuFeature> s_inst = new CpuFeature; 
        return s_inst.AsSelf();
    }

    void Init()
    {
        //int cpuInfo[4] = {-1};
        std::array<Int32, 4> cpui{ 0 };

        // Calling __cpuid with 0x0 as the function_id argument
        // gets the number of the highest valid function ID.
        _GetCpuId(reinterpret_cast<int *>(cpui.data()), 0);
        _ids = cpui[0];

        for (int i = 0; i <= _ids; ++i)
        {
            _GetCpuIdEx(reinterpret_cast<int *>(cpui.data()), i, 0);
            _data.push_back(cpui);
        }

        // Capture vendor string 获取供应商
        char vendor[0x20];
        ::memset(vendor, 0, sizeof(vendor));
        Int32 *vendorAddr = reinterpret_cast<int *>(vendor);
        Int32 *vendorAddrShift4 = reinterpret_cast<int *>(vendor + 4);
        Int32 *vendorAddrShift8 = reinterpret_cast<int *>(vendor + 8);
        *vendorAddr = _data[0][1];
        *vendorAddrShift4= _data[0][3];
        *vendorAddrShift8 = _data[0][2];
        _vendor = vendor;
        if (_vendor == "GenuineIntel")
        {
            _isIntel = true;
        }
        else if (_vendor == "AuthenticAMD")
        {
            _isAMD = true;
        }

        // load bitset with flags for function 0x00000001
        if (_ids >= 1)
        {
            _f_1_ECX = _data[1][2];
            _f_1_EDX = _data[1][3];
        }

        // load bitset with flags for function 0x00000007
        if (_ids >= 7)
        {
            _f_7_EBX = _data[7][1];
            _f_7_ECX = _data[7][2];
        }

        // Calling __cpuid with 0x80000000 as the function_id argument
        // gets the number of the highest valid extended ID.
        _GetCpuId(reinterpret_cast<int *>(cpui.data()), 0x80000000);
        // 最大的扩展id（子功能号）通过主功能号获取cpuinfo 然后cpui[0]就是最大支持的子功能号
        _exIds = cpui[0];

        char brand[0x40];
        ::memset(brand, 0, sizeof(brand));

        // 获取子功能号信息
        for (int i = 0x80000000; i <= _exIds; ++i)
        {
            _GetCpuIdEx(reinterpret_cast<int *>(cpui.data()), i, 0);
            _extdata.push_back(cpui);
        }

        // load bitset with flags for function 0x80000001
        if (_exIds >= Int32(0x80000001))
        {// extdata[] = {eax, ebx, ecx,edx}
            _f_81_ECX = _extdata[1][2];
            _f_81_EDX = _extdata[1][3];
        }

        // Interpret CPU brand string if reported
        if (_exIds >= Int32(0x80000004))
        {
            ::memcpy(brand, _extdata[2].data(), sizeof(cpui));
            ::memcpy(brand + 16, _extdata[3].data(), sizeof(cpui));
            ::memcpy(brand + 32, _extdata[4].data(), sizeof(cpui));
            _brand = brand;
        }

        // 0x80 00 00 07
        if(_exIds >= Int32(0x80000007))
        {
            // 0x80000007的功能标志位
            _f_87_ECX = _extdata[7][ExtRegisterType::ECX];
            _f_87_EDX = _extdata[7][ExtRegisterType::EDX];
        }
    }

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
    void _GetCpuId(int *info, Int32 functionId)
    {
        #if CRYSTAL_TARGET_PLATFORM_LINUX
            __cpuid(functionId, info[0], info[1], info[2], info[3]);
        #endif

        #if CRYSTAL_TARGET_PLATFORM_WINDOWS
            __cpuid(info, functionId);
        #endif
    }

    void _GetCpuIdEx(int *info, Int32 functionId, Int32 subFunctionId)
    {
        #if CRYSTAL_TARGET_PLATFORM_LINUX
            __cpuid_count(functionId, subFunctionId, info[0], info[1], info[2], info[3]);
        #endif

        #if CRYSTAL_TARGET_PLATFORM_WINDOWS
            __cpuidex(info, functionId, subFunctionId);
        #endif
    }

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


inline bool CpuFeature::SSE3(void) 
{ 
    return _f_1_ECX[0]; 
}

inline bool CpuFeature::PCLMULQDQ(void) 
{ 
    return _f_1_ECX[1]; 
}

inline bool CpuFeature::MONITOR(void) 
{ return _f_1_ECX[3]; }

inline bool CpuFeature::SSSE3(void) 
{ return _f_1_ECX[9]; }

inline bool CpuFeature::FMA(void) 
{ return _f_1_ECX[12]; }

inline bool CpuFeature::CMPXCHG16B(void) 
{ return _f_1_ECX[13]; }

inline bool CpuFeature::SSE41(void) 
{ return _f_1_ECX[19]; }

inline bool CpuFeature::SSE42(void) 
{ return _f_1_ECX[20]; }

inline bool CpuFeature::MOVBE(void) 
{ return _f_1_ECX[22]; }

inline bool CpuFeature::POPCNT(void) 
{ return _f_1_ECX[23]; }

inline bool CpuFeature::AES(void) 
{ return _f_1_ECX[25]; }

inline bool CpuFeature::XSAVE(void) 
{ return _f_1_ECX[26]; }

inline bool CpuFeature::OSXSAVE(void) 
{ return _f_1_ECX[27]; }

inline bool CpuFeature::AVX(void) 
{ return _f_1_ECX[28]; }

inline bool CpuFeature::F16C(void) 
{ return _f_1_ECX[29]; }

inline bool CpuFeature::RDRAND(void) 
{ return _f_1_ECX[30]; }

inline bool CpuFeature::MSR(void) 
{ return _f_1_EDX[5]; }

inline bool CpuFeature::CX8(void) 
{ return _f_1_EDX[8]; }

inline bool CpuFeature::SEP(void) 
{ return _f_1_EDX[11]; }

inline bool CpuFeature::CMOV(void) 
{ return _f_1_EDX[15]; }

inline bool CpuFeature::CLFSH(void) 
{ return _f_1_EDX[19]; }

inline bool CpuFeature::MMX(void) 
{ return _f_1_EDX[23]; }

inline bool CpuFeature::FXSR(void) 
{ return _f_1_EDX[24]; }

inline bool CpuFeature::SSE(void) 
{ return _f_1_EDX[25]; }

inline bool CpuFeature::SSE2(void) 
{ return _f_1_EDX[26]; }

inline bool CpuFeature::FSGSBASE(void) 
{ return _f_7_EBX[0]; }

inline bool CpuFeature::BMI1(void) 
{ return _f_7_EBX[3]; }

inline bool CpuFeature::HLE(void) 
{ return _isIntel && _f_7_EBX[4]; }

inline bool CpuFeature::AVX2(void) 
{ return _f_7_EBX[5]; }

inline bool CpuFeature::BMI2(void) 
{ return _f_7_EBX[8]; }

inline bool CpuFeature::ERMS(void) 
{ return _f_7_EBX[9]; }

inline bool CpuFeature::INVPCID(void) 
{ return _f_7_EBX[10]; }

inline bool CpuFeature::RTM(void) 
{ return _isIntel && _f_7_EBX[11]; }

inline bool CpuFeature::AVX512F(void) 
{ return _f_7_EBX[16]; }

inline bool CpuFeature::RDSEED(void) 
{ return _f_7_EBX[18]; }

inline bool CpuFeature::ADX(void) 
{ return _f_7_EBX[19]; }

inline bool CpuFeature::AVX512PF(void) 
{ return _f_7_EBX[26]; }

inline bool CpuFeature::AVX512ER(void) 
{ return _f_7_EBX[27]; }

inline bool CpuFeature::AVX512CD(void) 
{ return _f_7_EBX[28]; }

inline bool CpuFeature::SHA(void) 
{ return _f_7_EBX[29]; }

inline bool CpuFeature::PREFETCHWT1(void) 
{ return _f_7_ECX[0]; }

inline bool CpuFeature::LAHF(void) 
{ return _f_81_ECX[0]; }

inline bool CpuFeature::LZCNT(void) 
{ return _isIntel && _f_81_ECX[5]; }

inline bool CpuFeature::ABM(void) 
{ return _isAMD && _f_81_ECX[5]; }

inline bool CpuFeature::SSE4a(void) 
{ return _isAMD && _f_81_ECX[6]; }

inline bool CpuFeature::XOP(void) 
{ return _isAMD && _f_81_ECX[11]; }

inline bool CpuFeature::TBM(void) 
{ return _isAMD && _f_81_ECX[21]; }

inline bool CpuFeature::SYSCALL(void) 
{ return _isIntel && _f_81_EDX[11]; }

inline bool CpuFeature::MMXEXT(void) 
{ return _isAMD && _f_81_EDX[22]; }

inline bool CpuFeature::RDTSCP(void) 
{ return _isIntel && _f_81_EDX[27]; }

inline bool CpuFeature::ConstantTscRate() 
{ return _f_87_ECX[8]; }

inline const std::string &CpuFeature::GetVendor() const
{
    return _vendor;
}

inline const std::string &CpuFeature::GetBrand() const
{
    return _brand;
}

inline bool CpuFeature::_3DNOWEXT(void) 
{ return _isAMD && _f_81_EDX[30]; }

inline bool CpuFeature::_3DNOW(void) 
{ return _isAMD && _f_81_EDX[31]; }

KERNEL_END

#endif
