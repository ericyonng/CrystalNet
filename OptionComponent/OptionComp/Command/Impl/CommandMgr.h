// MIT License
// 
// Copyright (c) 2020 ericyonng<120453674@qq.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// Date: 2026-06-02 23:06:04
// Author: Eric Yonng
// Description:
// 1.windows 下 使用std::getline监听控制台
// 2.linux 下则监听文件


#ifndef __CRYSTAL_NET_OPTION_COMPONENT_IMPL_COMMAND_MGR_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_IMPL_COMMAND_MGR_H__

#pragma once

#include <OptionComp/Command/Interface/ICommandMgr.h>
#include <kernel/comp/Coroutines/CoTask.h>
#include <kernel/comp/Timer/LibTimer.h>

KERNEL_BEGIN

class LibEventLoopThread;

struct CommandThreadContainer
{
    POOL_CREATE_OBJ_DEFAULT(CommandThreadContainer);
    
    CommandThreadContainer(){}
    ~CommandThreadContainer();

    bool IsNoRelease = false;
    
    std::unordered_map<LibString, IDelegate<void> *> _cmdRefCallback;
    std::unordered_map<LibString, IDelegate<void, const KERNEL_NS::LibString &> *> _regularKeywordRefCallback;
};

class CommandMgr : public ICommandMgr
{
    POOL_CREATE_OBJ_DEFAULT_P1(ICommandMgr, CommandMgr);
    
public:
    CommandMgr();
    virtual ~CommandMgr() override;
    virtual void Release() override;
    virtual void DefaultMaskReady(bool isReady) override;

    // windows下是指令, linux下由于脱离控制台所以用文件代替(linux下扫描文件名作为cmd)
    virtual void AddCommand(const LibString &cmd, IDelegate<void> *callback) override;
    virtual void AddRegularCommand(const LibString &cmd, IDelegate<void, const KERNEL_NS::LibString &> *callback) override;

private:
    virtual Int32 _OnHostInit() override;
    virtual Int32 _OnCompsCreated() override;
    virtual Int32 _OnHostWillStart() override;

    virtual Int32 _OnHostStart() override;
    virtual void _OnHostBeforeCompsWillClose() override;
    virtual void _OnHostClose() override;
    void _Clear();

    void _WakeupCin() const;

    KERNEL_NS::CoTask<> _WindowsWork();

    // 定时扫描, 在Linux下启动
    void _OnTimeOut(KERNEL_NS::LibTimer *t);
    
    std::unordered_map<UInt64, CommandThreadContainer> _threadIdRefCmdContainer;
    
    SpinLock _lck;
    // windows下使用线程
    KERNEL_NS::LibEventLoopThread *_eventLoopThread;
    std::atomic_bool _isWorking;
    std::atomic_bool _inWaiting;
    bool _isLinuxMode;

    // linux下使用定时器, 在主线程上定时扫描
    KERNEL_NS::LibTimer *_timer;
    KERNEL_NS::LibString _scanPath;
};

KERNEL_END

#endif