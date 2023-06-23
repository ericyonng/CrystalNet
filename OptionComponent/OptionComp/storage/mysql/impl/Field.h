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
 * Date: 2023-06-22 21:19:00
 * Author: Eric Yonng
 * Description: Mysql一行中的一个字段
*/

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MYSQL_IMPL_FIELD_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MYSQL_IMPL_FIELD_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Delegate/Delegate.h>
#include <kernel/comp/memory/memory.h>

KERNEL_BEGIN

template<typename T>
class LibStream;

class Record;

class Field
{
    POOL_CREATE_OBJ_DEFAULT(Field);

public:
    Field(Record *owner = NULL);
    ~Field();

    template<typename T = _Build::TL>
    static Field *Create(Record *owner = NULL);
    void Release();

    void Write(const void *data, Int64 dataSize);

    // owner
    void SetOwner(Record *owner);
    const Record *GetOwner() const;
    Record *GetOwner();

    // 数据大小
    Int64 GetDataSize() const;
    // 字段名
    const LibString &GetName() const;

    // 字段所在行的索引
    Int32 GetIndexInRecord() const;
    void SetIndexInRecord(Int32 idx);

    LibString ToString() const;

private:
    // 设置释放的回调
    template<typename CallbackType>
    void _SetRelease(CallbackType &&cb);
    void _SetRelease(IDelegate<void, Field *> *cb);

private:
    Record *_owner;
    Int32 _index;
    LibString _name;
    LibStream<_Build::TL> *_data;
    IDelegate<void, Field *> *_release; 
};

template<typename T>
ALWAYS_INLINE Field *Field::Create(Record *owner)
{
    // 设置释放
    _SetRelease([](Field *ptr){
        Field::DeleteByAdapter_Field(T::V, ptr);
    });

    return Field::NewByAdapter_Field(T::V, owner);
}

ALWAYS_INLINE void Field::Release()
{
    if(UNLIKELY(_release))
    {
        _release->Invoke(this);
        return;
    }

    // 默认方式
    Field::DeleteThreadLocal_Field(this);
}

ALWAYS_INLINE void Field::SetOwner(Record *owner)
{
    _owner = owner;
}

ALWAYS_INLINE const Record *Field::GetOwner() const
{
    return _owner;
}

ALWAYS_INLINE Record *Field::GetOwner()
{
    return _owner;
}

ALWAYS_INLINE const LibString &Field::GetName() const
{
    return _name;
}

ALWAYS_INLINE Int32 Field::GetIndexInRecord() const
{
    return _index;
}

ALWAYS_INLINE void Field::SetIndexInRecord(Int32 idx)
{
    _index = idx;
}

ALWAYS_INLINE LibString Field::ToString() const
{
    LibString info;
    info.AppendFormat("field name:%s, index in record:%d, data size:%lld", _name.c_str(), _index, GetDataSize());
    return info;
}

template<typename CallbackType>
ALWAYS_INLINE void Field::_SetRelease(CallbackType &&cb)
{
    auto delg = KERNEL_CREATE_CLOSURE_DELEGATE(cb, void, Field *);
    _SetRelease(delg);
}

ALWAYS_INLINE void Field::_SetRelease(IDelegate<void, Field *> *cb)
{
    if(UNLIKELY(_release))
        _release->Release();

    _release = cb;
}


KERNEL_END

#endif
