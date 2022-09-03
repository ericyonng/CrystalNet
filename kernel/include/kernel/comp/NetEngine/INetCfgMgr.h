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
 * Date: 2021-04-18 21:30:25
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_INET_CFG_MGR_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_INET_CFG_MGR_H__

#pragma once

#include <kernel/kernel_inc.h>

KERNEL_BEGIN

class LibIniFile;

class KERNEL_EXPORT INetCfgMgr
{
public:
    INetCfgMgr();
    virtual ~INetCfgMgr();

public:
    Int32 Init(const Byte8 *iniFile);
    void Clear();

public:
    Int64 GetConnectorTimeoutMsPerTimes() const;
    Int64 GetReconnectTimes() const;
    UInt32 GetConnectorBlackWhiteFlagMode() const;

protected:
    LibIniFile *_ini;

    // // connector配置
    Int64 _connectTimoutMsPerTimes;     // 连接器每次超时重连的时间间隔
    Int64 _reconnectTimes;              // 超时重连次数
    UInt32 _connectBlackWhiteFlagMode;  // 连接器黑白名单模式
    
};

inline Int64 INetCfgMgr::GetConnectorTimeoutMsPerTimes() const
{
    return _connectTimoutMsPerTimes;
}

inline Int64 INetCfgMgr::GetReconnectTimes() const
{
    return _reconnectTimes;
}

inline UInt32 INetCfgMgr::GetConnectorBlackWhiteFlagMode() const
{
    return _connectBlackWhiteFlagMode;
}

KERNEL_END

#endif
