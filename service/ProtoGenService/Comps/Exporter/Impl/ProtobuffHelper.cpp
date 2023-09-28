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
 * Date: 2022-10-28 22:28:20
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <service/ProtoGenService/Comps/Exporter/Impl/ProtobuffHelper.h>
#include <service/ProtoGenService/Comps/Exporter/Defs/Defs.h>

SERVICE_BEGIN

KERNEL_NS::LibString ProtobuffHelper::DragMessageSeg(const KERNEL_NS::LibString &lineData)
{
    if(!lineData.IsStartsWith(ProtobufMessageParam::Message))
        return KERNEL_NS::LibString();

    auto cache = lineData.strip();
    auto dragMessageName = cache.DragAfter(ProtobufMessageParam::Message);
    dragMessageName.strip();

    // 移除注释
    auto sepPos = dragMessageName.GetRaw().find('/');
    if(sepPos != std::string::npos)
    {
        // 注释格式错误
        auto doubleSep =  dragMessageName.GetRaw().find("//");
        if(doubleSep == std::string::npos)
            return KERNEL_NS::LibString();

        dragMessageName = dragMessageName.GetRaw().substr(0, sepPos);
    }

    dragMessageName.strip();

    // 从非命名格式字符断开
    const Int32 len = static_cast<Int32>(dragMessageName.size());
    Int32 maxValidLen = len;
    for(Int32 idx = 0; idx < len; ++idx)
    {
        if(ProtobuffHelper::CheckValidName(dragMessageName[idx]))
            continue;

        maxValidLen = idx;
        break;
    }
    
    if(maxValidLen == 0)
        return KERNEL_NS::LibString();
    
    return dragMessageName.GetRaw().substr(0, maxValidLen);
}

KERNEL_NS::LibString ProtobuffHelper::DragMessageSeg(const KERNEL_NS::LibString &lineData, Int32 &endPos)
{
    if(!lineData.IsStartsWith(ProtobufMessageParam::Message))
        return KERNEL_NS::LibString();

    auto cache = lineData.strip();
    auto dragMessageName = cache.DragAfter(ProtobufMessageParam::Message);
    dragMessageName.strip();

    // 移除注释
    auto sepPos = dragMessageName.GetRaw().find('/');
    if(sepPos != std::string::npos)
    {
        // 注释格式错误
        auto doubleSep =  dragMessageName.GetRaw().find("//");
        if(doubleSep == std::string::npos)
            return KERNEL_NS::LibString();

        dragMessageName = dragMessageName.GetRaw().substr(0, sepPos);
    }

    dragMessageName.strip();

    // 从非命名格式字符断开
    const Int32 len = static_cast<Int32>(dragMessageName.size());
    Int32 maxValidLen = len;
    for(Int32 idx = 0; idx < len; ++idx)
    {
        if(ProtobuffHelper::CheckValidName(dragMessageName[idx]))
            continue;

        maxValidLen = idx;
        break;
    }
    
    if(maxValidLen == 0)
        return KERNEL_NS::LibString();
    
    return dragMessageName.GetRaw().substr(0, maxValidLen);
}

KERNEL_NS::LibString ProtobuffHelper::DragClass(const KERNEL_NS::LibString &lineData)
{
    if(!lineData.IsStartsWith(ProtobufMessageParam::ClassFlag))
        return KERNEL_NS::LibString();

    auto cache = lineData.strip();
    auto dragMessageName = cache.DragAfter(ProtobufMessageParam::ClassFlag);
    if (dragMessageName.empty())
        return dragMessageName;

    // 剔除基类
    auto pos = dragMessageName.GetRaw().find(":");
    if (pos != std::string::npos)
        dragMessageName = dragMessageName.GetRaw().substr(0, pos);

    // 第一个空格符是类名的结束
    dragMessageName.strip();
    pos = dragMessageName.GetRaw().find(" ");
    if(pos != std::string::npos)
        dragMessageName = dragMessageName.GetRaw().substr(0, pos);

    dragMessageName.strip();
    return dragMessageName;
}

bool ProtobuffHelper::HasMessage(const KERNEL_NS::LibString &lineData)
{
    auto cache = lineData.strip();
    return cache.IsStartsWith(ProtobufMessageParam::Message);
}

bool ProtobuffHelper::CheckValidMessage(const KERNEL_NS::LibString &messageName)
{
    if(messageName.empty())
        return false;

    if(KERNEL_NS::LibString::isdigit(messageName.at(0)))
        return false;

    const Int32 len = static_cast<Int32>(messageName.length());
    for(Int32 idx = 0; idx < len; ++idx)
    {
        bool isValid = false;
        const Byte8 ch = (messageName.data())[idx];
        if(!isValid && (KERNEL_NS::LibString::isdigit(ch)))
            isValid = true;
        if(!isValid && KERNEL_NS::LibString::isalpha(ch))
            isValid = true;
        if(!isValid && ch == '_')
            isValid = true;

        if(!isValid)
            return false;
    }

    return true;
}

bool ProtobuffHelper::CheckValidName(Byte8 ch)
{
    if(KERNEL_NS::LibString::isdigit(ch))
        return true;

    if(KERNEL_NS::LibString::isalpha(ch))
        return true;

    if(ch == '_')
        return true;

    return false;
}

bool ProtobuffHelper::HasAnnotation(const KERNEL_NS::LibString &lineData)
{
    return lineData.IsStartsWith(ProtobufMessageParam::ParamLineBegin);
}

bool ProtobuffHelper::Modifylines(std::vector<KERNEL_NS::LibString> &lines
    , std::vector<KERNEL_NS::LibString> &addLines
    , std::function<bool(Int32 curLine, KERNEL_NS::LibString &lineData
    , std::vector<KERNEL_NS::LibString> &addLinesBefore // 默认为空
    , std::vector<KERNEL_NS::LibString> &addLinesAfter  // 默认把 addLines 添加到 addLinesAfter
    , bool &isContinue)> &&matchFun)
{
    Int32 maxLine = static_cast<Int32>(lines.size());
    bool isMatched = false;
    for(Int32 idx = 0; idx < maxLine; ++idx)
    {
        bool isContinue = false;
        std::vector<KERNEL_NS::LibString> addLinesBefore;
        std::vector<KERNEL_NS::LibString> addLinesAfter = addLines;

        if(matchFun(idx, lines[idx], addLinesBefore, addLinesAfter, isContinue))
        {
            isMatched = true;
            if(addLinesBefore.empty() && addLinesAfter.empty())
            {
                if(isContinue)
                    continue;

                return true;
            }

            lines.reserve(maxLine + addLinesBefore.size() + addLinesAfter.size());
            if(!addLinesBefore.empty())
            {
                lines.insert(lines.begin() + idx, addLinesBefore.begin(), addLinesBefore.end());
                idx += static_cast<Int32>(addLinesBefore.size());
                maxLine += static_cast<Int32>(addLinesBefore.size());
            }

            if(!addLinesAfter.empty())
            {
                if(idx + 1 == maxLine)
                {
                    for(auto &addLineData : addLinesAfter)
                        lines.emplace_back(addLineData);
                }
                else
                {
                    lines.insert(lines.begin() + idx + 1, addLinesAfter.begin(), addLinesAfter.end());
                }

                idx += static_cast<Int32>(addLinesAfter.size());
            }

            maxLine = static_cast<Int32>(lines.size());

            if(isContinue)
                continue;

            return true;
        }
    }

    return isMatched;
}

bool ProtobuffHelper::Modifylines(std::vector<KERNEL_NS::LibString> &lines, 
    std::function<bool(Int32 curLine, KERNEL_NS::LibString &lineData, bool &isContinue)> &&matchFun)
{
    const Int32 maxLine = static_cast<Int32>(lines.size());
    bool isMatched = false;
    for(Int32 idx = 0; idx < maxLine; ++idx)
    {
        bool isContinue = false;
        if(matchFun(idx, lines[idx], isContinue))
        {
            isMatched = true;
            if(isContinue)
                continue;

            return true;
        }
    }

    return isMatched;
}

void ProtobuffHelper::ModifyClassDeclear(std::vector<KERNEL_NS::LibString> &lines, std::function<bool(KERNEL_NS::LibString &lineData)> &&modifyFunc)
{
    for(auto &lineData : lines)
    {
        if(!modifyFunc(lineData))
            return;
    }
}

bool ProtobuffHelper::SearchClassBodyBegin(KERNEL_NS::LibString &lineData, std::function<void(Int32 index, KERNEL_NS::LibString &lineData)> &&successHandler)
{
    // 跳过注释
    const Int32 len = static_cast<Int32>(lineData.size());
    const auto &raw = lineData.GetRaw();
    for(Int32 idx = len - 1; idx >= 0; --idx)
    {
        const auto &ch = raw[idx];
        if(ch == '{')
        {
            successHandler(idx, lineData);
            return true;
        }

        if(ch == '/')
        {// 可能是注释
            if(idx == 0)
                return false;
            
            if(raw[idx - 1] != '*' && raw[idx - 1] != '/')
                return false;

            if(raw[idx -1] == '*')
            {// 找到/*
                auto nextIdx = raw.rfind("/*");
                if(nextIdx == std::string::npos)
                    return false;

                idx = static_cast<Int32>(nextIdx);
                continue;
            }

            // 找到第一个不是'/'
            for(Int32 nextIdx = idx - 1; nextIdx >= 0; --nextIdx)
            {
                if(raw[nextIdx] == '/')
                    continue;

                idx = nextIdx + 1;
                break;
            }

            continue;
        }
    }

    return false;
}

KERNEL_NS::LibString ProtobuffHelper::TreatCsharpName(const KERNEL_NS::LibString &protoName)
{
    // _分割线
    auto parts = protoName.Split('_');
    if(parts.empty())
        return protoName;

    KERNEL_NS::LibString newName;
    for(auto &part : parts)
    {
        if(part.empty())
            continue;

        newName.AppendFormat("%s", KERNEL_NS::LibString(part[0]).toupper().c_str());
        if(static_cast<Int32>(part.size()) >= 2)
            newName.AppendFormat("%s", part.GetRaw().substr(1).c_str());
    }

    return newName;
}

bool ProtobuffHelper::IsNoteLine(const KERNEL_NS::LibString &lineData)
{
    static const std::regex pattern = std::regex("( |\\t|\\v|\\r|\\n|\\f)*//.*");

    return std::regex_match(lineData.GetRaw(), pattern);
}

bool ProtobuffHelper::GetPackageName(const KERNEL_NS::LibString &lineData, KERNEL_NS::LibString &packageName, const KERNEL_NS::LibString &replaceDotWith)
{
    static const KERNEL_NS::LibString packageFlag = "package ";
    const auto &leftLineData = lineData.strip();
    if(leftLineData.size() < packageFlag.size())
        return false;

    if(leftLineData.GetRaw().substr(0, packageFlag.size()) != packageFlag.GetRaw())
        return false;

    packageName = leftLineData.GetRaw().substr(packageFlag.size());
    packageName.strip();

    // 英文下划线数值
    const Int32 count = static_cast<Int32>(packageName.size());
    Int32 lastValidIndex = -1;
    for(Int32 idx = 0; idx < count; ++idx)
    {
        const Byte8 ch = packageName[idx];
        if(ch != '.' && !CheckValidName(ch))
            break;

        lastValidIndex = idx;
    }

    if(lastValidIndex < 0)
    {
        packageName.clear();
        return false;
    }

    packageName = packageName.GetRaw().substr(0, lastValidIndex + 1);
    packageName.strip();
    packageName.strip('.');
    packageName.findreplace(".", replaceDotWith);
    return true;
}

void ProtobuffHelper::RemoveNotePart(KERNEL_NS::LibString &lineData)
{
    // 剔除多行注释
    bool isNoteOn = false;
    Int32 startPos = 0;
    while(true)
    {
        if(startPos == lineData.length())
            break;

        auto multiStart = lineData.GetRaw().find("/*", startPos);
        if(multiStart != std::string::npos)
        {
            isNoteOn = true;

            startPos = multiStart + 1;
            if(lineData.length() == multiStart + 1)
                break;

            auto endFlagPos = lineData.GetRaw().find("*/", startPos);
            if(endFlagPos == std::string::npos)
                break;

            isNoteOn = false;
            startPos = endFlagPos + 1;
        }
        else
        {
            break;
        }
    }

    if(isNoteOn)
    {
        lineData.clear();
        return;
    }

    // TODO:
    lineData.GetRaw().substr();

    auto pos = lineData.GetRaw().find("//");
    if(pos == std::string::npos)
        return;

    lineData = lineData.GetRaw().substr(0, pos);
}

SERVICE_END
