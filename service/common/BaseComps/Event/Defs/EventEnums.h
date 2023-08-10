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

namespace EventEnums
{
    enum ENUMS
    {
        UNKNOWN = 0,

        // 会话事件[1 - 100]
        SESSION_EVENT_BEGIN = 1,    // 会话事件枚举开始
        
        SESSION_WILL_CREATED = SESSION_EVENT_BEGIN,   // 会话创建
                                                      /* 会话创建
                                                      * Attention: 用于会话管理创建相关会话信息
                                                      * @param(SESSION_ID):UInt64 会话id
                                                      * @param(LOCAL_ADDR):BriefSockAddr Ptr 本地地址
                                                      * @param(REMOTE_ADDR):BriefSockAddr Ptr 远端地址
                                                      * @param(PROTOCOL_TYPE):Int32 协议类型（udp/tcp/quic等）ProtocolType
                                                      * @param(SESSION_TYPE):Int32 会话类型SessionType
                                                      * @param(PRIORITY_LEVEL):UInt32 优先级等级 用于选择消息队列
                                                      * @param(SESSION_POLLER_ID):UInt64 pollerId
                                                      * @param(SERVICE_ID):UInt64 serviceId
                                                      * @param(STUB):UInt64 stub 存根
                                                      * @param(IS_FROM_CONNECT):bool 是否来自connect连接
                                                      * @param(IS_FROM_LINKER):bool 是否来自listen连入
                                                      */

        SESSION_CREATED,        // 会话创建
                                /* 会话创建
                                * Attention: 一般的组件监听这个
                                * @param(SESSION_ID):UInt64 会话id
                                * @param(LOCAL_ADDR):BriefSockAddr Ptr 本地地址
                                * @param(REMOTE_ADDR):BriefSockAddr Ptr 远端地址
                                * @param(PROTOCOL_TYPE):Int32 协议类型（udp/tcp/quic等）ProtocolType
                                * @param(SESSION_TYPE):Int32 会话类型SessionType
                                * @param(PRIORITY_LEVEL):UInt32 优先级等级 用于选择消息队列
                                * @param(SESSION_POLLER_ID):UInt64 pollerId
                                * @param(SERVICE_ID):UInt64 serviceId
                                * @param(STUB):UInt64 stub 存根
                                * @param(IS_FROM_CONNECT):bool 是否来自connect连接
                                * @param(IS_FROM_LINKER):bool 是否来自listen连入
                                */

        SESSION_WILL_DESTROY,   // 会话销毁
                                /* 会话销毁
                                * Attention: 一般的组件监听这个
                                * @param(SESSION_ID):UInt64 会话id
                                * @param(SESSION_CLOSE_REASON):Int32 关闭原因
                                * @param(SERVICE_ID):UInt64 serviceId
                                * @param(PRIORITY_LEVEL):UInt32 优先级等级 用于选择消息队列
                                * @param(STUB):UInt64 stub 存根
                                */

        SESSION_DESTROY,        // 会话销毁 
                                /* 会话销毁
                                * Attention: 用于会话管理销毁会话
                                * @param(SESSION_ID):UInt64 会话id
                                * @param(SESSION_CLOSE_REASON):Int32 关闭原因
                                * @param(SERVICE_ID):UInt64 serviceId
                                * @param(PRIORITY_LEVEL):UInt32 优先级等级 用于选择消息队列
                                * @param(STUB):UInt64 stub 存根
                                */

        ADD_LISTEN_RES,         //  添加监听回调事件 
                                /* 添加监听回调事件
                                * Attention: 
                                * @param(ERROR_CODE):Int32 错误码
                                * @param(LOCAL_ADDR):BriefSockAddr Ptr 本地地址
                                * @param(FAMILY):UInt16 AF
                                * @param(SERVICE_ID):UInt64 serviceId
                                * @param(STUB):UInt64 stub 存根
                                * @param(PRIORITY_LEVEL):UInt32 优先级等级 用于选择消息队列
                                * @param(PROTOCOL_TYPE):Int32 协议类型（udp/tcp/quic等）ProtocolType
                                * @param(SESSION_ID):UInt64 会话id
                                */

        ASYN_CONNECT_RES,       //  连接回调事件 
                                /* 连接回调事件
                                * Attention: 
                                * @param(ERROR_CODE):Int32 错误码
                                * @param(LOCAL_ADDR):BriefSockAddr Ptr 本地地址
                                * @param(REMOTE_ADDR):BriefSockAddr Ptr 远程地址
                                * @param(FAMILY):UInt16 AF
                                * @param(PROTOCOL_TYPE):Int32 协议类型（udp/tcp/quic等）ProtocolType
                                * @param(PRIORITY_LEVEL):UInt32 优先级等级 用于选择消息队列
                                * @param(SESSION_POLLER_ID):UInt64 pollerId
                                * @param(SERVICE_ID):UInt64 serviceId
                                * @param(STUB):UInt64 stub 存根
                                * @param(SESSION_ID):UInt64 会话id
                                */
        SESSION_EVENT_END = 100, // 会话事件枚举结束

        SERVICE_EVENT_BEGIN = 101,
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

        EVENT_COMMON_END = 65536,       // 公共事件集结束
    };
};