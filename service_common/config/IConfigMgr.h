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
 * Date: 2023-03-26 22:40:36
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_SERVICE_COMMON_CONFIG_ICONFIG_MGR_H__
#define __CRYSTAL_NET_SERVICE_COMMON_CONFIG_ICONFIG_MGR_H__

#pragma once

#include <service_common/common/common.h>
#include <kernel/kernel.h>

SERVICE_COMMON_BEGIN

class IConfigMgr : public KERNEL_NS::CompObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, IConfigMgr);

public:
    IConfigMgr();
    ~IConfigMgr();

    // virtual KERNEL_NS::LibString ToString() const override;
    // virtual void Clear() override;

    virtual Int32 Load() = 0;
    virtual Int32 Reload() = 0;
    virtual const std::vector<KERNEL_NS::LibString> &GetAllConfigFiles() const = 0;
    virtual const KERNEL_NS::LibString &GetConfigDataMd5() const = 0;

protected:
    // virtual void _OnClose() override;
    // void _Clear();
};

SERVICE_COMMON_END

#endif
