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
 * Date: 2022-04-19 21:57:06
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_DEFS_POLLER_CONFIG_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_DEFS_POLLER_CONFIG_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/memory.h>
#include <kernel/comp/NetEngine/Poller/Defs/TcpPollerConfig.h>
#include <kernel/comp/NetEngine/Poller/Defs/UdpPollerConfig.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

struct KERNEL_EXPORT PollerConfig
{
    POOL_CREATE_OBJ_DEFAULT(PollerConfig);

    PollerConfig();

    LibString ToString() const;

    void Copy(const PollerConfig &cfg);
    
    UInt32 _blackWhiteListFlag;         // 黑白名单模式
    UInt64 _maxSessionQuantity;
    TcpPollerConfig _tcpPollerConfig;   // tcp poller配置

    std::unordered_map<KERNEL_NS::LibString, Int32> _pollerFeatureStringRefId;  // poller feature id定义
    std::unordered_map<Int32, std::set<KERNEL_NS::LibString>> _pollerFeatureIdRefString;  // poller feature id定义
    
    // UdpPollerConfig _udpPollerConfig;   // upd poller 配置
};

KERNEL_END

#endif
