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
 * Date: 2021-10-16 22:06:02
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_NET_UTIL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_NET_UTIL_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/Delegate/Delegate.h>

KERNEL_BEGIN

struct NetCardInfo;
struct IfiInfo;
class MemoryPool;

// TODO:传入创建与释放内存的方法,这样可以兼容多线程和线程局部
struct KERNEL_EXPORT MemoryCreateFree
{
    IDelegate<void *, UInt64> *_alloc;
    IDelegate<void, void *> *_free;
};

// get/free必须统一线程,因为内部是thread local创建的：TODO:应该传入一个创建内存回收内存的方法
class KERNEL_EXPORT NetUtil
{// 需要安装unp才可以，所以废弃
public:
    // 获取网卡信息
    // @param(cardTypeMask):网卡类型掩码 NetCardType位操作控制获取有线网卡或者无线网卡等 在linux下无效
    // @param(ipTypeMask):ip类型掩码 控制获取ipv4/ipv6
    // @param(netCardList):输出
    // @param(makeCardInfoFunc):可以自由的使用tls/multithread版本创建NetCardInfo 对象
    // static Int32 GetNetCardInfo(Int32 cardTypeMask, UInt64 ipTypeMask, std::vector<NetCardInfo *> &netCardList, IDelegate<NetCardInfo *> *makeCardInfoFunc);
    // static Int32 GetNetCardInfo(Int32 cardTypeMask, UInt64 ipTypeMask, std::vector<NetCardInfo> &netCardList);

    // #if CRYSTAL_TARGET_PLATFORM_LINUX
    // // doaliases:默认取1
    // static Int32 GetIfiInfo(UInt64 ipTypeMask, Int32 doaliases, IDelegate<IfiInfo *> *makeIfiInfoFunc, MemoryPool *memoryPool, IfiInfo *&ifiHead);
    // static Int32 GetIfiInfo2(UInt64 ipTypeMask, Int32 doaliases, IDelegate<IfiInfo *> *makeIfiInfoFunc, MemoryPool *memoryPool, IfiInfo *&ifiHead);
    // static void FreeIfiInfo(IfiInfo *ifiHead, IDelegate<void,  IfiInfo *> *freeIfiInfoFunc, MemoryPool *memoryPool);
    // #endif
};


KERNEL_END

#endif
