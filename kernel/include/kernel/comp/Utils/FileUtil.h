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
 * Date: 2021-01-05 23:30:34
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_FILE_UTIL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_FILE_UTIL_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/comp/LibString.h>
#include <map>

KERNEL_BEGIN

struct FindFileInfo;
class LibTime;

class KERNEL_EXPORT FileUtil
{
public:
    static void DelFile(const Byte8 *filePath);
    static bool DelFileCStyle(const Byte8 *filePath);
    static FILE *CreateTmpFile();
    static const Byte8 *GenRandFileName(Byte8 randName[L_tmpnam]);
    static const Byte8 *GenRandFileNameNoDir(Byte8 randName[L_tmpnam]);
    // 清除文件错误 不是追加打开文件的，文件光标设置开头
    static FILE *OpenFile(const Byte8 *fileName, bool isCreate = false, const Byte8 *openType = "rb+");
    static bool CopyFile(const Byte8 *srcFile, const Byte8 *destFile);
    static bool CopyFile(FILE &src, FILE &dest);

    static UInt64 ReadFile(FILE &fp, UInt64 bufferSize, Byte8 *&buffer);
    static UInt64 ReadFile(FILE &fp, LibString &outString, Int64 sizeLimit = -1);
    static Int64 ReadUtf8File(FILE &fp, std::vector<KERNEL_NS::LibString> &lines, Int64 lineLimit = -1);
    static Int64 WriteFile(FILE &fp, const Byte8 *buffer, Int64 dataLenToWrite);
    static Int64 WriteFile(FILE &fp, const LibString &bitData);
    static bool IsEnd(FILE &fp);
    static bool CloseFile(FILE &fp);
    static bool IsFileExist(const Byte8 *fileName);
    static Int32 GetFileCusorPos(FILE &fp);
    // enum pos:FileCursorOffsetType::FILE_CURSOR_POS
    static bool SetFileCursor(FILE &fp, Int32 enumPos, Long offset);
    static void ResetFileCursor(FILE &fp);
    static bool FlushFile(FILE &fp);
    static Long GetFileSize(FILE &fp);
    static Int64 GetFileSizeEx(const Byte8 *filepath);
    static void InsertFileTime(const LibString &extensionName, const LibTime &timestamp, LibString &fileName);
    static void InsertFileTail(const LibString &extensionName, const Byte8 *tail, LibString &fileName);
    static LibString ExtractFileExtension(const LibString &fileName);
    static LibString ExtractFileWithoutExtension(const LibString &fileName); 
    static Int32 GetFileNo(FILE *fp);
    static Int32 Rename(const LibString &oldPathFile, const LibString &newPathFile);
    static bool ReplaceFile(const LibString &fileName, const std::vector<LibString> &newLines);
    static bool ReplaceFile(const LibString &fileName, const std::map<Int32, LibString> &newLines);

    //
    // 读取ASCII行
    static UInt64 ReadOneLine(FILE &fp, UInt64 bufferSize, Byte8 *&buffer);  // 不包含\n
    static UInt64 ReadOneLine(FILE &fp, LibString &outBuffer);              // 不包含\n

    // 读取utf8行
    static UInt64 ReadUtf8OneLine(FILE &fp, LibString &outBuffer, UInt64 *utf8CharCount = NULL); // 不包含\n(剔除\n)
    
    // 是否文件
    static bool IsFile(const FindFileInfo &fileAttr);
    // 是否目录
    static bool IsDir(const FindFileInfo &fileAttr);
};

ALWAYS_INLINE Int64 FileUtil::WriteFile(FILE &fp, const LibString &bitData)
{
    return WriteFile(fp, bitData.GetRaw().data(), bitData.size());
}


ALWAYS_INLINE void FileUtil::InsertFileTail(const LibString &extensionName, const Byte8 *tail, LibString &fileName)
{
    std::string &raw = fileName.GetRaw();
    auto endPos = raw.rfind('.', fileName.length() - 1);
    if(endPos == std::string::npos)
    {
        fileName << tail << extensionName;
        return;
    }

    raw.insert(endPos, tail);
}

ALWAYS_INLINE LibString FileUtil::ExtractFileExtension(const LibString &fileName)
{
    const std::string &raw = fileName.GetRaw();
    auto endPos = raw.rfind('.', fileName.length() - 1);
    if(endPos == std::string::npos)
        return "";

    return raw.substr(endPos, fileName.length() - endPos);
}

ALWAYS_INLINE LibString FileUtil::ExtractFileWithoutExtension(const LibString &fileName)
{
    const std::string &raw = fileName.GetRaw();
    auto endPos = raw.rfind('.', fileName.length() - 1);
    if(endPos == std::string::npos)
        return fileName;

    if(endPos == 0)
        return "";

    return raw.substr(0, endPos);
}

KERNEL_END

#endif
