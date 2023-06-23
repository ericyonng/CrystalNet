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
 * Description: Mysql一行的数据
*/

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MYSQL_IMPL_RECORD_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MYSQL_IMPL_RECORD_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/memory.h>
#include <kernel/comp/LibString.h>
#include <OptionComp/storage/mysql/impl/Field.h>

KERNEL_BEGIN

class Field;

class Record
{
    POOL_CREATE_OBJ_DEFAULT(Record);

public:
    Record();
    ~Record();

    void Clear();

    // 新增字段
    template<typename T = _Build::TL>
    Field *AddField(Int32 idx, const LibString &tableName, const LibString &name, const void *data, Int64 dataSize);
    template<typename T = _Build::TL>
    Field *AddField(const LibString &tableName, const LibString &name, const void *data, Int64 dataSize);

    // 必须是NewThreadLocal创建出来的
    Field *AddField(Field *field);

    bool HasField(const LibString &name) const;
    bool HasField(Int32 idx) const;

    void RemoveField(const LibString &name);
    void RemoveField(Int32 idx);
    Field *Pop(const LibString &name);
    Field *Pop(Int32 idx);

    // 字段数量
    void SetFieldAmount(Int32 amount);
    Int32 GetFieldAmount() const;

    LibString ToString() const;

    // 索引field
    Field *operator[](Int32 idx);
    const Field *operator[](Int32 idx) const;
    Field *operator[](const LibString &fieldName);
    const Field *operator[](const LibString &fieldName) const;

    // 索引field
    Field *GetField(Int32 idx);
    const Field *GetField(Int32 idx) const;
    Field *GetField(const LibString &fieldName);
    const Field *GetField(const LibString &fieldName) const;

    // 支持for(auto : record)遍历
    std::vector<Field *>::iterator begin();
    std::vector<Field *>::const_iterator begin() const;
    std::vector<Field *>::iterator end();
    std::vector<Field *>::const_iterator end() const;

private:
    std::vector<Field *> _fields;
    std::unordered_map<LibString, Field *> _fieldNameRefField;
};

template<typename T>
ALWAYS_INLINE Field *Record::AddField(Int32 idx, const LibString &tableName, const LibString &name, const void *data, Int64 dataSize)
{
    auto field = Field::Create<T>(tableName, name, this);
    field->SetIndexInRecord(idx);
    field->Write(data, dataSize);

    if(UNLIKELY(!AddField(field)))
    {
        field->Release();
        return NULL;
    }

    return field;
}

template<typename T>
ALWAYS_INLINE Field *Record::AddField(const LibString &tableName, const LibString &name, const void *data, Int64 dataSize)
{
    auto field = Field::Create<T>(tableName, name, this);
    field->Write(data, dataSize);
    if(UNLIKELY(!AddField(field)))
    {
        field->Release();
        return NULL;
    }

    return field;
}

ALWAYS_INLINE bool Record::HasField(const LibString &name) const
{
    return _fieldNameRefField.find(name) != _fieldNameRefField.end();
}

ALWAYS_INLINE bool Record::HasField(Int32 idx) const
{
    if(UNLIKELY(_fields.size() <= idx))
        return false;

    return _fields[idx] != NULL;
}

ALWAYS_INLINE void Record::RemoveField(const LibString &name)
{
    auto iter = _fieldNameRefField.find(name);
    if(UNLIKELY(iter == _fieldNameRefField.end()))
        return;

    auto field = iter->second;
    _fieldNameRefField.erase(iter);
    _fields.erase(_fields.begin() + field->GetIndexInRecord());

    field->Release();
}

ALWAYS_INLINE void Record::RemoveField(Int32 idx)
{
    if(UNLIKELY(_fields.size() <= idx))
        return;

    auto field = _fields[idx];
    if(UNLIKELY(!field))
        return;

    _fieldNameRefField.erase(field->GetName());
    _fields.erase(_fields.begin() + idx);

    field->Release();
}

ALWAYS_INLINE Field *Record::Pop(const LibString &name)
{
    auto iter = _fieldNameRefField.find(name);
    if(UNLIKELY(iter == _fieldNameRefField.end()))
        return NULL;

    auto field = iter->second;
    _fieldNameRefField.erase(iter);
    _fields.erase(_fields.begin() + field->GetIndexInRecord());

    return field;
}

ALWAYS_INLINE Field *Record::Pop(Int32 idx)
{
    if(UNLIKELY(_fields.size() <= idx))
        return NULL;

    auto field = _fields[idx];
    if(UNLIKELY(!field))
        return NULL;

    _fieldNameRefField.erase(field->GetName());
    _fields.erase(_fields.begin() + idx);

    return field;
}

ALWAYS_INLINE Int32 Record::GetFieldAmount() const
{
    return static_cast<Int32>(_fieldNameRefField.size());
}

ALWAYS_INLINE Field *Record::operator[](Int32 idx)
{
    if(UNLIKELY(_fields.size() <= idx))
        return NULL;

    return _fields[idx];
}

ALWAYS_INLINE const Field *Record::operator[](Int32 idx) const
{
    if(UNLIKELY(_fields.size() <= idx))
        return NULL;

    return _fields[idx];
}

ALWAYS_INLINE Field *Record::operator[](const LibString &fieldName)
{
    auto iter = _fieldNameRefField.find(fieldName);
    return iter == _fieldNameRefField.end() ? NULL : iter->second;
}

ALWAYS_INLINE const Field *Record::operator[](const LibString &fieldName) const
{
    auto iter = _fieldNameRefField.find(fieldName);
    return iter == _fieldNameRefField.end() ? NULL : iter->second;
}

ALWAYS_INLINE Field *Record::GetField(Int32 idx)
{
    return (*this)[idx];
}

ALWAYS_INLINE const Field *Record::GetField(Int32 idx) const
{
    return (*this)[idx];
}

ALWAYS_INLINE Field *Record::GetField(const LibString &fieldName)
{
    return (*this)[fieldName];
}

ALWAYS_INLINE const Field *Record::GetField(const LibString &fieldName) const
{
    return (*this)[fieldName];
}

ALWAYS_INLINE std::vector<Field *>::iterator Record::begin()
{
    return _fields.begin();
}

ALWAYS_INLINE std::vector<Field *>::const_iterator Record::begin() const
{
    return _fields.begin();
}

ALWAYS_INLINE std::vector<Field *>::iterator Record::end()
{
    return _fields.end();
}

ALWAYS_INLINE std::vector<Field *>::const_iterator Record::end() const
{
    return _fields.end();
}

KERNEL_END

#endif

