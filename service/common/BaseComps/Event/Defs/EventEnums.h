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
 * Date: 2022-08-28 05:18:30
 * Author: Eric Yonng
 * Description: 
 * 注意：不指定具体枚举值，只指定区间,使用枚举自增，避免事件的冲突
 * 1.填写Attention
 * 2.填写Params枚举，参数类型，以及参数含义
 * 3.以上为了接入事件时不必去找事件派发地方的参数，面向EventEnums文档极大方便了事件事件
 * 4.参数只增加不删除，避免逻辑错误
 * 5.这里是公共的事件定义，每个特定的service的事件从 EVENT_COMMON_END + 1开始
*/

#pragma once

#include <kernel/comp/CommonEvent.h>

namespace EventEnums
{
    enum ENUMS
    {
        BEGIN = EventEnums::KERNEL_EVENT_END,

        SERVICE_EVENT_BEGIN = BEGIN + 1,
        QUIT_SERVICE_EVENT = SERVICE_EVENT_BEGIN,   //  退出服务事件 
                                                    /* 退出服务事件
                                                    * Attention: 
                                                    */

        SERVICE_COMMON_SESSION_READY,   // 公共会话准备就绪

        SERVICE_WILL_STARTUP,           // service启动完成
        SERVICE_STARTUP,                // service 启动完成
        SERVICE_MSG_RECV,               // service来消息了
                                        /* service来消息了
                                        * Attention: 
                                        * @param(SESSION_ID):Int32 错误码
                                        * @param(OPCODE):Int32
                                        * @param(PACKET):LibPacket * Ptr
                                        */
        SERVICE_EVENT_END = 200,
        
        EVENT_COMMON_END = EventEnums::KERNEL_EVENT_END + 65536,       // 公共事件集结束
    };
};