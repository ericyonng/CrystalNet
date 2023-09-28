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
 * Date: 2022-10-28 22:28:07
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <service/ProtoGenService/ServiceCompHeader.h>

SERVICE_BEGIN

class ProtobuffHelper
{
public:
    // 提取message名
    static KERNEL_NS::LibString DragMessageSeg(const KERNEL_NS::LibString &lineData);
    static KERNEL_NS::LibString DragMessageSeg(const KERNEL_NS::LibString &lineData, Int32 &endPos);
    static KERNEL_NS::LibString DragClass(const KERNEL_NS::LibString &lineData);
    // 有没有message
    static bool HasMessage(const KERNEL_NS::LibString &lineData);
    // 检查message名
    static bool CheckValidMessage(const KERNEL_NS::LibString &messageName);
    static bool CheckValidName(Byte8 ch);
    // 有没有注解
    static bool HasAnnotation(const KERNEL_NS::LibString &lineData);

    // 满足匹配规则的下行插入
    static bool Modifylines(std::vector<KERNEL_NS::LibString> &lines, std::vector<KERNEL_NS::LibString> &addLines
    , std::function<bool(Int32 curLine, KERNEL_NS::LibString &lineData
    , std::vector<KERNEL_NS::LibString> &addLinesBefore // 默认为空
    , std::vector<KERNEL_NS::LibString> &addLinesAfter  // 默认把 addLines 添加到 addLinesAfter
    , bool &isContinue)> &&matchFun);

    static bool Modifylines(std::vector<KERNEL_NS::LibString> &lines, 
    std::function<bool(Int32 curLine, KERNEL_NS::LibString &lineData, bool &isContinue)> &&matchFun);

    // 修改类声明 std::function: return(bool):是否继续, KERNEL_NS::LibString &lineData是每行的数据，可以在回调函数中自行模式匹配并修改
    static void ModifyClassDeclear(std::vector<KERNEL_NS::LibString> &lines, std::function<bool(KERNEL_NS::LibString &lineData)> &&modifyFunc);

    // 搜索body起始
    static bool SearchClassBodyBegin(KERNEL_NS::LibString &lineData, std::function<void(Int32 index, KERNEL_NS::LibString &lineData)> &&successHandler);

    // csharp Name :TestOpcode必须首字母大写
    static KERNEL_NS::LibString TreatCsharpName(const KERNEL_NS::LibString &protoName);

    // 是否注释行
    static bool IsNoteLine(const KERNEL_NS::LibString &lineData);

    // 获取package名
    static bool GetPackageName(const KERNEL_NS::LibString &lineData, KERNEL_NS::LibString &packageName, const KERNEL_NS::LibString &replaceDotWith = "::");

    // 是否类型开始
    static void RemoveNotePart(KERNEL_NS::LibString &lineData);
};

SERVICE_END