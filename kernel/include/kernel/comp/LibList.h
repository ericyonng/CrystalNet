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
 *              TODO:测试 ListNode
 *              TODO:测试 LibList
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LIBLIST_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LIBLIST_H__

#pragma once

#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/LibString.h>
#include <kernel/common/CopyAdapter.h>

KERNEL_BEGIN

template<typename ObjType>
class ListNode
{
    POOL_CREATE_TEMPLATE_OBJ_DEFAULT(ListNode, ObjType);

public:
    ListNode()
        :_pre(NULL)
        ,_next(NULL)
    {

    }

    ListNode<ObjType> &operator++()
    {// prefix ++iter
        increment();
        return *this;
    }

    ListNode<ObjType> operator++(int)
    {// postfix iter++
        ListNode<ObjType> tmp;
        CopyAdapter<ObjType>::Invoke(tmp._data, _data);
        tmp._pre = _pre;
        tmp._next = _next;
        
        increment();
        return tmp;
    }

    ListNode<ObjType> &operator--() noexcept
    {// prefix --iter
        this->decrement();
        return *this;
    }

    ListNode<ObjType> operator--(int) noexcept
    {// postfix iter--
        ListNode<ObjType> tmp;
        CopyAdapter<ObjType>::Invoke(tmp._data, _data);
        tmp._pre = _pre;
        tmp._next = _next;
        this->decrement();
        return tmp;
    }

private:
    void increment()
    {
        if(LIKELY(_next))
        {
            CopyAdapter<ObjType>::Invoke(_data, _next->_data);
            _pre = _next->_pre;
            _next = _next->_next;
        }
    }

    void decrement()
    {
        if(LIKELY(_pre))
        {
            CopyAdapter<ObjType>::Invoke(_data, _pre->_data);
            _next = _pre->_next;
            _pre = _pre->_pre;
        }
    }

public:
    ObjType _data;
    ListNode<ObjType> *_pre;
    ListNode<ObjType> *_next;
};


template<typename ObjType>
POOL_CREATE_TEMPLATE_OBJ_DEFAULT_IMPL(ListNode, ObjType);

template<typename ObjType>
POOL_CREATE_TEMPLATE_OBJ_DEFAULT_TL_IMPL(ListNode, ObjType);

template<typename ObjType, typename BuildType = _Build::MT>
class LibList
{
    POOL_CREATE_TEMPLATE_OBJ_DEFAULT(LibList, ObjType, BuildType);
    
public:
    LibList()
        :_head(NULL)
        ,_tail(NULL)
        ,_nodeAmount(0)
    {
        
    }

    ~LibList()
    {
        Clear();
    }

    static LibList<ObjType, BuildType> *NewByBuildType()
    {
        return LibList<ObjType, BuildType>::NewByAdapter_LibList(BuildType::V);
    }
    static void DeleteByBuildType(LibList<ObjType, BuildType> *ptr)
    {
        return LibList<ObjType, BuildType>::DeleteByAdapter_LibList(BuildType::V, ptr);
    }

    // other会被置空
    void MergeTail(LibList<ObjType, BuildType> *other)
    {
        // 非链表
        if(LIKELY(_head))
        {
            _tail->_next = other->_head;
            if(other->_head)
            {
                other->_head->_pre = _tail;
                _tail = other->_tail;
            }
        }
        else
        {
            _head = other->_head;
            _tail = other->_tail;
        }

        // 节点合并数量
        _nodeAmount += other->_nodeAmount;

        // 数据转移后丢弃
        other->_GiveUp();
    }

    void MergeFront(LibList<ObjType, BuildType> *other)
    {
        // 非空链表
        if(LIKELY(_head))
        {
            _head->_pre = other->_tail;
            if(other->_tail)
            {
                other->_tail->_next = _head;
                _head = other->_head;
            }
        }
        else
        {
            _head = other->_head;
            _tail = other->_tail;
        }

        // 节点合并数量
        _nodeAmount += other->_nodeAmount;

        // 数据转移后丢弃
        other->_GiveUp(); 
    }

    // 交换
    void Swap(LibList<ObjType, BuildType> *other)
    {
        auto head = _head;
        auto tail = _tail;
        auto nodeAmount = _nodeAmount;

        _head = other->_head;
        _tail = other->_tail;
        _nodeAmount = other->_nodeAmount;
        other->_head = head;
        other->_tail = tail;
        other->_nodeAmount = nodeAmount;
    }

    // 数据转移后丢弃
    void GiveUp(ListNode<ObjType> *&head, ListNode<ObjType> *&tail, UInt64 &nodeAmount)
    {
        head = _head;
        tail = _tail;
        nodeAmount = _nodeAmount;

        _GiveUp();
    }

    // 插入
    void InsertBefore(ObjType obj, ListNode<ObjType> *node)
    {
        auto newNode = ListNode<ObjType>::NewByAdapter_ListNode(BuildType::V);
        CopyAdapter<ObjType>::Invoke(newNode->_data, obj);
        
        if(node->_pre)
            node->_pre->_next = newNode;
        newNode->_pre = node->_pre;
        newNode->_next = node;
        node->_pre = newNode;

        if(UNLIKELY(_head == node))
            _head = newNode;

        ++_nodeAmount;
    }

    // 结尾添加
    ListNode<ObjType> *PushBack(ObjType obj)
    {
        auto newNode = ListNode<ObjType>::NewByAdapter_ListNode(BuildType::V);
        CopyAdapter<ObjType>::Invoke(newNode->_data, obj);

        if(LIKELY(_tail))
        {
            _tail->_next = newNode;
            newNode->_pre = _tail;
            _tail = newNode;
        }
        else
        {
            _head = newNode;
            _tail = newNode;
        }

        ++_nodeAmount;
        return newNode;
    }

    // 头添加
    ListNode<ObjType> *PushFront(ObjType obj)
    {
        auto newNode = ListNode<ObjType>::NewByAdapter_ListNode(BuildType::V);
        CopyAdapter<ObjType>::Invoke(newNode->_data, obj);

        if(LIKELY(_head))
        {
            _head->_pre = newNode;
            newNode->_next = _head;
            _head = newNode;
        }
        else
        {
            _head = newNode;
            _tail = newNode;
        }

        ++_nodeAmount;
        return newNode;
    }

    // 移除尾部
    void PopBack()
    {
        if(LIKELY(_tail))
        {
            auto preNode = _tail->_pre;
            ListNode<ObjType>::DeleteByAdapter_ListNode(BuildType::V, _tail);
            if(preNode)
                preNode->_next = NULL;

            _tail = preNode;
            --_nodeAmount;

            if(UNLIKELY(!_nodeAmount))
                _head = NULL;
        }
    }

    // 移除头部
    void PopFront()
    {
        if(LIKELY(_head))
        {
            auto nextNode = _head->_next;
            ListNode<ObjType>::DeleteByAdapter_ListNode(BuildType::V, _head);
            if(nextNode)
                nextNode->_pre = NULL;

            _head = nextNode;
            --_nodeAmount;
            if(UNLIKELY(!_nodeAmount))
                _tail = NULL;
        }
    }
    
    // 移除
    ListNode<ObjType> *Erase(ListNode<ObjType> *node)
    {
        auto preNode = node->_pre;
        auto nextNode = node->_next;

        if(preNode)
            preNode->_next = nextNode;
        if(nextNode)
            nextNode->_pre = preNode;

        --_nodeAmount;
        if(UNLIKELY(_head == node))
            _head = nextNode;
        if(UNLIKELY(_tail == node))
            _tail = preNode;

        ListNode<ObjType>::DeleteByAdapter_ListNode(BuildType::V, node);

        return nextNode;
    }

    // 头
    ListNode<ObjType> *Begin()
    {
        return _head;
    }
    const ListNode<ObjType> *Begin() const
    {
        return _head;
    }

    // 尾
    ListNode<ObjType> *End()
    {
        return _tail;
    }
    const ListNode<ObjType> *End() const
    {
        return _tail;
    }

    // 清理数据
    void Clear()
    {
        for(auto head = _head; head;)
            head = _DeleteNode(head);

        _GiveUp();
    }

    UInt64 GetAmount() const
    {
        return _nodeAmount;
    }

    bool IsEmpty() const
    {
        return !_head;
    }

    // ElemMethod: LibString func(){};/or lambda with return LibString
    template<typename ElemMethod>
    LibString ToString(ElemMethod method) const
    {
        LibString info;
        info.AppendFormat("list size:%llu\n", GetAmount());
        info.AppendFormat("list:[");
        for(auto node = Begin(); node; node = node->_next)
            info.AppendFormat("%s", method(node->_data).c_str());
        info.AppendFormat("]");

        return info;
    }

protected:
    void _GiveUp()
    {
        _head = NULL;
        _tail = NULL;
        _nodeAmount = 0;
    }

    // 返回下个节点
    ListNode<ObjType> *_DeleteNode(ListNode<ObjType> *node)
    {
        auto nextNode = node->_next;
        ListNode<ObjType>::DeleteByAdapter_ListNode(BuildType::V, node);
        return nextNode;
    }

private:
    // 至少有一个tail节点用来做结尾标识
    ListNode<ObjType> *_head;
    ListNode<ObjType> *_tail;
    UInt64 _nodeAmount;
};

template<typename ObjType, typename BuildType>
POOL_CREATE_TEMPLATE_OBJ_DEFAULT_IMPL(LibList, ObjType, BuildType);

template<typename ObjType, typename BuildType>
POOL_CREATE_TEMPLATE_OBJ_DEFAULT_TL_IMPL(LibList, ObjType, BuildType);

KERNEL_END

#endif
