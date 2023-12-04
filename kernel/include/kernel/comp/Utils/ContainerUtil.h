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
 * Date: 2021-01-11 01:15:29
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_CONTAINER_UTIL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_CONTAINER_UTIL_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/comp/AutoDel.h>
#include <kernel/comp/LibList.h>

#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <vector>
#include <deque>
#include <list>
#include <stack>
#include <queue>

KERNEL_BEGIN

class KERNEL_EXPORT ContainerUtil
{
public:
    // // 特定容器方法
    template<typename _Type, AutoDelMethods::Way DelType = AutoDelMethods::Delete>
    static void DelMapContainer(_Type &container);

    template<typename _Type, AutoDelMethods::Way DelType = AutoDelMethods::NoSafeDelete>
    static void DelSetContainer(_Type &container);

    template<typename _Type, AutoDelMethods::Way DelType = AutoDelMethods::Delete>
    static void DelListContainer(_Type &container);

    template<typename _Type, AutoDelMethods::Way DelType = AutoDelMethods::Delete>
    static void DelVectorContainer(_Type &container);

    template<typename _Type, AutoDelMethods::Way DelType = AutoDelMethods::Delete>
    static void DelArray(_Type &container, UInt64 arrSize);

    template<typename _Type, AutoDelMethods::Way DelType = AutoDelMethods::Delete>
    static void DelDeque(_Type &container);

    template<typename _Type, AutoDelMethods::Way DelType = AutoDelMethods::Delete>
    static void DelStack(_Type &container);

    template<typename _Type, AutoDelMethods::Way DelType = AutoDelMethods::Delete>
    static void DelQueue(_Type &container);
    

    // // 抽象统一接口 重载接口是在运行时匹配的所以避免在底层使用
    template<typename _Ty, AutoDelMethods::Way DelType = AutoDelMethods::Delete>
    static void DelContainer(std::vector<_Ty> &container);

    template<typename _Ty, AutoDelMethods::Way DelType = AutoDelMethods::Delete>
    static void DelContainer(std::list<_Ty> &container);

    template<typename _Ty, AutoDelMethods::Way DelType = AutoDelMethods::Delete>
    static void DelContainer(std::deque<_Ty> &container);

    template<typename _Ty, AutoDelMethods::Way DelType = AutoDelMethods::Delete>
    static void DelContainer(std::stack<_Ty> &container);

    template<typename _Ty, AutoDelMethods::Way DelType = AutoDelMethods::Delete>
    static void DelContainer(std::queue<_Ty> &container);

    template<typename _Ty, AutoDelMethods::Way DelType = AutoDelMethods::NoSafeDelete>
    static void DelContainer(std::set<_Ty> &container);

    template<typename _Ty, AutoDelMethods::Way DelType = AutoDelMethods::Delete >
    static void DelContainer(_Ty &container, UInt64 arrSize);

    template<typename _Key, typename _Ty, AutoDelMethods::Way DelType = AutoDelMethods::Delete>
    static void DelContainer(std::map<_Key, _Ty> &container);

    template<typename _Key, typename _Ty, AutoDelMethods::Way DelType = AutoDelMethods::Delete>
    static void DelContainer(std::unordered_map<_Key, _Ty> &container);

    template<typename _Ty, AutoDelMethods::Way DelType = AutoDelMethods::Delete>
    static void DelContainer(std::unordered_set<_Ty> &container);
    

    // // 特殊方法删除对象 TODO: 需要测试
    template<typename _Ty, typename DelFuncType>
    static void DelContainer(std::vector<_Ty> &container, DelFuncType delMethod);

    template<typename _Ty, typename DelFuncType>
    static void DelContainer(std::list<_Ty> &container, DelFuncType delMethod);

    template<typename _Ty, typename DelFuncType>
    static void DelContainer(std::deque<_Ty> &container, DelFuncType delMethod);

    template<typename _Ty, typename DelFuncType>
    static void DelContainer(std::stack<_Ty> &container, DelFuncType delMethod);

    template<typename _Ty, typename DelFuncType>
    static void DelContainer(std::queue<_Ty> &container, DelFuncType delMethod);

    template<typename _Ty, typename DelFuncType>
    static void DelContainer(std::set<_Ty> &container, DelFuncType delMethod);

    template<typename _Ty, typename DelFuncType>
    static void DelContainer(_Ty &container, UInt64 arrSize, DelFuncType delMethod);

    template<typename _Key, typename _Ty, typename DelFuncType>
    static void DelContainer(std::map<_Key, _Ty> &container, DelFuncType delMethod);

    template<typename _Ty, typename BuildType, typename DelFuncType>
    static void DelContainer(LibList<_Ty, BuildType> &container, DelFuncType delMethod);

    template<typename _Key, typename _Ty, typename DelFuncType>
    static void DelContainer(std::unordered_map<_Key, _Ty> &container, DelFuncType delMethod);

    template<typename _Ty, typename DelFuncType>
    static void DelContainer(std::unordered_set<_Ty> &container, DelFuncType delMethod);

    template<typename _Key, typename _Ty, typename DelFuncType>
    static void DelContainer(std::multimap<_Key, _Ty> &container, DelFuncType delMethod);

    template<typename _Ty, typename DelFuncType>
    static void DelContainer(std::multiset<_Ty> &container, DelFuncType delMethod);

    // // 抽象统一接口 重载接口是在运行时匹配的所以避免在底层使用
    template<typename _Ty, AutoDelMethods::Way DelType = AutoDelMethods::Release>
    static void DelContainer2(std::vector<_Ty> &container);

    template<typename _Ty, AutoDelMethods::Way DelType = AutoDelMethods::Release>
    static void DelContainer2(std::list<_Ty> &container);

    template<typename _Ty, AutoDelMethods::Way DelType = AutoDelMethods::Release>
    static void DelContainer2(std::deque<_Ty> &container);

    template<typename _Ty, AutoDelMethods::Way DelType = AutoDelMethods::Release>
    static void DelContainer2(std::stack<_Ty> &container);

    template<typename _Ty, AutoDelMethods::Way DelType = AutoDelMethods::Release>
    static void DelContainer2(std::queue<_Ty> &container);

    template<typename _Ty, AutoDelMethods::Way DelType = AutoDelMethods::Release>
    static void DelContainer2(std::set<_Ty> &container);

    template<typename _Ty, AutoDelMethods::Way DelType = AutoDelMethods::Release >
    static void DelContainer2(_Ty &container, UInt64 arrSize);

    template<typename _Key, typename _Ty, AutoDelMethods::Way DelType = AutoDelMethods::Release>
    static void DelContainer2(std::map<_Key, _Ty> &container);

    template<typename _Key, typename _Ty, AutoDelMethods::Way DelType = AutoDelMethods::Release>
    static void DelContainer2(std::unordered_map<_Key, _Ty> &container);

    template<typename _Ty, AutoDelMethods::Way DelType = AutoDelMethods::Release>
    static void DelContainer2(std::unordered_set<_Ty> &container);

};


template<typename _Type, AutoDelMethods::Way DelType>
ALWAYS_INLINE void ContainerUtil::DelMapContainer(_Type &container)
{
    AutoDel<DelType> delObj;
    for(auto iterElement = container.begin(); iterElement != container.end();)
    {
        delObj.Release(iterElement->second);
        iterElement = container.erase(iterElement);
    }
}

template<typename _Type, AutoDelMethods::Way DelType>
ALWAYS_INLINE void ContainerUtil::DelSetContainer(_Type &container)
{
    AutoDel<DelType> delObj;
    for(auto iterElement = container.begin(); iterElement != container.end(); )
    {
        delObj.Release(*iterElement);
        iterElement = container.erase(iterElement);
    }
}

template<typename _Type, AutoDelMethods::Way DelType>
ALWAYS_INLINE void ContainerUtil::DelListContainer(_Type &container)
{
    AutoDel<DelType> delObj;
    for(auto iterElement = container.begin(); iterElement != container.end();)
    {
        delObj.Release(*iterElement);
        iterElement = container.erase(iterElement);
    }
}

template<typename _Type, AutoDelMethods::Way DelType>
ALWAYS_INLINE void ContainerUtil::DelVectorContainer(_Type &container)
{
    AutoDel<DelType> delObj;
    Int64 count = static_cast<Int64>(container.size());
    for(Int64 idx = count - 1; idx >= 0; --idx)
    {
        auto iter = container.begin() + idx;
        delObj.Release(*iter);
        container.erase(iter);
    }
}


template<typename _Type, AutoDelMethods::Way DelType>
ALWAYS_INLINE void ContainerUtil::DelArray(_Type &container, UInt64 arrSize)
{
    AutoDel<DelType> delObj;
    for(UInt64 i = 0; i < arrSize; ++i)
        delObj.Release(container[i]);
}

template<typename _Type, AutoDelMethods::Way DelType>
ALWAYS_INLINE void ContainerUtil::DelDeque(_Type &container)
{
    AutoDel<DelType> delObj;
    Int64 count = static_cast<Int64>(container.size());
    for(Int64 idx = count - 1; idx >= 0; --idx)
    {
        auto iter = container.begin() + idx;
        delObj.Release(*iter);
        container.erase(iter);
    }
}

template<typename _Type, AutoDelMethods::Way DelType>
ALWAYS_INLINE void ContainerUtil::DelStack(_Type &container)
{
    AutoDel<DelType> delObj;
    while (!container.empty())
    {
        delObj.Release(container.top());
        container.pop();
    }
}

template<typename _Type, AutoDelMethods::Way DelType>
ALWAYS_INLINE void ContainerUtil::DelQueue(_Type &container)
{
    AutoDel<DelType> delObj;
    while (!container.empty())
    {
        delObj.Release(container.front());
        container.pop();
    }
}

template<typename _Ty, AutoDelMethods::Way DelType>
ALWAYS_INLINE void ContainerUtil::DelContainer(std::vector<_Ty> &container)
{
    ContainerUtil::DelVectorContainer<std::vector<_Ty>, DelType>(container);
}

template<typename _Ty, AutoDelMethods::Way DelType>
ALWAYS_INLINE void ContainerUtil::DelContainer(std::list<_Ty> &container)
{
    ContainerUtil::DelListContainer<std::list<_Ty>, DelType>(container);
}

template<typename _Ty, AutoDelMethods::Way DelType>
ALWAYS_INLINE void ContainerUtil::DelContainer(std::deque<_Ty> &container)
{
    ContainerUtil::DelDeque<std::deque<_Ty>, DelType>(container);
}

template<typename _Ty, AutoDelMethods::Way DelType>
ALWAYS_INLINE void ContainerUtil::DelContainer(std::stack<_Ty> &container)
{
    ContainerUtil::DelStack<std::stack<_Ty>, DelType>(container);
}

template<typename _Ty, AutoDelMethods::Way DelType>
ALWAYS_INLINE void ContainerUtil::DelContainer(std::queue<_Ty> &container)
{
    ContainerUtil::DelQueue<std::queue<_Ty>, DelType>(container);
}

template<typename _Ty, AutoDelMethods::Way DelType>
ALWAYS_INLINE void ContainerUtil::DelContainer(std::set<_Ty> &container)
{
    ContainerUtil::DelSetContainer<std::set<_Ty>, DelType>(container);
}

template<typename _Ty, AutoDelMethods::Way DelType>
ALWAYS_INLINE void ContainerUtil::DelContainer(_Ty &container, UInt64 arrSize)
{
    ContainerUtil::DelArray<_Ty, DelType>(container, arrSize);
}

template<typename _Key, typename _Ty, AutoDelMethods::Way DelType>
ALWAYS_INLINE void ContainerUtil::DelContainer(std::map<_Key, _Ty> &container)
{
    ContainerUtil::DelMapContainer<std::map<_Key, _Ty>, DelType>(container);
}

template<typename _Key, typename _Ty, AutoDelMethods::Way DelType>
ALWAYS_INLINE void ContainerUtil::DelContainer(std::unordered_map<_Key, _Ty> &container)
{
    AutoDel<DelType> delObj;
    for(auto iterElement = container.begin(); iterElement != container.end();)
    {
        delObj.Release(iterElement->second);
        iterElement = container.erase(iterElement);
    }
}

template<typename _Ty, AutoDelMethods::Way DelType>
ALWAYS_INLINE void ContainerUtil::DelContainer(std::unordered_set<_Ty> &container)
{
    AutoDel<DelType> delObj;
    for(auto iterElement = container.begin(); iterElement != container.end(); )
    {
        delObj.Release(*iterElement);
        iterElement = container.erase(iterElement);
    }
}

template<typename _Ty, typename DelFuncType>
ALWAYS_INLINE void ContainerUtil::DelContainer(std::vector<_Ty> &container, DelFuncType delMethod)
{
    Int64 count = static_cast<Int64>(container.size());
    for(Int64 idx = count - 1; idx >= 0; --idx)
    {
        auto iter = container.begin() + idx;
        delMethod(*iter);
        container.erase(iter);
    }
}

template<typename _Ty, typename DelFuncType>
ALWAYS_INLINE void ContainerUtil::DelContainer(std::list<_Ty> &container, DelFuncType delMethod)
{
    for(auto iterElement = container.begin(); iterElement != container.end();)
    {
        delMethod(*iterElement);
        iterElement = container.erase(iterElement);
    }
}

template<typename _Ty, typename DelFuncType>
ALWAYS_INLINE void ContainerUtil::DelContainer(std::deque<_Ty> &container, DelFuncType delMethod)
{
    Int64 count = static_cast<Int64>(container.size());
    for(Int64 idx = count - 1; idx >= 0; --idx)
    {
        auto iter = container.begin() + idx;
        delMethod(*iter);
        container.erase(iter);
    }
}

template<typename _Ty, typename DelFuncType>
ALWAYS_INLINE void ContainerUtil::DelContainer(std::stack<_Ty> &container, DelFuncType delMethod)
{
    while (!container.empty())
    {
        delMethod(container.top());
        container.pop();
    }
}

template<typename _Ty, typename DelFuncType>
inline void ContainerUtil::DelContainer(std::queue<_Ty> &container, DelFuncType delMethod)
{
    while (!container.empty())
    {
        delMethod(container.front());
        container.pop();
    }
}

template<typename _Ty, typename DelFuncType>
ALWAYS_INLINE void ContainerUtil::DelContainer(std::set<_Ty> &container, DelFuncType delMethod)
{
    for(auto iterElement = container.begin(); iterElement != container.end(); )
    {
        delMethod(*iterElement);
        iterElement = container.erase(iterElement);
    }
}

template<typename _Ty, typename DelFuncType>
ALWAYS_INLINE void ContainerUtil::DelContainer(_Ty &container, UInt64 arrSize, DelFuncType delMethod)
{
    for(Int32 i = 0; i < arrSize; ++i)
        delMethod(container[i]);
}

template<typename _Key, typename _Ty, typename DelFuncType>
ALWAYS_INLINE void ContainerUtil::DelContainer(std::map<_Key, _Ty> &container, DelFuncType delMethod)
{
    for(auto iterElement = container.begin(); iterElement != container.end();)
    {
        delMethod(iterElement->second);
        iterElement = container.erase(iterElement);
    }
}

template<typename _Ty, typename BuildType, typename DelFuncType>
ALWAYS_INLINE void ContainerUtil::DelContainer(LibList<_Ty, BuildType> &container, DelFuncType delMethod)
{
    for(auto node = container.Begin(); node;)
    {
        delMethod(node->_data);
        node = container.Erase(node);
    }
}

template<typename _Key, typename _Ty, typename DelFuncType>
ALWAYS_INLINE void ContainerUtil::DelContainer(std::unordered_map<_Key, _Ty> &container, DelFuncType delMethod)
{
    for(auto iterElement = container.begin(); iterElement != container.end();)
    {
        delMethod(iterElement->second);
        iterElement = container.erase(iterElement);
    }
}

template<typename _Ty, typename DelFuncType>
ALWAYS_INLINE void ContainerUtil::DelContainer(std::unordered_set<_Ty> &container, DelFuncType delMethod)
{
    for(auto iterElement = container.begin(); iterElement != container.end(); )
    {
        delMethod(*iterElement);
        iterElement = container.erase(iterElement);
    }
}

template<typename _Key, typename _Ty, typename DelFuncType>
ALWAYS_INLINE void ContainerUtil::DelContainer(std::multimap<_Key, _Ty> &container, DelFuncType delMethod)
{
    for(auto iterElement = container.begin(); iterElement != container.end();)
    {
        delMethod(iterElement->second);
        iterElement = container.erase(iterElement);
    }
}

template<typename _Ty, typename DelFuncType>
ALWAYS_INLINE void ContainerUtil::DelContainer(std::multiset<_Ty> &container, DelFuncType delMethod)
{
    for(auto iterElement = container.begin(); iterElement != container.end(); )
    {
        delMethod(*iterElement);
        iterElement = container.erase(iterElement);
    }
}

template<typename _Ty, AutoDelMethods::Way DelType>
ALWAYS_INLINE void ContainerUtil::DelContainer2(std::vector<_Ty> &container)
{
    DelContainer<_Ty, DelType>(container);
}

template<typename _Ty, AutoDelMethods::Way DelType>
ALWAYS_INLINE void ContainerUtil::DelContainer2(std::list<_Ty> &container)
{
    DelContainer<_Ty, DelType>(container);
}

template<typename _Ty, AutoDelMethods::Way DelType>
ALWAYS_INLINE void ContainerUtil::DelContainer2(std::deque<_Ty> &container)
{
    DelContainer<_Ty, DelType>(container);
}

template<typename _Ty, AutoDelMethods::Way DelType>
ALWAYS_INLINE void ContainerUtil::DelContainer2(std::stack<_Ty> &container)
{
    DelContainer<_Ty, DelType>(container);
}

template<typename _Ty, AutoDelMethods::Way DelType>
ALWAYS_INLINE void ContainerUtil::DelContainer2(std::queue<_Ty> &container)
{
    DelContainer<_Ty, DelType>(container);
}

template<typename _Ty, AutoDelMethods::Way DelType>
ALWAYS_INLINE void ContainerUtil::DelContainer2(std::set<_Ty> &container)
{
    DelContainer<_Ty, DelType>(container);
}

template<typename _Ty, AutoDelMethods::Way DelType>
ALWAYS_INLINE void ContainerUtil::DelContainer2(_Ty &container, UInt64 arrSize)
{
    DelContainer<_Ty, DelType>(container, arrSize);
}

template<typename _Key, typename _Ty, AutoDelMethods::Way DelType>
ALWAYS_INLINE void ContainerUtil::DelContainer2(std::map<_Key, _Ty> &container)
{
    DelContainer<_Key, _Ty, DelType>(container);
}

template<typename _Key, typename _Ty, AutoDelMethods::Way DelType>
ALWAYS_INLINE void ContainerUtil::DelContainer2(std::unordered_map<_Key, _Ty> &container)
{
    DelContainer<_Key, _Ty, DelType>(container);
}

template<typename _Ty, AutoDelMethods::Way DelType>
ALWAYS_INLINE void ContainerUtil::DelContainer2(std::unordered_set<_Ty> &container)
{
    DelContainer<_Ty, DelType>(container);
}

KERNEL_END


#endif