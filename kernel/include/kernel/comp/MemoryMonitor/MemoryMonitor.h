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
 * Date: 2021-03-19 22:23:33
 * Author: Eric Yonng
 * Description: 
 *              1.对象池统计信息：
 *                              1. 对象名称
 *                              2. 对象尺寸
 *                              3. 当前对象分配个数
 *                              4. 历史对象分配个数
 *                              5. 当前总的对象占用大小
 *                              6. 当前该对象池总的占用内存大小
 *                              7. 额外信息[当前空buffer个数, 当前总的block个数, 当前使用的总的block数]
 * 
 *              2.内存池
 *                              1.内存块大小
 *                              2.当前内存块分配个数
 *                              3.历史内存块分配个数
 *                              4.当前总分配内存大小
 *                              5.当前内存池总的占用内存大小
 *                              6.额外信息[当前空buffer个数,当前总的block个数， 当前使用的总的block个数]
 * 
 *              3.其他分配器
 *                              1.名称/内存块大小
 *                              2.当前内存块分配个数
 *                              3.历史内存块分配个数
 *                              4.当前总分配内存大小
 *                              5.当前内存池总的占用内存大小
 *                              6.额外信息[当前空buffer个数,当前总的block个数， 当前使用的总的block个数]
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMORY_MONITOR_MEMORY_MONITOR_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMORY_MONITOR_MEMORY_MONITOR_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/MemoryMonitor/Statistics.h>
#include <kernel/comp/Delegate/Delegate.h>

KERNEL_BEGIN

class LibThread;

class KERNEL_EXPORT MemoryMonitor
{
public:
    MemoryMonitor();
    virtual ~MemoryMonitor();

    static MemoryMonitor *GetInstance();
    Int32 Init(Int64 milliSecInterval);
    void Start();
    void Close();

    Int64 GetMilliSecInterval() const;

    IDelegate<void> *MakeWorkTask();

    // 只提供注册不提供反注册
    static Statistics *GetStatistics();

private:
    void _DoWork();

private:
    std::atomic_bool _init;
    std::atomic_bool _isStart;
    Int64 _milliSecInterval;
};

ALWAYS_INLINE Int64 MemoryMonitor::GetMilliSecInterval() const
{
    return _milliSecInterval;
}

ALWAYS_INLINE IDelegate<void> *MemoryMonitor::MakeWorkTask()
{
    return DelegateFactory::Create(this, &MemoryMonitor::_DoWork);
}

KERNEL_END

#endif
