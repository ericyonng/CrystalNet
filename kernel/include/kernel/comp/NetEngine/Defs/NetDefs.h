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
 * Author: Eric Yonng
 * Date: 2021-03-30 15:39:53
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_DEFS_NET_DEFS_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_DEFS_NET_DEFS_H__

#pragma once

#include <kernel/kernel_inc.h>

#undef __LIB_POLLER_MONITOR_TIME_OUT_INTERVAL__
#define __LIB_POLLER_MONITOR_TIME_OUT_INTERVAL__    20

#define CRYSTAL_IPV6_ZERO_ADDR "0:0:0:0:0:0:0:0"
#define CRYSTAL_IPV4_ZERO_ADDR "0.0.0.0"

KERNEL_BEGIN

class KERNEL_EXPORT FamilyType
{
public:
    static const Byte8 *ToString(UInt16 family)
    {
        switch (family)
        {
        case AF_INET: return "AF_INET IPV4";
        case AF_INET6: return "AF_INET6 IPV6";
        default:
            break;
        }

        return "Unknown";
    } 
};

KERNEL_END

#endif

