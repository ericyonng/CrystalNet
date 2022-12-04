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
 * Date: 2022-09-18 22:37:25
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <service/ProtoGenService/Comps/BaseComps/SessionMgrComp/Impl/SessionMgr.h>
#include <service/ProtoGenService/Comps/BaseComps/SessionMgrComp/Impl/SessionMgrFactory.h>

SERVICE_BEGIN

KERNEL_NS::CompFactory *SessionMgrFactory::FactoryCreate()
{
    return KERNEL_NS::ObjPoolWrap<SessionMgrFactory>::NewByAdapter(_buildType.V);
}

void SessionMgrFactory::Release()
{
    KERNEL_NS::ObjPoolWrap<SessionMgrFactory>::DeleteByAdapter(_buildType.V, this);
}

KERNEL_NS::CompObject *SessionMgrFactory::Create() const
{
    return SessionMgr::NewByAdapter_SessionMgr(_buildType.V);
}

SERVICE_END