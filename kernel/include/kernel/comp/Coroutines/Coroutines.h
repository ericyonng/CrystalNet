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
 * Date: 2024-08-04 16:50:46
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_COROUTINES_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COROUTINES_COROUTINES_H__

#pragma once

#include <kernel/comp/Coroutines/CoTools.h>
#include <kernel/comp/Coroutines/Invocable.h>
#include <kernel/comp/Coroutines/CoTask.h>
#include <kernel/comp/Coroutines/Concept/Awaitable.h>
#include <kernel/comp/Coroutines/Concept/Future.h>
#include <kernel/comp/Coroutines/Concept/Promise.h>
#include <kernel/comp/Coroutines/AsyncTask.h>
#include <kernel/comp/Coroutines/CallStack.h>
#include <kernel/comp/Coroutines/CoDelay.h>
#include <kernel/comp/Coroutines/CoHandle.h>
#include <kernel/comp/Coroutines/CoResult.h>
#include <kernel/comp/Coroutines/Exceptions.h>
#include <kernel/comp/Coroutines/Gather.h>
#include <kernel/comp/Coroutines/Runner.h>
#include <kernel/comp/Coroutines/ScheduledTask.h>
#include <kernel/comp/Coroutines/VoldValue.h>
#include <kernel/comp/Coroutines/CoWaiter.h>
#include <kernel/comp/Coroutines/CoTaskParam.h>

#endif
