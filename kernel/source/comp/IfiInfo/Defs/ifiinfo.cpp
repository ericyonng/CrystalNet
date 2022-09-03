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
 * Date: 2021-10-17 21:51:06
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/IfiInfo/Defs/ifiinfo.h>

#if CRYSTAL_TARGET_PLATFORM_LINUX

KERNEL_BEGIN

IfiInfo::IfiInfo()
:ifi_name{0}
,ifi_index(0)
,ifi_mtu(0)
,ifi_haddr{0}
,ifi_hlen(0)
,ifi_flags(0)
,ifi_myflags(0)
,ifi_addr(NULL)
,ifi_brdaddr(NULL)
,ifi_dstaddr(NULL)
,ifi_next(NULL)
{
}

KERNEL_END

#endif
