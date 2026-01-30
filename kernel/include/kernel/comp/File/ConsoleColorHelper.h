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
// Date: 2026-01-30 22:01:51
// Author: Eric Yonng
// Description:

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_FILE_CONSOLE_COLOR_HELPER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_FILE_CONSOLE_COLOR_HELPER_H__

#pragma once

#include <kernel/comp/LibString.h>
#include <map>

KERNEL_BEGIN

class KERNEL_EXPORT ConsoleColorHelper
{
public:
    // 前景色
    static Int32 GetFrontColor(const LibString &str);
    // 背景色
    static Int32 GetBackColor(const LibString &str);
    // 前景色
    static LibString GetFrontColorStr(Int32 color);
    // 背景色
    static LibString GetBackColorStr(Int32 color);
private:
    static const std::map<Int32, LibString> &GetFrontDict2()
    {
        static std::map<Int32, LibString> ColorStrRefInt = {
            std::make_pair(0, "Black"),
            std::make_pair(4, "Red"),
            std::make_pair(2, "Green"),
            std::make_pair(1, "Blue"),
            std::make_pair(6, "Yellow"),
            std::make_pair(5, "Purple"),
            std::make_pair(3, "Cyan"),
            std::make_pair(7, "White"),
            std::make_pair(8, "Gray"),
            std::make_pair(14, "LightYellow"),
        };

        return ColorStrRefInt;
    }

    static const std::map<LibString, Int32> &GetFrontDict()
    {
        static std::map<LibString, Int32> ColorStrRefInt = {
            std::make_pair("Black", 0),
            std::make_pair("Red", 4),
            std::make_pair("Green", 2),
            std::make_pair("Blue", 1),
            std::make_pair("Yellow", 6),
            std::make_pair("Purple", 5),
            std::make_pair("Cyan", 3),
            std::make_pair("White", 7),
            std::make_pair("Gray", 8),
            std::make_pair("LightYellow", 14),
            std::make_pair("Highlight", 8),
            std::make_pair("FrontDefault", 7),
        };

        return ColorStrRefInt;
    }

    static const std::map<Int32, LibString> &GetBackDict2()
    {
        static std::map<Int32, LibString> ColorStrRefInt = {
            std::make_pair(0, "Black"),
            std::make_pair(64, "Red"),
            std::make_pair(32, "Green"),
            std::make_pair(16, "Blue"),
            std::make_pair(96, "Yellow"),
            std::make_pair(80, "Purple"),
            std::make_pair(48, "Cyan"),
            std::make_pair(112, "White"),
            std::make_pair(128, "Highlight"),
        };

        return ColorStrRefInt;
    }
    
    static const std::map<LibString, Int32> &GetBackDict()
    {
        static std::map<LibString, Int32> ColorStrRefInt = {
            std::make_pair("Black", 0),
            std::make_pair("Red", 64),
            std::make_pair("Green", 32),
            std::make_pair("Blue", 16),
            std::make_pair("Yellow", 96),
            std::make_pair("Purple", 80),
            std::make_pair("Cyan", 48),
            std::make_pair("White", 112),
            std::make_pair("Highlight", 128),
            std::make_pair("BackDefault", 0),
        };

        return ColorStrRefInt;
    }

};

ALWAYS_INLINE Int32 ConsoleColorHelper::GetFrontColor(const LibString &str)
{
    auto &dict = GetFrontDict();
    auto iter = dict.find(str);
    return iter == dict.end() ? 0 : iter->second;
}

ALWAYS_INLINE Int32 ConsoleColorHelper::GetBackColor(const LibString &str)
{
    auto &dict = GetBackDict();
    auto iter = dict.find(str);
    return iter == dict.end() ? 0 : iter->second;
}

// 前景色
ALWAYS_INLINE LibString ConsoleColorHelper::GetFrontColorStr(Int32 color)
{
    auto &dict = GetFrontDict2();
    auto iter = dict.find(color);
    return iter == dict.end() ? LibString() : iter->second;
}

// 背景色
ALWAYS_INLINE LibString ConsoleColorHelper::GetBackColorStr(Int32 color)
{
    auto &dict = GetBackDict2();
    auto iter = dict.find(color);
    return iter == dict.end() ? LibString() : iter->second;
}


KERNEL_END

#endif

