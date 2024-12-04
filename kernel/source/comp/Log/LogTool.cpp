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
 * Date: 2024-12-05 00:07:00
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Log/LogTool.h>
#include <kernel/comp/Log/ILog.h>
#include <kernel/common/statics.h>

KERNEL_BEGIN

bool LogTool::IsEnable(Int32 level)
{
  return g_Log->IsEnable(level);
}

void LogTool::Debug(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    g_Log->_Common5(tag, codeLine, LogLevel::Debug, fmt, va, finalSize);
    va_end(va);
}

void LogTool::Info(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    g_Log->_Common5(tag, codeLine, LogLevel::Info, fmt, va, finalSize);
    va_end(va);
}

void LogTool::Warn(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    g_Log->_Common5(tag, codeLine, LogLevel::Warn, fmt, va, finalSize);
    va_end(va);
}

void LogTool::Error(const Byte8 *tag, const char *fileName, const char *funcName, Int32 codeLine, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto finalSize = LibString::CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    g_Log->_Common1(tag, LogLevel::Error, fileName, funcName, codeLine, fmt, va, finalSize);
    va_end(va);
}

KERNEL_END