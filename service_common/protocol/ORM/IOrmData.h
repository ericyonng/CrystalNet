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
    virtual ~IOrmData();

    virtual void Release() = 0;

    virtual bool Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) const final;
    virtual bool Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) const final;

    virtual bool Decode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) final;
    virtual bool Decode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) final;

    // orm 唯一id
    virtual Int64 GetOrmId() const = 0;

    // 是否脏
    bool IsDirty() const;

    // 清洗脏
    void AfterPurge();

    // 设置标脏回调
    void SetMaskDirtyCallback(KERNEL_NS::IDelegate<void, IOrmData *> *cb);

    // 获取脏标签id
    const std::set<Int32> GetDirtyTagIds() const;

protected:
    // 标脏
    void _MaskDirty(Int32 tagId);

    // 编码成字节流
    virtual bool _OnEncode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) const = 0;
    virtual bool _OnEncode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) const = 0;

    // 反编码
    virtual bool _OnDecode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) = 0;
    virtual bool _OnDecode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) = 0;

private:
    // 最后一次清洗数据时间(纳秒)
    Int64 _lastPurgeTime;

    // 最后一次标脏时间(纳秒)
    Int64 _lastMaskDirtyTime;

    // 脏的tag
    std::set<Int32> _dirtyTagIds;

    // 脏回调
    KERNEL_NS::IDelegate<void, IOrmData *> *_maskDirtyCb;
};

ALWAYS_INLINE bool IOrmData::IsDirty() const 
{ 
    return _lastMaskDirtyTime > _lastPurgeTime; 
}

ALWAYS_INLINE void IOrmData::SetMaskDirtyCallback(KERNEL_NS::IDelegate<void, IOrmData *> *cb)
{
    CRYSTAL_RELEASE_SAFE(_maskDirtyCb);
    _maskDirtyCb = cb;
}

ALWAYS_INLINE const std::set<Int32> IOrmData::GetDirtyTagIds() const
{
    return _dirtyTagIds;
}

class IOrmDataFactory
{
    POOL_CREATE_OBJ_DEFAULT(IOrmDataFactory);

public:
    IOrmDataFactory() {}
    virtual ~IOrmDataFactory() {}
    virtual void Release() = 0;

    virtual IOrmData *Create() const = 0;
};

SERVICE_COMMON_END

#endif