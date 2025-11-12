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
 * Date: 2025-11-06 12:52:58
 * Author: Eric Yonng
 * Description: 
 * 为保证简单高效, 只支持Push和TryPop             
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_CONCURRENT_PRIORITY_QUEUE_LOCK_FREE_LIST_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_CONCURRENT_PRIORITY_QUEUE_LOCK_FREE_LIST_H__

#pragma once

#include <kernel/comp/memory/ObjPoolMacro.h>

KERNEL_BEGIN

// 链表节点
template<typename Elem>
class LockFreeNode
{
  POOL_CREATE_TEMPLATE_OBJ_DEFAULT(LockFreeNode, Elem)
    
public:
    LockFreeNode():_next(NULL){}

    LockFreeNode<Elem> *_next;
    Elem _data;
};

template<typename Elem>
POOL_CREATE_TEMPLATE_OBJ_DEFAULT_IMPL(LockFreeNode, Elem);

// 先入后出, Elem必须是可移动的
template<typename Elem, typename BuildType = _Build::MT>
requires std::movable<Elem> && requires(BuildType build)
{
    build.V;
}
class LockFreeList
{
    POOL_CREATE_TEMPLATE_OBJ_DEFAULT(LockFreeList, Elem, BuildType)

public:
    LockFreeList()
    :_tail{NULL}
    ,_count{0}
    {
    }

    // 注意:push e，内部使用移动语义
    void Push(Elem &&e);
    bool TryPop(Elem &e);

    // 数量只确保最终一致性(有可能会小于0, 小于0必定是中间状态,认为是空的即可)
    Int64 Size() const;

private:
    std::atomic<LockFreeNode<Elem> *> _tail;
    std::atomic<Int64> _count;
};

template<typename Elem, typename BuildType>
requires std::movable<Elem> && requires(BuildType build)
{
    build.V;
}
POOL_CREATE_TEMPLATE_OBJ_DEFAULT_IMPL(LockFreeList, Elem, BuildType);


template<typename Elem, typename BuildType>
requires std::movable<Elem> && requires(BuildType build)
{
    build.V;
}
ALWAYS_INLINE void LockFreeList<Elem, BuildType>::Push(Elem &&e)
{
    auto newNode = LockFreeNode<Elem>::NewByAdapter_LockFreeNode(BuildType::V);
    newNode->_data = std::move(e);
    
    // newNode连tail
    auto tail = _tail.load(std::memory_order_acquire);
    newNode->_next = tail;

    // cas交换
    while (!_tail.compare_exchange_weak(tail, newNode, std::memory_order_acq_rel))
    {
        newNode->_next = tail;
    }

    _count.fetch_add(1, std::memory_order_release);
}

template<typename Elem, typename BuildType>
requires std::movable<Elem> && requires(BuildType build)
{
    build.V;
}
ALWAYS_INLINE bool LockFreeList<Elem, BuildType>::TryPop(Elem &e)
{
    auto oldCount = _count.fetch_sub(1, std::memory_order_release);
    if (oldCount <= 0)
    {
        _count.fetch_add(1, std::memory_order_release);
        return false;
    }

    LockFreeNode<Elem> *newTail = NULL;

    // 先读取
    auto tail = _tail.load(std::memory_order_acquire);
    auto tailNode = tail;
    if (tail)
        newTail = tail->_next;

    // cas交换
    while (!_tail.compare_exchange_weak(tail, newTail, std::memory_order_acq_rel))
    {
        tailNode = tail;
        newTail = NULL;
        if (tail)
            newTail = tail->_next;
    }

    if (UNLIKELY(tailNode == NULL))
    {
        _count.fetch_add(1, std::memory_order_release);
        return false;
    }

    // 移动资源
    e = std::move(tailNode->_data);
    // 释放tailNode
    LockFreeNode<Elem>::DeleteByAdapter_LockFreeNode(BuildType::V, tailNode);

    return true;
}

template<typename Elem, typename BuildType>
requires std::movable<Elem> && requires(BuildType build)
{
    build.V;
}
ALWAYS_INLINE Int64 LockFreeList<Elem, BuildType>::Size() const
{
    return _count.load(std::memory_order_acquire);
}

KERNEL_END

#endif