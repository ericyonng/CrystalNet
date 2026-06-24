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
 * Date: 2026-06-16 14:49:39
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <service/common/BaseComps/GlobalSys/GlobalSys.h>
#include <kernel/comp/Coroutines/CoTask.h>

SERVICE_BEGIN

// mongodb代理
class IMongodbProxy : public IGlobalSys
{
    POOL_CREATE_OBJ_DEFAULT_P1(IGlobalSys, IMongodbProxy);

public:
    IMongodbProxy(UInt64 objTypeId) : IGlobalSys(objTypeId){}

    // 外部依赖注册(等待外部依赖退出后mongodb才退出)
    virtual void RegisterDependence(ILogicSys *obj) = 0;
    virtual void UnRegisterDependence(const ILogicSys *obj) = 0;

    // TODO:提供load数据接口直接外部调用Load接口(协程)

    // 标脏
    virtual void MaskLogicNumberKeyAddDirty(const ILogicSys *logic, UInt64 key) = 0;
    virtual void MaskLogicNumberKeyModifyDirty(const ILogicSys *logic, UInt64 key) = 0;
    virtual void MaskLogicNumberKeyDeleteDirty(const ILogicSys *logic, UInt64 key) = 0;
    
    virtual void MaskLogicStringKeyAddDirty(const ILogicSys *logic, const KERNEL_NS::LibString &key) = 0;
    virtual void MaskLogicStringKeyModifyDirty(const ILogicSys *logic, const KERNEL_NS::LibString &key) = 0;
    virtual void MaskLogicStringKeyDeleteDirty(const ILogicSys *logic, const KERNEL_NS::LibString &key) = 0;

    // 等待落库完成
    virtual KERNEL_NS::CoTask<> Purge() = 0;
    // 等待logic落库完成
    virtual KERNEL_NS::CoTask<> Purge(const ILogicSys *logic) = 0;
};

SERVICE_END