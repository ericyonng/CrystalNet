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
 * Date: 2023-11-19 00:03:49
 * Author: Eric Yonng
 * Description: 
*/



#pragma once

#include <Comps/SystemLog/Interface/ISystemLogGlobal.h>
#include <kernel/kernel.h>

#include <map>

SERVICE_BEGIN

class SystemLogData;

class SystemLogGlobal : public ISystemLogGlobal
{
    POOL_CREATE_OBJ_DEFAULT_P1(ISystemLogGlobal, SystemLogGlobal);

public:
    SystemLogGlobal();
    ~SystemLogGlobal();
    void Release() override;
    void OnRegisterComps() override;

    virtual Int32 _OnGlobalSysInit() override;

    virtual Int32 OnSave(UInt64 key, std::map<KERNEL_NS::LibString, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> *> &fieldRefdb) const override;
    
    virtual void AddLog(UInt64 libraryId, const KERNEL_NS::LibString &titleWordId, const std::vector<VariantParam> &titleParams, const KERNEL_NS::LibString &contentWordId, const std::vector<VariantParam> &contentParams) override;

    void _OnSystemLogDataListReq(KERNEL_NS::LibPacket *&packet);

    OBJ_GET_OBJ_TYPEID_DECLARE();

private:
    mutable std::map<UInt64, SystemLogData *> _idRefSystemLogData;
};

SERVICE_END
