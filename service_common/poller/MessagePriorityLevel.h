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
 * Date: 2022-06-21 13:09:31
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_SERVICE_COMMON_POLLER_MESSAGE_PRIORITY_LEVEL_H__
#define __CRYSTAL_NET_SERVICE_COMMON_POLLER_MESSAGE_PRIORITY_LEVEL_H__

#pragma once

#include <kernel/kernel.h>
#include <service_common/common/common.h>

SERVICE_COMMON_BEGIN

class MessagePriorityLevel
{
public:
    enum ENUMS
    {
        INNER_NODE = 0,             // 内部节点
        DB_NODE = 1,                // 数据库节点
        OUTSIDE_SESSION = 2,        // 外部连入的会话
        POLLER_MONITOR = 3,         // poller的monitor通道
        End,                        // 最大
    };
};

SERVICE_COMMON_END

#endif
