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
* Description: Generated By protogentool, Dont Modify This File!!!
*/


#include "pch.h"
#include <protocols/cplusplus/com_system_table.pb.h>
#include <protocols/orm_out/SimpleInfoOrmData.h>

SERVICE_COMMON_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(SimpleInfoOrmData);

SimpleInfoOrmData::SimpleInfoOrmData()
:_ormRawPbData(new ::CRYSTAL_NET::service::SimpleInfo)
{
}

SimpleInfoOrmData::SimpleInfoOrmData(::CRYSTAL_NET::service::SimpleInfo *pb)
:_ormRawPbData(NULL)
{
    AttachPb(pb);
}

SimpleInfoOrmData::SimpleInfoOrmData(const SimpleInfoOrmData &other)
:IOrmData(reinterpret_cast<const IOrmData &>(other))
,_ormRawPbData(other._ormRawPbData ? new ::CRYSTAL_NET::service::SimpleInfo(*other._ormRawPbData) : NULL)
{
    SetAttachPbFlag(false);
}

SimpleInfoOrmData::SimpleInfoOrmData(SimpleInfoOrmData &&other)
:IOrmData(std::forward<IOrmData>(other))
,_ormRawPbData(other._ormRawPbData)
{
    other._ormRawPbData = NULL;
}

SimpleInfoOrmData::SimpleInfoOrmData(const ::CRYSTAL_NET::service::SimpleInfo &pb)
:_ormRawPbData(new ::CRYSTAL_NET::service::SimpleInfo(pb))
{

}

SimpleInfoOrmData::~SimpleInfoOrmData()
{
    if(LIKELY(!IsAttachPb()))
        CRYSTAL_RELEASE_SAFE(_ormRawPbData);
}

void SimpleInfoOrmData::Release()
{
    SimpleInfoOrmData::DeleteThreadLocal_SimpleInfoOrmData(this);
}

SimpleInfoOrmData &SimpleInfoOrmData::operator =(const ::CRYSTAL_NET::service::SimpleInfo &pb)
{
    if(LIKELY(!IsAttachPb()))
        CRYSTAL_RELEASE_SAFE(_ormRawPbData);

    SetAttachPbFlag(false);
    _ormRawPbData = new ::CRYSTAL_NET::service::SimpleInfo(pb);
    _MaskDirty(true);
    return *this;
}

SimpleInfoOrmData &SimpleInfoOrmData::operator =(const SimpleInfoOrmData &other)
{
    if(this == &other)
        return *this;

    IOrmData::operator =(reinterpret_cast<const IOrmData &>(other));
    if(LIKELY(!IsAttachPb()))
        CRYSTAL_RELEASE_SAFE(_ormRawPbData);

    _ormRawPbData = NULL;
    SetAttachPbFlag(false);
    if(other._ormRawPbData)
        _ormRawPbData = new ::CRYSTAL_NET::service::SimpleInfo(*other._ormRawPbData);
    _MaskDirty(true);

    return *this;
}

SimpleInfoOrmData &SimpleInfoOrmData::operator =(SimpleInfoOrmData &&other)
{
    if(this == &other)
        return *this;

    IOrmData::operator =(std::forward<IOrmData>(other));
    _ormRawPbData = other._ormRawPbData;
    other._ormRawPbData = NULL;


    return *this;
}

void SimpleInfoOrmData::Clear()
{

    if(_ormRawPbData)
        _ormRawPbData->Clear();

    _MaskDirty(true);
}

void SimpleInfoOrmData::_AttachPb(void *pb)
{
    if(LIKELY(!IsAttachPb()))
        CRYSTAL_RELEASE_SAFE(_ormRawPbData);

    _ormRawPbData = reinterpret_cast<::CRYSTAL_NET::service::SimpleInfo *>(pb);


}

KERNEL_NS::LibString SimpleInfoOrmData::ToJsonString() const
{
    return _ormRawPbData->ToJsonString();
}

bool SimpleInfoOrmData::ToJsonString(std::string *data) const
{
    return _ormRawPbData->ToJsonString(data);
}

bool SimpleInfoOrmData::FromJsonString(const Byte8 *data, size_t len)
{
    return _ormRawPbData->FromJsonString(data, len);
}

const ::CRYSTAL_NET::service::SimpleInfo *SimpleInfoOrmData::GetPbRawData() const
{
    return _ormRawPbData;
}

void SimpleInfoOrmData::clear_maxincid()
{
    _ormRawPbData->clear_maxincid();
    _MaskDirty(true);
}

int64_t SimpleInfoOrmData::maxincid() const
{
    return _ormRawPbData->maxincid();
}

void SimpleInfoOrmData::set_maxincid(int64_t value)
{
    _ormRawPbData->set_maxincid(value);
    _MaskDirty(true);
}

void SimpleInfoOrmData::clear_dirtycount()
{
    _ormRawPbData->clear_dirtycount();
    _MaskDirty(true);
}

int64_t SimpleInfoOrmData::dirtycount() const
{
    return _ormRawPbData->dirtycount();
}

void SimpleInfoOrmData::set_dirtycount(int64_t value)
{
    _ormRawPbData->set_dirtycount(value);
    _MaskDirty(true);
}

void SimpleInfoOrmData::clear_versionno()
{
    _ormRawPbData->clear_versionno();
    _MaskDirty(true);
}

int64_t SimpleInfoOrmData::versionno() const
{
    return _ormRawPbData->versionno();
}

void SimpleInfoOrmData::set_versionno(int64_t value)
{
    _ormRawPbData->set_versionno(value);
    _MaskDirty(true);
}

bool SimpleInfoOrmData::_OnEncode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) const
{
    return _ormRawPbData->Encode(stream);
}

bool SimpleInfoOrmData::_OnEncode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) const
{
    return _ormRawPbData->Encode(stream);
}

bool SimpleInfoOrmData::_OnDecode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream)
{
    return _ormRawPbData->Decode(stream);
}

bool SimpleInfoOrmData::_OnDecode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream)
{
    return _ormRawPbData->Decode(stream);
}

POOL_CREATE_OBJ_DEFAULT_IMPL(SimpleInfoOrmDataFactory);

IOrmData *SimpleInfoOrmDataFactory::Create() const
{
    return SimpleInfoOrmData::NewThreadLocal_SimpleInfoOrmData();
}


SERVICE_COMMON_END
