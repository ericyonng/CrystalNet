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
 * Date: 2022-06-24 12:12:50
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_SERVICE_COMMON_APPLICATION_SERVICE_COMMON_CONFIG_H__
#define __CRYSTAL_NET_SERVICE_COMMON_APPLICATION_SERVICE_COMMON_CONFIG_H__

#pragma once

#include <service_common/common/macro.h>
#include <kernel/comp/LibString.h>
#include <vector>

SERVICE_COMMON_BEGIN

struct ServiceItemConfig
{
    KERNEL_NS::LibString _serviceItemName;      // 子服务名
    Int32 _serviceItemType = 0;                 // 子服务类型
    Int32 _serviceId = 0;                       // 服务id
};

struct ServiceCommonConfig
{
    std::vector<KERNEL_NS::LibString> _activeServiceItemList;   // 激活的服务项如：Logic, DB, 等
};

SERVICE_COMMON_END

#endif
