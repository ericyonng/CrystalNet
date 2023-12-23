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
 * Date: 2023-12-14 11:28:55
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <kernel/comp/LibString.h>
#include <kernel/common/statics.h>
#include <kernel/comp/Log/log.h>
#include <kernel/comp/Utils/StringUtil.h>

struct OrmInfo
{
    void Parse(const KERNEL_NS::LibString &info)
    {
        auto parts = info.Split('|');

        for(Int32 idx = 0; idx < static_cast<Int32>(parts.size()); ++idx)
        {
            auto &part = parts[idx];
            switch (idx)
            {
            case 0:
            {
                _orginPbFullName = part.strip();
            }break;
            case 1:
            {
                _ormDataType = part.strip();
            }break;
            case 2:
            {
                _ormId = KERNEL_NS::StringUtil::StringToInt64(part.c_str());
            }break;
            default:
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("unknown index :%d, part:%s, data:%s"), idx, part.c_str(), info.c_str());
                break;
            }
            }
        }
    }

    void Serialize(KERNEL_NS::LibString &info) const
    {
        info.AppendFormat("%s|%s|%lld", _orginPbFullName.c_str(), _ormDataType.c_str(), _ormId);
    }

    KERNEL_NS::LibString _orginPbFullName;
    KERNEL_NS::LibString _ormDataType;
    Int64 _ormId = 0;
};