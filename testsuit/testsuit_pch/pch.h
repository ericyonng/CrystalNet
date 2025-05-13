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
 * Date: 2020-10-11 23:10:29
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __TESTSUIT_TESTSUIT_PCH_PCH_H__
#define __TESTSUIT_TESTSUIT_PCH_PCH_H__

#define BSONCXX_POLY_USE_STD 1

#include <kernel/common/macro.h>
#include <kernel/common/os_libs.h>
#include <kernel/common/statics.h>
#include <kernel/common/status.h>

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#ifdef _WIN32
#include "targetver.h"
#include "windows.h"

#ifdef max
    #undef max  // 临时取消 max 宏定义
#endif
#ifdef min
    #undef min  // 临时取消 max 宏定义
#endif
#endif

#include <kernel/kernel.h>
#include <service_common/ServiceCommon.h>
#include <3rd/3rd.h>
// #include <simple_api/simple_api.h>

#endif

