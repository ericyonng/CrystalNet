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
 * Date: 2022-06-04 20:15:49
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/NetEngine/Defs/IoData.h>

#include <kernel/comp/NetEngine/Defs/IoEvent.h>

#if CRYSTAL_TARGET_PLATFORM_WINDOWS

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(IoEvent);

LibString IoEvent::ToString() const
{
    LibString info;
    LibString ioDataInfo;
    if(_ioData)
        ioDataInfo.AppendFormat("io type:%d, %s, sock:%d, session id:%llu, handledBytes:%llu, _tlStream:%p", _ioData->_ioType, IoEventType::ToString(_ioData->_ioType)
                            , _ioData->_sock, _ioData->_sessionId, _ioData->_handledBytes, _ioData->_tlStream);
    else
        ioDataInfo.AppendFormat("have no iodata");

    info.AppendFormat("session id:%llu, _bytesTrans:%llu, ioData:%s, _tlStream:%p", _sessionId, _bytesTrans, ioDataInfo.c_str());

    return info;
}

KERNEL_END

#endif
