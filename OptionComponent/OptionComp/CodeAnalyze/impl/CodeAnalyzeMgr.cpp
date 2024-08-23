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
 * Date: 2023-09-29 17:57:14
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <OptionComp/CodeAnalyze/impl/CodeAnalyzeMgr.h>
#include <OptionComp/CodeAnalyze/impl/CodeAnalyzeMgrFactory.h>
#include <kernel/comp/Utils/FileUtil.h>
#include <kernel/comp/Utils/DirectoryUtil.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(ICodeAnalyzeMgr);
POOL_CREATE_OBJ_DEFAULT_IMPL(CodeAnalyzeMgr);

CodeAnalyzeMgr::CodeAnalyzeMgr()
:ICodeAnalyzeMgr(KERNEL_NS::RttiUtil::GetTypeId<CodeAnalyzeMgr>())
{

}

CodeAnalyzeMgr::~CodeAnalyzeMgr()
{

}

void CodeAnalyzeMgr::Release()
{
    CodeAnalyzeMgr::DeleteByAdapter_CodeAnalyzeMgr(CodeAnalyzeMgrFactory::_buildType.V, this);
}

void CodeAnalyzeMgr::SetCommentFlag(const LibString &commentFlag)
{
    _commentFlag = commentFlag;
}

void CodeAnalyzeMgr::SetMultiCommentFlag(const LibString &startFlag, const LibString &endFlag)
{
    _multiCommentStartFlag = startFlag;
    _multiCommentEndFlag = endFlag;
}

void CodeAnalyzeMgr::ScanDir(const KERNEL_NS::LibString &dir, const std::set<LibString> &specifyFileExts, IDelegate<void, LibString &, Int32, const LibString &, const std::vector<LibString> &> *onScan, IDelegate<void> *onFileEnd)
{
    g_Log->Custom("scan dir:%s, specifyFileExts:%s", dir.c_str(), KERNEL_NS::StringUtil::ToString(specifyFileExts, ',').c_str());
    auto cb = [&specifyFileExts, onScan, this, onFileEnd](const FindFileInfo &fileInfo, bool &isContinueCurrentDir)->bool{
        isContinueCurrentDir = true;
        const auto &fileExt = fileInfo._extension.strip();

        g_Log->Custom("scan file:%s", fileInfo.ToString().c_str());

        // 指定的后缀
        if(!specifyFileExts.empty())
        {
            if(fileExt.empty())
                return true;

            if(specifyFileExts.find(fileExt) == specifyFileExts.end())
                return true;
        }
        
        _OnScanFile(fileInfo._fullName, onScan, onFileEnd);

        return true;
    };
    DirectoryUtil::TraverseDirRecursively(dir, cb);

    if(LIKELY(onScan))
        onScan->Release();
}

void CodeAnalyzeMgr::ScanFile(const KERNEL_NS::LibString &fullPath, IDelegate<void, LibString &, Int32, const LibString &, const std::vector<LibString> &> *onScan, IDelegate<void> *onFileEnd)
{
    _OnScanFile(fullPath, onScan, onFileEnd);

    if(LIKELY(onScan))
        onScan->Release();
}

void CodeAnalyzeMgr::AddCodeUnit(SmartPtr<CodeUnit, AutoDelMethods::Release> &codeUnit)
{
    const auto &fullName = codeUnit->GetFullName();
    if(UNLIKELY(_fullNameRefCodeUnit.find(fullName) != _fullNameRefCodeUnit.end()))
        return;

    _fullNameRefCodeUnit.insert(std::make_pair(fullName, codeUnit));
    g_Log->Custom("add new code unit, full name:%s, unit name:%s file:%s, line:%d", codeUnit->GetFullName().c_str(), codeUnit->_unitName.c_str(), codeUnit->_fileName.c_str(), codeUnit->_line);
}

std::map<LibString, SmartPtr<CodeUnit, AutoDelMethods::Release>> &CodeAnalyzeMgr::GetAllCodeUnits()
{
    return _fullNameRefCodeUnit;
}

const std::map<LibString, SmartPtr<CodeUnit, AutoDelMethods::Release>> &CodeAnalyzeMgr::GetAllCodeUnits() const
{
    return _fullNameRefCodeUnit;
}

SmartPtr<CodeUnit, AutoDelMethods::Release> &CodeAnalyzeMgr::GetCodeUnit(const LibString &codeUnitFullName)
{
    auto iter = _fullNameRefCodeUnit.find(codeUnitFullName);
    return iter == _fullNameRefCodeUnit.end() ? _nullPtr : iter->second;
}

const SmartPtr<CodeUnit, AutoDelMethods::Release> &CodeAnalyzeMgr::GetCodeUnit(const LibString &codeUnitFullName) const
{
    auto iter = _fullNameRefCodeUnit.find(codeUnitFullName);
    return iter == _fullNameRefCodeUnit.end() ? _nullPtr : iter->second;
}

const SmartPtr<CodeUnitStack, AutoDelMethods::Release> &CodeAnalyzeMgr::GetCodeUnitStack() const
{
    return _codeUnitStack;
}

SmartPtr<CodeUnitStack, AutoDelMethods::Release> &CodeAnalyzeMgr::GetCodeUnitStack()
{
    return _codeUnitStack;
}

LibString CodeAnalyzeMgr::ToString() const
{
    LibString info;

    info.AppendFormat("code unit count:%lld", static_cast<Int64>(_fullNameRefCodeUnit.size()));
    for(auto iter : _fullNameRefCodeUnit)
    {
        auto &codeUnit = iter.second;
        info.AppendFormat("code unit full name:%s, unit name:%s, file:%s, line:%d\n", codeUnit->GetFullName().c_str(), codeUnit->_unitName.c_str(), codeUnit->_fileName.c_str(), codeUnit->_line);
    }

    return info;
}

Int32 CodeAnalyzeMgr::_OnHostInit()
{
    _codeUnitStack = CodeUnitStack::Create();

    return Status::Success;
}

void CodeAnalyzeMgr::_OnScanFile(const KERNEL_NS::LibString &fullFilePath, IDelegate<void, LibString &, Int32, const LibString &, const std::vector<LibString> &> *onScan, IDelegate<void> *onFileEnd)
{
    SmartPtr<FILE, AutoDelMethods::CustomDelete> fp = KERNEL_NS::FileUtil::OpenFile(fullFilePath.c_str(), false, "rb");
    if(!fp)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("OpenFile fail fullFilePath:%s"), fullFilePath.c_str());
        return;
    }

    fp.SetClosureDelegate([](void *p){
        FileUtil::CloseFile(*KernelCastTo<FILE>(p));
    });

    Int32 currentLine = -1;
    std::vector<LibString> lineDatas;

    bool isInMultiComment = false;
    while (!FileUtil::IsEnd(*fp))
    {
        KERNEL_NS::LibString lineData;
        KERNEL_NS::FileUtil::ReadUtf8OneLine(*fp, lineData);
        ++currentLine;
        lineDatas.push_back(lineData);

        lineData.strip();
        if(lineData.empty())
            continue;

        size_t startPos = 0;
        size_t endPos = lineData.length() - 1;

        while (startPos != std::string::npos)
        {
            _GetValidRangeOfLineData(lineData, startPos, endPos, isInMultiComment);

            if(startPos != std::string::npos && endPos != std::string::npos)
            {
                KERNEL_NS::LibString validPart = lineData.GetRaw().substr(startPos, endPos - startPos + 1);
                startPos = endPos;
                if(lineData.length() == startPos + 1)
                {
                    startPos = std::string::npos;
                }
                else
                {
                    startPos = startPos + 1;
                }

                if(LIKELY(onScan))
                    onScan->Invoke(validPart, currentLine, fullFilePath, lineDatas);
            }
        }
    }

    if(onFileEnd)
        onFileEnd->Invoke();
}

void CodeAnalyzeMgr::_GetValidRangeOfLineData(KERNEL_NS::LibString &lineData, size_t &startPos, size_t &endPos, bool &isInMultiComment)
{
    const auto commentStartPos = lineData.GetRaw().find(_commentFlag.GetRaw(), startPos);
    const auto multiCommentStartPos = lineData.GetRaw().find(_multiCommentStartFlag.GetRaw(), startPos);
    const auto multiCommentEndPos = lineData.GetRaw().find(_multiCommentEndFlag.GetRaw(), startPos);
    
    if(isInMultiComment)
    {
        // 多行注释还没结束
        if(multiCommentEndPos == std::string::npos)
        {
            startPos = std::string::npos;
            return;
        }

        isInMultiComment = false;

        startPos = multiCommentEndPos;
        if(lineData.length() == startPos + _multiCommentEndFlag.size())
            startPos = std::string::npos;
        else
            startPos = multiCommentEndPos + _multiCommentEndFlag.size();

        // 看有没有单行注释
        if(startPos != std::string::npos)
        {
            if((commentStartPos != std::string::npos) && (startPos >= commentStartPos))
                startPos = std::string::npos;
            else
            {
                if((multiCommentStartPos != std::string::npos) && 
                (commentStartPos != std::string::npos))
                {
                    if(multiCommentStartPos < commentStartPos)
                    {
                        endPos = multiCommentStartPos - 1;
                    }
                    else
                    {
                        endPos = commentStartPos - 1;
                    }
                }
                else if(multiCommentStartPos != std::string::npos)
                {
                    endPos = multiCommentStartPos - 1;
                }
                else if(commentStartPos != std::string::npos)
                {
                    endPos = commentStartPos - 1;
                }
                else
                {
                    endPos = lineData.length() - 1;
                }
            }
        }

        return;
    }

    // 当前不是多行注释模式
    // 多行和单行同时存在
    if((commentStartPos != std::string::npos) && (multiCommentStartPos != std::string::npos))
    {
        // 单行注释模式
        if(multiCommentStartPos > commentStartPos)
        {
            // 被注释了
            if(startPos >= commentStartPos)
            {
                startPos = std::string::npos;
                endPos = std::string::npos;
            }
            else
            {
                if(commentStartPos >= 1)
                    endPos = commentStartPos - 1;
                else
                {
                    startPos = std::string::npos;
                    endPos = std::string::npos;
                }
            }
        }
        else
        {
            // 多行模式
            if(startPos >= multiCommentStartPos)
            {
                startPos = std::string::npos;
                endPos = std::string::npos;
                isInMultiComment = true;

                if(multiCommentEndPos != std::string::npos)
                {
                    startPos = multiCommentEndPos;
                }
            }
            else
            {
                if(multiCommentStartPos >= 1)
                    endPos = multiCommentStartPos - 1;
                else
                {
                    startPos = std::string::npos;
                    endPos = std::string::npos;

                    isInMultiComment = true;

                    if(multiCommentEndPos != std::string::npos)
                    {
                        startPos = multiCommentEndPos;
                    }
                }
            }


        }

        return;
    }

    // 仅仅多行模式
    if(multiCommentStartPos != std::string::npos)
    {
        // 多行模式
        if(startPos >= multiCommentStartPos)
        {
            startPos = std::string::npos;
            endPos = std::string::npos;
            isInMultiComment = true;

            if(multiCommentEndPos != std::string::npos)
            {
                if(multiCommentEndPos + _multiCommentEndFlag.size() != lineData.length())
                {
                    startPos = multiCommentEndPos;
                }
            }
        }
        else
        {
            if(multiCommentStartPos >= 1)
                endPos = multiCommentStartPos - 1;
            else
            {
                startPos = std::string::npos;
                endPos = std::string::npos;
                isInMultiComment = true;

                if(multiCommentEndPos != std::string::npos)
                {
                    startPos = multiCommentEndPos;
                }
            }
        }

        return;
    }

    // 仅仅单行模式
    if(commentStartPos != std::string::npos)
    {
        if(startPos >= commentStartPos)
        {
            startPos = std::string::npos;
        }
        else
        {
            if(commentStartPos >= 1)
                endPos = commentStartPos - 1;
            else
            {
                startPos = std::string::npos;
                endPos = std::string::npos;
            }
        }

        return;
    }

    // 没有注释
    endPos = lineData.length() - 1;
}


KERNEL_END