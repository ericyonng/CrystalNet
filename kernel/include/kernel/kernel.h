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
 * Date: 2020-10-06 16:52:12
 * Author: Eric Yonng
 * Description: 
 *              1. 底层不提供单例对象，需要应用层自己实现单例，避免出现性能瓶颈（单例容易锁冲突）
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_KERNEL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_KERNEL_H__

// 导出接口宏
#include<kernel/kernel_export.h>

// 导入接口
#ifdef CRYSTAL_NET_IMPORT_KERNEL_LIB
    // 第三方库导出
    // #include <3rd/tiny-utf8/include/tinyutf8.h>
    // 公共资源
    #include <kernel/common/common.h>
    // 组件资源
    #include "kernel/comp/comp.h"
#endif


#endif