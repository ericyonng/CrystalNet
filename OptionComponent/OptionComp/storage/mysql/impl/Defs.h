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
 * Date: 2023-07-08 18:19:00
 * Author: Eric Yonng
 * Description: Mysql定义
*/

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MYSQL_IMPL_DEFS_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MYSQL_IMPL_DEFS_H__

#pragma once

// mysql 网络错误 此时sql需要重新处理或者放入pending队列等到网络正常再处理
#ifndef IS_MYSQL_NETWORK_ERROR
 #define IS_MYSQL_NETWORK_ERROR(x) (((x) == CR_SERVER_GONE_ERROR) || ((x) == CR_SERVER_LOST))
#endif

// 是否mysql数值
#ifndef IS_MYSQL_NUM
 #define IS_MYSQL_NUM(x) (\
    ((x) == MYSQL_TYPE_TINY) ||\
    ((x) == MYSQL_TYPE_YEAR) ||\
    ((x) == MYSQL_TYPE_SHORT) ||\
    ((x) == MYSQL_TYPE_INT24) ||\
    ((x) == MYSQL_TYPE_LONG) ||\
    ((x) == MYSQL_TYPE_LONGLONG) ||\
    ((x) == MYSQL_TYPE_FLOAT) ||\
    ((x) == MYSQL_TYPE_DOUBLE) ||\
    )

#endif

// 释放需要 mysql_stmt_send_long_data
#ifndef IS_MYSQL_NEED_SEND_LONG_DATA
 #define IS_MYSQL_NEED_SEND_LONG_DATA(x)        \
    ((x) == MYSQL_TYPE_BLOB) ||                 \
    ((x) == MYSQL_TYPE_STRING) ||               \
    ((x) == MYSQL_TYPE_TINY_BLOB) ||            \
    ((x) == MYSQL_TYPE_MEDIUM_BLOB) ||          \
    ((x) == MYSQL_TYPE_LONG_BLOB) ||            \
    ((x) == MYSQL_TYPE_VAR_STRING)
#endif

#endif
