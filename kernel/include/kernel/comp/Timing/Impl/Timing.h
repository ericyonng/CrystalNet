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
 * Date: 2025-01-12 00:22:28
 * Author: Eric Yonng
 * Description: 不依赖系统授时组件
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TIMING_IMPL_TIMING_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TIMING_IMPL_TIMING_H__

#pragma once

#include <kernel/comp/CompObject/CompObject.h>
#include <kernel/comp/TimeSlice.h>
#include <kernel/comp/LibTime.h>

KERNEL_BEGIN

class LibTimer;

/**
 * 不依赖系统授时组件
 * 原理:
 *      1. 启动时候记录启动的时候的时间
 *      2. 外部可以通过设置刷新时间间隔
 *      3. 根据时间间隔会启动定时器, 并刷新当前时间(定时到后会加上一次的时间间隔, 不断往前)
 *      4. 授时服务一般在系统启动设置即可
 */
class KERNEL_EXPORT Timing : public CompObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, Timing);
    
public:
    Timing();
    ~Timing() override;
    void Release() override;

    /*
     * 重置授时
     * 1. 重设定时间隔
     * 2. 刷新当前时间
     * 3. 重新启动定时
     */
    void ReStart(const TimeSlice& ts);

    /**
     * 获取当前授时
     */
    const LibTime &GetTime() const;

private:
    virtual Int32 _OnStart() override;
    void _OnTick(LibTimer *t);
    
private:
    // 时间间隔
    TimeSlice _flushInterval;
    // 当前时间
    LibTime _currentTime;
    // 定时器
    LibTimer *_timer;
};

ALWAYS_INLINE const LibTime &Timing::GetTime() const
{
    return _currentTime;
}

KERNEL_END

#endif