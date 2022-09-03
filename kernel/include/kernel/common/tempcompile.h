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
 * Author: Eric Yonng
 * Date: 2021-03-25 20:44:13
 * Description: 
 *              临时的选项,用于开发中,发布时请禁用
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_TEMP_COMPILE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_TEMP_COMPILE_H__

#pragma once

// 临时选项,发布请置0
#undef TEMP_COMPILE_OPTION
#define TEMP_COMPILE_OPTION 0

#if TEMP_COMPILE_OPTION

    #undef CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    #define CRYSTAL_TARGET_PLATFORM_NON_WINDOWS 1

    #undef CRYSTAL_TARGET_PLATFORM_LINUX
    #define CRYSTAL_TARGET_PLATFORM_LINUX 1

    #undef CRYSTAL_TARGET_PLATFORM_WINDOWS
    #define CRYSTAL_TARGET_PLATFORM_WINDOWS 1

#endif


#endif

