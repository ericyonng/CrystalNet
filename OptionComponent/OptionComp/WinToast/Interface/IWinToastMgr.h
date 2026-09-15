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
 * Date: 2026-09-15 10:00:00
 * Author: Eric Yonng
 * Description: windows toast通知组件接口
*/

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_OPTIONCOMP_WIN_TOAST_INTERFACE_IWIN_TOAST_MGR_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_OPTIONCOMP_WIN_TOAST_INTERFACE_IWIN_TOAST_MGR_H__

#pragma once

#include <kernel/comp/CompObject/CompObject.h>

KERNEL_BEGIN

class IWinToastMgr : public CompObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, IWinToastMgr);

public:
    IWinToastMgr(UInt64 objTypeId) : CompObject(objTypeId) {}
    virtual ~IWinToastMgr() override {}

    // 设置AppUserModelId(windows toast通知要求), 需在组件Init之前调用, 默认:CrystalNet.WinToast
    virtual void SetAppUserModelId(const KERNEL_NS::LibString &aumid) = 0;

    // 弹右下角通知(异步非阻塞, 调用即返回), 标题使用默认值
    virtual void Notify(const KERNEL_NS::LibString &content) const = 0;

    // 弹右下角通知(异步非阻塞, 调用即返回), 自定义标题
    virtual void Notify(const KERNEL_NS::LibString &content, const KERNEL_NS::LibString &title) const = 0;
};

KERNEL_END


#endif
