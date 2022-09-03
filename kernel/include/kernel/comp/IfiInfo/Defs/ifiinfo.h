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
 * Date: 2021-10-17 21:44:44
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_IFI_INFO_DEFS_IFI_INFO_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_IFI_INFO_DEFS_IFI_INFO_H__

#pragma once

#if CRYSTAL_TARGET_PLATFORM_LINUX

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/memory.h>

#undef IFI_NAME
#define	IFI_NAME	16			/* same as IFNAMSIZ in <net/if.h> */
#undef IFI_HADDR
#define	IFI_HADDR	 8			/* allow for 64-bit EUI-64 in future */
#undef IFI_ALIAS
#define	IFI_ALIAS	1			/* ifi_addr is an alias */

KERNEL_BEGIN

struct KERNEL_EXPORT IfiInfo 
{
    POOL_CREATE_OBJ_DEFAULT(IfiInfo);

  IfiInfo();

  // 网卡是否工作状态
  bool IsUp() const;
  // 有没有有效的广播地址
  bool HasValidBroadcastAddress() const;
  // 是否支持多播
  bool SupportMulticast() const;
  // 是否回环地址
  bool IsLoopbackAddress() const;
  // 是否点对点连接
  bool IsP2PLink() const;

  Byte8    ifi_name[IFI_NAME];	/* interface name, null-terminated 适配器名 */
  Int16   ifi_index;			/* interface index 适配器标识 */
  Int16   ifi_mtu;				/* interface MTU */
  U8      ifi_haddr[IFI_HADDR];	/* hardware address MAC地址*/
  UInt16 ifi_hlen;				/* # bytes in hardware address: 0, 6, 8 */
  Int16   ifi_flags;			/* IFF_xxx constants from <net/if.h> */
  Int16   ifi_myflags;			/* our own IFI_xxx flags */
  struct sockaddr  *ifi_addr;	/* primary address */
  struct sockaddr  *ifi_brdaddr;/* broadcast address */
  struct sockaddr  *ifi_dstaddr;/* destination address */
  struct IfiInfo  *ifi_next;	/* next of these structures */
};

// 网卡是否工作状态
inline bool IfiInfo::IsUp() const
{
  return ifi_flags & IFF_UP;
}

// 有没有有效的广播地址
inline bool IfiInfo::HasValidBroadcastAddress() const
{
  return ifi_flags & IFF_BROADCAST;
}

// 是否支持多播
inline bool IfiInfo::SupportMulticast() const
{
  return ifi_flags & IFF_MULTICAST;
}

// 是否回环地址
inline bool IfiInfo::IsLoopbackAddress() const
{
  return ifi_flags & IFF_LOOPBACK;
}

// 是否点对点连接
inline bool IfiInfo::IsP2PLink() const
{
    return ifi_flags & IFF_POINTOPOINT;
}

KERNEL_END

#endif

#endif
