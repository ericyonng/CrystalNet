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
 * Date: 2020-11-22 17:11:47
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/memory/MemoryPool.h>
#include <kernel/comp/memory/BufferBlockLimit.h>
#include <kernel/comp/memory/MemoryAlloctor.h>
#include <kernel/comp/SmartPtr.h>
#include <kernel/comp/MemoryMonitor/memorymonitor_inc.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Utils/SystemUtil.h>


KERNEL_BEGIN

// MemoryPool *MemoryPool::GetInstance(const InitMemoryPoolInfo &initInfo)
// {
//     // compare_exchange_weak 会先将expected与g_MemoryPool当前值比较，相等则是我们所希望的安全情景，这时可以安全的设置成 setPool
//     // 若不相等，说明被其他线程设置，此时expected就会被更新成g_MemoryPool当前值 并返回false 这个时候为了实现单例直接返回expected即可
//      KERNEL_NS::MemoryPool *expected = NULL;
//      KERNEL_NS::MemoryPool *setPool = CRYSTAL_NEW_OBJ(KERNEL_NS::MemoryPool, initInfo);

//     // 当前值与expected不等时，更新expected为当前值
//     while(!g_MemoryPool.compare_exchange_weak(expected, setPool))
//     {
//         // 说明被其他线程设置过返回池子
//         if(expected)
//         {
//             CRYSTAL_DELETE_SAFE(setPool);
//             return expected;
//         }

//         // 避免空转消耗cpu
//         _mm_pause();
//     }

//     // 没有被其他线程设置过则返回setPool
//     return setPool;
// }


InitMemoryPoolInfo::InitMemoryPoolInfo(UInt64 createMemoryBufferNumWhenInit, UInt64 miniAllockBlockBytes, UInt64 maxAllockBlockBytes)
{
    // 必须是 MEMORY_POOL_MINI_BLOCK_BYTES 的最小公倍数
    _miniAllockBlockBytes = MathUtil::GetLcm(MEMORY_POOL_MINI_BLOCK_BYTES, miniAllockBlockBytes);
    // 必须是 MEMORY_POOL_MAX_BLOCK_BYTES 的最小公倍数
    _maxAllockBlockBytes = MathUtil::GetLcm(MEMORY_POOL_MAX_BLOCK_BYTES, maxAllockBlockBytes);

    // 初始化分配器配置
    UInt64 multi = _miniAllockBlockBytes;
    UInt64 unitBytes = 0;
    UInt64 end = 0;

    for (UInt64 i = 1; i <= _maxAllockBlockBytes;)
    {
        unitBytes = i + multi - 1;
        auto iterCfg = _alloctorCfgs._cfgs.insert(std::make_pair(unitBytes, MemoryAlloctorConfig(unitBytes, createMemoryBufferNumWhenInit))).first;
        iterCfg->second._bufferBlockNumLimit = BufferBlockLimit::GetLimit(unitBytes);
        
        end = std::min<UInt64>(i + multi - 1, _maxAllockBlockBytes + 1);
        i = end + 1;
        multi = end;
    }
}

MemoryPool::MemoryPool(const InitMemoryPoolInfo &initInfo, const std::string &reason)
:_isInit{false}
,_miniAllockBlockBytes(initInfo.GetMin())
,_maxAllockBlockBytes(initInfo.GetMax())
,_memoryBlockHeadSizeAfterAlign(__MEMORY_ALIGN__(sizeof(MemoryBlock)))
,_alloctorCfgs(initInfo.GetAlloctorCfgs())
,_monitorPrintDelg(NULL)
,_reason(reason)
,_threadId(0)
{
    Init();
}

MemoryPool *MemoryPool::GetInstance(const InitMemoryPoolInfo &initInfo)
{
    static SmartPtr<MemoryPool> pool = new MemoryPool(initInfo);

    return pool.AsSelf();
}

Int32 MemoryPool::Init()
{
    // exchange返回旧值
    if(_isInit.exchange(true))
        return Status::Success;

    // 池子内可分配的最大内存大小，其他的由系统分配
    _lck.Lock();
    const UInt64 maxBytes = _maxAllockBlockBytes;
    _bytesPosRefAlloctors.resize(maxBytes + 1);
    UInt64 multi = _miniAllockBlockBytes;
    UInt64 unitBytes = 0;
    MemoryAlloctor *newAlloctor = NULL;
    UInt64 end = 0;

    auto &alloctorCfgs = _alloctorCfgs._cfgs;
    for (UInt64 i = 1; i <= maxBytes;)
    {
        unitBytes = i + multi - 1;
        auto iterCfg = alloctorCfgs.find(unitBytes);
        newAlloctor = new MemoryAlloctor(iterCfg->second);
        end = std::min<UInt64>(i + multi - 1, maxBytes + 1);
        _InitMemRange(i, end,  newAlloctor);
        i = end + 1;
        multi = end;
    }

    // 内存监控
    auto staticstics = MemoryMonitor::GetStatistics();
    _monitorPrintDelg = DelegateFactory::Create<MemoryPool, UInt64, LibString &>(this, &MemoryPool::ToMonitor);
    staticstics->Lock();
    staticstics->GetDict().push_back(_monitorPrintDelg);
    staticstics->Unlock();

    _threadId = SystemUtil::GetCurrentThreadId();
    _lck.Unlock();
    return Status::Success;
}


void MemoryPool::Destroy()
{
    if(!_isInit.exchange(false))
        return;

    _lck.Lock();
    // 内存池日志
    if(LIKELY(_monitorPrintDelg))
    {
        auto staticstics = MemoryMonitor::GetStatistics();
        staticstics->Lock();
        staticstics->Remove(_monitorPrintDelg);
        staticstics->Unlock();
        _monitorPrintDelg = NULL;
    }

    for (auto iter : _alloctorArray)
        CRYSTAL_DELETE_SAFE(iter);
    _alloctorArray.clear();
    _bytesPosRefAlloctors.clear();
    _lck.Unlock();
}

void *MemoryPool::Alloc(UInt64 bytes)
{
    // TODO:多线程性能关系暂时使用系统分配 TODO:对比下跨线程new/delete 与对象池/内存池new/delete性能, 考虑释放优先使用ThreadLocal本地分配
    // return ::malloc(bytes);
   // 判断是否内存池可分配
    if(LIKELY(bytes <= _maxAllockBlockBytes))
    {
        MemoryAlloctor *alloctor = _bytesPosRefAlloctors[bytes];
        alloctor->Lock();
        void *ptr = alloctor->Alloc(bytes);
        alloctor->Unlock();
        return ptr;
    }

    // 非内存池分配
    const UInt64 alignBytes = __MEMORY_ALIGN__(bytes + _memoryBlockHeadSizeAfterAlign);

    Byte8 *cache = reinterpret_cast<Byte8 *>(::malloc(alignBytes));
    MemoryBlock *block = reinterpret_cast<MemoryBlock *>(cache);

    WRITE_MEMORY_BLOCK(block, 1, NULL, NULL, bytes, false);

    return  cache + _memoryBlockHeadSizeAfterAlign;
}

void *MemoryPool::Realloc(void *ptr, UInt64 bytes)
{
    // TODO:多线程性能关系暂时使用系统分配 TODO:对比下跨线程new/delete 与对象池/内存池new/delete性能, 考虑释放优先使用ThreadLocal本地分配
    // return ::realloc(ptr, bytes);
    if(UNLIKELY(ptr == NULL))
        return Alloc(bytes);

    if(UNLIKELY(bytes == 0))
    {
        Free(ptr);
        return NULL;
    }

    // 1.判断是否在内存池中
    MemoryBlock *block = reinterpret_cast<MemoryBlock *>(reinterpret_cast<Byte8 *>(ptr) - _memoryBlockHeadSizeAfterAlign);
    if(LIKELY(block->_isInAlloctor))
    {
        // 分配新内存，并迁移数据
        MemoryBuffer *buffer = block->_buffer;
        MemoryAlloctor *alloctor = buffer->_alloctor;

        // 剩余空间不够需要重新分配
        if(LIKELY((buffer->_blockSize - _memoryBlockHeadSizeAfterAlign) < bytes))
        {
            auto newCache = Alloc(bytes);

            // 三目运算符比if性能高
            ::memcpy(newCache, ptr, block->_realUseBytes);

            alloctor->Lock();
            MemoryAlloctor *otherAlloctor = alloctor->Free(block);
            alloctor->Unlock();
            if(UNLIKELY(otherAlloctor))
            {
                otherAlloctor->Lock();
                otherAlloctor->Free(block);
                otherAlloctor->Unlock();
            }

            return newCache;
        }

        // 原来空间够用就重新利用
        block->_realUseBytes = bytes;
        return ptr;
    }
    
    const UInt64 alignBytes = __MEMORY_ALIGN__(bytes + _memoryBlockHeadSizeAfterAlign);

    auto oldBlock = block;
    auto oldRef = block->_ref;
    Byte8 *newCache = reinterpret_cast<Byte8 *>(::realloc(block, alignBytes));
    block = reinterpret_cast<MemoryBlock *>(newCache);
    WRITE_MEMORY_BLOCK(block, (oldBlock == block ? oldRef : 1), NULL, NULL, bytes, false);

    return  newCache + _memoryBlockHeadSizeAfterAlign;
}

void MemoryPool::AddRef(void *ptr)
{
    // TODO:多线程性能关系暂时使用系统分配 TODO:对比下跨线程new/delete 与对象池/内存池new/delete性能, 考虑释放优先使用ThreadLocal本地分配
    MemoryBlock *block = reinterpret_cast<MemoryBlock*>(reinterpret_cast<Byte8 *>(ptr) - _memoryBlockHeadSizeAfterAlign);

    ++block->_ref;
}

void MemoryPool::Free(void *ptr)
{
    // TODO:多线程性能关系暂时使用系统分配 TODO:对比下跨线程new/delete 与对象池/内存池new/delete性能, 考虑释放优先使用ThreadLocal本地分配
    // ::free(ptr);
    MemoryBlock *block = reinterpret_cast<MemoryBlock *>(reinterpret_cast<Byte8 *>(ptr) - _memoryBlockHeadSizeAfterAlign);
    if(LIKELY(block->_isInAlloctor))
    {
        auto alloctor = block->_buffer->_alloctor;
        alloctor->Lock();
        MemoryAlloctor *otherAlloctor = alloctor->Free(block);
        alloctor->Unlock();

        if(UNLIKELY(otherAlloctor))
        {
            otherAlloctor->Lock();
            otherAlloctor->Free(block);
            otherAlloctor->Unlock();
        }
    }
    else if(--block->_ref == 0)
    {
        ::free(block);
    }
}

void *MemoryPool::AllocThreadLocal(UInt64 bytes)
{
    // 判断是否内存池可分配
    if(LIKELY(bytes <= _maxAllockBlockBytes))
        return _bytesPosRefAlloctors[bytes]->Alloc(bytes);

    // 非内存池分配
    const UInt64 alignBytes = __MEMORY_ALIGN__(bytes + _memoryBlockHeadSizeAfterAlign);

    Byte8 *cache = reinterpret_cast<Byte8 *>(::malloc(alignBytes));
    MemoryBlock *block = reinterpret_cast<MemoryBlock *>(cache);
    WRITE_MEMORY_BLOCK(block, 1, NULL, NULL, bytes, false);

    return  cache + _memoryBlockHeadSizeAfterAlign;
}

void *MemoryPool::ReallocThreadLocal(void *ptr, UInt64 bytes)
{
    if(UNLIKELY(ptr == NULL))
        return AllocThreadLocal(bytes);

    if(UNLIKELY(bytes == 0))
    {
        FreeThreadLocal(ptr);
        return NULL;
    }

// 1.判断是否在内存池中
    MemoryBlock *block = reinterpret_cast<MemoryBlock *>(reinterpret_cast<Byte8 *>(ptr) - _memoryBlockHeadSizeAfterAlign);
    
    if(LIKELY(block->_isInAlloctor))
    {
        MemoryBuffer *buffer = block->_buffer;
        MemoryAlloctor *alloctor = buffer->_alloctor;

        // 剩余空间不够需要重新分配
        if(LIKELY((buffer->_blockSize - _memoryBlockHeadSizeAfterAlign) < bytes))
        {
            auto newCache = AllocThreadLocal(bytes);

            // 三目运算符比if性能高
            ::memcpy(newCache, ptr, block->_realUseBytes);

            alloctor->Free(ptr);
            return newCache;
        }

        // 原来空间够用就重新利用
        block->_realUseBytes = bytes;
        return ptr;
    }
    
    const UInt64 alignBytes = __MEMORY_ALIGN__(bytes + _memoryBlockHeadSizeAfterAlign);

    auto oldBlock = block;
    auto oldRef = block->_ref;
    Byte8 *newCache = reinterpret_cast<Byte8 *>(::realloc(block, alignBytes));
    block = reinterpret_cast<MemoryBlock *>(newCache);
    WRITE_MEMORY_BLOCK(block, (oldBlock == block ? oldRef : 1), NULL, NULL, bytes, false);

    return  newCache + _memoryBlockHeadSizeAfterAlign;
}

void MemoryPool::AddRefThreadLocal(void *ptr)
{
    MemoryBlock *block = reinterpret_cast<MemoryBlock*>(reinterpret_cast<Byte8 *>(ptr) - _memoryBlockHeadSizeAfterAlign);

    //auto alloctor = block->_buffer->_alloctor;
    //alloctor->Lock();
    ++block->_ref;
    //alloctor->Unlock();
}

void MemoryPool::FreeThreadLocal(void *ptr)
{
    MemoryBlock *block = reinterpret_cast<MemoryBlock *>(reinterpret_cast<Byte8 *>(ptr) - _memoryBlockHeadSizeAfterAlign);
    if(LIKELY(block->_isInAlloctor))
    {
        auto alloctor = block->_buffer->_alloctor;
        MemoryAlloctor *otherAlloctor = alloctor->Free(block);
        if(UNLIKELY(otherAlloctor))
            otherAlloctor->Free(block);
    }
    else if(--block->_ref == 0)
    {
        ::free(block);
    }
}

void MemoryPool::_InitMemRange(UInt64 begin, UInt64 end, MemoryAlloctor *alloctor)
{
    alloctor->Init();
    for (UInt64 i = begin; i <= end; ++i)
        _bytesPosRefAlloctors[i] = alloctor;

    _alloctorArray.push_back(alloctor);
}

LibString MemoryPool::UsingInfo() const
{
    LibString str;
    UInt64 totalBytes = 0;
    UInt64 currentBufferBytes = 0;
    for (auto &iter : _alloctorArray)
        currentBufferBytes += iter->GetTotalBytes();
    totalBytes += currentBufferBytes;
    const auto bytesDictSize = _bytesPosRefAlloctors.size() *sizeof(MemoryAlloctor *);
    const auto allocArraySize = (_alloctorArray.size() *sizeof(MemoryAlloctor *));
    totalBytes += bytesDictSize;
    totalBytes += allocArraySize;

    str << "threadId:" << _threadId << ";\n"
        << "reason:" << _reason << ";\n"
        << "is init:" << (_isInit ? "true" : "false") << ";\n"
        << "mini allock block bytes:" << _miniAllockBlockBytes << ";\n"
        << "max allock block bytes:" << _maxAllockBlockBytes << ";\n"
        << "pool memory total occupy bytes:" << totalBytes << ";\n"
        << "pool current buffer total bytes:" << currentBufferBytes << ", is part of total bytes\n"
        << "pool _bytesPosRefAlloctors size :" << bytesDictSize << ", is part of total bytes\n"
        << "pool _alloctorArray size :" << allocArraySize << ", is part of total bytes\n"
        << "all alloctor info:\n"
        ;

   for (auto &iter : _alloctorArray)
       str << iter->GetUnitSize() << " Bytes Block Alloctor:" << iter->UsingInfo() << "\n";

    return str;
}

UInt64 MemoryPool::ToMonitor(LibString &str)
{
    str = UsingInfo();

    UInt64 totalBytes = 0;
    for (auto &iter : _alloctorArray)
        totalBytes += iter->GetTotalBytes();

    totalBytes += (_bytesPosRefAlloctors.size() *sizeof(MemoryAlloctor *));
    totalBytes += (_alloctorArray.size() *sizeof(MemoryAlloctor *));

    return totalBytes;
}

KERNEL_END

