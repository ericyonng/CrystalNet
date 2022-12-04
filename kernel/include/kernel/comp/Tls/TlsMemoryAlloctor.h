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
 * Description: 提供运行时类型识别缓冲,线程id
*/
#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TLS_TLS_MEMORY_ALLOCTOR_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TLS_TLS_MEMORY_ALLOCTOR_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/Tls/ITlsObj.h>
#include <kernel/comp/Tls/Defs.h>
#include <kernel/comp/Lock/Lock.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

class KERNEL_EXPORT TlsMemoryAlloctor : public ITlsObj
{
public:
    TlsMemoryAlloctor();
    ~TlsMemoryAlloctor();

public:
    virtual const char *GetObjTypeName(){ return _objTypeName.c_str(); }

    template<typename MemAllocType, typename MemAllocCfgType>
    MemAllocType *GetMemoryAlloctorAndCreate(UInt64 allocUnitBytes, UInt64 initBlockNumPerBuffer, const std::string &source)
    {
        // _sizeRefMemoryAlloc 没有多线程竞争
        auto iter = _sizeRefMemoryAlloc.find(allocUnitBytes);
        if(iter == _sizeRefMemoryAlloc.end())
            return _NewAlloctor<MemAllocType, MemAllocCfgType>(allocUnitBytes, initBlockNumPerBuffer, source);

        return reinterpret_cast<MemAllocType *>(iter->second);
    }

    template<typename MemAllocType, typename MemAllocCfgType>
    MemAllocType *CreateMemoryAlloctor(UInt64 allocUnitBytes, UInt64 initBlockNumPerBuffer, const std::string &source)
    {
        // _sizeRefMemoryAlloc 没有多线程竞争
        return _NewAlloctor<MemAllocType, MemAllocCfgType>(allocUnitBytes, initBlockNumPerBuffer, source);
    }

    template<typename MemAllocType, typename MemAllocCfgType>
    MemAllocType *GetMemoryAlloctor(UInt64 allocUnitBytes)
    {
        // _sizeRefMemoryAlloc 没有多线程竞争
        auto iter = _sizeRefMemoryAlloc.find(allocUnitBytes);
        return iter == _sizeRefMemoryAlloc.end() ? NULL : iter->second;
    }

    virtual void Destoy();
    UInt64 MemMonitor(LibString &info);

private:
    template<typename MemAllocType, typename MemAllocCfgType>
    MemAllocType *_NewAlloctor(UInt64 allocUnitBytes, UInt64 initBlockNumPerBuffer, const std::string &source)
    {
        // 1.创建alloctor
        auto newAlloctor = new MemAllocType(MemAllocCfgType(allocUnitBytes));
        newAlloctor->Init(initBlockNumPerBuffer, source);
        // 2.绑定删除器
        auto destructorFunc = [this, newAlloctor]() mutable ->void{
            #if _DEBUG
             CRYSTAL_TRACE("destroy a %s newAlloctor=%p, alloctor info = %s, ", _objTypeName.c_str(), newAlloctor, newAlloctor->ToString().c_str());
            #endif
            
            CRYSTAL_DELETE_SAFE(newAlloctor);
        };
        auto newDestructor = DelegateFactory::Create<decltype(destructorFunc), void>(destructorFunc);
        // 3.绑定分配器信息回调
        auto toStringFunc = [this, newAlloctor](UInt64 &totalBytes)->LibString {
            #if _DEBUG
             CRYSTAL_TRACE("print a %s newAlloctor=%p, alloctor info = %s, ", _objTypeName.c_str(), newAlloctor, newAlloctor->ToString().c_str());
            #endif
            
            totalBytes += newAlloctor->GetTotalBytes();
            return newAlloctor->ToString();
        };
        auto newToStringDelg = DelegateFactory::Create<decltype(toStringFunc), LibString, UInt64 &>(toStringFunc);

        _lck.Lock();
        _allocAddrRefDestructor.insert(std::make_pair(newAlloctor, newDestructor));
        _allocAddrRefAllocInfoDelg.insert(std::make_pair(newAlloctor, newToStringDelg));
        _sizeRefMemoryAlloc.insert(std::make_pair(allocUnitBytes, newAlloctor));
        _lck.Unlock();

        return newAlloctor;
    }

private:
    SpinLock _lck;
    const std::string _objTypeName;
    std::map<void *, IDelegate<void> *> _allocAddrRefDestructor;
    std::map<void *, IDelegate<LibString, UInt64 &> *> _allocAddrRefAllocInfoDelg;
    std::map<UInt64, void *> _sizeRefMemoryAlloc;                           // 指定大小的memoryalloc 映射 没有多线程竞争
    IDelegate<UInt64, LibString &> *_monitorLog;
};

KERNEL_END

#endif
