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
 * Date: 2021-04-18 23:47:44
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_DEFS_NET_CFG_DEFS_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_DEFS_NET_CFG_DEFS_H__

#pragma once

// // connector配置
#undef CONNECT_SEGMENT
#define CONNECTOR_SEGMENT "Connector"

// 连接超时间隔时间
#undef CONNECTOR_TIMEOUT_MS_PER_TIMES_FIELD_NAME
#define CONNECTOR_TIMEOUT_MS_PER_TIMES_FIELD_NAME "CONNECT_TIMEOUT_MS_PER_TIMES" 
#undef CONNECTOR_TIMEOUT_MS_PER_TIMES_FIELD_VALUE
#define CONNECTOR_TIMEOUT_MS_PER_TIMES_FIELD_VALUE 5000    // 每次连接5s

// 重连次数
#undef CONNECTOR_RETRY_TIMES_FIELD_NAME
#define CONNECTOR_RETRY_TIMES_FIELD_NAME "reconnectTimes"
#undef CONNECTOR_RETRY_TIMES_FIELD_VALUE
#define CONNECTOR_RETRY_TIMES_FIELD_VALUE 3

// 黑白名单模式
#undef CONNECTOR_BLACK_WHITE_MODE_FIELD_NAME
#define CONNECTOR_BLACK_WHITE_MODE_FIELD_NAME "ConnectBlackWhiteFlagMode"
#undef CONNECTOR_BLACK_WHITE_MODE_FIELD_VALUE
#define CONNECTOR_BLACK_WHITE_MODE_FIELD_VALUE (1U << BlackWhiteFlag::CheckBlack)


#endif
