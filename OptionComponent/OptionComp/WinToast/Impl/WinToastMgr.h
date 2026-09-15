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
 * Description: windows toast通知组件实现(仅windows平台生效)
 *              1.弹通知任务投递到内核全局线程池g_EventLoopHeavyTaskThreadPool执行, Notify不阻塞调用线程
 *              2.组件不创建任何线程
*/

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_OPTIONCOMP_WIN_TOAST_IMPL_WIN_TOAST_MGR_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_OPTIONCOMP_WIN_TOAST_IMPL_WIN_TOAST_MGR_H__

#pragma once

#include <OptionComp/WinToast/Interface/IWinToastMgr.h>

KERNEL_BEGIN

class WinToastMgr : public IWinToastMgr
{
    POOL_CREATE_OBJ_DEFAULT_P1(IWinToastMgr, WinToastMgr);

public:
    WinToastMgr();
    virtual ~WinToastMgr() override;

    void Release() override;

    // 设置AppUserModelId(windows toast通知要求), 需在组件Init之前调用, 默认:CrystalNet.WinToast
    virtual void SetAppUserModelId(const KERNEL_NS::LibString &aumid) override;

    // 弹右下角通知(异步非阻塞, 调用即返回), 标题使用默认值
    virtual void Notify(const KERNEL_NS::LibString &content) const override;

    // 弹右下角通知(异步非阻塞, 调用即返回), 自定义标题
    virtual void Notify(const KERNEL_NS::LibString &content, const KERNEL_NS::LibString &title) const override;

    // 初始化和关闭
protected:
    Int32 _OnInit() override;
    Int32 _OnStart() override;
    void _OnWillClose() override;
    void _OnClose() override;

private:
    void _Clear();

private:
    // 默认通知标题
    LibString _defaultTitle;
    // app user model id(windows toast通知要求)
    LibString _aumid;
    // 组件关闭标志(关闭后Notify直接丢弃)
    std::atomic_bool _closed;
    // toast上下文(定义在cpp, 隔离windows头文件, 与线程池任务共享所有权, 组件销毁后残留任务安全降级)
    void *_toastCtx;
};

KERNEL_END


#endif
