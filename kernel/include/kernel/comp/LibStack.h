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
 * Date: 2020-11-30 00:03:28
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LIBSTACK_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LIBSTACK_H__

#pragma once

#include <kernel/kernel_inc.h>

KERNEL_BEGIN

template<typename Elem, typename OwnerType>
class LibStack
{
public:
    LibStack(OwnerType *owner, Elem *stack_ptr, UInt64 elemCount);
    virtual ~LibStack();

public:
    void Init(OwnerType *owner, Elem *stack_ptr, UInt64 elemCount);
    void push(Elem e);
    Elem pop();
    OwnerType *get_owner();
    bool empty();
    Int64 count();
    bool full();

public:
    LibStack<Elem, OwnerType> *_pre;
    LibStack<Elem, OwnerType> *_next;
    bool _isInStackList;

private:
    // 栈结构 先进后出 元素是Elem *
    // [ | | |]
    OwnerType *_owner;
    UInt64 _elemCount; // 栈元素的最大个数
    Int64 _stack_top_pos;
    Elem *_stack;
};


template<typename Elem, typename OwnerType>
inline LibStack<Elem, OwnerType>::LibStack(OwnerType *owner, Elem *stack_ptr, UInt64 elemCount)
{
    Init(owner, stack_ptr, elemCount);
}


template<typename Elem, typename OwnerType>
inline LibStack<Elem, OwnerType>::~LibStack()
{
}

template<typename Elem, typename OwnerType>
inline void LibStack<Elem, OwnerType>::Init(OwnerType *owner, Elem *stack_ptr, UInt64 elemCount)
{
    _pre = NULL;
    _next = NULL;
    _isInStackList = false;

    _owner = owner;
    _elemCount = elemCount;
    _stack_top_pos = -1;
    _stack = stack_ptr;

    _stack[0] = NULL;
}

template<typename Elem, typename OwnerType>
inline void LibStack<Elem, OwnerType>::push(Elem e)
{
    _stack[++_stack_top_pos] = e;
}

template<typename Elem, typename OwnerType>
inline Elem LibStack<Elem, OwnerType>::pop()
{
    Elem e = _stack[_stack_top_pos];
    _stack[_stack_top_pos--] = NULL;
    return e;
}

template<typename Elem, typename OwnerType>
inline OwnerType *LibStack<Elem, OwnerType>::get_owner()
{
    return _owner;
}

template<typename Elem, typename OwnerType>
inline bool LibStack<Elem, OwnerType>::empty()
{
    return _stack_top_pos < 0;
}

template<typename Elem, typename OwnerType>
inline Int64 LibStack<Elem, OwnerType>::count()
{
    return _stack_top_pos + 1;
}

template<typename Elem, typename OwnerType>
inline bool LibStack<Elem, OwnerType>::full()
{
    return _stack_top_pos + 1 == _elemCount;
}

KERNEL_END

#endif