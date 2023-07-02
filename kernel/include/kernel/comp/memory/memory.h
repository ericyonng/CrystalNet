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
 * Date: 2020-11-15 16:02:57
 * Author: Eric Yonng
 * Description: 内存对齐规则：__MEMORY_ALIGN__(__MEMORY_ALIGN__(sizeof(MemoryBlock)) + bytes)
 * 1.垃圾回收线程只有一个,无论有多少个分配器
 * 2.内存管理模块包括：对象池,内存池
 * 3.对象池是特殊的内存池
 * 4.内存池用于分配任意大小内存块,
 * 5.内存池分配的最小单元是 MEMORY_POOL_MINI_BLOCK_BYTES 的最小公倍数，分配小于该内存块大小的内存会直接给最小单元
 * 6.内存池分配的最大单元是 MEMORY_POOL_MAX_BLOCK_BYTES 的最小公倍数,分配大于该内存块的会直接调用系统malloc进行分配
 * 7.内存池分配的内存单元支持配置 InitMemoryPoolInfo
 * 
 * 8.在保证如果对象仅在当前线程内分配与释放的,请使用对象的local thread相关接口进行内存的分配与释放！！！！！！！！！！
 * 9.memory内部所有组件不可引用本框架任何其他组件,避免不必要的文件重复包含导致编译失败
 * 
 * 10.对于业务层（非底层）请使用MemoryAssist包装宏，可以采集创建对象的相关信息避免内存泄漏
 * 11.建议新的对象池可以使用ObjPoolWrap对对象进行包装，这是一个新的简单的功能完备的对象池,它可支持所有对象的池化
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_MEMORY_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_MEMORY_H__

#pragma once

#include <kernel/comp/memory/MemoryDefs.h>
#include <kernel/comp/memory/Defs/Defs.h>
#include <kernel/comp/memory/Defs/TemplateMacro.h>
#include <kernel/comp/memory/Defs/MemoryAlloctorConfig.h>

#include <kernel/comp/memory/AlloctorInfoCollector.h>
#include <kernel/comp/memory/MemoryAlloctor.h>
#include <kernel/comp/memory/MemoryPool.h>
#include <kernel/comp/memory/ObjAlloctor.h>
#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/memory/GarbageThread.h>
#include <kernel/comp/memory/ObjPoolWrap.h>
#include <kernel/comp/memory/MemoryAssist.h>
#include <kernel/comp/memory/CenterMemoryCollector.h>
#include <kernel/comp/memory/CenterMemoryThreadInfo.h>
#include <kernel/comp/memory/MergeMemoryBufferInfo.h>
#include <kernel/comp/memory/CenterMemoryProfileInfo.h>
#include <kernel/comp/memory/CenterMemoryTopnThreadInfo.h>

#endif
