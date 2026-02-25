/*!
 * MIT License
 *  
 * Copyright (c) 2020 Eric Yonng<120453674@qq.com>
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *  
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *  
 * 
 * Date: 2020-10-06 16:50:27
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COMP_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COMP_H__

#pragma once


// 基础组件
#include <kernel/comp/LibString.h>
#include <kernel/comp/LibStringOut.h>
#include <kernel/comp/LibTime.h>
#include <kernel/comp/TimeSlice.h>
#include <kernel/comp/AutoDel.h>
#include <kernel/comp/BinaryArray.h>
#include <kernel/comp/LibStack.h>
#include <kernel/comp/Singleton.h>
#include <kernel/comp/SmartPtr.h>
#include <kernel/comp/Random/Random.h>
#include <kernel/comp/Cpu/cpu.h>
#include <kernel/comp/BitWidth.h>
#include <kernel/comp/Endian/LibEndian.h>
#include <kernel/comp/PerformanceRecord.h>
#include <kernel/comp/BigNum.h>
#include <kernel/comp/LibTraceId.h>
#include <kernel/comp/KernelFinally/KernelFinally.h>
#include <kernel/comp/ConcurrentPriorityQueue/MPMCQueue.h>
#include <kernel/comp/ConcurrentPriorityQueue/SPSCQueue.h>
#include <kernel/comp/ObjLife.h>
#include <kernel/comp/Config/Config.h>

// 简单组件
#include <kernel/comp/Delegate/LibDelegate.h>
#include <kernel/comp/Tls/Tls.h>
#include <kernel/comp/Encrypt/Encrypt.h>
#include <kernel/comp/Coder/coder.h>
#include <kernel/comp/Lock/Lock.h>
#include <kernel/comp/memory/memory.h>
#include <kernel/comp/Task/Task.h>
#include <kernel/comp/thread/thread.h>
#include <kernel/comp/File/File.h>
#include <kernel/comp/LibStream.h>
#include <kernel/comp/MessageQueue/message.h>
#include <kernel/comp/Variant/variant_inc.h>
#include <kernel/comp/Timer/Timer.h>
#include <kernel/comp/MemoryMonitor/memorymonitor_inc.h>
#include <kernel/comp/Event/event_inc.h>
#include <kernel/comp/BlackWhiteList/BlackWhiteList.h>
#include <kernel/comp/ConcurrentPriorityQueue/ConcurrentPriorityQueue.h>
#include <kernel/comp/CompObject/CompObjectInc.h>
#include <kernel/comp/Poller/PollerInc.h>
#include <kernel/comp/LibDirtyHelper.h>
#include <kernel/comp/LibList.h>
#include <kernel/comp/Pipline/pipeline.h>
#include <kernel/comp/Archive/archive.h>
#include <kernel/comp/xml/xml.h>
#include <kernel/comp/xlsx/xlsx.h>
#include <kernel/comp/params/params.h>
#include <kernel/comp/Coroutines/Coroutines.h>
#include <kernel/comp/Timing/Timing.h>
#include <kernel/comp/ShareLibraryLoader/ShareLibraryLoader.h>
#include <kernel/comp/ShareLibraryLoader/ShareLibraryLoaderFactory.h>

// 复杂组件
#include <kernel/comp/Utils/Utils.h>
#include <kernel/comp/Log/log.h>
#include <kernel/comp/NetEngine/NetEngine_Inc.h>
#include <kernel/comp/Service/Service.h>
#include <kernel/comp/App/app.h>
#include <kernel/comp/TlsMemoryCleanerComp.h>
#include <kernel/comp/Lua/Lua.h>
#include <kernel/comp/IdGenerator/IdGenerator.h>
#include <kernel/comp/FileMonitor/FileMonitorInc.h>

#endif
