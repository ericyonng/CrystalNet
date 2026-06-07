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
// Date: 2026-06-02 23:06:40
// Author: Eric Yonng
// Description:

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_INTERFACE_ICOMMAND_MGR_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_INTERFACE_ICOMMAND_MGR_H__

#pragma once

#include <kernel/comp/CompObject/CompObjectInc.h>
#include <kernel/comp/Delegate/IDelegate.h>

KERNEL_BEGIN

class ICommandMgr : public CompHostObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, ICommandMgr);
    
public:
    ICommandMgr(UInt64 typeId): CompHostObject(typeId) {}

    // 添加命令与回调(大小写不敏感, 均按小写处理) 在命令行处理线程执行回调
    virtual void AddCommand(const LibString &cmd, IDelegate<void> *callback) = 0;
    template <typename T>
    void AddCommand(const LibString &cmd, T *obj, void(T::*listener)());
    template <typename LambdaType>
    void AddCommand(const LibString &cmd, LambdaType &&lambda);

    // 添加命令, 使用正则表达式匹配(大小写不敏感, 均按小写处理) 在命令行处理线程执行回调
    virtual void AddRegularCommand(const LibString &cmd, IDelegate<void, const KERNEL_NS::LibString &> *callback) = 0;
    template <typename T>
    void AddRegularCommand(const LibString &cmd, T *obj, void(T::*listener)(const KERNEL_NS::LibString &));
    template <typename LambdaType>
    void AddRegularCommand(const LibString &cmd, LambdaType &&lambda);
};

template <typename T>
ALWAYS_INLINE void ICommandMgr::AddCommand(const LibString &cmd, T *obj, void(T::*callback)())
{
    auto deleg = KERNEL_NS::DelegateFactory::Create(obj, callback);
    AddCommand(cmd, deleg);
}

template <typename LambdaType>
ALWAYS_INLINE void ICommandMgr::AddCommand(const LibString &cmd, LambdaType &&lambda)
{
    auto deleg = KERNEL_CREATE_CLOSURE_DELEGATE(lambda, void);
    AddCommand(cmd, deleg);
}

template <typename T>
ALWAYS_INLINE void ICommandMgr::AddRegularCommand(const LibString &cmd, T *obj, void(T::*listener)(const KERNEL_NS::LibString &))
{
    auto deleg = KERNEL_NS::DelegateFactory::Create(obj, listener);
    AddRegularCommand(cmd, deleg);
}

template <typename LambdaType>
ALWAYS_INLINE void ICommandMgr::AddRegularCommand(const LibString &cmd, LambdaType &&lambda)
{
    auto deleg = KERNEL_CREATE_CLOSURE_DELEGATE(lambda, void,  const KERNEL_NS::LibString &);
    AddRegularCommand(cmd, deleg);
}

KERNEL_END

#endif