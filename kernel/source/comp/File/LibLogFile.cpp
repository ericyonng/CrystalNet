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

void LibLogFile::PartitionFile(bool isSysFirstCreate, LibTime *nowTime)
{
    if(isSysFirstCreate)
        return;

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

    // 转储文件
    auto dest = FileUtil::OpenFile(wholeName.c_str(), true);
    FileUtil::ResetFileCursor(*_fp);
    FileUtil::CopyFile(*_fp, *dest);
    FileUtil::CloseFile(*dest);

    // 删除并重开文件
    Close();
    auto tempFp = FileUtil::OpenFile(fileNameCache.c_str(), true, "w");
    if(!tempFp)
    {
        auto err = SystemUtil::GetErrNo();
        auto errStr = SystemUtil::GetErrString(err);
        CRYSTAL_TRACE("open file fail wit w type err:%d, %s", err, errStr.c_str());
    }
    else
    {
        FileUtil::CloseFile(*tempFp);
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

    #ifdef _DEBUG
        auto ret = Reopen(nowTime);
        ASSERT(ret);
    #else
        Reopen(nowTime);
    #endif
}

KERNEL_END