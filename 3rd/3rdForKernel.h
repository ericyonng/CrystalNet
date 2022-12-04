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
 * Date: 2021-02-07 02:02:53
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __3rd_3rd_FOR_KERNEL_H__
#define __3rd_3rd_FOR_KERNEL_H__

#pragma once

// openssl

// openssl 请注意内存释放
#include <openssl/md5.h>
#include <openssl/aes.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/buffer.h>
#include <openssl/sha.h>
#include <openssl/bn.h>

#ifdef _WIN32

    // lib static lib
    #ifdef _DEBUG
        // openssl依赖Crypt32
        #pragma comment(lib, "Crypt32.lib")
        #pragma comment(lib, "libcrypto.lib")
        #pragma comment(lib, "libssl.lib")
    #else
        // openssl依赖Crypt32
        #pragma comment(lib, "Crypt32.lib")
        #pragma comment(lib, "libcrypto.lib")
        #pragma comment(lib, "libssl.lib")
    #endif
    
#endif

// #ifdef _WIN32

//     // lib static lib
//     #ifdef _DEBUG
//         #pragma comment(lib, "libprotobufd.lib")
//         #pragma comment(lib, "libprotocd.lib")
//     #else
//         #pragma comment(lib, "libprotobuf.lib")
//         #pragma comment(lib, "libprotoc.lib")
//     #endif
    
// #endif



// 使用utf8_string 禁用第三方utf8库
// #include "3rd/tiny-utf8/include/tinyutf8.h"

// 使用dbghelp
#ifdef _WIN32
    #include <DbgHelp.h>
    #ifdef _DEBUG
        #pragma comment(lib, "DbgHelp\\debug\\DbgHelp.Lib")
    #else
        #pragma comment(lib, "DbgHelp\\release\\DbgHelp.Lib")
    #endif
    
#endif

#endif
