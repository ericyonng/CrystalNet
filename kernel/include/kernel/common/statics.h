/*!
 * MIT License
 *  
 * Copyright (c) 2020 Eric Yonng<120453674@qq.com>
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *  
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *  
 * 
 * Date: 2020-10-11 16:30:56
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_STATICS_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_STATICS_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <string>

KERNEL_BEGIN

class ILog;
class MemoryPool;
class LibEventLoopThreadPool;

class KERNEL_EXPORT ConstantGather
{
public:
    static const std::string anyIp;
    static const std::string interfacePrefix;
};

KERNEL_END

// 底层默认认为g_Log已经在上层初始化过,所以可以安全使用
extern KERNEL_EXPORT KERNEL_NS::ILog *g_Log;
extern KERNEL_EXPORT KERNEL_NS::MemoryPool *g_MemoryPool;
extern KERNEL_EXPORT KERNEL_NS::LibEventLoopThreadPool *g_LibEventLoopThreadPool;

#endif