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
 * Date: 2022-10-19 00:58:07
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_DEFS_FIND_FILE_INFO_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_DEFS_FIND_FILE_INFO_H__

#pragma once

#include <kernel/comp/LibString.h>

KERNEL_BEGIN

struct KERNEL_EXPORT FindFileInfo
{
    enum ENUMS : UInt64
    {
        F_FILE = 1LLU,
        F_DIR = (1LLU << 1),
    };

    LibString ToString() const
    {
        LibString info;
        info
            .AppendFormat("file name:%s, ", _fileName.c_str())
            .AppendFormat("root path:%s, ", _rootPath.c_str())
            .AppendFormat("extension:%s, ", _extension.c_str())
            .AppendFormat("full name:%s, ", _fullName.c_str())
            .AppendFormat("file attr:%llx, ", _fileAttr)
            .AppendFormat("modify time:%lld, ", _modifyTime)
            ;

        return info;
    }

    LibString _fileName;     // 文件名
    LibString _extension;    // 扩展名 .xxx
    LibString _rootPath;    // 文件所在目录路径
    LibString _fullName;    // 文件完整名
    UInt64 _fileAttr = 0;   // 文件属性
    Int64 _modifyTime = 0;  // 文件修改时间
};

KERNEL_END

#endif
