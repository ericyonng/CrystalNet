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
 * Date: 2023-06-11 22:57:49
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <OptionComp/storage/mysql/impl/SqlBuilder.h>
#include <mysql.h>

KERNEL_BEGIN

const LibString FullTextParser::WITH_PARSER_NGRAM = "WITH PARSER ngram";


void SqlEscape::escape(KERNEL_NS::LibString &str)
{
    auto buffer = reinterpret_cast<Byte8 *>(KERNEL_ALLOC_MEMORY_TL((str.size() * 2 + 1)));
    ::memset(buffer, 0, static_cast<size_t>(str.size() * 2 + 1));
    mysql_escape_string(buffer, str.data(), static_cast<ULong>(str.size()));
    str.clear();
    str.AppendData(buffer, ::strlen(buffer));
    KERNEL_FREE_MEMORY_TL(buffer);
}


KERNEL_END