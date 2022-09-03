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

#include <kernel/kernel_inc.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

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

private:
    // create sub dir
    static bool _CreateSubDir(const LibString &subDir);
    static bool _CreateSubDir(const std::string &subDir);
    // recursive create dir
    static bool _CreateRecursiveDir(const LibString &masterDir, const LibString &subDir);
};

inline LibString DirectoryUtil::GetFileNameInPath(const char *path)
{
    return GetFileNameInPath(LibString(path));
}

inline LibString DirectoryUtil::GetFileDirInPath(const char *path)
{
    return GetFileDirInPath(LibString(path));
}

inline bool DirectoryUtil::_CreateSubDir(const LibString &subDir)
{
    return _CreateSubDir(subDir.GetRaw());
}

KERNEL_END

#endif
