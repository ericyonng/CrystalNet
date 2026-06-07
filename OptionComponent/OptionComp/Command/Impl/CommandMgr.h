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

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_IMPL_COMMAND_MGR_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_IMPL_COMMAND_MGR_H__

#pragma once

#include <OptionComp/Command/Interface/ICommandMgr.h>

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

    std::unordered_map<UInt64, CommandThreadContainer> _threadIdRefCmdContainer;
    // std::unordered_map<LibString, IDelegate<void> *> _cmdRefCallback;
    // std::unordered_map<LibString, IDelegate<void, const KERNEL_NS::LibString &> *> _regularKeywordRefCallback;
    SpinLock _lck;
    KERNEL_NS::LibEventLoopThread *_eventLoopThread;
    std::atomic_bool _isWorking;
    std::atomic_bool _inWaiting;
};

KERNEL_END

#endif