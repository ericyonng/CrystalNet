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
 * Date: 2025-11-12 12:00:32
 * Author: Eric Yonng
 * Description:
 * 1. 提供EventLoop
 * 2. 提供GetPoller接口(协程/或者普通接口版)
 * 3. 提供SendAsync的普通接口版, 协程版
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_THREAD_LIB_EVENT_LOOP_THREAD_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_THREAD_LIB_EVENT_LOOP_THREAD_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>

#include "kernel/comp/SmartPtr.h"
#include <kernel/comp/Poller/Poller.h>

KERNEL_BEGIN

class IThreadStartUp;

 class LibThread;

class KERNEL_EXPORT LibEventLoopThread
{
 public:
 LibEventLoopThread(const LibString &threadName = "", IThreadStartUp *startUp = NULL);
 ~LibEventLoopThread();
 virtual void Release();

 // 启动
 void Start();
 // 关闭 = HalfClose + WaitDestroy合并
 void Close();
 // 半关闭 返回值表示可否执行WaitDestroy
 bool HalfClose();
 // 等待退出
 void FinishClose();

 CoTask<const Poller *> GetPoller() const;
 CoTask<Poller *> GetPoller();
 const Poller * GetPollerNoAsync() const;
 Poller * GetPollerNoAsync();
 
 // 调用者当前线程投递req给this
 // req暂时只能传指针，而且会在otherChannel（可能不同线程）释放
 // req/res 必须实现Release, ToString接口
 template<typename ResType, typename ReqType>
 requires requires(ReqType req, ResType res)
 {
  // req/res必须有Release接口
  req.Release();
  res.Release();
    
  // req/res必须有ToString接口
  req.ToString();
  res.ToString();
 }
 CoTask<KERNEL_NS::SmartPtr<ResType, AutoDelMethods::Release>> SendAsync(ReqType *req)
 {
   auto poller = co_await GetPoller().SetDisableSuspend();
   co_return co_await poller->template SendAsync<ResType, ReqType>(req);
 }

 // 调用者当前线程投递req给this
 // req暂时只能传指针，而且会在otherChannel（可能不同线程）释放
 // req/res 必须实现Release, ToString接口
 template<typename ReqType>
 requires requires(ReqType req)
 {
  // req/res必须有Release接口
  req.Release();
    
  // req/res必须有ToString接口
  req.ToString();
 }
 CoTask<> SendAsync2(ReqType *req)
 {
   auto poller = co_await GetPoller().SetDisableSuspend();
   poller->template Send<ReqType>(req);
 }
 
private:
 LibThread *_thread;
};

KERNEL_END

#endif
