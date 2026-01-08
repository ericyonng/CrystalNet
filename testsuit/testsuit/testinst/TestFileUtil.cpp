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
 * Date: 2021-01-06 01:50:04
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestFileUtil.h>

void TestFileUtil::Run() 
{
//     {
//         const KERNEL_NS::LibString logPath = "./log/net/bb/";
//         const KERNEL_NS::LibString logFile2 = logPath + "log2.txt";
//         std::cout << KERNEL_NS::DirectoryUtil::CreateDir(logPath) << std::endl;
//         auto fp = KERNEL_NS::FileUtil::OpenFile(logFile2, true);
//         std::cout << KERNEL_NS::FileUtil::IsFileExist(logFile2) << std::endl;
//         std::cout << KERNEL_NS::FileUtil::GetFileSize(*fp) << std::endl;
//         std::cout << KERNEL_NS::FileUtil::GetFileSizeEx(logFile2) << std::endl;
// 
//         KERNEL_NS::LibString buffer;
//         KERNEL_NS::FileUtil::ReadOneLine(*fp, buffer);
// 
//         KERNEL_NS::FileUtil::CopyFile("./log/net/bb/log2.txt", "./log/net/bb/log3.txt");
//         auto destFp = KERNEL_NS::FileUtil::OpenFile("./log/net/bb/log3.txt");
//         KERNEL_NS::LibString buffer3;
//         KERNEL_NS::FileUtil::ReadFile(*destFp, buffer3);
//         std::cout << buffer3 << std::endl;
//         //KERNEL_NS::FileUtil::CloseFile(*destFp);
// 
//         KERNEL_NS::FileUtil::DelFileCStyle("./log/net/bb/log3.txt");
//         std::cout << buffer << std::endl;
//     }

    // 读取一行UTF8字符串
    {
        const KERNEL_NS::LibString utf8File = "./utf8Text3.txt";
        auto fp = KERNEL_NS::FileUtil::OpenFile(utf8File.c_str(), true);

        auto writeBytes = KERNEL_NS::FileUtil::WriteFile(*fp, "hello world...");
        KERNEL_NS::FileUtil::ResetFileCursor(*fp);
        UInt64 utf8Count = 0;
        KERNEL_NS::LibString utf8Line;
        auto lineBytes = KERNEL_NS::FileUtil::ReadUtf8OneLine(*fp, utf8Line, &utf8Count);
        std::cout << "utf8 line:" << utf8Line << std::endl;
        std::cout << "lineBytes:" << lineBytes << std::endl;
        std::cout << "utf8Count:" << utf8Count << std::endl;

        utf8Line.clear();
        utf8Count = 0;
        lineBytes = 0;
        lineBytes = KERNEL_NS::FileUtil::ReadUtf8OneLine(*fp, utf8Line, &utf8Count);
        std::cout << "utf8 line:" << utf8Line << std::endl;
        std::cout << "lineBytes:" << lineBytes << std::endl;
        std::cout << "utf8Count:" << utf8Count << std::endl;

        
    }

}
