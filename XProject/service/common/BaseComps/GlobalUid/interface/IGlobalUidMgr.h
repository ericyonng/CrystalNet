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
 * Date: 2023-08-05 19:37:02
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <service/common/macro.h>
#include <service/common/status.h>
#include <service/common/BaseComps/GlobalSys/GlobalSys.h>

SERVICE_BEGIN

class IGlobalUidMgr : public IGlobalSys
{
    POOL_CREATE_OBJ_DEFAULT_P1(IGlobalSys, IGlobalUidMgr);

public:
    IGlobalUidMgr(UInt64 objTypeId) : IGlobalSys(objTypeId){}
    
    // 全球唯一id
    virtual UInt64 NewGuid() = 0;

    // 提前取的id数量
    virtual void SetGetIdAheadCount(Int32 aheadCount) = 0;

    // 强制持久化, 同步调用
    virtual void SetUpdateLastIdCallback(KERNEL_NS::IDelegate<Int32, ILogicSys *> *deleg) = 0; 

    // 回调
    template<typename ObjType>
    void SetUpdateLastIdCallback(ObjType *obj, Int32 (ObjType::*handler)(ILogicSys *));
};

template<typename ObjType>
ALWAYS_INLINE void IGlobalUidMgr::SetUpdateLastIdCallback(ObjType *obj, Int32 (ObjType::*handler)(ILogicSys *))
{
    auto delg = KERNEL_NS::DelegateFactory::Create(obj, handler);
    SetUpdateLastIdCallback(delg);
}

SERVICE_END