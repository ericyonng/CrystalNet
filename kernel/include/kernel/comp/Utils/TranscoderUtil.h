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
 * Date: 2023-06-10 17:13:00
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_TRANSCODER_UTIL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_TRANSCODER_UTIL_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>

KERNEL_BEGIN

class LibString;

/**
 * \brief The llbc library transcoder class encapsulation.
 */
class KERNEL_EXPORT TranscoderUtil
{
public:

    /**
     * Map a character string to a wide-character(Unicode) string.
     * @param[in]  fromCode - from code page, like UTF8, GP936, BIG5, ... eg.
     * @param[in]  src      - string to map.
     * @param[out] dest     - wide-character string.
     * @return int - return 0 if success, otherwise return -1.
     */
    static Int32 MultiByteToWideChar(const LibString &fromCode, const LibString &src, LibString &dest);

    /**
     * Map a wide-character(Unicode) to a character string.
     * @param[in]  toCode   - to code page, like UTF8, GP936, BIG5, ... eg.
     * @param[in]  src      - string to map.
     * @param[out] dest     - multi-byte character string.
     * @return int - return 0 if success, otherwise return -1.
     */
    static Int32 WideCharToMultiByte(const LibString &toCode, const LibString &src, LibString &dest);
    
    /**
     * Map a multi-byte character string to another character-set's multi-byte character string.
     * @param[in]  fromCode - from code page.
     * @param[in]  src      - string to map.
     * @param[in]  srcFile  - contain string to map.
     * @param[in]  toCode   - another codepage name.
     * @param[out] dest     - multi-byte character string.
     * @param[out] destFile - multi-byte character string file.
     * @return int - return 0 if success, otherwise return -1.
     */
    static Int32 MultiByteToMultiByte(const LibString &fromCode,
                                    const LibString &src,
                                    const LibString &toCode,
                                    LibString &dest);
};

KERNEL_END

#endif
