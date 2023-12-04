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
 * Date: 2021-01-17 22:12:49
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Tls/TlsMemoryAlloctor.h>
#include <kernel/comp/MemoryMonitor/memorymonitor_inc.h>
#include <kernel/comp/Utils/ContainerUtil.h>
#include <kernel/comp/Lock/Impl/SpinLock.h>

KERNEL_BEGIN

TlsMemoryAlloctor::TlsMemoryAlloctor()
    :_lck(new SpinLock)
    ,_objTypeName("TlsMemoryAlloctor")
    ,_monitorLog(NULL)
{
    _monitorLog = DelegateFactory::Create(this, &TlsMemoryAlloctor::MemMonitor);
    auto staticstics = MemoryMonitor::GetStatistics();
    staticstics->Lock();
    staticstics->GetDict().push_back(_monitorLog);
    staticstics->Unlock();
}

TlsMemoryAlloctor::~TlsMemoryAlloctor()
{
    Destoy();
}

void TlsMemoryAlloctor::Destoy()
{
    _lck->Lock();
    if(UNLIKELY(!_allocAddrRefDestructor.empty()))
    {
        CRYSTAL_TRACE("repeat destroy %p %s", this, _objTypeName.c_str());
        _lck->Unlock();
        return;
    }
    _lck->Unlock();

    CRYSTAL_TRACE("TlsMemoryAlloctor will destroy %llu memory alloctor", static_cast<UInt64>(_allocAddrRefDestructor.size()));

    if(LIKELY(_monitorLog))
    {
        auto staticstics = MemoryMonitor::GetStatistics();
        staticstics->Lock();
        staticstics->Remove(_monitorLog);
        staticstics->Unlock();
        _monitorLog = NULL;
    }

    _lck->Lock();
    CRYSTAL_TRACE("TlsMemoryAlloctor do destroy %llu memory alloctor", static_cast<UInt64>(_allocAddrRefDestructor.size()));
    ContainerUtil::DelContainer(_allocAddrRefAllocInfoDelg);
    for(auto iter = _allocAddrRefDestructor.begin(); iter != _allocAddrRefDestructor.end(); )
    {
        iter->second->Invoke();
        iter = _allocAddrRefDestructor.erase(iter);
    }
    _sizeRefMemoryAlloc.clear();
    _lck->Unlock();

    CRYSTAL_DELETE_SAFE(_lck);

    ITlsObj::Destoy();
}

UInt64 TlsMemoryAlloctor::MemMonitor(LibString &info)
{
    UInt64 totalBytes = 0;
    _lck->Lock();
    for(auto iter :_allocAddrRefAllocInfoDelg)
        info += iter.second->Invoke(totalBytes);
    _lck->Unlock();

    return totalBytes;
}

KERNEL_END