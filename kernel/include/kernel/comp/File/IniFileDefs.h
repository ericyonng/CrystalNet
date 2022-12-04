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
 * Date: 2021-02-07 22:33:04
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_FILE_INI_FILE_DEFS_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_FILE_INI_FILE_DEFS_H__

#pragma once

#include <kernel/kernel_inc.h>

KERNEL_BEGIN

class LibString;

class KERNEL_EXPORT IniFileDefs
{
public:
    enum ContentType
    {
        Invalid = 0,            // 无效
        Segment = 1,            // 段
        KeyValue = 2,           // 键值对数据
    };

public:
    static const Byte8 _annotationFlag;        // 注释符默认:';'
    static const Byte8 _leftSegmentFlag;       // 段名左括符:'['
    static const Byte8 _rightSegmentFlag;      // 段名右括符:']'
    static const Byte8 _keyValueJoinerFlag;    // 键值对连接符:'='
    static const Byte8 _changeLineFlag;        // 换行符:'\n'
    static const Byte8 _segKeyJoinerFlag;      // 段与键连接符:'-'用于创建一个segkey的键
    static const Byte8 *_annotationSpaceStr;   // 注释间隔字符串:"\t\t\t\t"
    static const Byte8 _groupDataSeparateFlag;     // 组合数据分隔符:','
};

class KERNEL_EXPORT IniFileMethods
{
public:
    static bool IsSegment(const LibString &content, LibString &segmentOut);
    static bool IfExistKeyValue(const LibString &content);
    static bool ExtractValidRawData(const LibString &content, Int32 &contentTypeOut, LibString &validRawDataOut); // true表示有segment或者kevalue
    static bool SpiltKeyValue(const LibString &validContent, LibString &key, LibString &value); // validContent需要剔除注释后的数据 true表示至少有key

    static bool IsEnglishChar(Byte8 ch);
    static bool IsNumChar(Byte8 ch);
    static bool IsUnderscoreLine(Byte8 ch);

    static void MakeSegKey(const LibString &segment, const LibString &key, LibString &segKeyOut);
    static void MakeKeyValuePairStr(const LibString &key, const LibString &value, LibString &keyValueStrOut);
};

KERNEL_END

#endif
