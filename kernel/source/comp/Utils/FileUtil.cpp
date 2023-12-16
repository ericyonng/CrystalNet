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
#include <kernel/common/macro.h>
#include <kernel/comp/File/FileCursorOffsetType.h>
#include <kernel/comp/Utils/FileUtil.h>
#include <kernel/comp/Utils/DirectoryUtil.h>
#include <kernel/comp/Utils/StringUtil.h>
#include <kernel/comp/Utils/SystemUtil.h>
#include <kernel/comp/LibTime.h>
#include <kernel/comp/SmartPtr.h>
#include <kernel/comp/Log/log.h>
#include <kernel/comp/Utils/Defs/FindFileInfo.h>
#include <kernel/comp/LibTime.h>
#include <stdio.h>
#include <stdlib.h>

#if CRYSTAL_TARGET_PLATFORM_LINUX
 #include <unistd.h>
#endif

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
 #include <io.h>          // access func 遍历目录
#endif

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

FILE *FileUtil::OpenFile(const Byte8 *fileName, bool isCreate /*= false*/, const Byte8 *openType /*= "rb+"*/)
{
    if(fileName == NULL || openType == NULL)
        return NULL;

    FILE *fp = NULL;
    fp = ::fopen(fileName, openType);
    if(!fp)
    {
        if(isCreate)
        {
            fp = ::fopen(fileName, "wb+");
            if(!fp)
                return NULL;
                
            FileUtil::CloseFile(*fp);
            fp = ::fopen(fileName, openType);
            if(!fp)
            {
                return NULL;
            }
        }
        else
        {
            return NULL;
        }
    }

    clearerr(fp);

    // 不是追加打开文件的设置开头
    if(!strchr(openType, 'a'))
        rewind(fp);
    return fp;
}

bool FileUtil::CopyFile(const Byte8 *srcFile, const Byte8 *destFile)
{
    if(UNLIKELY(!srcFile || !destFile))
        return false;

    auto srcFp = OpenFile(srcFile);
    if(!srcFp)
        return false;

    auto destFp = OpenFile(destFile, true, "wb+");
    if(!destFp)
        return false;

    unsigned char get_c = 0;
    char count = 0, wrCount = 0;
    bool isDirty = false;

    while(!feof(srcFp))
    {
        get_c = 0;
        count = char(fread(&get_c, 1, 1, srcFp));
        if(count != 1)
            break;

        wrCount = char(fwrite(&get_c, 1, 1, destFp));
        if(wrCount != 1)
            break;

        isDirty = true;
    }

    if(isDirty)
        FlushFile(*destFp);

    CloseFile(*srcFp);
    CloseFile(*destFp);
    return true;
}

bool FileUtil::CopyFile(FILE &src, FILE &dest)
{
    clearerr(&src);
    clearerr(&dest);
    unsigned char get_c = 0;
    char count = 0, wrCount = 0;
    bool isDirty = false;
    while(!feof(&src))
    {
        get_c = 0;
        count = char(fread(&get_c, 1, 1, &src));
        if(count != 1)
            break;

        wrCount = char(fwrite(&get_c, 1, 1, &dest));
        if(wrCount != 1)
            break;

        isDirty = true;
    }

    if(isDirty)
        FlushFile(dest);

    return true;
}

UInt64 FileUtil::ReadFile(FILE &fp, UInt64 bufferSize, Byte8 *&buffer)
{
    if(!buffer || !bufferSize)
        return 0;

    clearerr(&fp);
    UInt64 readCnt = 0;
    U8 *bufferTmp = reinterpret_cast<U8 *>(buffer);
    U8 get_c = 0;
    while(!feof(&fp))
    {
        get_c = 0;
        if(fread(&get_c, sizeof(get_c), 1, &fp) == 1)
        {
            *bufferTmp = get_c;
            ++bufferTmp;
            ++readCnt;

            if(readCnt >= bufferSize)
                break;
        }
        else
        {
            break;
        }
    }

    return readCnt;
}

UInt64 FileUtil::ReadFile(FILE &fp, LibString &outString, Int64 sizeLimit)
{
    clearerr(&fp);
    UInt64 readCnt = 0;
    U8 get_c = 0;
    while(!feof(&fp))
    {
        get_c = 0;
        if(fread(&get_c, sizeof(get_c), 1, &fp) == 1)
        {
            outString.AppendData(reinterpret_cast<const Byte8 *>(&get_c), 1);
            ++readCnt;

            if(sizeLimit > 0 && 
               static_cast<Int64>(readCnt) >= sizeLimit)
                break;
        }
        else
        {
            break;
        }
    }

    return readCnt;
}

Int64 FileUtil::WriteFile(FILE &fp, const Byte8 *buffer, Int64 dataLenToWrite)
{
//     if(!buffer || dataLenToWrite == 0)
//         return 0;

    clearerr(&fp);
    auto handle = static_cast<Int64>(::fwrite(buffer, dataLenToWrite, 1, &fp));
    if(LIKELY(handle != 1))
    {
        #if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
            perror("fwrite fail");
        #endif
        return 0;
    }

//     if(dataLenToWrite != cnt)
//         printf("write error!");
    return dataLenToWrite;
}

Long FileUtil::GetFileSize(FILE &fp)
{
    auto curPos = ftell(&fp);
    if(UNLIKELY(curPos < 0))
        return -1;

    if(UNLIKELY(!SetFileCursor(fp, FileCursorOffsetType::FILE_CURSOR_POS_END, 0L)))
        return -1;

    auto fileSize = ftell(&fp);
    if(UNLIKELY(fileSize < 0))
    {
        SetFileCursor(fp, FileCursorOffsetType::FILE_CURSOR_POS_SET, curPos);
        return -1;
    }

    SetFileCursor(fp, FileCursorOffsetType::FILE_CURSOR_POS_SET, curPos);
    return fileSize;
}

Int64 FileUtil::GetFileSizeEx(const Byte8 *filepath)
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    struct _stat info;
    if(::_stat(filepath, &info) != 0)
        return -1;

    return info.st_size;
#else
    struct stat info;
    if(::stat(filepath, &info) != 0)
        return -1;

    return info.st_size;
#endif

}

UInt64 FileUtil::ReadOneLine(FILE &fp, UInt64 bufferSize, Byte8 *&buffer)
{
    if(!buffer || bufferSize == 0)
        return 0;

    U8 get_c = 0;
    U8 *bufferTmp = reinterpret_cast<U8 *>(buffer);
    ::memset(bufferTmp, 0, bufferSize);

    clearerr(&fp);
    UInt64 cnt = 0;
    while(!feof(&fp))
    {
        get_c = 0;
        if(fread(&get_c, sizeof(get_c), 1, &fp) == 1)
        {
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
            if(get_c == '\r')
                continue;

            if(get_c != '\n')
            {
                *bufferTmp = get_c;
                ++bufferTmp;
                ++cnt;

                if(bufferSize <= cnt) 
                    break;
            }
            else
            {
                //SetFileCursor(fp, FileCursorOffsetType::FILE_CURSOR_POS_CUR, 0);
                break;
            }

#else
            if(get_c != '\n')
            {
                *bufferTmp++ = get_c;
                ++cnt;

                if(bufferSize <= cnt) 
                    break;
            }
            else
            {
                //fread(&get_c, sizeof(get_c), 1, fpOutCache);
                break;
            }
#endif

        }
        else
        {
            break;
        }
    }

    return cnt;
}

UInt64 FileUtil::ReadOneLine(FILE &fp, LibString &outBuffer)
{
    clearerr(&fp);
    unsigned char get_c = 0;
    UInt64 cnt = 0;
    while(!feof(&fp))
    {
        get_c = 0;
        if(fread(&get_c, sizeof(get_c), 1, &fp) == 1)
        {
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
            if(get_c == '\r')
                continue;

            if(get_c != '\n')
            {
                outBuffer.AppendData(reinterpret_cast<const char *>(&get_c), 1);
                ++cnt;
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
            }
            else
            {
                //fread(&get_c, sizeof(get_c), 1, fpOutCache);
                break;
            }
#endif
        }
        else
        {
            break;
        }
    }

    return cnt;
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

bool FileUtil::ReplaceFileBy(const KERNEL_NS::LibString &fileName, const std::vector<KERNEL_NS::LibString> &newLines)
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

bool FileUtil::ReplaceFileBy(const LibString &fileName, const std::map<Int32, LibString> &newLines)
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

bool FileUtil::IsFile(const FindFileInfo &fileAttr)
{
    return fileAttr._fileAttr & FindFileInfo::F_FILE;
}

bool FileUtil::IsDir(const FindFileInfo &fileAttr)
{
    return fileAttr._fileAttr & FindFileInfo::F_DIR;
}

void FileUtil::InsertFileTime(const LibString &extensionName, const LibTime &timestamp, LibString &fileName)
{
    std::string &raw = fileName.GetRaw();
    auto endPos = raw.rfind('.', fileName.length() - 1);
    const auto &timeFmtStr = timestamp.Format("-%Y-%m-%d");
    if(endPos == std::string::npos)
    {
        fileName << timeFmtStr << extensionName;
        return;
    }
    raw.insert(endPos, timeFmtStr.GetRaw());
}

Int32 FileUtil::GetFileNo(FILE *fp)
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    int fileNo = ::_fileno(fp);
#else
    int fileNo = ::fileno(fp);
#endif

    if (UNLIKELY(fileNo == -1))
    {
        CRYSTAL_TRACE("GetFileNo FAIL fp[%p]", fp);
        return -1;
    }

    return fileNo;
}


Int32 FileUtil::GetFileCusorPos(FILE &fp)
{
    return ftell(&fp);
}

bool FileUtil::SetFileCursor(FILE &fp, Int32 enumPos, Long offset)
{
    return fseek(&fp, offset, enumPos) == 0;
}

void FileUtil::ResetFileCursor(FILE &fp)
{
    clearerr(&fp);
    rewind(&fp);
}

bool FileUtil::FlushFile(FILE &fp)
{
    return fflush(&fp) == 0;
}

Int32 FileUtil::Rename(const LibString &oldPathFile, const LibString &newPathFile)
{
    auto ret = ::rename(oldPathFile.c_str(), newPathFile.c_str());
    if(ret != 0)
        return Status::Failed;

    return Status::Success;
}

void FileUtil::DelFile(const Byte8 *filePath)
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    std::string strDelCmd = "del ";
    strDelCmd += filePath;
    size_t findPos = 0;
    int nCount = 0;
    const auto strCount = strDelCmd.length();
    while((findPos = strDelCmd.find_first_of('/', findPos)) != std::string::npos)
    {
        strDelCmd[findPos] = '\\';
    }
    strDelCmd += " /f/s/q";

    system(strDelCmd.c_str());
#else
    std::string strDelCmd = "sudo rm -rf ";
    strDelCmd += filePath;
    if(system(strDelCmd.c_str()) == -1)
        perror("del files fail");
#endif
}

bool FileUtil::DelFileCStyle(const Byte8 *filePath)
{
    return remove(filePath) == 0;
}

FILE  *FileUtil::CreateTmpFile()
{
    return tmpfile();
}

// const Byte8 *FileUtil::GenRandFileName(Byte8 randName[L_tmpnam])
// {
//     return tmpnam(randName);
// }


bool FileUtil::IsEnd(FILE &fp)
{
    return feof(&fp);
}

bool FileUtil::CloseFile(FILE &fp)
{
    clearerr(&fp);
    if(fclose(&fp) != 0)
        return false;

    return true;
}

bool FileUtil::IsFileExist(const Byte8 *fileName)
{
    if(UNLIKELY(!fileName))
        return false;

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    if(::_access(fileName, 0) == -1)
        return false;
#else
    if(::access(fileName, 0) == -1)
        return false;
#endif

    return true;
}

KERNEL_END
