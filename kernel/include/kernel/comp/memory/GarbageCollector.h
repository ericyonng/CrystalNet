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

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/MemoryBuffer.h>
#include <kernel/comp/Lock/Lock.h>
#include <kernel/comp/Utils/ListUtil.h>
#include <kernel/comp/memory/GarbageThread.h>

KERNEL_BEGIN

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

inline GarbageCollector::GarbageCollector()
{
    _garbageList = new std::list<MemoryBuffer *>;
    _swapList = new std::list<MemoryBuffer *>;

    GarbageThread::GetInstence()->RegisterPurge(this, &GarbageCollector::_ThreadPurge);
}

inline GarbageCollector:: ~GarbageCollector()
{
    GarbageThread::GetInstence()->UnRegisterPurge(this);
    
    _lck.Lock();
    for (auto &iterMemoryBuffer:*_garbageList)
        ListUtil::Destroy(iterMemoryBuffer);

    for (auto &iterMemoryBuffer:*_swapList)
        ListUtil::Destroy(iterMemoryBuffer);

    CRYSTAL_DELETE_SAFE(_garbageList);
    CRYSTAL_DELETE_SAFE(_swapList);
    _lck.Unlock();
}

inline void GarbageCollector::push(MemoryBuffer *&head)
{
    _lck.Lock();
    _garbageList->push_back(head);
    head = NULL;
    _lck.Unlock();

    GarbageThread::GetInstence()->MaskPurge(this);
}

inline void GarbageCollector::_ThreadPurge()
{
    // 交换避免锁冲突
    _lck.Lock();
    std::list<MemoryBuffer *> *tmp = _swapList;
    _swapList = _garbageList;
    _garbageList = tmp;
    _lck.Unlock();

    MemoryBuffer *ele;
    for (auto iterEle = _swapList->begin(); iterEle != _swapList->end();)
    {
        ele = *iterEle;
        ListUtil::Destroy(ele);
        iterEle = _swapList->erase(iterEle);
    }
}

KERNEL_END



#endif
