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
 * Date: 2022-12-25 13:43:11
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_PIPELINE_MEMORY_PIPE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_PIPELINE_MEMORY_PIPE_H__

#pragma once

#include <kernel/comp/Pipline/IPipe.h>
#include <kernel/comp/Delegate/LibDelegate.h>
#include <kernel/comp/LibStream.h>
#include <kernel/comp/Pipline/PipeType.h>
#include <kernel/common/func.h>
#include <functional>

KERNEL_BEGIN


template<typename BuildType = _Build::MT>
class MemoryPipe : public IPipe
{
    POOL_CREATE_TEMPLATE_OBJ_DEFAULT_P1(IPipe, MemoryPipe, BuildType);

public:
    MemoryPipe();
    ~MemoryPipe();
    virtual void Release();

    void SetRelease(std::function<void()> &&releaseFunc);
    void Attach(Byte8 *buffer, Int64 sz, Int64 readPos = 0, Int64 writePos = 0);
    void Init(Int64 bufferSize, MemoryPool *pool = KernelMemoryPoolAdapter<BuildType>());
    void PopBuffer(Byte8 *&buffer, Int64 &size);

    virtual void SetPipeName(const LibString &name);
    virtual LibString GetPipeName() const;
    virtual Int32 Open();
    virtual bool Write(const Byte8 *buffer, Int64 &sz);
    virtual bool Read(Byte8 *buffer, Int64 &count);
    virtual void Close();

    Int64 GetWriteBytes() const;
    Int64 GetReadBytes() const;

private:
    IDelegate<void> *_release;
    LibStream<BuildType> _stream;
    LibString _name;
};

template<typename BuildType>
ALWAYS_INLINE MemoryPipe<BuildType>::MemoryPipe()
:IPipe(PipeType::MEMORY)
,_release(NULL)
{

}

template<typename BuildType>
ALWAYS_INLINE MemoryPipe<BuildType>::~MemoryPipe()
{
    Close();

    if(LIKELY(_release))
        CRYSTAL_RELEASE_SAFE(_release);
}

template<typename BuildType>
ALWAYS_INLINE void  MemoryPipe<BuildType>::Release()
{
    if(LIKELY(_release))
    {
        _release->Invoke();
        _release = NULL;
    }
}

template<typename BuildType>
ALWAYS_INLINE void MemoryPipe<BuildType>::SetRelease(std::function<void()> &&releaseFunc)
{
    if(UNLIKELY(_release))
        CRYSTAL_RELEASE_SAFE(_release);

    _release = KERNEL_CREATE_CLOSURE_DELEGATE(releaseFunc, void);
}

template<typename BuildType>
ALWAYS_INLINE void MemoryPipe<BuildType>::Attach(Byte8 *buffer, Int64 sz, Int64 readPos, Int64 writePos)
{
    _stream.Attach(buffer, sz, readPos, writePos);
}

template<typename BuildType>
ALWAYS_INLINE void MemoryPipe<BuildType>::Init(Int64 bufferSize, MemoryPool *pool)
{
    _stream.Init(bufferSize, pool);
}

template<typename BuildType>
ALWAYS_INLINE void MemoryPipe<BuildType>::PopBuffer(Byte8 *&buffer, Int64 &size)
{
    size = _stream.GetBufferSize();
    buffer = _stream.Pop();
}

template<typename BuildType>
ALWAYS_INLINE void MemoryPipe<BuildType>::SetPipeName(const LibString &name)
{
    _name = name;
}

template<typename BuildType>
ALWAYS_INLINE LibString  MemoryPipe<BuildType>::GetPipeName() const
{
    return _name;
}

template<typename BuildType>
ALWAYS_INLINE Int32  MemoryPipe<BuildType>::Open()
{
    return Status::Success;
}

template<typename BuildType>
ALWAYS_INLINE bool MemoryPipe<BuildType>::Write(const Byte8 *buffer, Int64 &sz)
{
    if(_stream.IsAttach())
    {
        const auto writableSize = _stream.GetWritableSize();
        bool ret = _stream.Write(buffer, sz);
        sz = sz > writableSize ? 0 : writableSize;
        return ret;
    }

    _stream.Write(buffer, sz);
    return true;
}

template<typename BuildType>
ALWAYS_INLINE bool MemoryPipe<BuildType>::Read(Byte8 *buffer, Int64 &count)
{
    const auto readableSize = _stream.GetReadableSize();
    count = count >= readableSize ? readableSize : count;
    _stream.Read(buffer, count);

    return true;
}

template<typename BuildType>
ALWAYS_INLINE void MemoryPipe<BuildType>::Close()
{
    _stream.Clear();
}

template<typename BuildType>
ALWAYS_INLINE Int64 MemoryPipe<BuildType>::GetWriteBytes() const
{
    return _stream.GetWriteBytes();
}

template<typename BuildType>
ALWAYS_INLINE Int64 MemoryPipe<BuildType>::GetReadBytes() const
{
    return _stream.GetReadBytes();
}

KERNEL_END

#endif
