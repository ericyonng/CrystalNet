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
 * Date: 2022-08-27 23:16:59
 * Author: Eric Yonng
 * Description: 
 * cpp 部分只需要include cpp目录下的所有.h
*/

#ifndef __CRYSTAL_NET_PROTOCOLS_MY_TEST_SERVICE_PROTOCOLS_PROTOCOLS_H__
#define __CRYSTAL_NET_PROTOCOLS_MY_TEST_SERVICE_PROTOCOLS_PROTOCOLS_H__

#pragma once

// protobuf 支持json
#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>
#include <google/protobuf/text_format.h>

#include <protocols/AllPbs.h>
#include <protocols/Opcodes.h>

#include <protocols/OrmMgrFactory.h>
#include <protocols/IOrmMgr.h>
#include <protocols/orm_out/orm_out.h>

#endif
