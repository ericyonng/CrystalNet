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
 * Date: 2025-01-23 14:25:06
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_SERVICE_COMMON_LIBRARY_HOTFIX_MONITOR_INTERFACE_ILIBRARY_HOTFIX_MONITOR_H__
#define __CRYSTAL_NET_SERVICE_COMMON_LIBRARY_HOTFIX_MONITOR_INTERFACE_ILIBRARY_HOTFIX_MONITOR_H__

#pragma once

#include <service_common/common/macro.h>
#include <kernel/comp/CompObject/CompObject.h>
#include <kernel/comp/Delegate/IDelegate.h>
#include <vector>
#include <service_common/LibraryHotfixMonitor/Impl/HotFixDefine.h>

SERVICE_COMMON_BEGIN

class ILibraryHotfixMonitor : public KERNEL_NS::CompObject
{
   POOL_CREATE_OBJ_DEFAULT_P1(CompObject, ILibraryHotfixMonitor);

public:
   ILibraryHotfixMonitor(UInt64 objTypeId) : KERNEL_NS::CompObject(objTypeId) {}

    // 检测到该文件, 则会根据文件的内容进行热更
    // 文件内容参数:
    //              FilePath:要热更的文件路径, 支持相对路径和绝对路径
    //              FileOwner: 该文件的拥有者, 例如: service表示拥有者是service, 那么就会把热更的结果投递到service
    //              如果有多个so热更, 那么中间需要至少空一行作为不同so热更的参数结束
    virtual void SetDetectionFile(const KERNEL_NS::LibString &detectionFile) = 0;

    // 设置检测时间间隔, 没有设置默认2秒检测一次
    virtual void SetDetectionTimeInterval(Int64 seconds) = 0;

    // 当前程序执行路径
    virtual void SetRootPath(const KERNEL_NS::LibString &rootPath) = 0;

    // 添加热更监听者
    // @param(hotfixKey):关注的热更key
    // @param(cb):完成加载后回调
    virtual void AddHotFixListener(KERNEL_NS::IDelegate<void, HotFixContainerElemType &> *cb) = 0;

    // cb 的参数是 HotFixContainerElemType &
    template<typename CallbackType>
    #ifdef CRYSTAL_NET_CPP20
    requires requires(CallbackType cb, HotFixContainerElemType &container)
    {
        cb(container);
    }
    #endif
    void AddHotFixListener(CallbackType &&cb);

    // 热更完成回调
    // @param(cb):cb的参数是当次热更的所有hotfixKey
    virtual void AddHotFixCompleteCallback(KERNEL_NS::IDelegate<void, const std::set<KERNEL_NS::LibString> &> *cb) = 0;
    template<typename CallbackType>
    #ifdef CRYSTAL_NET_CPP20
    requires requires(CallbackType cb, const std::set<KERNEL_NS::LibString> &container)
    {
        cb(container);
    }
    #endif
    void AddHotFixCompleteCallback(CallbackType &&cb);
};

// cb 的参数是 HotFixContainerElemType &
template<typename CallbackType>
#ifdef CRYSTAL_NET_CPP20
requires requires(CallbackType cb, HotFixContainerElemType &container)
{
    cb(container);
}
#endif
ALWAYS_INLINE void ILibraryHotfixMonitor::AddHotFixListener(CallbackType &&cb)
{
    auto deleg = KERNEL_CREATE_CLOSURE_DELEGATE(cb, void, HotFixContainerElemType &);
    AddHotFixListener(deleg);
}

template<typename CallbackType>
#ifdef CRYSTAL_NET_CPP20
requires requires(CallbackType cb, const std::set<KERNEL_NS::LibString> &container)
{
    cb(container);
}
#endif
ALWAYS_INLINE void ILibraryHotfixMonitor::AddHotFixCompleteCallback(CallbackType &&cb)
{
    auto deleg = KERNEL_CREATE_CLOSURE_DELEGATE(cb, void, const std::set<KERNEL_NS::LibString> &);
    AddHotFixCompleteCallback(deleg);
}

SERVICE_COMMON_END

#endif
