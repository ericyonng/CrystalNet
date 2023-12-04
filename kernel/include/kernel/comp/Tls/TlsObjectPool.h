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
 * Date: 2021-01-17 22:07:38
 * Author: Eric Yonng
 * Description: thread local 对象池 请调用对象池thread local 相关接口,并保证该对象分配与释放都是同一线程
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TLS_TLS_OBJECT_POOL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TLS_TLS_OBJECT_POOL_H__

#pragma once

#include <kernel/comp/Tls/ITlsObj.h>
#include <kernel/comp/Tls/Defs.h>
#include <string>

KERNEL_BEGIN

template<typename ObjPoolType>
class TlsObjectPool : public ITlsObj
{
public:
    TlsObjectPool()
    :_objTypeName("TlsObjectPool<")
    ,_pool(NULL)
    {
        _objTypeName.append(typeid(ObjPoolType).name()).append(">");
    }
    ~TlsObjectPool()
    {
        Destoy();
    }

public:
    virtual const char *GetObjTypeName(){ return _objTypeName.c_str(); }
    template<typename MemoryAllocCfg>
    ObjPoolType *GetPool(UInt64 initBlockNumPerBuffer, const MemoryAllocCfg &alloctorCfg)
    {
        if(UNLIKELY(!_pool))
            _pool = new ObjPoolType(true, initBlockNumPerBuffer, alloctorCfg);

        return _pool;
    }

    virtual void Destoy()
    {
        if(UNLIKELY(!_pool))
        {
            CRYSTAL_TRACE("TlsObjectPool repeat destroy %p %s", this, _objTypeName.c_str());
            return;
        }
        
        // CRYSTAL_TRACE("destroy a %s %p, %s, ", _objTypeName.c_str(), _pool, _pool->ToString().c_str());
        CRYSTAL_DELETE_SAFE(_pool);
        ITlsObj::Destoy();
    }

public:
    std::string _objTypeName;
    ObjPoolType *_pool;
};

KERNEL_END

#endif

