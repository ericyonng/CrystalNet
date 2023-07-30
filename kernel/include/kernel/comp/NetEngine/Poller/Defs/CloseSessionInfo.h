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
 * Date: 2022-05-09 12:48:09
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_DEFS_CLOSE_SESSION_INFO_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_DEFS_CLOSE_SESSION_INFO_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Utils/BitUtil.h>
#include <kernel/comp/memory/memory.h>

KERNEL_BEGIN

struct KERNEL_EXPORT CloseSessionInfo
{
    POOL_CREATE_OBJ_DEFAULT(CloseSessionInfo);

    enum CLOSE_REASON : UInt64
    {
        NONE = 0,
        REMOTE_DISONNECT = 1,       // 远端关闭
        SOCK_ERR = 2,               // 套接字错误
        LOCAL_FORCE_CLOSE = 3,      // 本地关闭
        PACKET_PARSING_ERROR = 4,   // 解包错误
        PACKET_PACKET_TO_BIN_ERROR = 5,   // 打包错误
        OTHER = 6,                  // 其他错误
        SERVICE_FINISH = 7,         // 服务结束
        NET_ERROR = 8,              // 网络错误
        CLOSED_BEFORE = 9,          // 之前已关闭过
        SEND_ERROR = 10,            // 发送失败
        BY_BLACK_WHITE_LIST_CHECK = 11, // 黑白名单校验不通过
        REASON_END,                 // 结束
    };

    CloseSessionInfo()
    {
        _lastErrNo = 0;
        _reason = CloseSessionInfo::NONE;
    }
    // return(bool):ischange
    bool Mask(UInt64 reasonFlag)
    {
        if(UNLIKELY(IsMask(reasonFlag)))
            return false;

        _reason = BitUtil::Set(_reason, reasonFlag);
        _lastErrNo = static_cast<Int32>(reasonFlag);
        return true;
    }

    bool IsMask(UInt64 reasonFlag) const
    {
        return BitUtil::IsSet(_reason, reasonFlag);
    }

    LibString ToString() const
    {
        LibString reason;
        reason.AppendFormat("_reason = [%llu, %llx, %s], _lastErrNo=[%d]", _reason, _reason, ToReasonString(_reason).c_str(),  _lastErrNo);

        return reason;
    }

    LibString ToReasonString(UInt64 reason) const
    {  
        LibString info;
        for(UInt64 idx = CloseSessionInfo::NONE; idx < CloseSessionInfo::REASON_END; ++idx)
        {
            if(IsMask(idx))
                info.AppendFormat("%s|", _TurnStr(static_cast<Int32>(idx)));
        }

        return info;
    }

    static const Byte8 *GetCloseReason(Int32 reason)
    {
        return _TurnStr(reason);
    }

private:
    static const Byte8 *_TurnStr(Int32 reasonEnum)
    {
        switch (reasonEnum)
        {
        case CLOSE_REASON::REMOTE_DISONNECT: return "REMOTE_DISONNECT";
        case CLOSE_REASON::SOCK_ERR: return "SOCK_ERR";
        case CLOSE_REASON::LOCAL_FORCE_CLOSE: return "LOCAL_FORCE_CLOSE";
        case CLOSE_REASON::PACKET_PARSING_ERROR: return "PACKET_PARSING_ERROR";
        case CLOSE_REASON::PACKET_PACKET_TO_BIN_ERROR: return "PACKET_PACKET_TO_BIN_ERROR";
        case CLOSE_REASON::OTHER: return "OTHER";
        case CLOSE_REASON::SERVICE_FINISH: return "SERVICE_FINISH";
        case CLOSE_REASON::NET_ERROR: return "NET_ERROR";
        case CLOSE_REASON::CLOSED_BEFORE: return "CLOSED_BEFORE";
        case CLOSE_REASON::SEND_ERROR: return "SEND_ERROR";
        case CLOSE_REASON::BY_BLACK_WHITE_LIST_CHECK: return "BY_BLACK_WHITE_LIST_CHECK";
        default:
            break;

        }

        return "UNKNOWN";
    }

    UInt64 _reason;         // 关闭原因
    Int32 _lastErrNo;       // 最后一次错误码
};

KERNEL_END

#endif
