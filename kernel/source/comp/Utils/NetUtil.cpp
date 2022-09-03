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
 * Date: 2021-10-16 22:24:38
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Utils/NetUtil.h>
#include <kernel/comp/Utils/Defs/NetCardInfo.h>
#include <kernel/comp/memory/memory.h>
#include <kernel/comp/Utils/TlsUtil.h>
#include <kernel/comp/IfiInfo/ifi.h>
#include <kernel/comp/SmartPtr.h>

// #undef WORKING_BUFFER_SIZE
// #define WORKING_BUFFER_SIZE 15000

// #if CRYSTAL_TARGET_PLATFORM_WINDOWS
// static inline void GetIpBy(LPSOCKADDR &sockAddr, UInt64 ipTypeMask, KERNEL_NS::LibString &outStr)
// {
//     BUFFER256 ip = {0};
//     UInt64 ipSize = sizeof(ip);

//     if((AF_INET == sockAddr->sa_family) && 
//         KERNEL_NS::BitUtil::IsSet(ipTypeMask, KERNEL_NS::IpTypeMask::IPV4))
//     {
//         sockaddr_in *sockAddrPtr = reinterpret_cast<sockaddr_in *>(sockAddr);
//         inet_ntop(PF_INET, &(sockAddrPtr->sin_addr), ip, ipSize);
//         outStr = ip;
//     }
//     else if((AF_INET6 == sockAddr->sa_family) && 
//             KERNEL_NS::BitUtil::IsSet(ipTypeMask, KERNEL_NS::IpTypeMask::IPV6))
//     {
//         sockaddr_in6 *sockAddrPtr = reinterpret_cast<sockaddr_in6 *>(sockAddr);
//         inet_ntop(PF_INET6, &(sockAddrPtr->sin6_addr), ip, ipSize);
//         outStr = ip;
//     }
// }

// static inline void FillNetCardInfo(PIP_ADAPTER_ADDRESSES curAddresses, KERNEL_NS::NetCardInfo &netCardInfo)
// {
//     // ADAPTER
//     if(KERNEL_NS::BitUtil::IsSet(netCardInfo._infoMask, KERNEL_NS::NetCardInfoMask::ADAPTER_NAME))
//         netCardInfo._adapterName = curAddresses->AdapterName;

//     // MAC
//     if(LIKELY(KERNEL_NS::BitUtil::IsSet(netCardInfo._infoMask, KERNEL_NS::NetCardInfoMask::MAC)))
//     {
//         const Int32 len = static_cast<Int32>(curAddresses->PhysicalAddressLength);
//         if(LIKELY(len))
//         {
//             auto &macAddr = netCardInfo._mac;
//             for(Int32 idx = 0; idx < len; ++idx)
//             {
//                 macAddr.AppendFormat("%02X", curAddresses->PhysicalAddress[idx]);
//                 if(idx != (len - 1))
//                     macAddr.AppendFormat("-");
//             }
//         }
//     }

//     // mtu
//     if(KERNEL_NS::BitUtil::IsSet(netCardInfo._infoMask, KERNEL_NS::NetCardInfoMask::MTU))
//         netCardInfo._mtu = static_cast<UInt64>(curAddresses->Mtu);

//     // IP
//     if(LIKELY(KERNEL_NS::BitUtil::IsSet(netCardInfo._infoMask, KERNEL_NS::NetCardInfoMask::IP)))
//     {
//         PIP_ADAPTER_UNICAST_ADDRESS unicast = curAddresses->FirstUnicastAddress;
//         while(unicast)
//         {
//             KERNEL_NS::LibString outStr;
//             auto &sockAddr = unicast->Address.lpSockaddr;

//             GetIpBy(sockAddr, netCardInfo._ipTypeMask, outStr);
//             if(sockAddr->sa_family == AF_INET && (!outStr.empty()))
//                 netCardInfo._ipv4.push_back(outStr);
//             else if(sockAddr->sa_family == AF_INET6 &&(!outStr.empty()))
//                 netCardInfo._ipv6.push_back(outStr);

//             unicast = unicast->Next;
//         }
//     }

//     // gateway
//     if(KERNEL_NS::BitUtil::IsSet(netCardInfo._infoMask, KERNEL_NS::NetCardInfoMask::GATWAY))
//     {
//         auto gateway = curAddresses->FirstGatewayAddress;
//         while(gateway)
//         {
//             auto &sockAddr = gateway->Address.lpSockaddr;
//             if(sockAddr)
//             {
//                 KERNEL_NS::LibString outStr;
//                 GetIpBy(sockAddr, netCardInfo._ipTypeMask, outStr);
//                 if(sockAddr->sa_family == AF_INET && (!outStr.empty()))
//                     netCardInfo._gatewayIpv4.push_back(outStr);
//                 else if(sockAddr->sa_family == AF_INET6 &&(!outStr.empty()))
//                     netCardInfo._gatewayIpv6.push_back(outStr);
//             }
//             gateway = gateway->Next;
//         }
//     }

//     // dhcp
//     if(KERNEL_NS::BitUtil::IsSet(netCardInfo._infoMask, KERNEL_NS::NetCardInfoMask::DHCP))
//     {
//         auto &sockAddr = curAddresses->Dhcpv4Server.lpSockaddr;
//         if(sockAddr)
//             GetIpBy(sockAddr, netCardInfo._ipTypeMask, netCardInfo._dhcpIpv4);
//     }

//     // dns
//     if(KERNEL_NS::BitUtil::IsSet(netCardInfo._infoMask, KERNEL_NS::NetCardInfoMask::DNS))
//     {
//         IP_ADAPTER_DNS_SERVER_ADDRESS *dnsServer = curAddresses->FirstDnsServerAddress;
//         while(dnsServer)
//         {
//             auto &sockAddr = dnsServer->Address.lpSockaddr;
//             if(sockAddr)
//             {
//                 KERNEL_NS::LibString outStr;
//                 GetIpBy(sockAddr, netCardInfo._ipTypeMask, outStr);
//                 if(sockAddr->sa_family == AF_INET && (!outStr.empty()))
//                     netCardInfo._dnsIpv4.push_back(outStr);
//                 else if(sockAddr->sa_family == AF_INET6 &&(!outStr.empty()))
//                     netCardInfo._dnsIpv6.push_back(outStr);
//             }

//             dnsServer = dnsServer->Next;
//         }
//     }

//     // send
//     if(KERNEL_NS::BitUtil::IsSet(netCardInfo._infoMask, KERNEL_NS::NetCardInfoMask::SEND_SPEED))
//         netCardInfo._sendSpead = static_cast<UInt64>(curAddresses->TransmitLinkSpeed);
//     // recieve
//     if(KERNEL_NS::BitUtil::IsSet(netCardInfo._infoMask, KERNEL_NS::NetCardInfoMask::RECV_SPEED))
//         netCardInfo._recvSpead = static_cast<UInt64>(curAddresses->ReceiveLinkSpeed);
// }

// #endif

// static inline bool IsFamilyEnable(Int32 family, UInt64 ipTypeMask)
// {
//     if(family == AF_INET && KERNEL_NS::BitUtil::IsSet(ipTypeMask, KERNEL_NS::IpTypeMask::IPV4))
//         return true;
//     if(family == AF_INET6 && KERNEL_NS::BitUtil::IsSet(ipTypeMask, KERNEL_NS::IpTypeMask::IPV6))
//         return true;

//     return false;
// }

// #if CRYSTAL_TARGET_PLATFORM_LINUX
// KERNEL_NS::LibString sock_ntop_host(const struct sockaddr *sa)
// {
//     Byte8 str[128];		/* Unix domain is largest */

// 	switch (sa->sa_family) {
// 	case AF_INET: {
// 		struct sockaddr_in	*sin = (struct sockaddr_in *) sa;
// 		if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
// 			return"";
// 		return(str);
// 	}
// 	case AF_INET6: {
// 		struct sockaddr_in6	*sin6 = (struct sockaddr_in6 *) sa;
// 		if (inet_ntop(AF_INET6, &sin6->sin6_addr, str, sizeof(str)) == NULL)
// 			return "";
// 		return(str);
// 	}

// #ifdef	AF_UNIX
// 	case AF_UNIX: {
// 		struct sockaddr_un	*unp = (struct sockaddr_un *) sa;

// 			/* OK to have no pathname bound to the socket: happens on
// 			   every connect() unless client calls bind() first. */
// 		if (unp->sun_path[0] == 0)
// 			::strcpy(str, "(no pathname bound)");
// 		else
// 			::snprintf(str, sizeof(str), "%s", unp->sun_path);
// 		return(str);
// 	}
// #endif

// #ifdef	HAVE_SOCKADDR_DL_STRUCT
// 	case AF_LINK: {
// 		struct sockaddr_dl	*sdl = (struct sockaddr_dl *) sa;
// 		if (sdl->sdl_nlen > 0)
// 			::snprintf(str, sizeof(str), "%*s",
// 					 sdl->sdl_nlen, &sdl->sdl_data[0]);
// 		else
// 			::snprintf(str, sizeof(str), "AF_LINK, index=%d", sdl->sdl_index);
// 		return(str);
// 	}
// #endif
// 	default:
// 		::snprintf(str, sizeof(str), "sock_ntop_host: unknown AF_xxx: %d",
// 				 sa->sa_family);
// 		return(str);
// 	}

//     return "";
// }

// static inline KERNEL_NS::LibString Sock_ntop_host(const SA *sa)
// {
//     KERNEL_NS::LibString str = sock_ntop_host(sa);
//     if(str.empty())
//         g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::NetUtil, "sock_ntop_host error"));	/* inet_ntop() sets errno */
// 	return (str);
// }

// #endif


// KERNEL_BEGIN

// Int32 NetUtil::GetNetCardInfo(Int32 cardTypeMask, UInt64 ipTypeMask, std::vector<NetCardInfo *> &netCardList, IDelegate<NetCardInfo *> *makeCardInfoFunc)
// {
//     // check cardTypeMask
//     if(UNLIKELY(!NetCardType::CheckMask(cardTypeMask)))
//     {
//         CRYSTAL_TRACE("unknown card type mask:%x", cardTypeMask);
//         return Status::Failed;
//     }

// #if CRYSTAL_TARGET_PLATFORM_WINDOWS
//     // 控制信息
//     ULong flags = GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_INCLUDE_GATEWAYS;
//     // ipv4/ipv6
//     ULong family = 0;
//     bool isSetIpv4 = BitUtil::IsSet(ipTypeMask, IpTypeMask::IPV4);
//     bool isSetIpv6 = BitUtil::IsSet(ipTypeMask, IpTypeMask::IPV6);
//     if(LIKELY(isSetIpv4 && isSetIpv6))
//         family = AF_UNSPEC;
//     else if(isSetIpv4)
//         family = AF_INET;
//     else if(isSetIpv6)
//         family = AF_INET6;
//     else
//     {
//         CRYSTAL_TRACE("ERROR:unknown ip type mask:%llu", ipTypeMask);
//         return Status::Failed;
//     }

//     // 变量
//     auto memoryPool = KernelGetTlsMemoryPool();
//     ULong ret = 0; 
//     PIP_ADAPTER_ADDRESSES addresses = NULL;
//     ULong outBufLen = WORKING_BUFFER_SIZE;

//     // 获取网卡信息
//     do 
//     {
//         addresses = memoryPool->AllocThreadLocal<IP_ADAPTER_ADDRESSES>(static_cast<UInt64>(outBufLen));
//         ret = ::GetAdaptersAddresses(family, flags, NULL, addresses, &outBufLen);
//         if(ret == ERROR_BUFFER_OVERFLOW)
//         {
//             CRYSTAL_TRACE("WARN: GetAdaptersAddresses buffer overflow now outBufLen=%lu", outBufLen);
//             memoryPool->FreeThreadLocal(addresses);
//             addresses = NULL;

//         }
//         else
//             break;

//     }while(ret == ERROR_BUFFER_OVERFLOW);

//     // 有错误
//     if(ret != NO_ERROR)
//     {
//         CRYSTAL_TRACE("GetAdaptersAddresses fail ret = %lu", ret);
//         return Status::Failed;
//     }
//     CRYSTAL_TRACE("suc GetAdaptersAddresses");

//     // 获取具体的网卡信息
//     PIP_ADAPTER_ADDRESSES curAddresses = addresses;
//     while(curAddresses)
//     {
//         // 网卡类型
//         Int32 cardType = NetCardType::AdapterTypeToNetCardType(curAddresses->IfType);
//         if(cardType == NetCardType::NONE)
//         {
//             CRYSTAL_TRACE("AdapterTypeToNetCardType unknown cardtype");
//             continue;
//         }

//         // 控制信息
//         auto newCardInfo = makeCardInfoFunc->Invoke();
//         newCardInfo->_cardType = cardType;
//         newCardInfo->_ipTypeMask = ipTypeMask;
//         FillNetCardInfo(curAddresses, *newCardInfo);
//         netCardList.push_back(newCardInfo);
//     };

//     memoryPool->FreeThreadLocal(addresses);
//     addresses = NULL;
//     return Status::Success;

// #endif

// #if CRYSTAL_TARGET_PLATFORM_LINUX
//     // 创建/销毁ifi对象
//     auto __makeIfiInfoFunc = []() -> IfiInfo *
//     {
//         return IfiInfo::NewThreadLocal_IfiInfo();
//     };
//     auto makeDelg = DelegateFactory::Create<decltype(__makeIfiInfoFunc), IfiInfo *>(__makeIfiInfoFunc);
//     auto __freeIfiInfoFunc = [](IfiInfo *ifi) -> void
//     {
//         IfiInfo::DeleteThreadLocal_IfiInfo(ifi);
//     };
//     auto freeDelg = DelegateFactory::Create<decltype(__freeIfiInfoFunc), void, IfiInfo *>(__freeIfiInfoFunc);
//     auto memoryPool = KernelGetTlsMemoryPool();
    
//     IfiInfo *head = NULL;
//     Int32 err = NetUtil::GetIfiInfo(ipTypeMask, 1, makeDelg, memoryPool, head);
//     if(err != Status::Success)
//     {
//         if(head)
//             NetUtil::FreeIfiInfo(head, freeDelg, memoryPool);

//         freeDelg->Release();
//         freeDelg = NULL;
//         makeDelg->Release();
//         makeDelg = NULL;
//         g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::NetUtil, "GetIfiInfo fail err:%d"), err);
//         return err;
//     }

//     CRYSTAL_TRACE("suc GetIfiInfo start get net card info");

//     for(auto ifi = head; ifi != NULL; ifi = ifi->ifi_next)
//     {
//         auto newCardInfo = makeCardInfoFunc->Invoke();
//         newCardInfo->_ipTypeMask = ipTypeMask;

//         newCardInfo->_adapterName = ifi->ifi_name;
//         newCardInfo->_infoMask = BitUtil::Set(newCardInfo->_infoMask, NetCardInfoMask::ADAPTER_NAME);

//         const Int32 len = static_cast<Int32>(ifi->ifi_hlen);
//         if(LIKELY(len))
//         {
//             auto &macAddr = newCardInfo->_mac;
//             for(Int32 idx = 0; idx < len; ++idx)
//             {
//                 macAddr.AppendFormat("%02X", ifi->ifi_haddr[idx]);
//                 if(idx != (len - 1))
//                     macAddr.AppendFormat("-");
//             }
//             newCardInfo->_infoMask = BitUtil::Set(newCardInfo->_infoMask, NetCardInfoMask::MAC);
//         }

//         if(ifi->ifi_mtu)
//         {
//             newCardInfo->_mtu = ifi->ifi_mtu;
//             newCardInfo->_infoMask = BitUtil::Set(newCardInfo->_infoMask, NetCardInfoMask::MTU);
//         }

//         // ip
//         if(ifi->ifi_addr)
//         {
//             if(ifi->ifi_addr->sa_family == AF_INET)
//             {
//                 newCardInfo->_ipv4.push_back(Sock_ntop_host(ifi->ifi_addr));
//             }
//             else if(ifi->ifi_addr->sa_family == AF_INET6)
//             {
//                 newCardInfo->_ipv6.push_back(Sock_ntop_host(ifi->ifi_addr));
//             }
//             newCardInfo->_infoMask = BitUtil::Set(newCardInfo->_infoMask, NetCardInfoMask::IP);
//         }

//         netCardList.push_back(newCardInfo);
//     }
    
//     NetUtil::FreeIfiInfo(head, freeDelg, memoryPool);
//     freeDelg->Release();
//     freeDelg = NULL;
//     makeDelg->Release();
//     makeDelg = NULL;

//     CRYSTAL_TRACE("suc get net card info!");

//     return Status::Success;
// #endif
// }

// Int32 NetUtil::GetNetCardInfo(Int32 cardTypeMask, UInt64 ipTypeMask, std::vector<NetCardInfo> &netCardList)
// {
//    // check cardTypeMask
//     if(UNLIKELY(!NetCardType::CheckMask(cardTypeMask)))
//     {
//         CRYSTAL_TRACE("unknown card type mask:%x", cardTypeMask);
//         return Status::Failed;
//     }

// #if CRYSTAL_TARGET_PLATFORM_WINDOWS
//     // 控制信息
//     ULong flags = GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_INCLUDE_GATEWAYS;
//     // ipv4/ipv6
//     ULong family = 0;
//     bool isSetIpv4 = BitUtil::IsSet(ipTypeMask, IpTypeMask::IPV4);
//     bool isSetIpv6 = BitUtil::IsSet(ipTypeMask, IpTypeMask::IPV6);
//     if(LIKELY(isSetIpv4 && isSetIpv6))
//         family = AF_UNSPEC;
//     else if(isSetIpv4)
//         family = AF_INET;
//     else if(isSetIpv6)
//         family = AF_INET6;
//     else
//     {
//         CRYSTAL_TRACE("ERROR:unknown ip type mask:%llu", ipTypeMask);
//         return Status::Failed;
//     }

//     // 变量
//     auto memoryPool = KernelGetTlsMemoryPool();
//     ULong ret = 0; 
//     PIP_ADAPTER_ADDRESSES addresses = NULL;
//     ULong outBufLen = WORKING_BUFFER_SIZE;

//     // 获取网卡信息
//     do 
//     {
//         addresses = memoryPool->AllocThreadLocal<IP_ADAPTER_ADDRESSES>(static_cast<UInt64>(outBufLen));
//         ret = ::GetAdaptersAddresses(family, flags, NULL, addresses, &outBufLen);
//         if(ret == ERROR_BUFFER_OVERFLOW)
//         {
//             CRYSTAL_TRACE("WARN: GetAdaptersAddresses buffer overflow now outBufLen=%lu", outBufLen);
//             memoryPool->FreeThreadLocal(addresses);
//             addresses = NULL;

//         }
//         else
//             break;

//     }while(ret == ERROR_BUFFER_OVERFLOW);

//     // 有错误
//     if(ret != NO_ERROR)
//     {
//         CRYSTAL_TRACE("GetAdaptersAddresses fail ret = %lu", ret);
//         return Status::Failed;
//     }
//     CRYSTAL_TRACE("suc GetAdaptersAddresses");

//     // 获取具体的网卡信息
//     PIP_ADAPTER_ADDRESSES curAddresses = addresses;
//     while(curAddresses)
//     {
//         // 网卡类型
//         Int32 cardType = NetCardType::AdapterTypeToNetCardType(curAddresses->IfType);
//         if(cardType == NetCardType::NONE)
//         {
//             CRYSTAL_TRACE("AdapterTypeToNetCardType unknown cardtype");
//             continue;
//         }

//         // 控制信息
//         NetCardInfo newCardInfo;
//         newCardInfo._cardType = cardType;
//         newCardInfo._ipTypeMask = ipTypeMask;
//         FillNetCardInfo(curAddresses, newCardInfo);
//         netCardList.push_back(newCardInfo);
//     };

//     memoryPool->FreeThreadLocal(addresses);
//     addresses = NULL;
//     return Status::Success;
// #endif


// #if CRYSTAL_TARGET_PLATFORM_LINUX
//     // 创建/销毁ifi对象
//     auto __makeIfiInfoFunc = []() -> IfiInfo *
//     {
//         return IfiInfo::NewThreadLocal_IfiInfo();
//     };
//     auto makeDelg = DelegateFactory::Create<decltype(__makeIfiInfoFunc), IfiInfo *>(__makeIfiInfoFunc);
//     auto __freeIfiInfoFunc = [](IfiInfo *ifi) -> void
//     {
//         IfiInfo::DeleteThreadLocal_IfiInfo(ifi);
//     };
//     auto freeDelg = DelegateFactory::Create<decltype(__freeIfiInfoFunc), void, IfiInfo *>(__freeIfiInfoFunc);
//     auto memoryPool = KernelGetTlsMemoryPool();
    
//     IfiInfo *head = NULL;
//     Int32 err = NetUtil::GetIfiInfo(ipTypeMask, 1, makeDelg, memoryPool, head);
//     if(err != Status::Success)
//     {
//         if(head)
//             NetUtil::FreeIfiInfo(head, freeDelg, memoryPool);

//         freeDelg->Release();
//         freeDelg = NULL;
//         makeDelg->Release();
//         makeDelg = NULL;
//         g_Log->Warn(LOGFMT_NON_OBJ_TAG(KERNEL_NS::NetUtil, "GetIfiInfo fail err:%d"), err);
//         return err;
//     }

//     CRYSTAL_TRACE("suc GetIfiInfo start get net card info");

//     for(auto ifi = head; ifi != NULL; ifi = ifi->ifi_next)
//     {
//         NetCardInfo newCardInfo;
//         newCardInfo._ipTypeMask = ipTypeMask;

//         newCardInfo._adapterName = ifi->ifi_name;
//         newCardInfo._infoMask = BitUtil::Set(newCardInfo._infoMask, NetCardInfoMask::ADAPTER_NAME);

//         const Int32 len = static_cast<Int32>(ifi->ifi_hlen);
//         if(LIKELY(len))
//         {
//             auto &macAddr = newCardInfo._mac;
//             for(Int32 idx = 0; idx < len; ++idx)
//             {
//                 macAddr.AppendFormat("%02X", ifi->ifi_haddr[idx]);
//                 if(idx != (len - 1))
//                     macAddr.AppendFormat("-");
//             }
//             newCardInfo._infoMask = BitUtil::Set(newCardInfo._infoMask, NetCardInfoMask::MAC);
//         }

//         if(ifi->ifi_mtu)
//         {
//             newCardInfo._mtu = ifi->ifi_mtu;
//             newCardInfo._infoMask = BitUtil::Set(newCardInfo._infoMask, NetCardInfoMask::MTU);
//         }

//         // ip
//         if(ifi->ifi_addr)
//         {
//             if(ifi->ifi_addr->sa_family == AF_INET)
//             {
//                 newCardInfo._ipv4.push_back(Sock_ntop_host(ifi->ifi_addr));
//             }
//             else if(ifi->ifi_addr->sa_family == AF_INET6)
//             {
//                 newCardInfo._ipv6.push_back(Sock_ntop_host(ifi->ifi_addr));
//             }
//             newCardInfo._infoMask = BitUtil::Set(newCardInfo._infoMask, NetCardInfoMask::IP);
//         }

//         if(newCardInfo._infoMask)
//             netCardList.push_back(newCardInfo);
//     }
    
//     NetUtil::FreeIfiInfo(head, freeDelg, memoryPool);
//     freeDelg->Release();
//     freeDelg = NULL;
//     makeDelg->Release();
//     makeDelg = NULL;

//     CRYSTAL_TRACE("suc get net card info!");

//     return Status::Success;
// #endif
// }

// #if CRYSTAL_TARGET_PLATFORM_LINUX

// static inline Int32 Ioctl(Int32 fd, Int32 request, void *arg)
// {
//     Int32 n;
// 	if ( (n = ioctl(fd, request, arg)) == -1)
// 		g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::NetUtil, "Ioctl error"));
// 	return(n);	/* streamio of I_LIST returns value */
// }

// static inline Byte8 *DoNetRtIfList(UInt64 ipTypeMask, Int32 flags, UInt64 *lenp, MemoryPool *memoryPool)
// {
// 	Int32	mib[6];
// 	Byte8	*buf;

//     // 协议族选择 0表示所有
//     Int32 family = 0;
//     bool isSetIpv4 = BitUtil::IsSet(ipTypeMask, IpTypeMask::IPV4);
//     bool isSetIpv6 = BitUtil::IsSet(ipTypeMask, IpTypeMask::IPV6);
//     if(!isSetIpv4 || !isSetIpv6)
//     {
//         if(isSetIpv4)
//             family = AF_INET;
//         else
//             family = AF_INET6;
//     }

//     // 获取
// 	mib[0] = CTL_NET;
// 	mib[1] = AF_ROUTE;
// 	mib[2] = 0;
// 	mib[3] = family;		/* only addresses of this family set 0 will return all familys address */
// 	mib[4] = NET_RT_IFLIST;
// 	mib[5] = flags;			/* interface index or 0 */
// 	if (sysctl(mib, 6, NULL, lenp, NULL, 0) < 0)
// 		return (NULL);

//     // 用返回的lenp开辟空间
// 	if ( (buf = memoryPool->AllocThreadLocal<Byte8>(*lenp)) == NULL)
// 		return (NULL);

//     // 获取信息
// 	if (sysctl(mib, 6, buf, lenp, NULL, 0) < 0) 
//     {
//         g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::NetUtil, "sysctl fail"));
// 		memoryPool->FreeThreadLocal(buf);
// 		return (NULL);
// 	}

// 	return (buf);
// }
// /* end net_rt_iflist */

// static inline  Byte8 *NetRtIfList(UInt64 ipTypeMask, Int32 flags, UInt64 *lenp, MemoryPool *memoryPool)
// {
// 	Byte8 *ptr;
// 	if ( (ptr = DoNetRtIfList(ipTypeMask, flags, lenp, memoryPool)) == NULL)
// 		g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::NetUtil, "net_rt_iflist error"));
// 	return ptr;
// }

// static inline void GetRtAddrs(Int32 addrs, SA *sa, SA **rti_info)
// {
// 	Int32 i;
// 	for (i = 0; i < RTAX_MAX; i++) {
// 		if (addrs & (1 << i)) {
// 			rti_info[i] = sa;
// 			NEXT_SA(sa);
// 		} else
// 			rti_info[i] = NULL;
// 	}
// }

// Int32 NetUtil::GetIfiInfo(UInt64 ipTypeMask, Int32 doaliases, IDelegate<IfiInfo *> *makeIfiInfoFunc, MemoryPool *memoryPool, IfiInfo *&ifiHead)
// {
//     struct IfiInfo		*ifi, **ifipnext;
// 	Int32				sockfd, len, lastlen, flags, myflags, idx = 0, hlen = 0;
// 	Byte8				*ptr, *buf, lastname[IFNAMSIZ], *cptr, *haddr, *sdlname;
// 	struct ifconf		ifc;
// 	struct ifreq		*ifr, ifrcopy;
// 	struct sockaddr_in	*sinptr;
// 	struct sockaddr_in6	*sin6ptr;

// 	sockfd = ::socket(AF_INET, SOCK_DGRAM, 0);
//     if(sockfd < 0)
//     {
//         g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::NetUtil, "socket fail err:%d"), errno);
//         return Status::Failed;
//     }

// 	lastlen = 0;
// 	len = 100 * sizeof(struct ifreq);	/* initial buffer size guess */
// 	for ( ; ; ) {
// 		buf = memoryPool->AllocThreadLocal<Byte8>(static_cast<UInt64>(len));
// 		ifc.ifc_len = len;
// 		ifc.ifc_buf = buf;
// 		if (ioctl(sockfd, SIOCGIFCONF, &ifc) < 0) {
// 			if (errno != EINVAL || lastlen != 0)
// 				g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::NetUtil, "ioctl error"));
// 		} else {
// 			if (ifc.ifc_len == lastlen)
// 				break;		/* success, len has not changed */
// 			lastlen = ifc.ifc_len;
// 		}
// 		len += 10 * sizeof(struct ifreq);	/* increment */
// 		memoryPool->FreeThreadLocal(buf);
// 	}
// 	ifiHead = NULL;
// 	ifipnext = &ifiHead;
// 	lastname[0] = 0;
// 	sdlname = NULL;
// /* end get_ifi_info1 */

// /* include get_ifi_info2 */
// 	for (ptr = buf; ptr < buf + ifc.ifc_len; ) {
// 		ifr = reinterpret_cast<struct ifreq *>(ptr);

// #ifdef	HAVE_SOCKADDR_SA_LEN
// 		len = max(sizeof(struct sockaddr), ifr->ifr_addr.sa_len);
// #else
// 		switch (ifr->ifr_addr.sa_family) {
// 		case AF_INET6:	
// 			len = sizeof(struct sockaddr_in6);
// 			break;
// 		case AF_INET:	
// 		default:	
// 			len = sizeof(struct sockaddr);
// 			break;
// 		}
// #endif	/* HAVE_SOCKADDR_SA_LEN */
// 		ptr += sizeof(ifr->ifr_name) + len;	/* for next one in buffer */

// #ifdef	HAVE_SOCKADDR_DL_STRUCT
// 		/* assumes that AF_LINK precedes AF_INET or AF_INET6 */
// 		if (ifr->ifr_addr.sa_family == AF_LINK) {
// 			struct sockaddr_dl *sdl = (struct sockaddr_dl *)&ifr->ifr_addr;
// 			sdlname = ifr->ifr_name;
// 			idx = sdl->sdl_index;
// 			haddr = sdl->sdl_data + sdl->sdl_nlen;
// 			hlen = sdl->sdl_alen;
// 		}
// #endif

//         if(!IsFamilyEnable(ifr->ifr_addr.sa_family, ipTypeMask))
//             continue;/* ignore if not desired address family */

// 		myflags = 0;
// 		if ( (cptr = strchr(ifr->ifr_name, ':')) != NULL)
// 			*cptr = 0;		/* replace colon with null */
// 		if (strncmp(lastname, ifr->ifr_name, IFNAMSIZ) == 0) {
// 			if (doaliases == 0)
// 				continue;	/* already processed this interface */
// 			myflags = IFI_ALIAS;
// 		}
// 		::memcpy(lastname, ifr->ifr_name, IFNAMSIZ);

// 		ifrcopy = *ifr;
// 		Ioctl(sockfd, SIOCGIFFLAGS, &ifrcopy);
// 		flags = ifrcopy.ifr_flags;
// 		if ((flags & IFF_UP) == 0)
// 			continue;	/* ignore if interface not up */
// /* end get_ifi_info2 */

// /* include get_ifi_info3 */
// 		ifi = makeIfiInfoFunc->Invoke();
// 		*ifipnext = ifi;			/* prev points to this new one */
// 		ifipnext = &ifi->ifi_next;	/* pointer to next one goes here */

// 		ifi->ifi_flags = flags;		/* IFF_xxx values */
// 		ifi->ifi_myflags = myflags;	/* IFI_xxx values */
// #if defined(SIOCGIFMTU) && defined(HAVE_STRUCT_IFREQ_IFR_MTU)
// 		Ioctl(sockfd, SIOCGIFMTU, &ifrcopy);
// 		ifi->ifi_mtu = ifrcopy.ifr_mtu;
// #else
// 		ifi->ifi_mtu = 0;
// #endif
// 		::memcpy(ifi->ifi_name, ifr->ifr_name, IFI_NAME);
// 		ifi->ifi_name[IFI_NAME-1] = '\0';
// 		/* If the sockaddr_dl is from a different interface, ignore it */
// 		if (sdlname == NULL || ::strcmp(sdlname, ifr->ifr_name) != 0)
// 			idx = hlen = 0;
// 		ifi->ifi_index = idx;
// 		ifi->ifi_hlen = hlen;
// 		if (ifi->ifi_hlen > IFI_HADDR)
// 			ifi->ifi_hlen = IFI_HADDR;
// 		if (hlen)
// 			::memcpy(ifi->ifi_haddr, haddr, ifi->ifi_hlen);
// /* end get_ifi_info3 */
// /* include get_ifi_info4 */
// 		switch (ifr->ifr_addr.sa_family) {
// 		case AF_INET:
// 			sinptr = reinterpret_cast<struct sockaddr_in *>(&ifr->ifr_addr);
//             const UInt64 addrSize = static_cast<UInt64>(sizeof(struct sockaddr_in));
// 			ifi->ifi_addr = memoryPool->AllocThreadLocal<sockaddr>(addrSize);
//             ::memset(ifi->ifi_addr, 0, addrSize);
// 			::memcpy(ifi->ifi_addr, sinptr, addrSize);

// #ifdef	SIOCGIFBRDADDR
// 			if (flags & IFF_BROADCAST) {
// 				Ioctl(sockfd, SIOCGIFBRDADDR, &ifrcopy);
// 				sinptr = reinterpret_cast<struct sockaddr_in *>(&ifrcopy.ifr_broadaddr);
// 				ifi->ifi_brdaddr = memoryPool->AllocThreadLocal<sockaddr>(addrSize);
//                 ::memset(ifi->ifi_brdaddr, 0, addrSize);
// 				::memcpy(ifi->ifi_brdaddr, sinptr, addrSize);
// 			}
// #endif

// #ifdef	SIOCGIFDSTADDR
// 			if (flags & IFF_POINTOPOINT) {
// 				Ioctl(sockfd, SIOCGIFDSTADDR, &ifrcopy);
// 				sinptr = reinterpret_cast<struct sockaddr_in *>(&ifrcopy.ifr_dstaddr);
// 				ifi->ifi_dstaddr = memoryPool->AllocThreadLocal<sockaddr>(addrSize);
//                 ::memset(ifi->ifi_dstaddr, 0, addrSize);
// 				::memcpy(ifi->ifi_dstaddr, sinptr, addrSize);
// 			}
// #endif
// 			break;

// 		case AF_INET6:
// 			sin6ptr = reinterpret_cast<struct sockaddr_in6 *>(&ifr->ifr_addr);
//             const UInt64 addr6Size = static_cast<UInt64>(sizeof(struct sockaddr_in6));
// 			ifi->ifi_addr = memoryPool->AllocThreadLocal<sockaddr>(addr6Size);
//             ::memset(ifi->ifi_addr, 0, addr6Size);
// 			::memcpy(ifi->ifi_addr, sin6ptr, addr6Size);

// #ifdef	SIOCGIFDSTADDR
// 			if (flags & IFF_POINTOPOINT) {
// 				Ioctl(sockfd, SIOCGIFDSTADDR, &ifrcopy);
// 				sin6ptr = reinterpret_cast<struct sockaddr_in6 *>(&ifrcopy.ifr_dstaddr);
// 				ifi->ifi_dstaddr = memoryPool->AllocThreadLocal<sockaddr>(addr6Size);
//                 ::memset(ifi->ifi_dstaddr, 0, addr6Size);
// 				::memcpy(ifi->ifi_dstaddr, sin6ptr, addr6Size);
// 			}
// #endif
// 			break;

// 		default:
// 			break;
// 		}
// 	}

//     close(sockfd);
// 	memoryPool->FreeThreadLocal(buf);
// 	return Status::Success;	/* pointer to first structure in linked list */
// }

// Int32 NetUtil::GetIfiInfo2(UInt64 ipTypeMask, Int32 doaliases, IDelegate<IfiInfo *> *makeIfiInfoFunc, MemoryPool *memoryPool, IfiInfo *&ifiHead)
// {
//     Int32 				flags;
// 	Byte8				*buf, *next, *lim;
// 	UInt64				len;
// 	struct if_msghdr	*ifm;
// 	struct ifa_msghdr	*ifam;
// 	struct sockaddr		*sa, *rti_info[RTAX_MAX];
// 	struct sockaddr_dl	*sdl;
// 	struct IfiInfo		*ifi, *ifisave, **ifipnext;

//     // len 返回buffer大小
// 	buf = NetRtIfList(ipTypeMask, 0, &len, memoryPool);
//     if(UNLIKELY(!buf))
//     {
//         g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::NetUtil, "GetIfiInfo fail of NetRtIfList"));
//         return Status::Failed;
//     }

// 	ifiHead = NULL;
// 	ifipnext = &ifiHead;

// 	lim = buf + len;
// 	for (next = buf; next < lim; next += ifm->ifm_msglen) {
// 		ifm = (struct if_msghdr *) next;
// 		if (ifm->ifm_type == RTM_IFINFO) {
// 			if ( ((flags = ifm->ifm_flags) & IFF_UP) == 0)
// 				continue;	/* ignore if interface not up */

// 			sa = (struct sockaddr *) (ifm + 1);
// 			GetRtAddrs(ifm->ifm_addrs, sa, rti_info);
// 			if ( (sa = rti_info[RTAX_IFP]) != NULL) {
// 				ifi = makeIfiInfoFunc->Invoke();
// 				*ifipnext = ifi;			/* prev points to this new one */
// 				ifipnext = &ifi->ifi_next;	/* ptr to next one goes here */

// 				ifi->ifi_flags = flags;
// 				if (sa->sa_family == AF_LINK) {
// 					sdl = reinterpret_cast<struct sockaddr_dl *>(sa);
// 					ifi->ifi_index = sdl->sdl_index;
// 					if (sdl->sdl_nlen > 0)
// 						::snprintf(ifi->ifi_name, IFI_NAME, "%*s",
// 								 sdl->sdl_nlen, &sdl->sdl_data[0]);
// 					else
// 						::snprintf(ifi->ifi_name, IFI_NAME, "index %d",
// 								 sdl->sdl_index);

// 					if ( (ifi->ifi_hlen = sdl->sdl_alen) > 0)
// 						::memcpy(ifi->ifi_haddr, LLADDR(sdl),
// 							   min(IFI_HADDR, sdl->sdl_alen));
// 				}
// 			}
// /* end get_ifi_info1 */

// /* include get_ifi_info3 */
// 		} else if (ifm->ifm_type == RTM_NEWADDR) {
// 			if (ifi->ifi_addr) {	/* already have an IP addr for i/f */
// 				if (doaliases == 0)
// 					continue;

// 				/* 4we have a new IP addr for existing interface */
// 				ifisave = ifi;
// 				ifi = makeIfiInfoFunc->Invoke();
// 				*ifipnext = ifi;			/* prev points to this new one */
// 				ifipnext = &ifi->ifi_next;	/* ptr to next one goes here */
// 				ifi->ifi_flags = ifisave->ifi_flags;
// 				ifi->ifi_index = ifisave->ifi_index;
// 				ifi->ifi_hlen = ifisave->ifi_hlen;
// 				::memcpy(ifi->ifi_name, ifisave->ifi_name, IFI_NAME);
// 				::memcpy(ifi->ifi_haddr, ifisave->ifi_haddr, IFI_HADDR);
// 			}

// 			ifam = reinterpret_cast<struct ifa_msghdr *>(next);
// 			sa = reinterpret_cast<struct sockaddr *>(ifam + 1);
// 			GetRtAddrs(ifam->ifam_addrs, sa, rti_info);

// 			if ( (sa = rti_info[RTAX_IFA]) != NULL) {
// 				ifi->ifi_addr = memoryPool->AllocThreadLocal<struct sockaddr>(sa->sa_len);
//                 ::memset(ifi->ifi_addr, 0, sa->sa_len);
// 				::memcpy(ifi->ifi_addr, sa, sa->sa_len);
// 			}

// 			if ((flags & IFF_BROADCAST) &&
// 				(sa = rti_info[RTAX_BRD]) != NULL) {
// 				ifi->ifi_brdaddr = memoryPool->AllocThreadLocal<struct sockaddr>(sa->sa_len);
//                 ::memset(ifi->ifi_brdaddr, 0, sa->sa_len);
// 				::memcpy(ifi->ifi_brdaddr, sa, sa->sa_len);
// 			}

// 			if ((flags & IFF_POINTOPOINT) &&
// 				(sa = rti_info[RTAX_BRD]) != NULL) {
// 				ifi->ifi_dstaddr = memoryPool->AllocThreadLocal<struct sockaddr>(sa->sa_len);
//                 ::memset(ifi->ifi_dstaddr, 0, sa->sa_len);
// 				::memcpy(ifi->ifi_dstaddr, sa, sa->sa_len);
// 			}

// 		} else
// 			g_Log->Error(LOGFMT_NON_OBJ_TAG(KERNEL_NS::NetUtil, "unexpected message type %d"), ifm->ifm_type);
//             return Status::Failed;
// 	}

// 	/* "ifihead" points to the first structure in the linked list */
// 	return Status::Success;	/* ptr to first structure in linked list */
// }

// void NetUtil::FreeIfiInfo(IfiInfo *ifiHead, IDelegate<void,  IfiInfo *> *freeIfiInfoFunc, MemoryPool *memoryPool)
// {
// 	struct IfiInfo	*ifi, *ifinext;

// 	for (ifi = ifiHead; ifi != NULL; ifi = ifinext) {
// 		if (ifi->ifi_addr != NULL)
// 			memoryPool->FreeThreadLocal(ifi->ifi_addr);
// 		if (ifi->ifi_brdaddr != NULL)
// 			memoryPool->FreeThreadLocal(ifi->ifi_brdaddr);
// 		if (ifi->ifi_dstaddr != NULL)
// 			memoryPool->FreeThreadLocal(ifi->ifi_dstaddr);
// 		ifinext = ifi->ifi_next;		/* can't fetch ifi_next after free() */
// 		freeIfiInfoFunc->Invoke(ifi);					/* the ifi_info{} itself */
// 	}
// }

// #endif

// KERNEL_END

// KERNEL_BEGIN

// void test()
// {
//         {                                                                                                           
//             DEF_STATIC_THREAD_LOCAL_DECLEAR KERNEL_NS::ObjAlloctor<MemoryAssistInfoByType< _Build::TL >> *staticAlloctor =       
//             KERNEL_NS::TlsUtil::GetTlsStack()->New< KERNEL_NS::TlsObjectPool<KERNEL_NS::ObjAlloctor<MemoryAssistInfoByType< _Build::TL >>> >()->GetPool(1 
//             , KERNEL_NS::MemoryAlloctorConfig(sizeof(MemoryAssistInfoByType< _Build::TL >), true));     
//         }    
// }

// KERNEL_END