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
 * Date: 2022-12-08 13:03:00
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <file_tool/FileTool.h>

// 分割文件
// @param(filePath):原文件
// @param(partitionSize):分立文件大小
static bool PartitionFile(const KERNEL_NS::LibString &filePath, Int64 partitionSize)
{
    // 生成序号连续的文件
    if(!KERNEL_NS::FileUtil::IsFileExist(filePath.c_str()))
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(FileTool, "file:%s not found."), filePath.c_str());
        return false;
    }

    const auto fileSize = KERNEL_NS::FileUtil::GetFileSizeEx(filePath.c_str());
    const auto fileCount = static_cast<Int32>(fileSize / partitionSize + (((fileSize % partitionSize) > 0) ? 1 : 0));

    // 移除文件,多移除一个文件保证不会和之前的文件有连续性关联
    for(Int32 idx = 0; idx <= fileCount; ++idx)
    {
        const auto partFile = filePath + KERNEL_NS::LibString().AppendFormat(".part%d", idx);
        KERNEL_NS::FileUtil::DelFileCStyle(partFile.c_str());
    }

    // 读取原文件
    KERNEL_NS::SmartPtr<FILE, KERNEL_NS::AutoDelMethods::CustomDelete> fp = KERNEL_NS::FileUtil::OpenFile(filePath.c_str());
    if(!fp)
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(FileTool, "open file fail %s"), filePath.c_str());
        return false;
    }

    fp.SetClosureDelegate([](void *ptr){
        auto fpPtr = reinterpret_cast<FILE *>(ptr);
        KERNEL_NS::FileUtil::CloseFile(*fpPtr);
    });


    g_Log->Custom("[PARTITION FILE]:%s start..."
            , KERNEL_NS::DirectoryUtil::GetFileNameInPath(filePath).c_str());

    FILE *dest = NULL;
    const Int64 rBytes = 1024 * 1024;
    Int64 handleBytes = 0;
    Int32 partNo = 0;
    Int64 leftBytes = fileSize;
    while (true)
    {
        KERNEL_NS::LibString outData;
        auto ret = KERNEL_NS::FileUtil::ReadFile(*fp, outData, std::min<Int64>(rBytes, leftBytes));
        if(ret == 0)
        {
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(FileTool, "read file fail %s"), filePath.c_str());
            return false;
        }
        
        if(!dest)
        {
            auto destFilePath = filePath + KERNEL_NS::LibString().AppendFormat(".part%d", partNo);
            g_Log->Custom("[... ...] %s", KERNEL_NS::DirectoryUtil::GetFileNameInPath(destFilePath).c_str());
            dest = KERNEL_NS::FileUtil::OpenFile(destFilePath.c_str(), true);
            if(!dest)
            {
                g_Log->Warn(LOGFMT_NON_OBJ_TAG(FileTool, "open fail fail destFilePath:%s"), destFilePath.c_str());
                return false;
            }
        }

        KERNEL_NS::FileUtil::WriteFile(*dest, outData);
        handleBytes += ret;
        leftBytes -= ret;
        if(handleBytes >= partitionSize)
        {// 
            // 关闭文件
            KERNEL_NS::FileUtil::CloseFile(*dest);
            ++partNo;

            dest = NULL;
            handleBytes = 0;
        }

        if(leftBytes <= 0)
        {
            if(dest)
            {
                KERNEL_NS::FileUtil::CloseFile(*dest);
                dest = NULL;
            }

            break;
        }
    }

    return true;
}

// 合并文件
static bool MergeFile(const KERNEL_NS::LibString &target)
{
    // 1.找到分立文件
    // 2.从最小的分立id开始合并

    // 遍历文件夹找到target.partxx文件,找到个数
    auto name = KERNEL_NS::DirectoryUtil::GetFileNameInPath(target);
    name.findreplace(".", "\\.");
    auto pattern = std::regex(KERNEL_NS::LibString().AppendFormat(".*%s\\.part.*", name.c_str()).GetRaw());
    const auto &dir = KERNEL_NS::DirectoryUtil::GetFileDirInPath(target.c_str());
    bool hasPartFile = false;
    auto travel = [&pattern, &hasPartFile](const KERNEL_NS::FindFileInfo &findFileInfo, bool &isParentPathContinue)->bool{

        bool isContinue = true;
        do
        {
            if(KERNEL_NS::FileUtil::IsDir(findFileInfo))
                break;

            if(std::regex_match(findFileInfo._fileName.GetRaw(), pattern))
            {
                hasPartFile = true;
                isContinue = false;
                break;
            }
            
            /* code */
        } while (false);
        
        return isContinue;
    };

    auto delg =  KERNEL_CREATE_CLOSURE_DELEGATE(travel, bool, const KERNEL_NS::FindFileInfo &, bool &);
    KERNEL_NS::DirectoryUtil::TraverseDirRecursively(dir, delg);
    delg->Release();
    
    if(!hasPartFile)
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(FileTool, "have no part file, target:%s"), target.c_str());
        return false;
    }

    Int32 partNoStart = -1;
    for(Int32 idx = 0;; ++idx)
    {
        auto partFile = target + KERNEL_NS::LibString().AppendFormat(".part%d", idx);
        if(KERNEL_NS::FileUtil::IsFileExist(partFile.c_str()))
        {
            partNoStart = idx;
            break;
        }
    }

    if(partNoStart < 0)
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(FileTool, "have no part file to merge, target:%s"), target.c_str());
        return false;
    }

    g_Log->Custom("[MERGE FILE] >> %s start..."
            , KERNEL_NS::DirectoryUtil::GetFileNameInPath(target).c_str());

    // 开始合并
    FILE *dest = NULL;
    for(Int32 idx = partNoStart; ; ++idx)
    {
        auto partFile = target + KERNEL_NS::LibString().AppendFormat(".part%d", idx);
        if(!KERNEL_NS::FileUtil::IsFileExist(partFile.c_str()))
        {
            if(dest)
                KERNEL_NS::FileUtil::CloseFile(*dest);

            break;
        }

        if(!dest)
        {
            KERNEL_NS::FileUtil::DelFileCStyle(target.c_str());
            dest = KERNEL_NS::FileUtil::OpenFile(target.c_str(), true);
        }

        g_Log->Custom("\t[%s] >> [%s]", KERNEL_NS::DirectoryUtil::GetFileNameInPath(partFile).c_str(), KERNEL_NS::DirectoryUtil::GetFileNameInPath(target).c_str());

        auto src = KERNEL_NS::FileUtil::OpenFile(partFile.c_str());
        KERNEL_NS::FileUtil::CopyFile(*src, *dest);
        KERNEL_NS::FileUtil::CloseFile(*src);
    }

    return true;
}

void FileTool::Run(int argc, char const *argv[])
{
    // 1.设置传入的参数
    std::vector<KERNEL_NS::LibString> args;
    for(Int32 idx = 0; idx < argc; ++idx)
        args.push_back(KERNEL_NS::LibString(argv[idx]));

    // 2.参数定义
    // --partition_targets
    KERNEL_NS::LibString partitionTargetsPath;
    // --function=partition/merge
    KERNEL_NS::LibString func;
    // --partition_size 分立文件大小
    Int64 partitionSize = 100 * 1024 * 1024;
    // --root_path
    KERNEL_NS::LibString rootPath;
    
    // 解析参数
    for(auto &param : args)
    {
        const auto &seps = param.Split("=");
        if(seps.empty() || seps.size() != 2)
            continue;

        auto key = seps[0].strip();
        auto value = seps[1].strip();
        if(key == "--partition_targets")
        {
            partitionTargetsPath = value;
        }
        else if(key == "--function")
        {
            func = value;
        }
        else if(key == "--partition_size")
        {
            partitionSize = KERNEL_NS::StringUtil::StringToInt64(value.c_str());
        }
        else if(key == "--root_path")
        {
            rootPath = value;
        }
    }

    if(rootPath.empty())
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(FileTool, "have no root path"));
        return;
    }
    if(func.empty())
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(FileTool, "have no func"));
        return;
    }

    {// 执行func
        if(func == "partition")
        {
            KERNEL_NS::SmartPtr<FILE, KERNEL_NS::AutoDelMethods::CustomDelete> fp = KERNEL_NS::FileUtil::OpenFile(partitionTargetsPath.c_str());
            if(!fp)
            {
                g_Log->Warn(LOGFMT_NON_OBJ_TAG(FileTool, "open file fail :%s"), partitionTargetsPath.c_str());
                return;
            }

            fp.SetClosureDelegate([](void *ptr){
                auto fpPtr = reinterpret_cast<FILE *>(ptr);
                KERNEL_NS::FileUtil::CloseFile(*fpPtr);
            });
            
            std::vector<KERNEL_NS::LibString> lines;
            KERNEL_NS::FileUtil::ReadUtf8File(*fp, lines);

            for (auto& handleFilePath : lines)
            {
                handleFilePath.strip();
                if (handleFilePath.empty())
                    continue;

                PartitionFile(rootPath + handleFilePath, partitionSize);
            }

            return;
        }

        if(func == "merge")
        {
            KERNEL_NS::SmartPtr<FILE, KERNEL_NS::AutoDelMethods::CustomDelete> fp = KERNEL_NS::FileUtil::OpenFile(partitionTargetsPath.c_str());
            if(!fp)
            {
                g_Log->Warn(LOGFMT_NON_OBJ_TAG(FileTool, "open file fail :%s"), partitionTargetsPath.c_str());
                return;
            }

            fp.SetClosureDelegate([](void *ptr){
                auto fpPtr = reinterpret_cast<FILE *>(ptr);
                KERNEL_NS::FileUtil::CloseFile(*fpPtr);
            });
            
            std::vector<KERNEL_NS::LibString> lines;
            KERNEL_NS::FileUtil::ReadUtf8File(*fp, lines);

            for (auto& handleFilePath : lines)
            {
                if (handleFilePath.empty())
                    continue;

                MergeFile(rootPath + handleFilePath);
            }
            return;
        }
    }
}