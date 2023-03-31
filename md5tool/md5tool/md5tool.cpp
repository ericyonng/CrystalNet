/*!
 * MIT License
 *  
 * Copyright (c) 2020 Eric Yonng<120453674@qq.com>
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *  
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *  
 * 
 * Date: 2023-03-31 13:05:29
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <md5tool/ini.h>
class LibTestLog : public KERNEL_NS::LibLog
{
public:
    LibTestLog() {}
    ~LibTestLog() {}
};

class LogFactory : public KERNEL_NS::ILogFactory
{
public:
    virtual KERNEL_NS::ILog *Create()
    {
        return new LibTestLog();
    }
};

int main(int argc, char const *argv[])
{
    // 1.初始化内核
    LogFactory logFactory;
    KERNEL_NS::LibString programPath = KERNEL_NS::SystemUtil::GetCurProgRootPath();
    KERNEL_NS::LibString logIniPath;
    logIniPath = programPath + "/ini/";
    KERNEL_NS::SystemUtil::GetProgramPath(true, programPath);
    Int32 err = KERNEL_NS::KernelUtil::Init(&logFactory, "LogCfg.ini", logIniPath.c_str(), s_logIniContent, s_consoleIniContent);
    if(err != Status::Success)
    {
        CRYSTAL_TRACE("kernel init fail err:%d", err);
        return Status::Failed;
    }

    KERNEL_NS::KernelUtil::Start();

    bool isErr = false;
    KERNEL_NS::LibString targetFile;
    bool isFirstParam = true;
    auto count = SERVICE_COMMON_NS::ParamsHandler::GetStandardParams(argc, argv, [&targetFile, &isErr, &isFirstParam](const KERNEL_NS::LibString &param, std::vector<KERNEL_NS::LibString> &leftParam){
        // 只解析第一个非空参数
        if(isFirstParam)
        {
            isFirstParam = false;
            return false;
        }

        if(targetFile.empty() && !param.empty())
        {
            targetFile = param;

            // 其他参数都丢弃
            leftParam.clear();
        }

        return true;
    });

    if(count == 0)
        return Status::Success;
        
    if(isErr)
    {
        return Status::Failed;
    }

    if(!KERNEL_NS::FileUtil::IsFileExist(targetFile.c_str()))
    {
        return Status::Failed;
    }

    KERNEL_NS::LibString md5Out;
    if(!KERNEL_NS::LibDigest::MakeFileMd5(targetFile, md5Out))
    {
        return Status::Failed;
    }

    KERNEL_NS::LibString hexStr;
    KERNEL_NS::StringUtil::ToHexString(md5Out, hexStr);
    std::cout << hexStr << std::endl;

    KERNEL_NS::KernelUtil::Destroy();

    return err;
}