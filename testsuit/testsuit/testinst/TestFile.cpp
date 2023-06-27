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
 * Date: 2021-02-08 00:11:24
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestFile.h>


void TestFile::Run() 
{
    // test lib file
    {
        KERNEL_NS::LibFile file;
        file.Open("./yizhihua.log", NULL, true, "rb+");
        KERNEL_NS::LibString str;
        std::cout << "one line bytes:" << file.ReadOneLine(str) << std::endl;
        std::cout << "line data:" << str << std::endl;
        std::cout << file.IsOpen() << std::endl;
        std::cout << file.GetPath()<< std::endl;
        std::cout << file.GetFileName() << std::endl;
        std::cout << "fp pos:" << KERNEL_NS::FileUtil::GetFileCusorPos(*file.GetFp()) << std::endl;

        KERNEL_NS::FileUtil::SetFileCursor(*file.GetFp()
            , KERNEL_NS::FileCursorOffsetType::FILE_CURSOR_POS_END, 0);

        KERNEL_NS::LibString toWr = "ABC ABC ABC hello world";
        file.Write(toWr.c_str(), toWr.length());
        // FILE *fileptr = file;
        file.Close();
    }  

    // test ini file
    {
        KERNEL_NS::LibIniFile inifile;
        inifile.Init("./testinifile.ini");

        auto port = inifile.ReadInt("SeverCfg", "port", 123456);
        port = 0;
        std::cout << "port:" << port << std::endl;

        KERNEL_NS::LibString ip;
        inifile.ReadStr("SeverCfg", "ip", "127.0.0.1", ip);
        std::cout << "ip:" << ip << std::endl;

        inifile.WriteStr("SeverCfg", "MaxClient2", "10");
        inifile.WriteStr("SeverCfg", "MaxClient", "100");
        inifile.WriteStr("ddksk", "MaxClient", "100");        
    } 

    // test memory ini file
    {
        KERNEL_NS::LibString iniContent = 
        "[MemoryIni]\n\n\n"
        ";memory pool size \n"
        "MemoryPoolSize = 1000\n"
        "ip = 127.0.0.1";

        KERNEL_NS::LibIniFile inifile;
        inifile.SetMemoryIniContent(iniContent);
        inifile.Init(NULL);

        auto poolSize = inifile.ReadInt("MemoryIni", "MemoryPoolSize", 123456);
        std::cout << "port:" << poolSize << std::endl;

        KERNEL_NS::LibString ip;
        inifile.ReadStr("MemoryIni", "ip", "127.0.0.1", ip);
        std::cout << "ip:" << ip << std::endl;

        inifile.WriteStr("SeverCfg", "MaxClient2", "10");

        std::cout << "memory ini content:" << inifile.GetMemoryIniContent() << std::endl;
    } 

    // test log file
    {
        std::cout << "TestLogFile" << std::endl;
        KERNEL_NS::LibLogFile logFile;
        logFile.Open("./log2.log", NULL, true, "ab+", true);
        KERNEL_NS::LibString logStr = "hello test log file";
        logFile.Write(logStr.c_str(), logStr.length());
        logFile.Flush();

        // 跨天
        KERNEL_NS::LibTime nowTime;
        nowTime.UpdateTime(KERNEL_NS::LibTime::Now().AddDays(1).GetNanoTimestamp());

        if(logFile.IsDayPass(nowTime))
            logFile.Reopen(&nowTime);

        KERNEL_NS::FileUtil::ResetFileCursor(*static_cast<FILE *>(logFile));
        KERNEL_NS::LibString strRead;
        logFile.Write("dkaflask", strlen("dkaflask"));
        logFile.Flush();
        KERNEL_NS::FileUtil::ResetFileCursor(*static_cast<FILE *>(logFile));
        logFile.ReadOneLine(strRead);
        std::cout << "log line data:" << strRead << std::endl;
        logFile.PartitionFile(false, &nowTime);        
    }
}
