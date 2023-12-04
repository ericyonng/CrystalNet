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
 * Description: thread local 内存池
*/
#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TLS_TLS_MEMORY_POLL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TLS_TLS_MEMORY_POLL_H__

#pragma once

#include <kernel/comp/Tls/ITlsObj.h>
#include <kernel/comp/Tls/Defs.h>
#include <kernel/comp/Utils/BackTraceUtil.h>
#include <string>

KERNEL_BEGIN

class KERNEL_EXPORT TlsMemoryPool : public ITlsObj
{
public:
    TlsMemoryPool();
    ~TlsMemoryPool();

public:
    virtual const char *GetObjTypeName(){ return _objTypeName.c_str(); }

    template<typename MemoryPoolType, typename InitMemoryPoolInfoType>
    MemoryPoolType *GetPoolAndCreate(const std::string &source)
    {
        auto pool = GetPool<MemoryPoolType>();
        if(UNLIKELY(!pool))
            pool = CreatePool<MemoryPoolType, InitMemoryPoolInfoType>(source);
        
        return pool;
    }

    template<typename MemoryPoolType, typename InitMemoryPoolInfoType>
    MemoryPoolType *CreatePool(const std::string &source = "")
    {
        if(UNLIKELY(_pool))
        {
            CRYSTAL_TRACE("repeat create tls pool cur pool = %p, source:%s,\n backtrace:%s", _pool, source.c_str(), BackTraceUtil::CrystalCaptureStackBackTrace().c_str());
			return KernelCastTo<MemoryPoolType>(_pool);
        }

        // 1.创建alloctor
        auto newPool = new MemoryPoolType(true, InitMemoryPoolInfoType(), source);
        Int32 st = static_cast<Int32>(newPool->Init());
        if(st != Status::Success)
        {
            CRYSTAL_TRACE("create tool fail st=[%d]", st);
            newPool->Destroy();
            CRYSTAL_DELETE_SAFE(newPool);
            return NULL;
        }
        _pool = newPool;

        // 2.绑定删除器
        auto destructorFunc = [this, newPool]() mutable ->void  {
            // #if _DEBUG
            //  CRYSTAL_TRACE("destroy a %s TlsMemoryPool=%p, pool info = %s, ", _objTypeName.c_str(), newPool, newPool->ToString().c_str());
            // #endif
            
            CRYSTAL_DELETE_SAFE(newPool);
            _pool = NULL;
        };
        _deductor = KERNEL_CREATE_CLOSURE_DELEGATE(destructorFunc, void);

		return KernelCastTo<MemoryPoolType>(_pool);
    }

    template<typename MemoryPoolType>
    MemoryPoolType *GetPool()
    {
        return reinterpret_cast<MemoryPoolType *>(_pool);
    }

    virtual void Destoy();


private:
    const std::string _objTypeName;
    void *_pool;
    IDelegate<void> *_deductor;
};

KERNEL_END

#endif


