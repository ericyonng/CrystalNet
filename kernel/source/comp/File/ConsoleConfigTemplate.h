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
 * Date: 2021-02-20 00:55:53
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_SOURCE_COMP_FILE_CONSOLE_CONFIG_TEMPLATE_H__
#define __CRYSTAL_NET_KERNEL_SOURCE_COMP_FILE_CONSOLE_CONFIG_TEMPLATE_H__

#pragma once

#include <kernel/comp/LibString.h>

KERNEL_BEGIN

class ConsoleConfigTemplate
{
public:
    static LibString GetConsoleIniContent();
};

inline LibString ConsoleConfigTemplate::GetConsoleIniContent()
{
    LibString content;
    content = \
"; Author: EricYonng(120453674@qq.com)\n"
"; Date:2021.02.17\n"
"; Desc:控制台颜色配置\n"
"\n"
"[Win32FrontConsoleColor]\n"
"; 前景色."
"\nBlack        = 0      ; 黑色. \n"
"Red          = 4        ; 红色. FOREGROUND_RED\n"
"Green        = 2        ; 绿色. FOREGROUND_GREEN\n"
"Blue         = 1        ; 蓝色. FOREGROUND_BLUE\n"
"Yellow       = 6        ; 黄色. FOREGROUND_RED | FOREGROUND_GREEN\n"
"Purple       = 5        ; 紫色. FOREGROUND_RED | FOREGROUND_BLUE\n"
"Cyan         = 3        ; 蓝绿色. FOREGROUND_BLUE | FOREGROUND_GREEN\n"
"White        = 7        ; 白色. FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE\n"
"Gray         = 8        ; 灰色. FOREGROUND_INTENSITY\n"
"LightYellow  = 14       ; 淡黄色. Gray | Yellow\n"
"Highlight    = 8        ; 高亮前景色. FOREGROUND_INTENSITY\n"
"FrontDefault = 7        ; 默认前景色. White\n"
"\n"
"[Win32BackConsoleColor]\n"
"; 背景色."
"\nBlack       = 0       ; 黑色. \n"
"Red         = 64        ; 红色. BACKGROUND_RED\n"
"Green       = 32        ; 绿色. BACKGROUND_GREEN\n"
"Blue        = 16        ; 蓝色. BACKGROUND_BLUE\n"
"Yellow      = 96        ; 黄色. BACKGROUND_RED  | BACKGROUND_GREEN\n"
"Purple      = 80        ; 紫色. BACKGROUND_RED  | BACKGROUND_BLUE\n"
"Cyan        = 48        ; 蓝绿色. BACKGROUND_BLUE | BACKGROUND_GREEN\n"
"White       = 112       ; 白色. BACKGROUND_RED  | BACKGROUND_GREEN | BACKGROUND_BLUE\n"
"Highlight   = 128       ; 高亮背景. BACKGROUND_INTENSITY  | BACKGROUND_GREEN | BACKGROUND_BLUE\n"
"BackDefault = 0         ; 默认背景色. Black\n";

    return content;
}

KERNEL_END

#endif
