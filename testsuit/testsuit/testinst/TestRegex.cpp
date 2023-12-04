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
 * Date: 2022-11-20 16:00:42
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <regex>
#include <testsuit/testinst/TestRegex.h>

void TestRegex::Run(int argc, char const *argv[])
{
    std::vector<KERNEL_NS::LibString> info;
    for(Int32 idx = 0; idx < argc; ++idx)
        info.push_back(argv[idx]);

    // 获取文件与匹配模式
    KERNEL_NS::LibString handleFile;
    KERNEL_NS::LibString handleRegex;

    const Int32 maxLen = static_cast<Int32>(info.size());
    for(Int32 idx = 0; idx < maxLen; ++idx)
    {
        auto &arg = info[idx];
        if(arg.empty())
            continue;

        auto parts = arg.Split("=");
        if(parts.empty())
            continue;

        if(2 != static_cast<Int32>(parts.size()))
            continue;

        auto &k = parts[0];
        auto &v = parts[1];
        if(k == "--file")
        {
            handleFile = v;
        }
        else if(k == "--regex")
        {
            handleRegex = v;
        }
    }

    for(Int32 idx = 0; idx < static_cast<Int32>(info.size()); ++idx)
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestRegex, "input program arg[%d]:%s"), idx, info[idx].c_str());
    
    if(handleFile.empty())
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(TestRegex, "have no file to handle"));
        return;
    }

    if(handleRegex.empty())
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(TestRegex, "have no regex pattern to handle"));
        return;
    }

    KERNEL_NS::SmartPtr<FILE, KERNEL_NS::AutoDelMethods::CustomDelete> fp = KERNEL_NS::FileUtil::OpenFile(handleFile.c_str());
    if(!fp)
    {
        auto err = KERNEL_NS::SystemUtil::GetErrNo();
        auto errStr = KERNEL_NS::SystemUtil::GetErrString(err);

        g_Log->Warn(LOGFMT_NON_OBJ_TAG(TestRegex, "open file fail file:%s, err:%d, %s "), handleFile.c_str(), err, errStr.c_str());
        return;
    }

    fp.SetClosureDelegate([](void *fp){
        auto fpPtr = KERNEL_NS::KernelCastTo<FILE>(fp);
        KERNEL_NS::FileUtil::CloseFile(*fpPtr);
    });

    std::vector<KERNEL_NS::LibString> lines;
    KERNEL_NS::FileUtil::ReadUtf8File(*fp, lines);
    
    const Int32 maxLine = static_cast<Int32>(lines.size());
    for(Int32 idx = 0; idx < maxLine; ++idx)
    {
        auto &lineData = lines[idx];
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestRegex, "will match idx:%d, line data:%s, pattern:%s")
                , idx, lineData.c_str(), handleRegex.c_str());

        if(std::regex_match(lineData.GetRaw(), std::regex(handleRegex.c_str())))
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestRegex, "match success"));
            
            break;
        }
    }
}