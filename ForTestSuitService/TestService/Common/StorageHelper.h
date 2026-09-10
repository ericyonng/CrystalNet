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
 * Date: 2023-08-02 22:38:00
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <service/common/macro.h>
#include <service/common/status.h>

#include <kernel/comp/LibString.h>
#include <google/protobuf/message.h>
#include <set>


SERVICE_BEGIN

class IStorageInfo;

class StorageHelper
{
public:
    // field是map/repeated/Message的会使用text 请序列化的时候序列化成json
    static bool AddMysqlStorageInfoWithPb(IStorageInfo *ownerStorageInfo
    , const ::google::protobuf::Descriptor *descriptor
    , const KERNEL_NS::LibString *primaryKey
    , const std::set<KERNEL_NS::LibString> &uniqueKeys
    , const std::set<KERNEL_NS::LibString> &indexs);
};

SERVICE_END
