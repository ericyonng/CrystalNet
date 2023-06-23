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

#include <pch.h>
#include <OptionComp/storage/mysql/impl/Record.h>
#include <OptionComp/storage/mysql/impl/Field.h>
#include <kernel/comp/LibStream.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(Field);

Field::Field(const LibString &name, Record *owner)
:_owner(owner)
,_index(-1)
,_name(name)
,_data(NULL)
,_release(NULL)
{

}

Field::~Field()
{
    if(_data)
    {
        LibStream<_Build::TL>::DeleteThreadLocal_LibStream(_data);
        _data = NULL;
    }

    if(_release)
    {
        _release->Release();
        _release = NULL;
    }
}

void Field::Write(const void *data, Int64 dataSize)
{
    if(UNLIKELY(!_data))
    {
        // 默认4个字节空间
        _data = LibStream<_Build::TL>::NewThreadLocal_LibStream();
        _data->Init(static_cast<Int64>(sizeof(Int32)));
    }

    _data->Write(data, dataSize);
}

Int64 Field::GetDataSize() const
{
    return _data->GetWriteBytes();
}

KERNEL_END