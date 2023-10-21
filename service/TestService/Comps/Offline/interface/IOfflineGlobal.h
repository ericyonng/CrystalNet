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
 * Date: 2023-10-21 19:08:00
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <ServiceCompHeader.h>

SERVICE_BEGIN

class IOfflineGlobal : public IGlobalSys
{
    POOL_CREATE_OBJ_DEFAULT_P1(IGlobalSys, IOfflineGlobal);

public:
    virtual bool AddOfflineData(Int32 offlineType, UInt64 userId, const KERNEL_NS::LibString &offlineData) = 0;
    template<typename PbMsgType>
    bool AddOfflineData(Int32 offlineType, UInt64 userId, const PbMsgType &offlineData);

};

template<typename PbMsgType>
ALWAYS_INLINE bool IOfflineGlobal::AddOfflineData(Int32 offlineType, UInt64 userId, const PbMsgType &offlineData)
{
    auto stream = KERNEL_NS::LibStreamTL::NewThreadLocal_LibStream();

    if(!offlineData.Encode(*stream))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("pb encode fail offlineType:%d, userId:%llu, pb:%s")
        , offlineType, userId, offlineData.ToJsonString().c_str());

        KERNEL_NS::LibStreamTL::DeleteThreadLocal_LibStream(stream);
        return false;
    }

    KERNEL_NS::LibString data;
    if(!stream->SerializeTo(data))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("stream serialize to string fail offlineType:%d, userId:%llu, pb:%s")
        , offlineType, userId, offlineData.ToJsonString().c_str());

        KERNEL_NS::LibStreamTL::DeleteThreadLocal_LibStream(stream);
        return false;
    }

    KERNEL_NS::LibStreamTL::DeleteThreadLocal_LibStream(stream);
    return AddOfflineData(offlineType, userId, data);
}

SERVICE_END