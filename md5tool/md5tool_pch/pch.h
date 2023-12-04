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
 * Date: 2023-03-31 13:05:29
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CONFIG_EXPORTER_CONFIG_EXPORTER_PCH_PCH_H__
#define __CONFIG_EXPORTER_CONFIG_EXPORTER_PCH_PCH_H__

#include <kernel/common/macro.h>
#include <kernel/common/os_libs.h>
#include <kernel/common/statics.h>
#include <kernel/common/status.h>

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
// #define CRYSTAL_DEBUG_ENABLE 0

#ifdef _WIN32
#include "targetver.h"
#include "windows.h"
#endif

#include <kernel/kernel.h>
#include <3rd/3rd.h>
#include <service_common/ServiceCommon.h>

#endif

