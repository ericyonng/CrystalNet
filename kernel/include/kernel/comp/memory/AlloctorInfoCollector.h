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
 * Date: 2021-01-12 01:54:31
 * Author: Eric Yonng
 * Description: 通过对象池宏留下的继承链来收集整个继承关系的对象分配信息,可以帮助分析对象分配情况
 * 收集信息包括：
 * 1.对象名
 * 2.对象池总占用
 * 3.对象池对象历史分配总个数 反应频繁度
 * 4.对象池对象历史释放总个数 反应频繁度
 * 5.对象池当前已分配的block个数 分析是否有泄漏风险
 * 6.对象池当前buffer总个数 反应已经分配多少个buffer,以备后期调整策略
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_ALLOCTOR_INFO_COLLECTOR_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_ALLOCTOR_INFO_COLLECTOR_H__

#pragma once

#include <kernel/comp/LibString.h>

KERNEL_BEGIN

class KERNEL_EXPORT AlloctorInfoCollector
{
public:
    AlloctorInfoCollector();
    ~AlloctorInfoCollector();

public:
    template <typename ObjType>
    void Collect(ObjType &alloctor);

    void AddDecorate(const LibString &str);

    const LibString &Result() const;

private:
    LibString _collector;
};

template <typename ObjType>
ALWAYS_INLINE void AlloctorInfoCollector::Collect(ObjType &alloctor)
{
    // 收集信息
    _collector.AppendFormat("[%s]", alloctor.UsingInfo().c_str());
}

ALWAYS_INLINE void AlloctorInfoCollector::AddDecorate(const LibString &str)
{
    _collector += str;
}

ALWAYS_INLINE const LibString &AlloctorInfoCollector::Result() const
{
    return _collector;
}

KERNEL_END


#endif

