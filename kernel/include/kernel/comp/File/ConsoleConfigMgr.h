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
 * Date: 2021-02-18 22:59:07
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_FILE_CONSOLE_CONFIG_MGR_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_FILE_CONSOLE_CONFIG_MGR_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

class LibIniFile;

// 前景色
struct KERNEL_EXPORT ConsoleFrontColor
{
    Int32 _black;
    Int32 _red;
    Int32 _green;
    Int32 _blue;
    Int32 _yellow;
    Int32 _purple;
    Int32 _cyan;
    Int32 _white;
    Int32 _gray;
    Int32 _lightYellow;
    Int32 _highLight;
    Int32 _frontDefault;
};

// 背景色
struct KERNEL_EXPORT ConsoleBackColor
{
    Int32 _black;
    Int32 _red;
    Int32 _green;
    Int32 _blue;
    Int32 _yellow;
    Int32 _purple;
    Int32 _cyan;
    Int32 _white;
    Int32 _highLight;
    Int32 _backDefault;
};

class KERNEL_EXPORT ConsoleConfigMgr
{
public:
    ConsoleConfigMgr();
    ~ConsoleConfigMgr();

public: 
    // 放在程序根目录ini子目录下
    static ConsoleConfigMgr *GetInstance();
    // 绝对路径
    bool Init(const Byte8 *cfgFileName = "ConsoleConfig.ini", const Byte8 *dirName = NULL, const LibString &content = LibString());
    void Close();

    const ConsoleFrontColor &GetFrontColor() const;
    const ConsoleBackColor &GetBackColor() const;

    const Int32 GetFrontColor(const Byte8 *colorName) const;
    const Int32 GetBackColor(const Byte8 *colorName) const;

    bool IsInit() const;

private:
    LibIniFile *_ini;
    ConsoleFrontColor _front;   // 前景色
    std::map<LibString, Int32> _frontNameRefColor;

    ConsoleBackColor _back;     // 背景色
    std::map<LibString, Int32> _backNameRefColor;
};

inline const ConsoleFrontColor &ConsoleConfigMgr::GetFrontColor() const
{
    return _front;
}

inline const ConsoleBackColor &ConsoleConfigMgr::GetBackColor() const
{
    return _back;
}

inline bool ConsoleConfigMgr::IsInit() const
{
    return _ini != NULL;
}


KERNEL_END

#endif
