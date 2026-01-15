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
 * Date: 2020-10-11 22:05:55
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_DIRECTORY_UTIL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_DIRECTORY_UTIL_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Delegate/LibDelegate.h>
#ifdef CRYSTAL_NET_CPP20
#include <concepts>
#endif

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
 #include <io.h>
#endif

KERNEL_BEGIN

struct FindFileInfo;

class KERNEL_EXPORT DirectoryUtil 
{
public:
    // create folder
    static bool CreateDir(const LibString &path);

    // get file/dir from path
    static LibString GetFileNameInPath(const LibString &path);
    static LibString GetFileNameInPath(const char *path);
    static LibString GetFileDirInPath(const LibString &path);
    static LibString GetFileDirInPath(const char *path);
    static LibString GetFinalDirInPath(const LibString &path);

    // return(bool):是否继续遍历
    // callback: 
    // return(bool):是否继续遍历当前目录, 
    // FileAttr::ENUMS:文件类型, 
    // const _finddata_t &: 文件属性
    // const LibString &:文件所在目录路径, 
    // bool &:父目录需不需要继续搜索 当前目录所在的上级目录是否需要继续遍历
    // depth:目录递归层数, -1:无限层数
    static bool TraverseDirRecursively(const LibString &dir
    , IDelegate<bool, const FindFileInfo &, bool &> *stepCallback, Int32 depth = -1);

    template<typename CallbackType>
    #ifdef CRYSTAL_NET_CPP20
    requires requires(CallbackType cb, const FindFileInfo &findFileInfo, bool &isParentDirContinue)
    {
        { cb(findFileInfo, isParentDirContinue) } -> std::same_as<bool>;
    }
    #endif
    static bool TraverseDirRecursively(const LibString &dir
    , CallbackType &&cb, Int32 depth = -1);

    static bool IsDirExists(const LibString &dir);

private:
    static bool _TraverseDirRecursively(const LibString &dir
    , IDelegate<bool, const FindFileInfo &, bool &> *stepCallback, Int32 &currentDepth, Int32 depth = -1);

    // create sub dir
    static bool _CreateSubDir(const LibString &subDir);
    static bool _CreateSubDir(const std::string &subDir);
    // recursive create dir
    static bool _CreateRecursiveDir(const LibString &masterDir, const LibString &subDir);
    
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    static bool _FindNextFile(intptr_t handle, _finddata_t &fileData);
#endif
};

ALWAYS_INLINE LibString DirectoryUtil::GetFileNameInPath(const char *path)
{
    return GetFileNameInPath(LibString(path));
}

ALWAYS_INLINE LibString DirectoryUtil::GetFileDirInPath(const char *path)
{
    return GetFileDirInPath(LibString(path));
}

ALWAYS_INLINE bool DirectoryUtil::_CreateSubDir(const LibString &subDir)
{
    return _CreateSubDir(subDir.GetRaw());
}

ALWAYS_INLINE bool DirectoryUtil::TraverseDirRecursively(const LibString &dir
    , IDelegate<bool, const FindFileInfo &, bool &> *stepCallback, Int32 depth)
{
    Int32 currentDepth = 1;
    return _TraverseDirRecursively(dir, stepCallback, currentDepth, depth);
}

template<typename CallbackType>
#ifdef CRYSTAL_NET_CPP20
requires requires(CallbackType cb, const FindFileInfo &findFileInfo, bool &isParentDirContinue)
{
    { cb(findFileInfo, isParentDirContinue) } -> std::same_as<bool>;
}
#endif
ALWAYS_INLINE bool DirectoryUtil::TraverseDirRecursively(const LibString &dir
, CallbackType &&cb, Int32 depth)
{
    auto delg = KERNEL_CREATE_CLOSURE_DELEGATE(cb, bool, const FindFileInfo &, bool &);
    auto isContinue = TraverseDirRecursively(dir, delg, depth);
    delg->Release();
    return isContinue;
}

KERNEL_END

#endif
