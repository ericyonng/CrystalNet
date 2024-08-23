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
 * Date: 2022-09-19 02:24:17
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <service/GateService/ServiceCompHeader.h>

SERVICE_BEGIN

class IStubHandleMgr : public IGlobalSys
{
    POOL_CREATE_OBJ_DEFAULT_P1(IGlobalSys, IStubHandleMgr);

public:
    IStubHandleMgr(UInt64 objTypeId) : IGlobalSys(objTypeId) {}

    // 新存根
    virtual UInt64 NewStub() = 0;
    // 存根是否存在
    virtual bool HasStub(UInt64 stub) const = 0;
    // 新存根回调
    virtual Int32 NewHandle(UInt64 stub, KERNEL_NS::IDelegate<void, UInt64, Int32, const KERNEL_NS::Variant *> *delg) = 0;
    // 新存根回调
    virtual Int32 NewHandle(KERNEL_NS::IDelegate<void, UInt64, Int32, const KERNEL_NS::Variant *> *delg, UInt64 &stub) = 0;
    // 调用回调
    virtual void InvokeHandle(UInt64 stub, Int32 errCode, const KERNEL_NS::Variant *params) = 0;
};

SERVICE_END
