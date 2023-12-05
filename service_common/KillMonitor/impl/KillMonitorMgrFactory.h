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
 * Date: 2023-07-01 14:33:33
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_SERVICE_COMMON_KILL_MONITOR_IMPL_KILL_MONITOR_FACTORY_H__
#define __CRYSTAL_NET_SERVICE_COMMON_KILL_MONITOR_IMPL_KILL_MONITOR_FACTORY_H__

#pragma once

#include <service_common/common/macro.h>
#include <kernel/common/LibObject.h>
#include <kernel/comp/CompObject/CompFactory.h>
#include <kernel/comp/CompObject/CompObject.h>

SERVICE_COMMON_BEGIN

class KillMonitorMgrFactory : public KERNEL_NS::CompFactory
{
    // 创建factory对象时候使用创建的方法类型
public:
    static constexpr KERNEL_NS::_Build::TL _buildType{};

    static KERNEL_NS::CompFactory *FactoryCreate();

    virtual void Release() override;

public:
    virtual KERNEL_NS::CompObject *Create() const;
};

SERVICE_COMMON_END

#endif
