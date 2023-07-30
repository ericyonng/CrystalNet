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
 * Date: 2023-07-16 14:56:00
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <service/TestService/Comps/DB/impl/MysqlMgrFactory.h>
#include <service/TestService/Comps/DB/impl/MysqlMgr.h>

SERVICE_BEGIN

KERNEL_NS::CompFactory *MysqlMgrFactory::FactoryCreate()
{
    return KERNEL_NS::ObjPoolWrap<MysqlMgrFactory>::NewByAdapter(_buildType.V);
}

void MysqlMgrFactory::Release()
{
    KERNEL_NS::ObjPoolWrap<MysqlMgrFactory>::DeleteByAdapter(_buildType.V, this);
}

KERNEL_NS::CompObject *MysqlMgrFactory::Create() const
{
    return MysqlMgr::NewByAdapter_MysqlMgr(_buildType.V);
}

SERVICE_END