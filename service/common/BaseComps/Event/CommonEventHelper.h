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
 * Date: 2026-11-06 14:38:00
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <service/common/macro.h>
#include <kernel/common/BaseMacro.h>

KERNEL_BEGIN

struct SessionCreatedEvent;
struct AsynConnectResEvent;
struct AddListenResEvent;
struct SessionDestroyEvent;

class EventManager;

KERNEL_END

SERVICE_BEGIN

class CommonEventHelper
{
public:
    static void FireCreateSessionEvent(KERNEL_NS::EventManager *eventMgr, KERNEL_NS::SessionCreatedEvent *sessionCreatedEv);
    static void FireAsynConnectRes(KERNEL_NS::EventManager *eventMgr, KERNEL_NS::AsynConnectResEvent *connectRes);
    static void FireAddListenRes(KERNEL_NS::EventManager *eventMgr, KERNEL_NS::AddListenResEvent *addListenEv);
    static void FireSessionDestroy(KERNEL_NS::EventManager *eventMgr, KERNEL_NS::SessionDestroyEvent *destroyEv);
    
};

SERVICE_END