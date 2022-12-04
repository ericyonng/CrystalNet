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
 * Author: Eric Yonng
 * Date: 2021-03-22 17:33:46
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/NetEngine/LibPacket.h>
#include <kernel/comp/NetEngine/Protocol/Protocol.h>
#include <kernel/comp/Log/log.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(LibPacket);

LibPacket::LibPacket()
:_packetId(0)
,_sessionId(0)
,_opcode(0)
,_localAddr(true)
,_remoteAddr(true)
,_coder(NULL)
{

}

LibPacket::~LibPacket()
{
    if(_coder)
        _coder->Release();
    _coder = NULL;
}

void LibPacket::ReleaseUsingPool()
{
    LibPacket::Delete_LibPacket(this);
}

bool LibPacket::Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream)
{
    if(UNLIKELY(!_coder))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("have no coder packet:%s"), ToString().c_str());
        return false;
    }

    return _coder->Encode(stream);
}

bool LibPacket::Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream)
{
    if(UNLIKELY(!_coder))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("have no coder packet:%s"), ToString().c_str());
        return false;
    }

    return _coder->Encode(stream);
}

bool LibPacket::Decode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream)
{
    if(UNLIKELY(!_coder))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("have no coder packet:%s"), ToString().c_str());
        return false;
    }

    return _coder->Decode(stream);
}

bool LibPacket::Decode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream)
{
    if(UNLIKELY(!_coder))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("have no coder packet:%s"), ToString().c_str());
        return false;
    }

    return _coder->Decode(stream);
}

void LibPacket::SetCoder(ICoder *coder)
{
    if(_coder == coder)
        return;

    if(_coder)
        _coder->Release();

    _coder = coder;
}

KERNEL_END
