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
 * Date: 2021-06-18 01:14:57
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_DEFS_POLLER_EVENT_TYPE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_DEFS_POLLER_EVENT_TYPE_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/BaseMacro.h>
#include <kernel/common/BaseType.h>
#include <kernel/comp/Poller/PollerEventInternalType.h>

KERNEL_BEGIN

class KERNEL_EXPORT PollerEventType
{
public:
    enum Type
    {
        EvNone = PollerEventInternalType::MAX_INTERNAL_TYPE,  // 无效
        Write,                  // 发数据 框架层监听的事件
        AsynConnect,            // 连接 框架层监听的事件
        NewSession,             // connect/accept suc 框架层监听的事件
        Monitor,                // 监听器事件 框架层监听的事件
        CloseSession,           // 关闭 框架层监听的事件
        AddListen,              // 监听事件 框架层监听的事件


        SessionCreated,         // 会话创建（连入,连出） 业务层监听的事件
        AsynConnectRes,         // 连接回执 业务层监听的事件
        AddListenRes,           // 监听回执 业务层监听的事件
        SessionDestroy,        // 会话销毁 业务层监听事件
        RecvMsg,               // 收到网络消息 业务层监听事件

        IpRuleControl,         // ip规则控制
        QuitServiceSessionsEvent,  // 退出所有session
        QuitServiceEvent,  // 退出service
        RealDoQuitServiceSessionEvent, // 真正的踢session

        QuitApplicationEvent, // 退出app事件
        HotfixShareLibrary,     // 热更共享库
        HotfixShareLibraryComplete,     // 热更共享库完成
        EvMax,                      // 枚举
    };

    static const Byte8 *ToString(Int32 type)
    {
        switch(type)
        {
        case PollerEventType::EvNone: return "EvNone";
        case PollerEventType::Write: return "Write";                // 框架层监听的事件
        case PollerEventType::AsynConnect: return "AsynConnect";    // 框架层监听的事件
        case PollerEventType::NewSession: return "NewSession";      // 框架层监听的事件
        case PollerEventType::Monitor: return "Monitor";            // 框架层监听的事件
        case PollerEventType::CloseSession: return "CloseSession";  // 框架层监听的事件
        case PollerEventType::AddListen: return "AddListen";        // 框架层监听的事件
        case PollerEventType::IpRuleControl: return "IpRuleControl";  // 框架层监听的事件
        case PollerEventType::QuitServiceSessionsEvent: return "QuitServiceSessionsEvent";  // 退出所有会话 框架层监听的事件

        case PollerEventType::SessionCreated: return "SessionCreated";  // 业务层监听的事件
        case PollerEventType::AsynConnectRes: return "AsynConnectRes";  // 业务层监听的事件 res之前会先收到SessionCreated事件,初始化业务层session, 然后收到res
        case PollerEventType::AddListenRes: return "AddListenRes";  // 业务层监听的事件 res之前会先收到SessionCreated事件,初始化业务层session, 然后收到res
        case PollerEventType::SessionDestroy: return "SessionDestroy";  // 会话销毁 业务层监听事件
        case PollerEventType::RecvMsg: return "RecvMsg";  // 收到网络消息 业务层监听事件
        case PollerEventType::QuitServiceEvent: return "QuitServiceEvent";  // 退出服务 业务层监听事件
        case PollerEventType::QuitApplicationEvent: return "QuitApplicationEvent";  // 退出app
        case PollerEventType::HotfixShareLibrary: return "HotfixShareLibrary";  // 热更事件
        default:
            break;
        };

        return "Unknown";
    }
};

KERNEL_END

#endif
