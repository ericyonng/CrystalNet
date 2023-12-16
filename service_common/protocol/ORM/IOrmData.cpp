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
 * Date: 2023-12-07 21:07:00
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include "service_common/protocol/ORM/IOrmData.h"

#include <kernel/comp/LibTime.h>

SERVICE_COMMON_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(IOrmData);

IOrmData::IOrmData()
:_lastPurgeTime(0)
,_lastMaskDirtyTime(0)
,_maskDirtyCb(NULL)
,_isAttachPb(false)
{

}

IOrmData::~IOrmData()
{
    CRYSTAL_RELEASE_SAFE(_maskDirtyCb);
}

bool IOrmData::Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) const
{
    if(UNLIKELY(!stream.GetBuffer()))
        stream.Init(16);

    stream.Write(_lastPurgeTime);
    stream.Write(_lastMaskDirtyTime);

    return _OnEncode(stream);
}

bool IOrmData::Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) const
{
    if(UNLIKELY(!stream.GetBuffer()))
        stream.Init(16);

    stream.Write(_lastPurgeTime);
    stream.Write(_lastMaskDirtyTime);

    return _OnEncode(stream);
}

bool IOrmData::Decode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream)
{
    _lastPurgeTime = stream.ReadInt64();
    _lastMaskDirtyTime = stream.ReadInt64();

    return _OnDecode(stream);
}

bool IOrmData::Decode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream)
{
    _lastPurgeTime = stream.ReadInt64();
    _lastMaskDirtyTime = stream.ReadInt64();

    return _OnDecode(stream);
}

void IOrmData::AfterPurge()
{
    _lastPurgeTime = KERNEL_NS::LibTime::NowNanoTimestamp();
}

void IOrmData::_MaskDirty(bool isForce)
{
    const auto isDirty = IsDirty();

    _lastMaskDirtyTime = KERNEL_NS::LibTime::NowNanoTimestamp();

    if(!isDirty && _maskDirtyCb || isForce)
        _maskDirtyCb->Invoke(this);
}

POOL_CREATE_OBJ_DEFAULT_IMPL(IOrmDataFactory);

SERVICE_COMMON_END

