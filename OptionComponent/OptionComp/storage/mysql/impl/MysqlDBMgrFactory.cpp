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
 * Date: 2023-07-15 20:12:00
 * Author: Eric Yonng
 * Description:
*/

#include <pch.h>
#include <OptionComp/storage/mysql/impl/MysqlDBMgrFactory.h>
#include <OptionComp/storage/mysql/impl/MysqlDBMgr.h>

KERNEL_BEGIN

CompFactory *MysqlDBMgrFactory::FactoryCreate()
{
    return KERNEL_NS::ObjPoolWrap<MysqlDBMgrFactory>::NewByAdapter(_buildType.V);
}

void MysqlDBMgrFactory::Release()
{
    ObjPoolWrap<MysqlDBMgrFactory>::DeleteByAdapter(_buildType.V, this);
}

CompObject *MysqlDBMgrFactory::Create() const
{
    return MysqlDBMgr::NewByAdapter_MysqlDBMgr(_buildType.V);
}

KERNEL_END
