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

#ifndef __CRYSTAL_NET_SERVICE_COMMON_PARAMS_PARAMS_HANDLER_H__
#define __CRYSTAL_NET_SERVICE_COMMON_PARAMS_PARAMS_HANDLER_H__

#pragma once

#include <kernel/kernel.h>
#include <service_common/common/common.h>

SERVICE_COMMON_BEGIN

struct ParamsInfo;

class ParamsHandler
{
public:
    // return 返回解析出来的参数数量
    static Int32 GetParams(int argc, char const *argv[], ParamsInfo &paramInfo, KERNEL_NS::LibString &susParamsInfo, KERNEL_NS::LibString &warnParamsInfo);

    // 参数格式：exe_prog --config=release --xxx="dadfa dafas", 
    // @param(cb) :key:键, value:值, return:返回键值对解析是否成功, 成功会添加到susParamsInfo, 不成功会添加到warnParamsInfo
    // @return(Int32):返回成功解析的参数个数
    static Int32 GetParams(int argc, char const *argv[], KERNEL_NS::LibString &susParamsInfo, KERNEL_NS::LibString &warnParamsInfo
    , std::function<bool(const KERNEL_NS::LibString &key, const KERNEL_NS::LibString &value)> &&cb, Byte8 kvSepChar = '=');

    // 参数格式：exe_prog --config=release --xxx="dadfa dafas", 
    // @param(cb) :key:键, value:值, return:返回键值对解析是否成功, 成功会添加到susParamsInfo, 不成功会添加到warnParamsInfo
    // @return(Int32):返回成功解析的参数个数
    static Int32 GetParams(const std::vector<KERNEL_NS::LibString> &params, KERNEL_NS::LibString &susParamsInfo, KERNEL_NS::LibString &warnParamsInfo
    , std::function<bool(const KERNEL_NS::LibString &key, const KERNEL_NS::LibString &value)> &&cb, Byte8 kvSepChar = '=');

    static Int32 GetParams(const std::vector<KERNEL_NS::LibString> &params, std::function<bool(const KERNEL_NS::LibString &key, const KERNEL_NS::LibString &value)> &&cb, Byte8 kvSepChar = '=');
};

ALWAYS_INLINE Int32 ParamsHandler::GetParams(int argc, char const *argv[], KERNEL_NS::LibString &susParamsInfo, KERNEL_NS::LibString &warnParamsInfo
    , std::function<bool(const KERNEL_NS::LibString &key, const KERNEL_NS::LibString &value)> &&cb, Byte8 kvSepChar)
{
     // 传入的参数
    std::vector<KERNEL_NS::LibString> args;
    for(Int32 idx = 0; idx < argc; ++idx)
        args.push_back(KERNEL_NS::LibString(argv[idx]));

    return ParamsHandler::GetParams(args, susParamsInfo, warnParamsInfo, std::forward<decltype(cb)>(cb), kvSepChar);
}

ALWAYS_INLINE Int32 ParamsHandler::GetParams(const std::vector<KERNEL_NS::LibString> &params, std::function<bool(const KERNEL_NS::LibString &key, const KERNEL_NS::LibString &value)> &&cb, Byte8 kvSepChar)
{
    KERNEL_NS::LibString susParamsInfo;
    KERNEL_NS::LibString warnParamsInfo;

    return ParamsHandler::GetParams(params, susParamsInfo, warnParamsInfo, std::forward<decltype(cb)>(cb), kvSepChar);
}

SERVICE_COMMON_END

#endif
