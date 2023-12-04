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
 * Date: 2022-01-30 22:58:29
 * Author: Eric Yonng
 * Description: 
*/
#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_CPU_CPU_DEFS_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_CPU_CPU_DEFS_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>

KERNEL_BEGIN

class KERNEL_EXPORT CpuVendor
{
public:
    enum Type
    {
        NONE = 0,           // 无牌或不知晓
        INTEL = 1,          // INTEL CPU
        AMD = 2,            // AMD CPU
    };

    static const Byte8 *ToString(Int32 vendorType)
    {
        switch (vendorType)
        {
        case CpuVendor::INTEL: return "INTEL";
        case CpuVendor::AMD: return "AMD";
        default:
            break;
        }

        return "Unknown";
    }
};

KERNEL_END

#endif
