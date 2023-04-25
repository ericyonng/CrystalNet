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
 * Date: 2021-02-07 23:54:55
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/File/LibLogFile.h>
#include <kernel/comp/Utils/FileUtil.h>
#include <kernel/comp/Utils/SystemUtil.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(LibLogFile);

LibLogFile::LibLogFile()
    :_partNo(0)
{
    
}

LibLogFile::~LibLogFile()
{

}

Int32 LibLogFile::PartitionFile(bool isSysFirstCreate, LibTime *nowTime)
{
    if(isSysFirstCreate)
        return Status::Success;

    // 构建文件名
    LibString fileNameCache;
    GetCurrentFileName(fileNameCache);

    // 查找不存在的文件名
    LibString wholeName;
    wholeName.AppendFormat("%sOld%d", fileNameCache.c_str(), ++_partNo);
    while(FileUtil::IsFileExist(wholeName.c_str()))
    {
        wholeName.clear();
        wholeName.AppendFormat("%sOld%d", fileNameCache.c_str(), ++_partNo);
    }

    // 关闭文件
    Close();

    // 将源文件改名
    auto err = FileUtil::Rename(fileNameCache, wholeName);
    if(err != Status::Success)
    {
        CRYSTAL_TRACE("rename frome %s to %s fail err:%d", fileNameCache.c_str(), wholeName.c_str());
        if(!Reopen(nowTime))
            CRYSTAL_TRACE("reopen log file:%s fail when PartitionFile at Rename fail.", fileNameCache.c_str());

        return err;
    }

    // if(!FileUtil::DelFileCStyle(fileNameCache.c_str()))
    // {
    //     auto err = SystemUtil::GetErrNo();
    //     auto errStr = SystemUtil::GetErrString(err);
    //     CRYSTAL_TRACE("del file fail err:%d, %s", err, errStr.c_str());

    //     auto tempFp = FileUtil::OpenFile(wholeName.c_str(), true, "w");
    //     if(!tempFp)
    //     {
    //         err = SystemUtil::GetErrNo();
    //         errStr = SystemUtil::GetErrString(err);
    //         CRYSTAL_TRACE("open file fail wit w type err:%d, %s", err, errStr.c_str());
    //     }
    //     else
    //     {
    //         FileUtil::CloseFile(*tempFp);
    //     }
    // }

    // 重新打开原文件
    #ifdef _DEBUG
        auto ret = Reopen(nowTime);
        ASSERT(ret);
    #else
       auto ret = Reopen(nowTime);
    #endif

    return ret ? Status::Success : Status::Failed;
}

KERNEL_END