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
 * Date: 2023-09-29 17:57:08
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_CODE_ANALYZE_IMPL_CODE_ANALYZE_MGR_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_CODE_ANALYZE_IMPL_CODE_ANALYZE_MGR_H__

#pragma once

#include <OptionComp/CodeAnalyze/interface/ICodeAnalyzeMgr.h>
#include <OptionComp/CodeAnalyze/impl/CodeUnit.h>

KERNEL_BEGIN

class CodeAnalyzeMgr : public ICodeAnalyzeMgr
{
    POOL_CREATE_OBJ_DEFAULT_P1(ICodeAnalyzeMgr, CodeAnalyzeMgr);

public:
    CodeAnalyzeMgr();
    ~CodeAnalyzeMgr();
    void Release() override;

public:
    // 单行注释标记
    virtual void SetCommentFlag(const LibString &commentFlag) override;
    // 多行注释
    virtual void SetMultiCommentFlag(const LibString &startFlag, const LibString &endFlag) override;

    // 扫描目录
    virtual void ScanDir(const KERNEL_NS::LibString &dir, const std::set<LibString> &specifyFileExts, IDelegate<void, LibString &, Int32, const LibString &, const std::vector<LibString> &> *onScan, IDelegate<void> *onFileEnd) override;
    virtual void ScanFile(const KERNEL_NS::LibString &fullPath, IDelegate<void, LibString &, Int32, const LibString &, const std::vector<LibString> &> *onScan, IDelegate<void> *onFileEnd) override;

    // 添加单元
    virtual void AddCodeUnit(SmartPtr<CodeUnit, AutoDelMethods::Release> &codeUnit) override;
    // 查找所有单元
    virtual std::map<LibString, SmartPtr<CodeUnit, AutoDelMethods::Release>> &GetAllCodeUnits() override;
    virtual const std::map<LibString, SmartPtr<CodeUnit, AutoDelMethods::Release>> &GetAllCodeUnits() const override;
    // 查找单元
    virtual SmartPtr<CodeUnit, AutoDelMethods::Release> &GetCodeUnit(const LibString &codeUnitFullName) override;
    virtual const SmartPtr<CodeUnit, AutoDelMethods::Release> &GetCodeUnit(const LibString &codeUnitFullName) const override;

    // 获取单元堆栈
    virtual const SmartPtr<CodeUnitStack, AutoDelMethods::Release> &GetCodeUnitStack() const override;
    virtual SmartPtr<CodeUnitStack, AutoDelMethods::Release> &GetCodeUnitStack() override;

    // 代码信息
    virtual LibString ToString() const override;

    OBJ_GET_OBJ_TYPEID_DECLARE();

protected:
    virtual Int32 _OnHostInit() override;

    void _OnScanFile(const KERNEL_NS::LibString &fullFilePath, IDelegate<void, LibString &, Int32, const LibString &, const std::vector<LibString> &> *onScan, IDelegate<void> *onFileEnd);
    void _GetValidRangeOfLineData(KERNEL_NS::LibString &lineData, size_t &startPos, size_t &endPos, bool &isInMultiComment);

private:
    KERNEL_NS::LibString _commentFlag;
    KERNEL_NS::LibString _multiCommentStartFlag;
    KERNEL_NS::LibString _multiCommentEndFlag;

    SmartPtr<CodeUnitStack, AutoDelMethods::Release> _codeUnitStack;
    std::map<LibString, SmartPtr<CodeUnit, AutoDelMethods::Release>> _fullNameRefCodeUnit;

    SmartPtr<CodeUnit, AutoDelMethods::Release> _nullPtr;
};

KERNEL_END

#endif