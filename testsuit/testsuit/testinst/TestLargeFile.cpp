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
 * Date: 2024-05-11 23:48:18
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestLargeFile.h>

void TestLargeFile::Run(int argc, char const *argv[])
{
    // 参数
    Int32 count = 0;
    KERNEL_NS::LibString fileName;
    KERNEL_NS::ParamsHandler::GetStandardParams(argc, argv, [&count, &fileName](const KERNEL_NS::LibString &param, std::vector<KERNEL_NS::LibString> &leftParam){

        bool ret = false;
        do
        {
            if(count == 1)
            {
                fileName = param.strip();
                ret = true;
            }

        } while (false);
        
        ++count;
        return ret;
    });

    fileName = "./largetfile.txt";

    if(fileName.empty())
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(TestLargeFile, "have no file"));
        return;
    }

    // 打开大文件
    KERNEL_NS::SmartPtr<FILE, KERNEL_NS::AutoDelMethods::CustomDelete> fp = KERNEL_NS::FileUtil::OpenFile(fileName.c_str());
    if(!fp)
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(TestLargeFile, "cant open file:%s"), fileName.c_str());
        return;
    } 

    fp.SetClosureDelegate([](void *p){
        auto ptr = KERNEL_NS::KernelCastTo<FILE>(p);
        KERNEL_NS::FileUtil::CloseFile(*ptr);
    });

    // 文件大小
    const auto fileSize = KERNEL_NS::FileUtil::GetFileSize(*fp);
    const auto &fileSizeStr = KERNEL_NS::MathUtil::ToFmtDataSize(fileSize);
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLargeFile, "file name:%s, file size:%s"), fileName.c_str(), fileSizeStr.c_str());

    // 读写一行
    KERNEL_NS::LibString buffer;
    UInt64 readCount = 0;
    UInt64 readSize = 0;
    readSize = KERNEL_NS::FileUtil::ReadUtf8OneLine(*fp, buffer, &readCount);
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLargeFile, "file name read line:%s, size:%llu, count:%llu"), buffer.c_str(), readSize, readCount);

    // 写到末尾
    KERNEL_NS::FileUtil::SetFileCursor(*fp, KERNEL_NS::FileCursorOffsetType::FILE_CURSOR_POS_END, 0);
    auto firstPos = KERNEL_NS::FileUtil::GetFileCusorPos(*fp);
    KERNEL_NS::FileUtil::WriteFile(*fp, "a------------------------dafalskfl----2------\n");

    // 刷新文件
    KERNEL_NS::FileUtil::FlushFile(*fp);

    auto afterWritePos = KERNEL_NS::FileUtil::GetFileCusorPos(*fp);

    // 设会原来末尾位置并读一行
    KERNEL_NS::FileUtil::SetFileCursor(*fp, KERNEL_NS::FileCursorOffsetType::FILE_CURSOR_POS_SET, firstPos);
    auto finalPos = KERNEL_NS::FileUtil::GetFileCusorPos(*fp);
    auto err = KERNEL_NS::SystemUtil::GetErrNo();
    const auto &errStr = KERNEL_NS::SystemUtil::GetErrString(err);
    buffer.clear();
    readCount = 0;
    readSize = 0;
    readSize = KERNEL_NS::FileUtil::ReadUtf8OneLine(*fp, buffer, &readCount);
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestLargeFile, "file name read end line:%s, size:%llu, count:%llu, firstPos:%lld, finalPos:%lld, afterWritePos:%lld, err:%d,%s")
    , buffer.c_str(), readSize, readCount, firstPos, finalPos, afterWritePos, err, errStr.c_str());
}