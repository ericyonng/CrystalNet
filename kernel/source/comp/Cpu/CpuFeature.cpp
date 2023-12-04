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
 * Date: 2022-01-08 06:01:54
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Cpu/CpuFeature.h>
#include <kernel/comp/SmartPtr.h>

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    #include <intrin.h> // 获取cpuid
#endif

#if CRYSTAL_TARGET_PLATFORM_LINUX
    #include <cpuid.h>  // 获取cpuid
#endif

KERNEL_BEGIN

CpuFeature *CpuFeature::GetInstance()
{
    static SmartPtr<CpuFeature> s_inst = new CpuFeature; 
    return s_inst.AsSelf();
}

void CpuFeature::Init()
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

void CpuFeature::_GetCpuId(int *info, Int32 functionId)
{
    #if CRYSTAL_TARGET_PLATFORM_LINUX
        __cpuid(functionId, info[0], info[1], info[2], info[3]);
    #endif

    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
        __cpuid(info, functionId);
    #endif
}

void CpuFeature::_GetCpuIdEx(int *info, Int32 functionId, Int32 subFunctionId)
{
    #if CRYSTAL_TARGET_PLATFORM_LINUX
        __cpuid_count(functionId, subFunctionId, info[0], info[1], info[2], info[3]);
    #endif

    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
        __cpuidex(info, functionId, subFunctionId);
    #endif
}

KERNEL_END
