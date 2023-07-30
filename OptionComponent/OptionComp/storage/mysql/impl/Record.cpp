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

#include <pch.h>
#include <OptionComp/storage/mysql/impl/Field.h>
#include <OptionComp/storage/mysql/impl/Record.h>
#include <kernel/comp/Utils/ContainerUtil.h>
#include <kernel/comp/Utils/StringUtil.h>
#include <mysql.h>

KERNEL_BEGIN
POOL_CREATE_OBJ_DEFAULT_IMPL(Record);

Record::Record()
:_primaryKey(NULL)
{

}

Record::~Record()
{
    Clear();
}

void Record::Clear()
{
    ContainerUtil::DelContainer2(_fields);
    _fieldNameRefField.clear();
}

Field *Record::AddField(Field *field)
{
    if(UNLIKELY(HasField(field->GetName())))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("field already exists name:%s, idx:%d, data size:%lld")
                , field->GetName().c_str(), field->GetIndexInRecord(), field->GetDataSize());
        return NULL;
    }

    if(field->HasIndex())
    {
        const auto idx = field->GetIndexInRecord();
        if(UNLIKELY(idx >= static_cast<Int32>(_fields.size())))
            _fields.resize(idx + 1);

        _fields[idx] = field;
    }
    else
    {
        field->SetIndexInRecord(static_cast<Int32>(_fields.size()));
        _fields.push_back(field);
    }

    auto flags = field->GetFlags();
    field->SetAutoIncField(flags & AUTO_INCREMENT_FLAG);
    field->SetIsUnsigned(flags & UNSIGNED_FLAG);
    field->SetIsPrimaryKey(flags & PRI_KEY_FLAG);

    if(field->IsPrimaryKey())
    {
        if(_primaryKey)
            g_Log->Warn(LOGFMT_OBJ_TAG("multi primary key old field is:%s"), _primaryKey->ToString().c_str());

        _primaryKey = field;
    }

    _fieldNameRefField[field->GetName()] = field;
    _fieldNameRefData[field->GetName()] = field->GetData();

    return field;
}

void Record::SetFieldAmount(Int32 amount)
{
    const Int32 currentCount = static_cast<Int32>(_fields.size());
    if(UNLIKELY(currentCount == amount))
        return;

    if(currentCount < amount)
    {
        _fields.resize(amount);
        return;
    }

    // 缩小需要释放后几个field
    for(Int32 idx = currentCount - 1;  idx > (amount - 1); --idx)
        _fields[idx]->Release();

    _fields.resize(amount);
}

LibString Record::ToString() const
{
    return StringUtil::ToString(_fields, "\n");
}


KERNEL_END