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
 * Date: 2024-08-24 22:00:54
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_DEFS_ADDR_IP_CONFIG_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_DEFS_ADDR_IP_CONFIG_H__

#pragma once

#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

struct KERNEL_EXPORT AddrIpConfig
{
    POOL_CREATE_OBJ_DEFAULT(AddrIpConfig);

    // ip或者域名(因为域名对应的ip可能不同时间不一样, 此时必须实时的解析ip,不能预解析)
    KERNEL_NS::LibString _ip;
    
    // 是否域名
    bool _isHostName = false;

    // 如果是域名是否转成ipv4
    bool _toIpv4 = false;

    KERNEL_NS::LibString ToString() const;

    Int32 GetAf() const;
};

KERNEL_END

#endif
