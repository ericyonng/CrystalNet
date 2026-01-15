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
 * Date: 2026-01-16 01:15:47
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <kernel/common/common.h>
#include <atomic>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

class LibThread;
class IApplication;

KERNEL_END

struct AccountData
{
    KERNEL_NS::LibString ip;
    Int32 port;
    KERNEL_NS::LibString Account;
    KERNEL_NS::LibString Pwd;
};

class Entry
{
public:
    static  KERNEL_NS::LibThread *EntryThread;
    static std::atomic<KERNEL_NS::IApplication *> Application;
    static std::atomic<SERVICE_COMMON_NS::IService *> Service;
    static AccountData AccountInfo;
    static bool Run();
};