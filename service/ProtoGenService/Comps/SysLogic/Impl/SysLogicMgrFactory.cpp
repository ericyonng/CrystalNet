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
 * Date: 2022-09-22 15:03:11
 * Author: Eric Yonng
 * Description: 系统的业务,如连接，监听等
*/

#include <pch.h>
#include <service/ProtoGenService/Comps/SysLogic/Impl/SysLogicMgr.h>
#include <service/ProtoGenService/Comps/SysLogic/Impl/SysLogicMgrFactory.h>

SERVICE_BEGIN

KERNEL_NS::CompFactory *SysLogicMgrFactory::FactoryCreate()
{
    return KERNEL_NS::ObjPoolWrap<SysLogicMgrFactory>::NewByAdapter(_buildType.V);
}

void SysLogicMgrFactory::Release()
{
    KERNEL_NS::ObjPoolWrap<SysLogicMgrFactory>::DeleteByAdapter(_buildType.V, this);
}

KERNEL_NS::CompObject *SysLogicMgrFactory::Create() const
{
    return SysLogicMgr::NewByAdapter_SysLogicMgr(_buildType.V);
}

SERVICE_END
