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
 * Date: 2023-03-17 23:23:14
 * Author: Eric Yonng
 * Description: 
*/
#include <pch.h>
#include <service_common/params/params_info.h>
#include <service_common/params/params_handler.h>


SERVICE_COMMON_BEGIN

Int32 ParamsHandler::GetParams(int argc, char const *argv[], ParamsInfo &paramInfo, KERNEL_NS::LibString &susParamsInfo, KERNEL_NS::LibString &warnParamsInfo)
{
    Int32 paramsNum = 0;

    // 传入的参数
    std::vector<KERNEL_NS::LibString> args;
    for(Int32 idx = 0; idx < argc; ++idx)
        args.push_back(KERNEL_NS::LibString(argv[idx]));

    // 1.传入的参数
    const Int32 argCount = static_cast<Int32>(args.size());
    for(Int32 idx = 0; idx < argCount; ++idx)
    {
        const auto &arg = args[idx];
        auto kv = arg.Split("=");
        if(kv.empty())
            continue;

        if(kv.size() < 2)
            continue;

        auto &k = kv[0];
        k.strip();
        KERNEL_NS::LibString &v = kv[1];
        if(k.empty())
            continue;

        v.strip();

        // 生成的语言版本
        ++paramsNum;
        if(k == "--file_soft")
        {
            paramInfo._fileSoftLimit = KERNEL_NS::StringUtil::StringToInt64(v.c_str());
        }
        else if(k == "--file_hard")
        {
            paramInfo._fileHardLimit = KERNEL_NS::StringUtil::StringToInt64(v.c_str());
        }
        else
        {
            --paramsNum;
            warnParamsInfo.AppendFormat("key:%s, value:%s\n", k.c_str(), v.c_str());
            continue;
        }

        susParamsInfo.AppendFormat("key:%s, value:%s\n", k.c_str(), v.c_str());
    }

    return paramsNum;
}

Int32 ParamsHandler::GetParams(const std::vector<KERNEL_NS::LibString> &params, KERNEL_NS::LibString &susParamsInfo, KERNEL_NS::LibString &warnParamsInfo
    , std::function<bool(const KERNEL_NS::LibString &key, const KERNEL_NS::LibString &value)> &&cb, Byte8 kvSepChar)
{
    Int32 paramsNum = 0;
    const Int32 argCount = static_cast<Int32>(params.size());
    for(Int32 idx = 0; idx < argCount; ++idx)
    {
        const auto &arg = params[idx];
        auto kv = arg.Split(kvSepChar);
        if(kv.empty())
            continue;

        if(kv.size() < 2)
            continue;

        auto &k = kv[0];
        k.strip();
        KERNEL_NS::LibString &v = kv[1];
        if(k.empty())
            continue;

        v.strip();

        if(cb(k, v))
        {
            ++paramsNum;
            susParamsInfo.AppendFormat("key:%s, value:%s\n", k.c_str(), v.c_str());
        }
        else
        {
            warnParamsInfo.AppendFormat("key:%s, value:%s\n", k.c_str(), v.c_str());
        }
    }

    return paramsNum;
}


SERVICE_COMMON_END