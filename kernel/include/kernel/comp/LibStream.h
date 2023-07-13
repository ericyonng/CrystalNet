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
 * Author: Eric Yonng
 * Date: 2021-02-09 15:55:24
 * Description: 
 *              单线程下读写
 *              Attach模式下请保证buffer大小是足够的,整个过程Stream是不会重新构建buffer的,也就是说如果buffer不足 Write是有可能会失败的
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LIB_STREAM_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LIB_STREAM_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/memory.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Utils/RttiUtil.h>

KERNEL_BEGIN

template<typename BuildType = _Build::MT>
class LibStream
{
    POOL_CREATE_TEMPLATE_OBJ_DEFAULT(LibStream, BuildType);

public:
    LibStream();
    virtual ~LibStream();

    // 拷贝无论之前是否attach都会创建一个新的缓存, 并把buffer数据拷贝过来
    LibStream(const LibStream<BuildType> &other);
    // 将控制权从other转移过来
    LibStream(LibStream<BuildType> &&other);
    template<typename OtherBuildType>
    LibStream(const LibStream<OtherBuildType> &other);
    template<typename OtherBuildType>
    LibStream(LibStream<OtherBuildType> &&other);

    // // 初始化stream
    void Init(Int64 bufferSize, MemoryPool *pool = KernelMemoryPoolAdapter<BuildType>());
    // 托管,托管状态下buffer的生命周期由外部管理，且不可realloc, 此时需要外部明确buffer需要多大
    void Attach(Byte8 *buffer, Int64 bytes, Int64 readPos = 0, Int64 writePos = 0);
    template<typename OtherBuildType>
    void Attach(const LibStream<OtherBuildType> &stream);
    Byte8 *Pop();
    // 接管
    void TakeOver(LibStream<BuildType> &stream);
    void TakeOver(LibStream<BuildType> &&stream);
    

    // 重置读写
    void Reset();
    void ResetRead();

    // 扩展空间如果是attach模式，则会失败
    bool AppendCapacity(Int64 appendSize);

    // 清理内存 重置,若要重新读写流需要Init
    void Clear();
    void Compress();

    /* 辅助 */
    // assist
    /*
    *   brief:
    *       1. - GetDataAddr 获取数据缓冲区变量的地址（由于数据缓冲区会自动扩展空间所以需要取得空间所在地址）
    *       2. - GetWrPos 已写入数据缓冲区的长度
    *       3. - GetReadPos 已读取数据缓冲区的长度
    *       4. - CanRead 还能读出len字节的数据吗?
    *       5. - CanWrite 还能写入len字节的数据吗?
    *       6. - _OnWritePos 已写入位置，写入位置_writePos 偏移len字节长度
    *       8. - SetWritePos 设置写入位置 _writePos
    *       9. - ShiftReadPos 设置读位置 _readPos
    *       10. - ShiftWritePos 偏移写位置 _writePos
    *       11. - ShiftReadPos 偏移读位置 _readPos
    */
public:
    // 由于内部可能会realloc故buffer的地址有可能出现变化
    Byte8 **GetInnerBufferAddr();
    const Byte8 **GetInnerBufferAddr() const;
    Byte8 *GetBuffer();
    const Byte8 *GetBuffer() const;
    Int64 GetBufferSize() const;
    bool IsAttach() const;
    
    template<typename StreamObjType>
    StreamObjType *CastTo();
    template<typename StreamObjType>
    const StreamObjType *CastTo() const;

    Int64 GetWriteBytes() const;
    bool CanWrite(Int64 len) const;
    bool SetWritePos(Int64 n);
    void ShiftWritePos(Int64 n);
    Int64 GetWritableSize() const;
    Byte8 *GetWriteBegin() const;

    Int64 GetReadBytes() const;
    bool CanRead(Int64 len) const;
    bool SetReadPos(Int64 n);
    void ShiftReadPos(Int64 n);
    Int64 GetReadableSize() const;
    bool IsReadFull() const;
    const Byte8 *GetReadBegin() const;
    Byte8 *GetReadBegin();

    // 字节流序列化反序列化
    bool SerializeTo(LibString &str) const;
    // 反序列化 由string重新构建LibStream 各种属性从String中获得
    // @param(str):数据
    // @param(pool):可选，若是NULL会使用系统的new/delete
    bool DeserializeFrom(const LibString &str, MemoryPool *pool = KernelMemoryPoolAdapter<BuildType>());

    // 自身序列化反序列化
    template<typename FromeBuildType>
    bool DeSerialize(LibStream<FromeBuildType> &from);
    template<typename FromeBuildType>
    bool DeSerialize(const LibStream<FromeBuildType> &from);
    template<typename FromeBuildType>
    bool Serialize(LibStream<FromeBuildType> &to) const;

    LibStream<BuildType> &operator =(const LibStream<BuildType> &other);
    LibStream<BuildType> &operator =(LibStream<BuildType> &&other);
    template<typename FromeBuildType>
    LibStream<BuildType> &operator =(const LibStream<FromeBuildType> &other);
    template<typename FromeBuildType>
    LibStream<BuildType> &operator =(LibStream<FromeBuildType> &&other);
    
    

    /* 读字节流 */
    // Read
public:
    template<typename ObjType>
    bool Read(ObjType &obj);
    template<typename ObjType>
    bool Read(ObjType &obj) const;
    bool Read(LibString &str);
    bool Read(LibString &str) const;
    bool Read(std::string &str);
    bool Read(std::string &str) const;
    // 需要外部自己判断调用 ShiftReadPos
    void Read(void *data, Int64 specifyBytes);
    void Read(void *data, Int64 specifyBytes) const;

    Byte8 ReadInt8();
    Byte8 ReadInt8() const;
    Int16 ReadInt16();
    Int16 ReadInt16() const;
    Int32 ReadInt32();
    Int32 ReadInt32() const;
    Int64 ReadInt64();
    Int64 ReadInt64() const;
    U8    ReadUInt8();
    U8    ReadUInt8() const;
    UInt16 ReadUInt16();
    UInt16 ReadUInt16() const;
    UInt32 ReadUInt32();
    UInt32 ReadUInt32() const;
    UInt64 ReadUInt64();
    UInt64 ReadUInt64() const;
    Float ReadFloat();
    Float ReadFloat() const;
    Double ReadDouble();
    Double ReadDouble() const;

    template<typename T>
    LibStream<BuildType> &operator >>(T &out);
    template<typename T>
    LibStream<BuildType> &operator >>(T &out) const;
    

    /* 写字节流 */
    // Write
public:
    template<typename ObjType>
    bool Write(const ObjType &obj);
    bool Write(const LibString &str);
    bool Write(const std::string &str);

    bool WriteInt8(Byte8 n);
    bool WriteInt16(Int16 n);
    bool WriteInt32(Int32 n);
    bool WriteFloat(Float n);
    bool WriteDouble(Double n);
    bool WriteUInt64(UInt64 n);
    bool WriteInt64(Int64 n);

    bool Write(const void *data, Int64 dataSize);
    template<typename T>
    LibStream<BuildType> &operator <<(const T &input);
    

private:
    void _Swap(LibStream<BuildType> &other);
    void _Copy(const LibStream<BuildType> &other);
    template<typename OtherBuildType>
    void _CopyOtherBuildType(const LibStream<OtherBuildType> &other);
    template<typename OtherBuildType>
    void _SwapOtherBuildType(LibStream<OtherBuildType> &other)
    {
        static_assert(BuildType::V != OtherBuildType::V, "lib stream only surpport diffrent build type for _SwapOtherBuildType function");
        _SwapOtherBuildType(other);
    }
    void _SwapOtherBuildType(LibStream<_Build::MT> &other)
    {
        _CopyOtherBuildType<_Build::MT>(other);
        other.Clear();
    }
    void _SwapOtherBuildType(LibStream<_Build::TL> &other)
    {
        _CopyOtherBuildType<_Build::TL>(other);
        // TL 要十分小心,因为不同线程有不同的内存池不可释放错
    }

    void _InitBuffer();
    void _Read(void *data, Int64 specifyBytes) const;
    void _Write(const void *data, Int64 dataSize);
    void _ReallocBuffer(Int64 newBufferSize);
    bool _MatchBuffToWriteBy(Int64 writeDataSize);

    Int64 _CalcValidDataBytes() const;
    
    // 适配读接口 匹配对象 DeSerialize(LibStream &)接口
    template<typename ObjType, bool(ObjType::*)(LibStream &)>
    struct deserialize_type;
    template<typename ObjType>
    bool _ReadObj(ObjType &obj, deserialize_type<ObjType, &ObjType::DeSerialize> *)
    {
        return obj.DeSerialize(*this);
    }

    template<typename ObjType>
    bool _ReadObj(ObjType &obj, ...);

    // 适配写接口匹配对象void Serialize(LibStream &)const
    template<typename ObjType, bool (ObjType::*)(LibStream &) const>
    struct serialize_type;
    template<typename ObjType>
    bool _WriteObj(const ObjType &obj, serialize_type<ObjType, &ObjType::Serialize> *)
    {
        return obj.Serialize(*this);
    }

    template<typename ObjType>
    bool _WriteObj(const ObjType &obj, ...);

private:
    friend class LibStream<_Build::MT>;
    friend class LibStream<_Build::TL>;

    // 缓冲区总的空间大小，字节长度
    Int64 _size;
    // 已写入数据的尾部位置，已写入数据长度，可以写数据的开始位置
    Int64 _writePos;
    // 已读取数据的尾部位置 可读的开始位置
    Int64 _readPos;
    // 附加的不用管释放,由外部释放, 托管的不可realloc
    bool _isAttach;
    // 数据缓冲区
    Byte8 *_buff;
    MemoryPool *_pool;
};

template<typename BuildType>
POOL_CREATE_TEMPLATE_OBJ_DEFAULT_IMPL(LibStream, BuildType);

template<typename BuildType>
POOL_CREATE_TEMPLATE_OBJ_DEFAULT_TL_IMPL(LibStream, BuildType);

template<typename BuildType>
ALWAYS_INLINE LibStream<BuildType>::LibStream()
:_size(0)
,_writePos(0)
,_readPos(0)
,_isAttach(false)
,_buff(NULL)
,_pool(NULL)
{

}

template<typename BuildType>
ALWAYS_INLINE LibStream<BuildType>::~LibStream()
{
    Clear();
}

template<typename BuildType>
ALWAYS_INLINE LibStream<BuildType>::LibStream(const LibStream<BuildType> &other)
{
    _Copy(other);
}

template<typename BuildType>
ALWAYS_INLINE LibStream<BuildType>::LibStream(LibStream<BuildType> &&other)
{
    _Swap(other);
}

template<typename BuildType>
template<typename OtherBuildType>
ALWAYS_INLINE LibStream<BuildType>::LibStream(const LibStream<OtherBuildType> &other)
{
    static_assert(BuildType::V != OtherBuildType::V, "lib stream only surpport diffrent build type for copy constructor function");
    _CopyOtherBuildType<OtherBuildType>(other);
}

template<typename BuildType>
template<typename OtherBuildType>
ALWAYS_INLINE LibStream<BuildType>::LibStream(LibStream<OtherBuildType> &&other)
{
    static_assert(BuildType::V != OtherBuildType::V, "lib stream only surpport diffrent build type for move constructor function");
    _SwapOtherBuildType<OtherBuildType>(other);
}

template<typename BuildType>
ALWAYS_INLINE void LibStream<BuildType>::Init(Int64 bufferSize, MemoryPool *pool)
{
    Clear();

    _size = bufferSize;
    _writePos = 0;
    _readPos = 0;
    _isAttach = false;
    _pool = pool;

    _InitBuffer();
}

template<typename BuildType>
ALWAYS_INLINE void LibStream<BuildType>::Attach(Byte8 *buffer, Int64 bytes, Int64 readPos, Int64 writePos)
{
    Clear();

    _size = bytes;
    _writePos = std::min<Int64>(_size, writePos);
    _readPos = std::min<Int64>(_writePos, readPos);
    _isAttach = true;
    _buff = buffer;
}

template<typename BuildType>
template<typename OtherBuildType>
ALWAYS_INLINE void LibStream<BuildType>::Attach(const LibStream<OtherBuildType> &other)
{
    this->Attach(const_cast<Byte8 *>(other.GetBuffer()), other.GetBufferSize(), other.GetReadBytes(), other.GetWriteBytes());
}

template<typename BuildType>
ALWAYS_INLINE Byte8 *LibStream<BuildType>::Pop()
{
    auto buffer = _buff;
    _buff = NULL;
    Clear();

    return buffer;
}


template<typename BuildType>
ALWAYS_INLINE void LibStream<BuildType>::TakeOver(LibStream<BuildType> &stream)
{
    if(UNLIKELY(this == &stream))
        return;

    Clear();
    _Swap(stream);
}

template<typename BuildType>
ALWAYS_INLINE void LibStream<BuildType>::TakeOver(LibStream<BuildType> &&stream)
{
    if(UNLIKELY(this == &stream))
        return;

    Clear();
    _Swap(stream);
}

template<typename BuildType>
ALWAYS_INLINE void LibStream<BuildType>::Reset()
{
    _writePos = 0;
    _readPos = 0;
}

template<typename BuildType>
ALWAYS_INLINE void LibStream<BuildType>::ResetRead()
{
    _readPos = 0;
}

template<typename BuildType>
ALWAYS_INLINE bool LibStream<BuildType>::AppendCapacity(Int64 appendSize)
{
    if(UNLIKELY(_isAttach))
        return false;

    _size += appendSize;

    _ReallocBuffer(_size);
    return true;
}

template<typename BuildType>
ALWAYS_INLINE void LibStream<BuildType>::Clear()
{
    if(!_isAttach && _buff)
    {
        if(_pool)
        {
            _pool->FreeAdapter<BuildType>(_buff);
            _buff = NULL;
        }
        else
        {
            ::free(_buff);
        }
    }

    _size = 0;
    _writePos = 0;
    _readPos = 0;
    _buff = NULL;

    // 解除attach
    _isAttach = false; 
    _pool = NULL;
}

template<typename BuildType>
ALWAYS_INLINE void LibStream<BuildType>::Compress()
{
    if(UNLIKELY(!_buff))
        return;

    // 移动数据
    if(_readPos != 0)
        ::memcpy(_buff, _buff + _readPos, _writePos - _readPos);

    _writePos -= _readPos;
    _readPos = 0;

    // 托管的不可改变内存大小
    if(_isAttach)
        return;

    _size = _writePos;
    _buff = KernelCastTo<Byte8>(_pool->ReallocAdapter<BuildType>(_buff, _size));
}

template<typename BuildType>
ALWAYS_INLINE Byte8 **LibStream<BuildType>::GetInnerBufferAddr()
{
    return &_buff;
}

template<typename BuildType>
ALWAYS_INLINE const Byte8 **LibStream<BuildType>::GetInnerBufferAddr() const
{
    return const_cast<const Byte8 **>(&_buff);
}

template<typename BuildType>
ALWAYS_INLINE Byte8 *LibStream<BuildType>::GetBuffer()
{
    return _buff;
}

template<typename BuildType>
ALWAYS_INLINE const Byte8 *LibStream<BuildType>::GetBuffer() const
{
    return _buff;
}

template<typename BuildType>
ALWAYS_INLINE Int64 LibStream<BuildType>::GetBufferSize() const
{
    return _size;
}

template<typename BuildType>
ALWAYS_INLINE bool LibStream<BuildType>::IsAttach() const
{
    return _isAttach;
}

template<typename BuildType>
template<typename StreamObjType>
ALWAYS_INLINE StreamObjType *LibStream<BuildType>::CastTo()
{
    return reinterpret_cast<StreamObjType *>(this);
}

template<typename BuildType>
template<typename StreamObjType>
ALWAYS_INLINE const StreamObjType *LibStream<BuildType>::CastTo() const
{
    return reinterpret_cast<const StreamObjType *>(this);
}

template<typename BuildType>
ALWAYS_INLINE Int64 LibStream<BuildType>::GetWriteBytes() const
{
    return _writePos;
}

template<typename BuildType>
ALWAYS_INLINE bool LibStream<BuildType>::CanWrite(Int64 len) const
{
    return _size - _writePos >= len;
}

template<typename BuildType>
ALWAYS_INLINE bool LibStream<BuildType>::SetWritePos(Int64 n)
{
    if(UNLIKELY(n >= _size))
    {
        CRYSTAL_TRACE("LibStream::SetWritePos fail n:%llu bigger than _size:%lld", n, _size);
        return false;
    }

    _writePos = n;
    return true;
}

template<typename BuildType>
ALWAYS_INLINE void LibStream<BuildType>::ShiftWritePos(Int64 n)
{
    _writePos += n;
}

template<typename BuildType>
ALWAYS_INLINE Int64 LibStream<BuildType>::GetWritableSize() const
{
    return _size - _writePos;
}

template<typename BuildType>
ALWAYS_INLINE Byte8 *LibStream<BuildType>::GetWriteBegin() const
{
    return _buff + _writePos;
}

template<typename BuildType>
ALWAYS_INLINE Int64 LibStream<BuildType>::GetReadBytes() const
{
    return _readPos;
}

template<typename BuildType>
ALWAYS_INLINE bool LibStream<BuildType>::CanRead(Int64 len) const
{
    return (GetReadableSize()) >= len;
}

template<typename BuildType>
ALWAYS_INLINE bool LibStream<BuildType>::SetReadPos(Int64 n)
{
    if(UNLIKELY(n >= _writePos))
    {
        CRYSTAL_TRACE("LibStream::SetReadPos fail n:%llu bigger than _writePos:%llu", n, _writePos);
        return false;
    }

    _readPos = n;
    return true;
}

template<typename BuildType>
ALWAYS_INLINE void LibStream<BuildType>::ShiftReadPos(Int64 n)
{
    _readPos += n;
}

template<typename BuildType>
ALWAYS_INLINE Int64 LibStream<BuildType>::GetReadableSize() const
{
    return _writePos - _readPos;
}


template<typename BuildType>
ALWAYS_INLINE bool LibStream<BuildType>::IsReadFull() const
{
    return _readPos >= _writePos;
}

template<typename BuildType>
ALWAYS_INLINE const Byte8 *LibStream<BuildType>::GetReadBegin() const
{
    return _buff + _readPos;
}

template<typename BuildType>
ALWAYS_INLINE Byte8 *LibStream<BuildType>::GetReadBegin()
{
    return _buff + _readPos;
}

// 字节流序列化反序列化 只序列化pos size 和数据
template<typename BuildType>
ALWAYS_INLINE bool LibStream<BuildType>::SerializeTo(LibString &str) const
{
    // 先写入数据占用空间
    str.AppendData(reinterpret_cast<const char *>(&_size), sizeof(_size));
    str.AppendData(reinterpret_cast<const char *>(&_writePos), sizeof(_writePos));
    str.AppendData(reinterpret_cast<const char *>(&_readPos), sizeof(_readPos));
    const auto validSize = _CalcValidDataBytes();
    str.AppendData(reinterpret_cast<const Byte8 *>((&validSize)), sizeof(validSize));

    // buffer 有的情况下才需要 将有效数据拷入str
    if(validSize)
        str.AppendData(_buff, validSize);

    return !str.empty();
}

template<typename BuildType>
ALWAYS_INLINE bool LibStream<BuildType>::DeserializeFrom(const LibString &str, MemoryPool *pool)
{
    if(str.empty())
        return false;

    // 1.校验参数
    const UInt64 sizeBytes = sizeof(_size);
    const UInt64 wrPosBytes = sizeof(_writePos);
    const UInt64 rdPosBytes = sizeof(_readPos);
    const UInt64 validDataSizeBytes = sizeof(UInt64);
    const UInt64 minRdBytes = sizeBytes + wrPosBytes + rdPosBytes + validDataSizeBytes;
    UInt64 leftBytes = str.size();
    if(leftBytes < minRdBytes)
    {
        CRYSTAL_TRACE("DeserializeFrom fail left bytes:%llu not match limit:%llu", leftBytes, minRdBytes);
        return false;
    }

    // 2.读取size
    UInt64 offsetPos = 0;
    UInt64 cacheSize = 0;
    UInt64 realSizeBytes = str.CopyTo(reinterpret_cast<char *>(&cacheSize), sizeBytes, sizeBytes, offsetPos);
    if(realSizeBytes != sizeBytes)
    {
        CRYSTAL_TRACE("DeserializeFrom read _size fail realSizeBytes:%llu, sizeBytes:%llu", realSizeBytes, sizeBytes);
        return false;
    }
    leftBytes -= realSizeBytes;

    // 3.读取writepos
    UInt64 cacheWrPos = 0;
    offsetPos += realSizeBytes;
    UInt64 resWr = str.CopyTo(reinterpret_cast<char *>(&cacheWrPos), wrPosBytes, wrPosBytes, offsetPos);
    if(resWr != wrPosBytes)
    {
        CRYSTAL_TRACE("DeserializeFrom read _writePos fail resWr:%llu, wrPosBytes:%llu", resWr, wrPosBytes);
        return false;
    }
    leftBytes -= resWr;

    // 4.读取readpos
    offsetPos += resWr;
    UInt64 cacheRdPos = 0;
    UInt64 resRd = str.CopyTo(reinterpret_cast<char *>(&cacheRdPos), rdPosBytes, rdPosBytes, offsetPos);
    if(resRd != rdPosBytes)
    {
        CRYSTAL_TRACE("DeserializeFrom read _readPos fail resWr:%llu, wrPosBytes:%llu", resRd, rdPosBytes);
        return false;
    }
    leftBytes -= resRd;

    // 5.读取有效数据长度
    offsetPos += resRd;
    UInt64 cacheValidDataSize = 0;
    UInt64 realValidDataSizeBytes = str.CopyTo(reinterpret_cast<char *>(&cacheValidDataSize), validDataSizeBytes, validDataSizeBytes, offsetPos);
    if(realValidDataSizeBytes != validDataSizeBytes)
    {
        CRYSTAL_TRACE("DeserializeFrom read cacheValidDataSize fail realValidDataSizeBytes:%llu, validDataSizeBytes:%llu", realValidDataSizeBytes, validDataSizeBytes);
        return false;
    }
    leftBytes -= realValidDataSizeBytes;

    // 5.创建临时缓存 读取buffer数据
    Byte8 *cacheBuffer = NULL;
    if(LIKELY(cacheValidDataSize))
    {
        cacheBuffer = KernelCastTo<Byte8>(KernelAllocMemory<BuildType>(cacheValidDataSize));
        offsetPos += realValidDataSizeBytes;
        UInt64 resBuff = str.CopyTo(cacheBuffer, cacheValidDataSize, cacheValidDataSize, offsetPos);
        if(resBuff != cacheValidDataSize)
        {
            CRYSTAL_TRACE("DeserializeFrom read valid data fail resBuff:%llu, cacheValidDataSize:%llu", resBuff, cacheValidDataSize);
            KernelFreeMemory<BuildType>(cacheBuffer);
            return false;
        }
    }

    // 7.初始化LibStream
    Clear();
    _size = cacheSize;
    _writePos = cacheWrPos;
    _readPos = cacheRdPos;
    _isAttach = false;
    _pool = pool;
    _InitBuffer();
    
    // 8.拷贝有效数据
    if(cacheValidDataSize)
        ::memcpy(_buff, cacheBuffer, cacheValidDataSize);

    if(LIKELY(cacheBuffer))
        KernelFreeMemory<BuildType>(cacheBuffer);

    return true;
}

template<typename BuildType>
template<typename FromeBuildType>
ALWAYS_INLINE bool LibStream<BuildType>::DeSerialize(LibStream<FromeBuildType> &from)
{
    if(UNLIKELY(KernelToVoid(this) == KernelToVoid(&from)))
    {
        CRYSTAL_TRACE("deserialize libstream itsself please check :%p!", this);
        return true;
    }

     // 1.校验参数
    const UInt64 sizeBytes = sizeof(_size);
    const UInt64 wrPosBytes = sizeof(_writePos);
    const UInt64 rdPosBytes = sizeof(_readPos);
    const UInt64 cacheValidDataSizeBytes = sizeof(UInt64);
    const UInt64 minRdBytes = sizeBytes + wrPosBytes + rdPosBytes + cacheValidDataSizeBytes;
    if(!from.CanRead(minRdBytes))
    {
        CRYSTAL_TRACE("DeSerialize fail left bytes:%llu not match limit:%llu", from.GetReadableSize(), minRdBytes);
        return false;
    }

    // 2.读取控制数据
    UInt64 cacheSize = 0;
    UInt64 cacheWrPos = 0;
    UInt64 cacheRdPos = 0;
    UInt64 cacheValidDataSize = 0;
    from.Read(cacheSize);
    from.Read(cacheWrPos);
    from.Read(cacheRdPos);
    from.Read(cacheValidDataSize);

    // 3.创建临时缓存 读取buffer数据
    Byte8 *cacheBuffer = NULL;
    if(cacheValidDataSize)
    {
        cacheBuffer = KernelCastTo<Byte8>(KernelAllocMemory<BuildType>(cacheValidDataSize));
        if(UNLIKELY(!from.CanRead(cacheValidDataSize)))
        {
            CRYSTAL_TRACE("DeSerialize read valid data fail leftReadData:%llu, cacheValidDataSize:%llu", from.GetReadableSize(), cacheValidDataSize);
            KernelFreeMemory<BuildType>(cacheBuffer);
            return false;
        }

        from.Read(cacheBuffer, cacheValidDataSize);
    }

    // 4.拷贝
    Clear();
    _size = cacheSize;
    _writePos = cacheWrPos;
    _readPos = cacheRdPos;
    _isAttach = false;
    _pool = KernelMemoryPoolAdapter<BuildType>();
    _InitBuffer();
    if(cacheValidDataSize)
        ::memcpy(_buff, cacheBuffer, cacheValidDataSize);

    if(LIKELY(cacheBuffer))
        KernelFreeMemory<BuildType>(cacheBuffer);
    return true;
}

template<typename BuildType>
template<typename FromeBuildType>
ALWAYS_INLINE bool LibStream<BuildType>::DeSerialize(const LibStream<FromeBuildType> &from)
{
    if(UNLIKELY(KernelToVoid(this) == KernelToVoid(&from)))
    {
        CRYSTAL_TRACE("deserialize libstream const itsself please check :%p!", this);
        return true;
    }

    LibStream<_Build::TL> *newStream = LibStream<_Build::TL>::NewThreadLocal_LibStream();
    newStream->Attach(const_cast<Byte8 *>(from.GetBuffer()), from.GetBufferSize(), from.GetReadBytes(), from.GetWriteBytes());
    bool ret = DeSerialize(*newStream);
    LibStream<_Build::TL>::DeleteThreadLocal_LibStream(newStream);
    
    return ret;
}

template<typename BuildType>
template<typename FromeBuildType>
ALWAYS_INLINE bool LibStream<BuildType>::Serialize(LibStream<FromeBuildType> &to) const
{
    if(UNLIKELY(KernelToVoid(this) == KernelToVoid(&to)))
    {
        CRYSTAL_TRACE("Serialize libstream to itsself please check :%p!", this);
        return false;
    }

    // 计算需要的空间大小 size + writepos + readpos + buff数据长度的size
    UInt64 needSize = sizeof(_size) + sizeof(_writePos) + sizeof(_readPos) + sizeof(UInt64);

    // buffer有的情况下才会序列化有效数据
    const auto validSize = _CalcValidDataBytes();
    if(LIKELY(_buff && validSize))
        needSize += validSize;

    if(UNLIKELY(!to._MatchBuffToWriteBy(needSize)))
        return false;

    to.Write(_size);
    to.Write(_writePos);
    to.Write(_readPos);
    to.Write(validSize);
    if(LIKELY(validSize))
        to._Write(_buff, validSize);
    
    return true;
}

template<typename BuildType>
ALWAYS_INLINE LibStream<BuildType> &LibStream<BuildType>::operator =(const LibStream<BuildType> &other)
{
    if(UNLIKELY(this == &other))
        return *this;
    
    Clear();
    _Copy(other);
    return *this;
}

template<typename BuildType>
ALWAYS_INLINE LibStream<BuildType> &LibStream<BuildType>::operator =(LibStream<BuildType> &&other)
{
    if(UNLIKELY(this == &other))
        return *this;

    Clear();
    _Swap(other);
    return *this;
}

template<typename BuildType>
template<typename FromeBuildType>
ALWAYS_INLINE LibStream<BuildType> &LibStream<BuildType>::operator =(const LibStream<FromeBuildType> &other)
{
    static_assert(BuildType::V != FromeBuildType::V, "lib stream only surpport diffrent build type for assign function");
    Clear();
    _CopyOtherBuildType<FromeBuildType>(other);
    return *this;
}

template<typename BuildType>
template<typename FromeBuildType>
ALWAYS_INLINE LibStream<BuildType> &LibStream<BuildType>::operator =(LibStream<FromeBuildType> &&other)
{
    static_assert(BuildType::V != FromeBuildType::V, "lib stream only surpport diffrent build type for move assign function");
    Clear();
    _SwapOtherBuildType<FromeBuildType>(other);
    return *this;
}

template<typename BuildType>
template<typename ObjType>
ALWAYS_INLINE bool LibStream<BuildType>::Read(ObjType &obj)
{
    return _ReadObj<ObjType>(obj, 0);
}

template<typename BuildType>
template<typename ObjType>
ALWAYS_INLINE bool LibStream<BuildType>::Read(ObjType &obj) const
{
    LibStream<_Build::TL> *newSteam = LibStream<BuildType>::NewThreadLocal_LibStream();
    newSteam->Attach(_buff, _size, _readPos, _writePos);
    auto ret = newSteam->Read(obj);
    LibStream<BuildType>::DeleteThreadLocal_LibStream(newSteam);
    return ret;
}

template<typename BuildType>
ALWAYS_INLINE bool LibStream<BuildType>::Read(LibString &str)
{
    return Read(str.GetRaw());
}

template<typename BuildType>
ALWAYS_INLINE bool LibStream<BuildType>::Read(LibString &str) const
{
    LibStream<_Build::TL> *newSteam = LibStream<BuildType>::NewThreadLocal_LibStream();
    newSteam->Attach(_buff, _size, _readPos, _writePos);
    auto ret = newSteam->Read(str);
    LibStream<BuildType>::DeleteThreadLocal_LibStream(newSteam);
    return ret;
}

template<typename BuildType>
ALWAYS_INLINE bool LibStream<BuildType>::Read(std::string &str)
{
    UInt64 len = 0;
    static const UInt64 lenBytes = sizeof(len);
    if(UNLIKELY(!CanRead(lenBytes)))
        return false;
    
    _Read(&len, lenBytes);
    if(len)
    {
        // 判断能不能读出
        if(LIKELY(CanRead(len + lenBytes)))
        {
            // 计算已读位置+数组长度所占有空间
            _readPos += lenBytes;

            // 将要读取的数据 拷贝出来
            str.append(_buff + _readPos, len);

            // 计算已读数据位置
            _readPos += len;
            return true;
        }
    }

    return false;
}

template<typename BuildType>
ALWAYS_INLINE bool LibStream<BuildType>::Read(std::string &str) const
{
    LibStream<_Build::TL> *newSteam = LibStream<BuildType>::NewThreadLocal_LibStream();
    newSteam->Attach(_buff, _size, _readPos, _writePos);
    auto ret = newSteam->Read(str);
    LibStream<BuildType>::DeleteThreadLocal_LibStream(newSteam);
    return ret;
}

template<typename BuildType>
ALWAYS_INLINE void LibStream<BuildType>::Read(void *data, Int64 specifyBytes)
{
    // 将要读取的数据 拷贝出来
    _Read(data, specifyBytes);
    _readPos += specifyBytes;
}

template<typename BuildType>
ALWAYS_INLINE void LibStream<BuildType>::Read(void *data, Int64 specifyBytes) const
{
    LibStream<_Build::TL> *newSteam = LibStream<BuildType>::NewThreadLocal_LibStream();
    newSteam->Attach(_buff, _size, _readPos, _writePos);
    newSteam->Read(data, specifyBytes);
    LibStream<BuildType>::DeleteThreadLocal_LibStream(newSteam);
}

template<typename BuildType>
ALWAYS_INLINE Byte8 LibStream<BuildType>::ReadInt8()
{
    Byte8 value = 0;
    Read(value);
    return value;
}

template<typename BuildType>
ALWAYS_INLINE Byte8 LibStream<BuildType>::ReadInt8() const
{
    LibStream<_Build::TL> *newSteam = LibStream<BuildType>::NewThreadLocal_LibStream();
    newSteam->Attach(_buff, _size, _readPos, _writePos);
    auto value = newSteam->ReadInt8();
    LibStream<BuildType>::DeleteThreadLocal_LibStream(newSteam);
    return value;
}

template<typename BuildType>
ALWAYS_INLINE Int16 LibStream<BuildType>::ReadInt16()
{
    Int16 value = 0;
    Read(value);
    return value; 
}

template<typename BuildType>
ALWAYS_INLINE Int16 LibStream<BuildType>::ReadInt16() const
{
    LibStream<_Build::TL> *newSteam = LibStream<BuildType>::NewThreadLocal_LibStream();
    newSteam->Attach(_buff, _size, _readPos, _writePos);
    auto value = newSteam->ReadInt16();
    LibStream<BuildType>::DeleteThreadLocal_LibStream(newSteam);
    return value;
}

template<typename BuildType>
ALWAYS_INLINE Int32 LibStream<BuildType>::ReadInt32()
{
    Int32 value = 0;
    Read(value);
    return value; 
}

template<typename BuildType>
ALWAYS_INLINE Int32 LibStream<BuildType>::ReadInt32() const
{
    LibStream<_Build::TL> *newSteam = LibStream<BuildType>::NewThreadLocal_LibStream();
    newSteam->Attach(_buff, _size, _readPos, _writePos);
    auto value = newSteam->ReadInt32();
    LibStream<BuildType>::DeleteThreadLocal_LibStream(newSteam);
    return value;
}

template<typename BuildType>
ALWAYS_INLINE Int64 LibStream<BuildType>::ReadInt64()
{
    Int64 value = 0;
    Read(value);
    return value; 
}

template<typename BuildType>
ALWAYS_INLINE Int64 LibStream<BuildType>::ReadInt64() const
{
    LibStream<_Build::TL> *newSteam = LibStream<BuildType>::NewThreadLocal_LibStream();
    newSteam->Attach(_buff, _size, _readPos, _writePos);
    auto value = newSteam->ReadInt64();
    LibStream<BuildType>::DeleteThreadLocal_LibStream(newSteam);
    return value;
}

template<typename BuildType>
ALWAYS_INLINE U8 LibStream<BuildType>::ReadUInt8()
{
    U8 value = 0;
    Read(value);
    return value; 
}

template<typename BuildType>
ALWAYS_INLINE U8 LibStream<BuildType>::ReadUInt8() const
{
    LibStream<_Build::TL> *newSteam = LibStream<BuildType>::NewThreadLocal_LibStream();
    newSteam->Attach(_buff, _size, _readPos, _writePos);
    auto value = newSteam->ReadUInt8();
    LibStream<BuildType>::DeleteThreadLocal_LibStream(newSteam);
    return value;
}

template<typename BuildType>
ALWAYS_INLINE UInt16 LibStream<BuildType>::ReadUInt16()
{
    UInt16 value = 0;
    Read(value);
    return value; 
}

template<typename BuildType>
ALWAYS_INLINE UInt16 LibStream<BuildType>::ReadUInt16() const
{
    LibStream<_Build::TL> *newSteam = LibStream<BuildType>::NewThreadLocal_LibStream();
    newSteam->Attach(_buff, _size, _readPos, _writePos);
    auto value = newSteam->ReadUInt16();
    LibStream<BuildType>::DeleteThreadLocal_LibStream(newSteam);
    return value;
}

template<typename BuildType>
ALWAYS_INLINE UInt32 LibStream<BuildType>::ReadUInt32()
{
    UInt32 value = 0;
    Read(value);
    return value; 
}

template<typename BuildType>
ALWAYS_INLINE UInt32 LibStream<BuildType>::ReadUInt32() const
{
    LibStream<_Build::TL> *newSteam = LibStream<BuildType>::NewThreadLocal_LibStream();
    newSteam->Attach(_buff, _size, _readPos, _writePos);
    auto value = newSteam->ReadUInt32();
    LibStream<BuildType>::DeleteThreadLocal_LibStream(newSteam);
    return value;
}

template<typename BuildType>
ALWAYS_INLINE UInt64 LibStream<BuildType>::ReadUInt64()
{
    UInt64 value = 0;
    Read(value);
    return value; 
}

template<typename BuildType>
ALWAYS_INLINE UInt64 LibStream<BuildType>::ReadUInt64() const
{
    LibStream<_Build::TL> *newSteam = LibStream<BuildType>::NewThreadLocal_LibStream();
    newSteam->Attach(_buff, _size, _readPos, _writePos);
    auto value = newSteam->ReadUInt64();
    LibStream<BuildType>::DeleteThreadLocal_LibStream(newSteam);
    return value;
}

template<typename BuildType>
ALWAYS_INLINE Float LibStream<BuildType>::ReadFloat()
{
    Float value = 0;
    Read(value);
    return value;    
}

template<typename BuildType>
ALWAYS_INLINE Float LibStream<BuildType>::ReadFloat() const
{
    LibStream<_Build::TL> *newSteam = LibStream<BuildType>::NewThreadLocal_LibStream();
    newSteam->Attach(_buff, _size, _readPos, _writePos);
    auto value = newSteam->ReadFloat();
    LibStream<BuildType>::DeleteThreadLocal_LibStream(newSteam);
    return value;
}

template<typename BuildType>
ALWAYS_INLINE Double LibStream<BuildType>::ReadDouble()
{
    Double value = 0;
    Read(value);
    return value;  
}

template<typename BuildType>
ALWAYS_INLINE Double LibStream<BuildType>::ReadDouble() const
{
    LibStream<_Build::TL> *newSteam = LibStream<BuildType>::NewThreadLocal_LibStream();
    newSteam->Attach(_buff, _size, _readPos, _writePos);
    auto value = newSteam->ReadDouble();
    LibStream<BuildType>::DeleteThreadLocal_LibStream(newSteam);
    return value;
}

template<typename BuildType>
template<typename T>
ALWAYS_INLINE LibStream<BuildType> &LibStream<BuildType>::operator >>(T &out)
{
    if(UNLIKELY(!Read(out)))
        CRYSTAL_TRACE("LibStream operator >> :read fail");

    return *this;
}

template<typename BuildType>
template<typename T>
ALWAYS_INLINE LibStream<BuildType> &LibStream<BuildType>::operator >>(T &out) const
{
    LibStream<_Build::TL> *newStream = LibStream<_Build::TL>::NewThreadLocal_LibStream();
    newStream->Attach(*this);
    if(!newStream->Read(out))
        CRYSTAL_TRACE("LibStream operator >> const :read fail");
    
    LibStream<_Build::TL>::DeleteThreadLocal_LibStream(newStream);
    return *this;
}

template<typename BuildType>
template<typename ObjType>
ALWAYS_INLINE bool LibStream<BuildType>::Write(const ObjType &obj)
{
    return _WriteObj<ObjType>(obj, 0);
}

template<typename BuildType>
ALWAYS_INLINE bool LibStream<BuildType>::Write(const LibString &str)
{
    return Write(str.GetRaw());
}
    
template<typename BuildType>
ALWAYS_INLINE bool LibStream<BuildType>::Write(const std::string &str)
{
    static const UInt64 lenSize = sizeof(UInt64);
    const UInt64 bytes = str.size();
    if(UNLIKELY(!_MatchBuffToWriteBy(lenSize + bytes)))
        return false;

    _Write(&bytes, lenSize);
    _Write(str.data(), bytes);

    return true;
}

template<typename BuildType>
ALWAYS_INLINE bool LibStream<BuildType>::WriteInt8(Byte8 n)
{
    return Write(n);
}

template<typename BuildType>
ALWAYS_INLINE bool LibStream<BuildType>::WriteInt16(Int16 n)
{
    return Write(n);
}

template<typename BuildType>
ALWAYS_INLINE bool LibStream<BuildType>::WriteInt32(Int32 n)
{
    return Write(n);
}

template<typename BuildType>
ALWAYS_INLINE bool LibStream<BuildType>::WriteFloat(Float n)
{
    return Write(n);
}

template<typename BuildType>
ALWAYS_INLINE bool LibStream<BuildType>::WriteDouble(Double n)
{
    return Write(n);
}

template<typename BuildType>
ALWAYS_INLINE bool LibStream<BuildType>::WriteUInt64(UInt64 n)
{
    return Write(n);
}
 
template<typename BuildType>
ALWAYS_INLINE bool LibStream<BuildType>::WriteInt64(Int64 n)
{
    return Write(n);
}

template<typename BuildType>
ALWAYS_INLINE bool LibStream<BuildType>::Write(const void *data, Int64 dataSize)
{
    if(UNLIKELY(!_MatchBuffToWriteBy(dataSize)))
        return false;

    // 将要写入的数据 拷贝到缓冲区尾部
    ::memcpy(_buff + _writePos, data, dataSize);
    _writePos += dataSize;

    return true;
}

template<typename BuildType>
template<typename T>
ALWAYS_INLINE LibStream<BuildType> &LibStream<BuildType>::operator <<(const T &input)
{
    if(!Write(input))
        CRYSTAL_TRACE("LibStream operator << fail");

    return *this;
}

template<typename BuildType>
ALWAYS_INLINE void LibStream<BuildType>::_Swap(LibStream<BuildType> &other)
{
    _size = other._size;
    _writePos = other._writePos;
    _readPos = other._readPos;
    _isAttach = other._isAttach;
    _buff = other._buff;
    _pool = other._pool;

    // 置空
    other._size = 0;
    other._writePos = 0;
    other._readPos = 0;
    other._isAttach = false;
    other._buff = NULL;
    other._pool = NULL;
}

template<typename BuildType>
ALWAYS_INLINE void LibStream<BuildType>::_Copy(const LibStream<BuildType> &other)
{
    // 拷贝
    _size = other._size;
    _writePos = other._writePos;
    _readPos = other._readPos;
    _isAttach = false;
    _pool = KernelMemoryPoolAdapter<BuildType>();
    _InitBuffer();
    const Int64 validData = other._CalcValidDataBytes();
    if(validData)
        ::memcpy(_buff, other._buff, validData);
}

template<typename BuildType>
template<typename OtherBuildType>
ALWAYS_INLINE void LibStream<BuildType>::_CopyOtherBuildType(const LibStream<OtherBuildType> &other)
{
    // 拷贝
    _size = other._size;
    _writePos = other._writePos;
    _readPos = other._readPos;
    _isAttach = false;
    _pool = KernelMemoryPoolAdapter<BuildType>();
    _InitBuffer();
    const auto validData = other._CalcValidDataBytes();
    if(validData)
        ::memcpy(_buff, other._buff, validData);
}

template<typename BuildType>
ALWAYS_INLINE void LibStream<BuildType>::_InitBuffer()
{
    if(UNLIKELY(_size == 0))
        return;

    if(LIKELY(_pool))
    {
        _buff = KernelCastTo<Byte8>(_pool->AllocAdapter<Byte8, BuildType>(_size));
        return;
    }

    _buff = KernelCastTo<Byte8>(::malloc(_size));
}

template<typename BuildType>
ALWAYS_INLINE void LibStream<BuildType>::_Read(void *data, Int64 specifyBytes) const
{
    // 将要读取的数据 拷贝出来
    ::memcpy(data, _buff + _readPos, specifyBytes);
}

template<typename BuildType>
ALWAYS_INLINE void LibStream<BuildType>::_Write(const void *data, Int64 dataSize)
{
    // 将要写入的数据 拷贝到缓冲区尾部
    ::memcpy(_buff + _writePos, data, dataSize);
    _writePos += dataSize;
}

template<typename BuildType>
ALWAYS_INLINE void LibStream<BuildType>::_ReallocBuffer(Int64 newBufferSize)
{
    if (LIKELY(_buff))
    {
        if(LIKELY(_pool))
        {
            _buff = KernelCastTo<Byte8>(_pool->ReallocAdapter<BuildType>(_buff, newBufferSize));
        }
        else
        {
            _buff = CRYSTAL_REALLOC(Byte8, _buff, newBufferSize);
        }
    }
    else
    {
        if(LIKELY(_pool))
        {
            _buff = KernelCastTo<Byte8>(_pool->AllocAdapter<Byte8, BuildType>(newBufferSize));
        }
        else
        {
            _buff = CRYSTAL_NEW_MULTI(Byte8, newBufferSize);
        }
    }
}

template<typename BuildType>
ALWAYS_INLINE bool LibStream<BuildType>::_MatchBuffToWriteBy(Int64 writeDataSize)
{
    const auto leftBytes = _size - _writePos;
    if (UNLIKELY(leftBytes < writeDataSize))
    {// 不能写且是由内存池创建的缓冲区则自增长为2倍
        // 附加状态是不可扩展内存的
        if(UNLIKELY(_isAttach))
            return false;

        _size += std::max<Int64>(_size, writeDataSize);
        _ReallocBuffer(_size);
    }

    return true;
}

template<typename BuildType>
ALWAYS_INLINE Int64 LibStream<BuildType>::_CalcValidDataBytes() const
{
    return _buff ? (std::min<Int64>(_writePos, _size)) : 0;
}

template<typename BuildType>
template<typename ObjType>
ALWAYS_INLINE bool LibStream<BuildType>::_WriteObj(const ObjType &obj, ...)
{
    static_assert(std::is_pod<ObjType>::value, "LibStream::_WriteObj not pod type:");
    const Int64 bytes = static_cast<Int64>(sizeof(ObjType));
    if(LIKELY(_MatchBuffToWriteBy(bytes)))
    {
        _Write(&obj, bytes);
        return true;
    }

    return false;
}

template<typename BuildType>
template<typename ObjType>
ALWAYS_INLINE bool LibStream<BuildType>::_ReadObj(ObjType &obj, ...)
{
    static_assert(std::is_pod<ObjType>::value, "_ReadObj obj is not pod type");
    // 计算要读取数据的字节长度
    UInt64 objSize = sizeof(ObjType);

    // 判断能不能读
    if(LIKELY(CanRead(objSize)))
    {
        Read(&obj, objSize);
        return true;
    }

    CRYSTAL_TRACE("error, Cant Read. struct name[%s].", RttiUtil::GetByType<ObjType>());
    return false;
}

// 简化类型
using LibStreamMT = LibStream<_Build::MT>;
using LibStreamTL = LibStream<_Build::TL>;

KERNEL_END

#endif

