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

#ifndef __CRYSTAL_NET_SERVICE_COMMON_PROTOCOL_CRYSTAL_PROTOCOL_ORM_IORM_DATA_H__
#define __CRYSTAL_NET_SERVICE_COMMON_PROTOCOL_CRYSTAL_PROTOCOL_ORM_IORM_DATA_H__

#pragma once

#include <service_common/common/common.h>
#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/LibStream.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Delegate/LibDelegate.h>

#include <set>

SERVICE_COMMON_BEGIN

class IOrmData
{
    POOL_CREATE_OBJ_DEFAULT(IOrmData);

public:
    IOrmData();
    IOrmData(const IOrmData &other);
    IOrmData(IOrmData &&other);
    virtual ~IOrmData();

    virtual void Release() = 0;

    IOrmData &operator =(const IOrmData &other);
    IOrmData &operator =(IOrmData &&other);

    // 附着pb数据
    void AttachPb(void *pb);
    bool IsAttachPb() const;
    void SetAttachPbFlag(bool isAttach);

    virtual bool Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) const final;
    virtual bool Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) const final;

    bool Decode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream);
    bool Decode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream);
    bool Decode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream);
    bool Decode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream);

    // pb的序列化/反序列化
    bool PbEncode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) const;
    bool PbEncode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) const;

    bool PbDecode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream);
    bool PbDecode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream);
    bool PbDecode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream);
    bool PbDecode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream);

    // pb的json接口
    virtual KERNEL_NS::LibString ToJsonString() const { return ""; }
    virtual bool ToJsonString(std::string *data) const { return false; }
    virtual bool FromJsonString(const Byte8 *data, size_t len) { return false; }

    // orm 唯一id
    virtual Int64 GetOrmId() const = 0;

    // 是否脏
    bool IsDirty() const;

    // 清洗脏
    void AfterPurge();

    // 设置标脏回调
    void SetMaskDirtyCallback(KERNEL_NS::IDelegate<void, IOrmData *> *cb);

    template<typename ObjType>
    void SetMaskDirtyCallback(ObjType *objType, void (ObjType::*cb)(IOrmData *));
    template<typename LambdaType>
    void SetMaskDirtyCallback(LambdaType&& cb);

    // 类型转换
    template<typename T>
    T *CastTo();

protected:
    // 标脏
    void _MaskDirty(bool isForce = false);

    // pb的序列化
    virtual bool _OnEncode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) const = 0;
    virtual bool _OnEncode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) const = 0;

    // pb反序列化
    virtual bool _OnDecode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) = 0;
    virtual bool _OnDecode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) = 0;

    // 附着
    virtual void _AttachPb(void *pb) = 0;

private:
    // 最后一次清洗数据时间(纳秒)
    Int64 _lastPurgeTime;

    // 最后一次标脏时间(纳秒)
    Int64 _lastMaskDirtyTime;

    // 脏回调
    KERNEL_NS::IDelegate<void, IOrmData *> *_maskDirtyCb;

    // 是否附着pb数据
    bool _isAttachPb;
};

ALWAYS_INLINE void IOrmData::AttachPb(void *pb)
{
    _AttachPb(pb);

    _isAttachPb = true;
}

ALWAYS_INLINE bool IOrmData::IsAttachPb() const
{
    return _isAttachPb;
}

ALWAYS_INLINE void IOrmData::SetAttachPbFlag(bool isAttach)
{
    _isAttachPb = isAttach;
}

ALWAYS_INLINE bool IOrmData::PbEncode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) const
{
    return _OnEncode(stream);
}

ALWAYS_INLINE bool IOrmData::PbEncode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) const
{
    return _OnEncode(stream);
}

ALWAYS_INLINE bool IOrmData::PbDecode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream)
{
    auto attachStream = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
    attachStream->Attach(stream);

    auto ret = _OnDecode(*attachStream);
    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(attachStream);
    return ret;
}

ALWAYS_INLINE bool IOrmData::PbDecode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream)
{
    auto attachStream = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
    attachStream->Attach(stream);

    auto ret = _OnDecode(*attachStream);
    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(attachStream);
    return ret;
}

ALWAYS_INLINE bool IOrmData::PbDecode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream)
{
    return _OnDecode(stream);
}

ALWAYS_INLINE bool IOrmData::PbDecode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream)
{
    return _OnDecode(stream);
}

ALWAYS_INLINE bool IOrmData::IsDirty() const 
{ 
    return _lastMaskDirtyTime > _lastPurgeTime; 
}

ALWAYS_INLINE void IOrmData::SetMaskDirtyCallback(KERNEL_NS::IDelegate<void, IOrmData *> *cb)
{
    CRYSTAL_RELEASE_SAFE(_maskDirtyCb);
    _maskDirtyCb = cb;
}

template<typename ObjType>
ALWAYS_INLINE void IOrmData::SetMaskDirtyCallback(ObjType *objType, void (ObjType::*cb)(IOrmData *))
{
    auto delg = KERNEL_NS::DelegateFactory::Create(objType, cb);
    SetMaskDirtyCallback(delg);
}

template<typename LambdaType>
ALWAYS_INLINE void IOrmData::SetMaskDirtyCallback(LambdaType&& cb)
{
    auto delg = KERNEL_CREATE_CLOSURE_DELEGATE(cb, void, IOrmData *);
    SetMaskDirtyCallback(delg);
}

template<typename T>
ALWAYS_INLINE T *IOrmData::CastTo()
{
    return reinterpret_cast<T *>(this);
}
    
class IOrmDataFactory
{
    POOL_CREATE_OBJ_DEFAULT(IOrmDataFactory);

public:
    IOrmDataFactory() {}
    virtual ~IOrmDataFactory() {}
    
    virtual void Release() = 0;

    // 创建orm data
    virtual IOrmData *Create() const = 0;

    // 获取orm id
    virtual Int64 GetOrmId() const = 0;
};

SERVICE_COMMON_END

#endif
