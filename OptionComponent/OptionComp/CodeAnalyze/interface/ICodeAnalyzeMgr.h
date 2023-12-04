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
 * Date: 2023-09-29 17:59:11
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_CODE_ANALYZE_INTERFACE_ICODE_ANALYZE_MGR_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_CODE_ANALYZE_INTERFACE_ICODE_ANALYZE_MGR_H__

#pragma once

#include <kernel/comp/CompObject/CompHostObject.h>
#include <kernel/comp/Delegate/LibDelegate.h>
#include <OptionComp/CodeAnalyze/impl/CodeUnit.h>

KERNEL_BEGIN

class ICodeAnalyzeMgr : public CompHostObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, ICodeAnalyzeMgr);
public:
    // 单行注释标记
    virtual void SetCommentFlag(const LibString &commentFlag) = 0;
    // 多行注释
    virtual void SetMultiCommentFlag(const LibString &startFlag, const LibString &endFlag) = 0;

    // 扫描
    virtual void ScanDir(const KERNEL_NS::LibString &dir, const std::set<LibString> &specifyFileExts, IDelegate<void, LibString &, Int32, const LibString &, const std::vector<LibString> &> *onScan, IDelegate<void> *onFileEnd) = 0;
    virtual void ScanFile(const KERNEL_NS::LibString &fullPath, IDelegate<void, LibString &, Int32, const LibString &, const std::vector<LibString> &> *onScan, IDelegate<void> *onFileEnd) = 0;

    template<typename CallbackType, typename FileEndCallbackType>
    void ScanDir(const KERNEL_NS::LibString &dir, const std::set<LibString> &specifyFileExts, CallbackType &&cb, FileEndCallbackType &&fileEndCb);
    template<typename CallbackType, typename FileEndCallbackType>
    void ScanFile(const KERNEL_NS::LibString &fullPath, CallbackType &&cb, FileEndCallbackType &&fileEndCb);

    // 添加单元
    virtual void AddCodeUnit(SmartPtr<CodeUnit, AutoDelMethods::Release> &codeUnit) = 0;
    // 查找所有单元
    virtual std::map<LibString, SmartPtr<CodeUnit, AutoDelMethods::Release>> &GetAllCodeUnits() = 0;
    virtual const std::map<LibString, SmartPtr<CodeUnit, AutoDelMethods::Release>> &GetAllCodeUnits() const = 0;
    // 查找单元
    virtual SmartPtr<CodeUnit, AutoDelMethods::Release> &GetCodeUnit(const LibString &codeUnitFullName) = 0;
    virtual const SmartPtr<CodeUnit, AutoDelMethods::Release> &GetCodeUnit(const LibString &codeUnitFullName) const = 0;

    // 获取单元堆栈
    virtual const SmartPtr<CodeUnitStack, AutoDelMethods::Release> &GetCodeUnitStack() const = 0;
    virtual SmartPtr<CodeUnitStack, AutoDelMethods::Release> &GetCodeUnitStack() = 0;

};

template<typename CallbackType, typename FileEndCallbackType>
ALWAYS_INLINE void ICodeAnalyzeMgr::ScanDir(const KERNEL_NS::LibString &dir, const std::set<LibString> &specifyFileExts, CallbackType &&cb, FileEndCallbackType &&fileEndCb)
{
    auto delg = KERNEL_CREATE_CLOSURE_DELEGATE(cb, void, LibString &, Int32, const LibString &, const std::vector<LibString> &);
    auto delg2 = KERNEL_CREATE_CLOSURE_DELEGATE(fileEndCb, void);
    ScanDir(dir, specifyFileExts, delg, delg2);
}

template<typename CallbackType, typename FileEndCallbackType>
ALWAYS_INLINE void ICodeAnalyzeMgr::ScanFile(const KERNEL_NS::LibString &fullPath, CallbackType &&cb, FileEndCallbackType &&fileEndCb)
{
    auto delg = KERNEL_CREATE_CLOSURE_DELEGATE(cb, void, LibString &, Int32, const LibString &, const std::vector<LibString> &);
    auto delg2 = KERNEL_CREATE_CLOSURE_DELEGATE(fileEndCb, void);
    ScanFile(fullPath, delg, delg2);
}

KERNEL_END

#endif