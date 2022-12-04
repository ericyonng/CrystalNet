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
 * Date: 2021-01-05 23:44:29
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Utils/FileUtil.h>
#include <kernel/comp/Utils/DirectoryUtil.h>
#include <kernel/comp/Utils/StringUtil.h>
#include <kernel/comp/Utils/SystemUtil.h>
#include <kernel/comp/LibTime.h>
#include <kernel/comp/SmartPtr.h>
#include <kernel/comp/Log/log.h>

KERNEL_BEGIN


const Byte8 * FileUtil::GenRandFileNameNoDir(Byte8 randName[L_tmpnam])
{
#if CRYSTAL_TARGET_PLATFORM_LINUX
    // mkstemp(文件名)/mkdtemp(目录)
    ::memset(randName, 0, L_tmpnam);
    ::strcpy(randName, "tmp_XXXXXX");
    int fd = ::mkstemp(randName);
    if(fd == -1)
        return NULL;
    
    ::unlink(randName);
    const auto &fileName = DirectoryUtil::GetFileNameInPath(randName);
    if(fileName.GetRaw().length() > 0)
    {
        randName[0] = 0;
        auto len = sprintf(randName, "%s", fileName.c_str());
        len = ((len < L_tmpnam) ? std::max<Int32>(len, 0) : (L_tmpnam - 1));
        randName[len] = 0;
        return randName;
    }

    return NULL;
#endif

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    if(tmpnam(randName))
    {
        const auto &fileName = DirectoryUtil::GetFileNameInPath(randName);
        if(fileName.GetRaw().length() > 0)
        {
            randName[0] = 0;
            auto len = sprintf(randName, "%s", fileName.c_str());
            len = ((len < L_tmpnam) ? std::max<Int32>(len, 0) : (L_tmpnam - 1));
            randName[len] = 0;
            return randName;
        }
    }

    return NULL;
#endif
}

UInt64 FileUtil::ReadUtf8OneLine(FILE &fp, LibString &outBuffer, UInt64 *utf8CharCount)
{
    // 读取单字节字符时候判断是否\n
    clearerr(&fp);
    U8 get_c = 0;
    UInt64 cnt = 0;
    while(!feof(&fp))
    {
        get_c = 0;
        auto bytes = fread(&get_c, sizeof(get_c), 1, &fp);
        if(bytes == 1)
        {
            // 该utf8字符总字节数
            auto totalBytes = StringUtil::CalcUtf8CharBytes(get_c);
            UInt64 leftBytes = totalBytes - 1;
            if(!leftBytes)
            {// 单字节字符

                #if CRYSTAL_TARGET_PLATFORM_WINDOWS
                            
                    if(get_c == '\r')
                        continue;

                    if(get_c != '\n')
                    {
                        outBuffer.AppendData(reinterpret_cast<const char *>(&get_c), 1);
                        ++cnt;

                        if(UNLIKELY(utf8CharCount))
                            ++(*utf8CharCount);
                    }
                    else
                    {
                        break;
                    }
                #else
                    if(get_c != '\n')
                    {
                        outBuffer.AppendData(reinterpret_cast<const char *>(&get_c), 1);
                        ++cnt;
                        
                        if(UNLIKELY(utf8CharCount))
                            ++(*utf8CharCount);
                    }
                    else
                    {
                        //fread(&get_c, sizeof(get_c), 1, fpOutCache);
                        break;
                    }
                #endif
            }
            else
            {// 多字节字符

                outBuffer.AppendData(reinterpret_cast<const char *>(&get_c), 1);
                ++cnt;
                
                if(UNLIKELY(utf8CharCount))
                    ++(*utf8CharCount);

                // 读取剩下字节
                do
                {
                    if(fread(&get_c, sizeof(get_c), 1, &fp) == 1)
                    {
                        outBuffer.AppendData(reinterpret_cast<const char *>(&get_c), 1);
                        ++cnt;
                    }
                    else
                    {// 错误字符
                        break;
                    }
                } while (--leftBytes);

                // 错误字符
                if(UNLIKELY(leftBytes))
                {
                    CRYSTAL_TRACE("ERROR UTF8 Char totalBytes[%llu], leftBytes[%llu], fread fail", totalBytes, leftBytes);
                    break;
                }
            }
        }
        else
        {
            // 文件结束
            break;
        }
    }

    return cnt;
}

Int64 FileUtil::ReadUtf8File(FILE &fp, std::vector<KERNEL_NS::LibString> &lines, Int64 lineLimit)
{
    Int64 line = 0;
    while(!::feof(&fp))
    {
        if(lineLimit > 0 && line >= lineLimit)
            break;

        KERNEL_NS::LibString content;
        ReadUtf8OneLine(fp, content);
        lines.push_back(content);
        ++line;
    }

    return line;
}

bool FileUtil::ReplaceFile(const KERNEL_NS::LibString &fileName, const std::vector<KERNEL_NS::LibString> &newLines)
{
    if(newLines.empty())
    {
        auto fp = FileUtil::OpenFile(fileName.c_str(), true, "wb");
        if(!fp)
        {
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(FileUtil, "open file to clear fail:%s"), fileName.c_str());
            return false;
        }

        FileUtil::CloseFile(*fp);
        return true;
    }

    auto now = LibTime::NowMicroTimestamp();
    auto cache = fileName;
    cache.AppendFormat(".kernel.%lld", now);

    if(FileUtil::IsFileExist(cache.c_str()))
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(FileUtil, "cache file already exists:%s"), cache.c_str());
        return false;
    }

    SmartPtr<FILE, AutoDelMethods::CustomDelete> fp = FileUtil::OpenFile(cache.c_str(), true);
    if(!fp)
    {
        auto errNo = SystemUtil::GetErrNo();
        auto errNoStr = SystemUtil::GetErrString(errNo);
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(FileUtil, "open cache file fail:%s, err:%d, %s"), cache.c_str(), errNo, errNoStr.c_str());
        return false;
    }
    
    fp.SetClosureDelegate([](void *ptr){
        auto fpPtr = reinterpret_cast<FILE *>(ptr);
        FileUtil::CloseFile(*fpPtr);
    });

    Int32 maxLine = static_cast<Int32>(newLines.size());
    for(Int32 idx = 0; idx < maxLine; ++idx)
    {
        auto &lineData = newLines[idx];
        auto ret = FileUtil::WriteFile(*fp, lineData);
        if(ret != static_cast<Int64>(lineData.size()))
        {
            g_Log->Error(LOGFMT_NON_OBJ_TAG(FileUtil, "write line not match write bytes:%lld, real bytes:%lld"), ret, static_cast<Int64>(lineData.size()));
            return false;
        }

        if(idx != (maxLine - 1))
            FileUtil::WriteFile(*fp, LibString("\n"));
    }

    // 落盘
    FileUtil::FlushFile(*fp);
    FileUtil::CloseFile(*fp);
    fp.pop();

    // 原文件转成back文件
    auto backFile = fileName;
    backFile.AppendFormat(".back.%lld", now);
    const bool isSourceFileExist = FileUtil::IsFileExist(fileName.c_str());
    if (isSourceFileExist)
    {
        auto st = FileUtil::Rename(fileName, backFile);
        if (st != Status::Success)
        {
            auto errNo = SystemUtil::GetErrNo();
            auto errNoStr = SystemUtil::GetErrString(errNo);
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(FileUtil, "turn old file:%s to back file:%s fail err:%d, %s")
                , fileName.c_str(), backFile.c_str(), errNo, errNoStr.c_str());

            FileUtil::DelFileCStyle(cache.c_str());
            return false;
        }
    }

    // cache文件转成原文件
    auto st = FileUtil::Rename(cache, fileName);
    if(st != Status::Success)
    {
        auto errNo = SystemUtil::GetErrNo();
        auto errNoStr = SystemUtil::GetErrString(errNo);
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(FileUtil, "turn cache file:%s to fileName:%s fail err:%d, %s")
                    , cache.c_str(), fileName.c_str(), errNo, errNoStr.c_str());

        FileUtil::DelFileCStyle(cache.c_str());

        if (isSourceFileExist)
        {
            st = FileUtil::Rename(backFile, fileName);
            if (st != Status::Success)
            {
                errNo = SystemUtil::GetErrNo();
                errNoStr = SystemUtil::GetErrString(errNo);
                g_Log->Warn(LOGFMT_NON_OBJ_TAG(FileUtil, "turn backFile:%s to fileName:%s fail err:%d, %s")
                    , backFile.c_str(), fileName.c_str(), errNo, errNoStr.c_str());
            }
        }

        return false;
    }

    // 删除back文件
    if(isSourceFileExist)
        FileUtil::DelFileCStyle(backFile.c_str());

    return true;
}

bool FileUtil::ReplaceFile(const LibString &fileName, const std::map<Int32, LibString> &newLines)
{
    if(newLines.empty())
    {
        auto fp = FileUtil::OpenFile(fileName.c_str(), true, "wb");
        if(!fp)
        {
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(FileUtil, "open file to clear fail:%s"), fileName.c_str());
            return false;
        }

        FileUtil::CloseFile(*fp);
        return true;
    }

    auto now = LibTime::NowMicroTimestamp();
    auto cache = fileName;
    cache.AppendFormat(".kernel.%lld", now);

    if(FileUtil::IsFileExist(cache.c_str()))
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(FileUtil, "cache file already exists:%s"), cache.c_str());
        return false;
    }

    SmartPtr<FILE, AutoDelMethods::CustomDelete> fp = FileUtil::OpenFile(cache.c_str(), true);
    if(!fp)
    {
        auto errNo = SystemUtil::GetErrNo();
        auto errNoStr = SystemUtil::GetErrString(errNo);
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(FileUtil, "open cache file fail:%s, err:%d, %s"), cache.c_str(), errNo, errNoStr.c_str());
        return false;
    }
    
    fp.SetClosureDelegate([](void *ptr){
        auto fpPtr = reinterpret_cast<FILE *>(ptr);
        FileUtil::CloseFile(*fpPtr);
    });

    Int32 maxLine = newLines.rbegin()->first;
    for(auto &kv : newLines)
    {
        auto &lineData = kv.second;
        auto ret = FileUtil::WriteFile(*fp, lineData);
        if(ret != static_cast<Int64>(lineData.size()))
        {
            g_Log->Error(LOGFMT_NON_OBJ_TAG(FileUtil, "write line not match write bytes:%lld, real bytes:%lld"), ret, static_cast<Int64>(lineData.size()));
            return false;
        }

        if(kv.first != maxLine)
            FileUtil::WriteFile(*fp, LibString("\n"));
    }

    // 落盘
    FileUtil::FlushFile(*fp);
    FileUtil::CloseFile(*fp);
    fp.pop();

    // 原文件转成back文件
    auto backFile = fileName;
    backFile.AppendFormat(".back.%lld", now);
    const bool isSourceFileExist = FileUtil::IsFileExist(fileName.c_str());
    if(isSourceFileExist)
    {
        auto st = FileUtil::Rename(fileName, backFile);
        if(st != Status::Success)
        {
            auto errNo = SystemUtil::GetErrNo();
            auto errNoStr = SystemUtil::GetErrString(errNo);
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(FileUtil, "turn old file:%s to back file:%s fail err:%d, %s")
                        , fileName.c_str(), backFile.c_str(), errNo, errNoStr.c_str());

            FileUtil::DelFileCStyle(cache.c_str());
            return false;
        }
    }

    // cache文件转成原文件
    auto st = FileUtil::Rename(cache, fileName);
    if(st != Status::Success)
    {
        auto errNo = SystemUtil::GetErrNo();
        auto errNoStr = SystemUtil::GetErrString(errNo);
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(FileUtil, "turn cache file:%s to fileName:%s fail err:%d, %s")
                    , cache.c_str(), fileName.c_str(), errNo, errNoStr.c_str());

        FileUtil::DelFileCStyle(cache.c_str());

        if (isSourceFileExist)
        {
            st = FileUtil::Rename(backFile, fileName);
            if (st != Status::Success)
            {
                errNo = SystemUtil::GetErrNo();
                errNoStr = SystemUtil::GetErrString(errNo);
                g_Log->Warn(LOGFMT_NON_OBJ_TAG(FileUtil, "turn backFile:%s to fileName:%s fail err:%d, %s")
                    , backFile.c_str(), fileName.c_str(), errNo, errNoStr.c_str());
            }
        }

        return false;
    }

    // 删除back文件
    if(isSourceFileExist)
        FileUtil::DelFileCStyle(backFile.c_str());

    return true;
}


KERNEL_END
