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
 * Date: 2025-01-23 14:37:06
 * Author: Eric Yonng
 * Description: 
*/
#ifndef __CRYSTAL_NET_SERVICE_COMMON_LIBRARY_HOTFIX_MONITOR_IMPL_LIBRARY_HOTFIX_MONITOR_H__
#define __CRYSTAL_NET_SERVICE_COMMON_LIBRARY_HOTFIX_MONITOR_IMPL_LIBRARY_HOTFIX_MONITOR_H__

#pragma once

#include <service_common/LibraryHotfixMonitor/Interface/ILibraryHotfixMonitor.h>
#include <kernel/comp/TimeSlice.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

class LibTimer;

KERNEL_END

SERVICE_COMMON_BEGIN

class LibraryHotfixMonitor : public ILibraryHotfixMonitor
{
 POOL_CREATE_OBJ_DEFAULT_P1(ILibraryHotfixMonitor, LibraryHotfixMonitor);

public:
 LibraryHotfixMonitor();
 ~LibraryHotfixMonitor();
 virtual void Release() override;

 virtual void SetDetectionFile(const KERNEL_NS::LibString &detectionFile) override;
 virtual void SetDetectionTimeInterval(Int64 seconds) override;
 virtual void SetRootPath(const KERNEL_NS::LibString &rootPath) override;
 
 virtual void AddHotFixListener(KERNEL_NS::IDelegate<void, HotFixContainerElemType &> *cb) override;
 virtual void AddHotFixCompleteCallback(KERNEL_NS::IDelegate<void, const std::set<KERNEL_NS::LibString> &> *cb) override;

protected:
  virtual Int32 _OnInit() override;
  virtual void _OnClose() override;
  void _Clear();

  // 删除检测文件
  void _DelDetectionFile() const;

  // 检测定时器
  void _OnDetectionTimerOut(KERNEL_NS::LibTimer *timer);

private:
    KERNEL_NS::LibTimer *_tick;
    bool _enableDetection;
    KERNEL_NS::TimeSlice _detectionInterval;
    KERNEL_NS::LibString _deadthDetectionFile;
    KERNEL_NS::LibString _detectionFileNameWithoutDir;
    KERNEL_NS::LibString _currentRootPath;

    std::vector<KERNEL_NS::IDelegate<void, HotFixContainerElemType &> *> _hotfixDeleg;
    std::vector<KERNEL_NS::IDelegate<void, const std::set<KERNEL_NS::LibString> &> *> _hotfixCompleteCallback;
};

SERVICE_COMMON_END

#endif