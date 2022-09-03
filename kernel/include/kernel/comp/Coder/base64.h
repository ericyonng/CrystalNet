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
 * Date: 2021-02-06 20:41:54
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_CODER_BASE64_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_CODER_BASE64_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

#undef CRYSTAL_BASE64_PADDING
#define CRYSTAL_BASE64_PADDING '='

class KERNEL_EXPORT LibBase64
{
public:
    static LibString Encode(const LibString &src);
    static bool Encode(const Byte8 *data, UInt64 len, LibString &base64Text);
    static bool Encode(const Byte8 *data, UInt64 len, Byte8 *outBuffer, UInt64 &outLen);
    static LibString Decode(const LibString &src);
    static bool Decode(const Byte8 *data, UInt64 inputLen, Byte8 *buffer, UInt64 &len);
    static bool Decode(const Byte8 *data, UInt64 inputLen, LibString &plainText);

    // 计算反编码出来的字符串长度decodeLen, 实际需要的buffer大小 = decodeLen + 1
    static UInt64 CalcDecodeLen(const Byte8 *input, UInt64 len);
    static UInt64 CalcEncodeLen(UInt64 dataLen);
};

inline LibString LibBase64::Encode(const LibString &src)
{
    LibString outStr;
    Encode(src.data(), src.size(), outStr);
    return outStr;
}

inline bool LibBase64::Encode(const Byte8 *data, UInt64 len, LibString &base64Text)
{
	// BUF_MEM *bufferPtr;
	// BIO *b64 = BIO_new(BIO_f_base64());
	// BIO *bio = BIO_new(BIO_s_mem());
	// bio = BIO_push(b64, bio);

	// // Ignore newlines - write everything in one line
	// BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
	// BIO_write(bio, data, len);
	// BIO_flush(bio);
	// BIO_get_mem_ptr(bio, &bufferPtr);
    
    // // 拷贝数据
    // base64Text.AppendData(bufferPtr->data, bufferPtr->length);
	// BIO_free_all(bio);
    UInt64 outputLen = CalcEncodeLen(len);
    if (UNLIKELY(outputLen == 0))
    {
        base64Text.clear();
        return true;
    }

    base64Text.resize(outputLen);
    return Encode(data, len, const_cast<Byte8 *>(base64Text.data()), outputLen);
}


//  inline bool LibBase64::Decode(const Byte8 *data, Int32 decodeLen, Byte8 *buffer, Int32 *len)
//  {
// 	BIO *bio = BIO_new_mem_buf(data, -1);
// 	BIO *b64 = BIO_new(BIO_f_base64());
// 	bio = BIO_push(b64, bio);

// 	// Do not use newlines to flush buffer
// 	BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
// 	*len = BIO_read(bio, buffer, static_cast<Int32>(::strlen(data)));

// 	// len should equal decodeLen, else something went horribly wrong
//     if(UNLIKELY(*len != decodeLen))
//     {
//         CRYSTAL_TRACE("BIO_read fail len[%d] is not equal decodeLen[%d]", *len, decodeLen);
// 	    BIO_free_all(bio);
//         return false;
//     }

// 	buffer[decodeLen] = '\0';

// 	BIO_free_all(bio);

//     return true;
//  }

// inline bool LibBase64::Decode(const Byte8 *data, Byte8 *buffer, Int32 *len)
// {
// 	Int32 decodeLen = CalcDecodeLen(data);
//     return Decode(data, decodeLen, buffer, len);
// }

inline LibString LibBase64::Decode(const LibString &src)
{
    LibString outStr;
    Decode(src.data(), src.size(), outStr);
    return outStr;
}

inline bool LibBase64::Decode(const Byte8 *data, UInt64 inputLen, LibString &plainText)
{
    UInt64 outputLen = CalcDecodeLen(data, inputLen);
    if (UNLIKELY(outputLen == 0))
    {
        plainText.clear();
        return true;
    }

    plainText.resize(outputLen);
    return Decode(data, inputLen, const_cast<char *>(plainText.data()), outputLen);
}


inline UInt64 LibBase64::CalcDecodeLen(const Byte8 *input, UInt64 len)
{
    UInt64 placeHolderCount = 0;
    for (UInt64 i = len - 1; input[i] == CRYSTAL_BASE64_PADDING; --i)
        ++placeHolderCount;

    return ((6 * len) / 8) - placeHolderCount;
}

inline UInt64 LibBase64::CalcEncodeLen(UInt64 dataLen)
{
    return (dataLen + 2 - ((dataLen + 2) % 3)) / 3 * 4;
}
KERNEL_END

#endif
