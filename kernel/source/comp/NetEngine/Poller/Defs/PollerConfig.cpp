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
 * Date: 2022-04-19 22:23:44
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/NetEngine/Poller/Defs/PollerConfig.h>

KERNEL_BEGIN

PollerConfig::PollerConfig()
:_blackWhiteListFlag(0)
,_maxSessionQuantity(0)
{

}

LibString PollerConfig::ToString() const
{
    LibString info;
    info.AppendFormat("_blackWhiteListFlag:%u, 0x%x\n", _blackWhiteListFlag, _blackWhiteListFlag)
        .AppendFormat("_maxSessionQuantity:%llu\n", _maxSessionQuantity)
        .AppendFormat("tcp poller config:%s", _tcpPollerConfig.ToString().c_str())
        ;

    return info;
}

void PollerConfig::Copy(const PollerConfig &cfg)
{
    _blackWhiteListFlag = cfg._blackWhiteListFlag;
    _maxSessionQuantity = cfg._maxSessionQuantity;
    _tcpPollerConfig.Copy(cfg._tcpPollerConfig);
}

KERNEL_END