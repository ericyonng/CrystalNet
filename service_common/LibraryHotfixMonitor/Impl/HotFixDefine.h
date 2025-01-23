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
 * Date: 2025-01-23 15:55:06
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_SERVICE_COMMON_LIBRARY_HOTFIX_MONITOR_IMPL_HOTFIX_DEFINE_H__
#define __CRYSTAL_NET_SERVICE_COMMON_LIBRARY_HOTFIX_MONITOR_IMPL_HOTFIX_DEFINE_H__

#pragma once

#include <service_common/common/macro.h>
#include <kernel/comp/SmartPtr.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

class ShareLibraryLoader;

KERNEL_END

SERVICE_COMMON_BEGIN

 struct HotFixCommonParam
 {
    POOL_CREATE_OBJ_DEFAULT(HotFixCommonParam);

    KERNEL_NS::LibString ToString() const;
    
    KERNEL_NS::SmartPtr<KERNEL_NS::ShareLibraryLoader, KERNEL_NS::AutoDelMethods::Release> _shareLib;
    KERNEL_NS::LibString _hotfixKey;
 };

// 热更文件中参数名
// FilePath:...
// HotfixKey:...
 class HotfixParamName
 {
 public:
     static const KERNEL_NS::LibString FILE_PATH;
     static const KERNEL_NS::LibString HOTFIX_KEY;
 };

// 热更参数
using HotFixContainerElemType = KERNEL_NS::SmartPtr<SERVICE_COMMON_NS::HotFixCommonParam, KERNEL_NS::AutoDelMethods::CustomDelete>;

SERVICE_COMMON_END

#endif