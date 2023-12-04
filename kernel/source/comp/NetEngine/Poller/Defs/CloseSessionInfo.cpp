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
 * Date: 2022-09-30 12:57:12
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/NetEngine/Poller/Defs/CloseSessionInfo.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(CloseSessionInfo);

LibString CloseSessionInfo::ToString() const
{
    LibString reason;
    reason.AppendFormat("_reason = [%llu, %llx, %s], _lastErrNo=[%d]", _reason, _reason, ToReasonString(_reason).c_str(),  _lastErrNo);

    return reason;
}

LibString CloseSessionInfo::ToReasonString(UInt64 reason) const
{  
    LibString info;
    for(UInt64 idx = CloseSessionInfo::NONE; idx < CloseSessionInfo::REASON_END; ++idx)
    {
        if(IsMask(idx))
            info.AppendFormat("%s|", _TurnStr(static_cast<Int32>(idx)));
    }

    return info;
}

KERNEL_END