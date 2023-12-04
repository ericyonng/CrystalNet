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
 * Date: 2022-10-19 23:29:07
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <service/ProtoGenService/Comps/Exporter/Defs/ProtoContentInfo.h>
#include <service/ProtoGenService/Comps/Exporter/Defs/MessageInfo.h>
#include <service/ProtoGenService/Comps/Exporter/Defs/Defs.h>

SERVICE_BEGIN
POOL_CREATE_OBJ_DEFAULT_IMPL(ProtoContentInfo);

ProtoContentInfo::ProtoContentInfo()
:_isMd5Change(false)
{

}

ProtoContentInfo::~ProtoContentInfo()
{
    KERNEL_NS::ContainerUtil::DelContainer2(_messageNameRefMessageInfo);
}

void ProtoContentInfo::Release()
{
    ProtoContentInfo::Delete_ProtoContentInfo(this);
}

void ProtoContentInfo::MakeMd5()
{
    MD5_CTX ctx;
    if(!KERNEL_NS::LibDigest::MakeMd5Init(&ctx))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("md5 ctx init fail"));
        return;
    }
    for(auto &iterLineData: _lineRefContent)
    {
        UInt64 len = static_cast<UInt64>(iterLineData.second.size());
        if(!KERNEL_NS::LibDigest::MakeMd5Continue(&ctx, (iterLineData.second + "\n").c_str(), len))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("md5 ctx continue fail, line:%d, lineData:%s, size:%llu")
            , iterLineData.first, iterLineData.second.c_str(), len);
            KERNEL_NS::LibDigest::MakeMd5Clean(&ctx);
            return;
        }
    }

    if(!KERNEL_NS::LibDigest::MakeMd5Final(&ctx, _md5))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("md5 final fail line data lines:%llu"), static_cast<UInt64>(_lineRefContent.size()));
    }

    KERNEL_NS::LibDigest::MakeMd5Clean(&ctx);
}

KERNEL_NS::LibString ProtoContentInfo::ToString() const
{
    KERNEL_NS::LibString info;
    info
    .AppendFormat("md5:%s, ", _md5.c_str())
    .AppendFormat("is md5 change:%s, ", _isMd5Change ? "true" : "false")
    .AppendFormat("_protoInfo:%s, ", _protoInfo.ToString().c_str())
    .AppendFormat("full path:%s, ", _fullPathName.c_str())
    .AppendFormat("file lines:%llu, ", static_cast<UInt64>(_lineRefContent.size()))
    .AppendFormat("pb message count:%llu, ", static_cast<UInt64>(_messageNameRefMessageInfo.size()))
    ;

    return info;
}

PbCacheFileInfo ProtoContentInfo::ToPbCache() const
{
    PbCacheFileInfo info;
    info._protoName = _protoInfo._fileName;
    info._protoPath = _fullPathName;
    info._md5 = _md5;
    info._modifyTime = _protoInfo._modifyTime;

    return info;
}

const MessageInfo *ProtoContentInfo::GetMessageInfo(const KERNEL_NS::LibString &messageName) const
{
    auto iter = _messageNameRefMessageInfo.find(messageName);
    return iter == _messageNameRefMessageInfo.end() ? NULL : iter->second;
}


SERVICE_END
