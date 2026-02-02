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
 * Date: 2023-07-14 22:46:00
 * Author: Eric Yonng
 * Description: 数据库消息
*/
#include <pch.h>
#include <OptionComp/storage/mysql/impl/MysqlMsg.h>
#include <OptionComp/storage/mysql/impl/DbEvent.h>

KERNEL_BEGIN


DbEvent::DbEvent(Int32 type)
:PollerEvent(type)
{

}

DbEvent::~DbEvent()
{
    if(_res)
    {
        KERNEL_NS::MysqlResponse::Delete_MysqlResponse(_res);
        _res = NULL;
    }
}

void DbEvent::Release()
{
    DbEvent::Delete_DbEvent(this);
}

KERNEL_NS::LibString DbEvent::ToString() const
{
    return LibString().AppendFormat("DbEvent type:%d", _type);
}

KERNEL_END
