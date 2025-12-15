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
 * Date: 2020-12-06 18:44:11
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_GARBAGE_COLLECTOR_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_GARBAGE_COLLECTOR_H__

#pragma once

#include <list>
#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>
#include <kernel/comp/Lock/Impl/SpinLock.h>

KERNEL_BEGIN

class MemoryBuffer;

class KERNEL_EXPORT GarbageCollector
{
public:
    GarbageCollector();
    virtual ~GarbageCollector();

public:
    void push(MemoryBuffer *&head);

private:
    void _ThreadPurge();

private:
    SpinLock _lck;
    std::list<MemoryBuffer *> *_garbageList;
    std::list<MemoryBuffer *> *_swapList;
};

KERNEL_END



#endif
