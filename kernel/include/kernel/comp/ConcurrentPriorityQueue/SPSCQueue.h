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
 * Date: 2025-11-18 14:40:58
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_CONCURRENT_PRIORITY_QUEUE_SPSC_QUEUE_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_CONCURRENT_PRIORITY_QUEUE_SPSC_QUEUE_H__

#pragma once

#include <kernel/comp/memory/ObjPoolMacro.h>

KERNEL_BEGIN


template <typename Elem, UInt64 CapacitySize = 16 * 1024>
#ifdef CRYSTAL_NET_CPP20
requires std::is_copy_constructible<Elem>::value && std::movable<Elem> &&  requires
{
  // 需要至少一个元素, 避免溢出
  requires CapacitySize >= 1 && (CapacitySize <= (SIZE_MAX -1 ));
}
#endif
class SPSCQueue
{
  // 至少冗余一个元素
  static constexpr UInt64 CapacityCount = CapacitySize + 1;
  // static constexpr UInt64 kPadding = (SYSTEM_ALIGN_SIZE - 1) / sizeof(T) + 1;
  // 一个元素经过内存对齐后的内存大小, 防止元素间false sharing(伪共享)
  static constexpr UInt64 ELEM_BLOCK_SIZE = sizeof(Elem) / SYSTEM_ALIGN_SIZE * SYSTEM_ALIGN_SIZE + ((sizeof(Elem) % SYSTEM_ALIGN_SIZE) ? SYSTEM_ALIGN_SIZE :0);
  // padding ELEM_BLOCK_SIZE换算成T的倍数, 方便移动指针(扣除自身T之后剩余的padding)
  static constexpr UInt64 ALIGN_PADDING_COUNT = (ELEM_BLOCK_SIZE - 1) / sizeof(Elem) + 1;

  POOL_CREATE_TEMPLATE_OBJ_DEFAULT(SPSCQueue, Elem, CapacitySize)

public:
  explicit SPSCQueue()
      : _slots(NULL)
  {
    // 预分配内存(额外的2个PADDING避免Queue之间的false sharing)
    _memory = KERNEL_ALLOC_MEMORY_TL(ELEM_BLOCK_SIZE * (CapacityCount + 1 + 2 * ALIGN_PADDING_COUNT + 1));
    // 超对齐( over-aligned types)需要验证 见:http://eel.is/c++draft/allocator.requirements#10
    // _slots的地址必须与ELEM_BLOCK_SIZE对齐
    // 一定能在 [_memory, _memory + ELEM_BLOCK_SIZE]之间找到一个与ELEM_BLOCK_SIZE对齐的内存地址
    Byte8 *endAddr = reinterpret_cast<Byte8 *>(_memory) + (2 * ELEM_BLOCK_SIZE);
    // 起始的位置必须间隔一个ELEM_BLOCK_SIZE, 避免与其他queue false sharing
    for (Byte8 *addr = reinterpret_cast<Byte8 *>(_memory) + ELEM_BLOCK_SIZE; addr <= endAddr; ++addr)
    {
      if (UInt64(addr) % ELEM_BLOCK_SIZE == 0)
      {
        _slots = KERNEL_NS::KernelCastTo<Elem>(addr);
        break;
      }
    }
    
    static_assert(alignof(SPSCQueue<Elem>) == SYSTEM_ALIGN_SIZE, "spcqueue must align of : SYSTEM_ALIGN_SIZE");
    static_assert(sizeof(SPSCQueue<Elem>) >= 3 * SYSTEM_ALIGN_SIZE, "spcqueue must 3 multiple than SYSTEM_ALIGN_SIZE");
    assert(reinterpret_cast<char *>(&_readIdx) -
               reinterpret_cast<char *>(&_writeIdx) >=
           static_cast<Int64>(ELEM_BLOCK_SIZE));
  }

  ~SPSCQueue()
  {
    // 如果元素是类类型, 则执行销毁
    if constexpr (LibTraitsDataType<Elem>::value == LibDataType::CLASS_TYPE)
    {
      while (Front()) 
        Pop();
    }

    if (LIKELY(_memory))
    {
      KERNEL_FREE_MEMORY_TL(_memory);
    }
    _memory = NULL;
    _slots = NULL;
  }

  // non-copyable and non-movable
  SPSCQueue(const SPSCQueue &) = delete;
  SPSCQueue &operator=(const SPSCQueue &) = delete;

  // T可以被Args &&...构造, 如果下一个数据没读走则阻塞等待
  template <typename... Args>
  #ifdef CRYSTAL_NET_CPP20
  requires std::is_constructible<Elem, Args &&...>::value
  #endif
  void Emplace(Args &&...args) noexcept;

  // 如果下个数据没读走则退出, 返回false
  template <typename... Args>
  #ifdef CRYSTAL_NET_CPP20
  requires std::is_constructible<Elem, Args &&...>::value
  #endif
  bool TryEmplace(Args &&...args) noexcept;

  // 如果下一个数据没读走则阻塞等待
  void Push(const Elem &v) noexcept;

  // 如果下一个数据没读走则阻塞等待
  template <typename P>
  #ifdef CRYSTAL_NET_CPP20
  requires std::is_constructible<Elem, P &&>::value
  #endif
  void Push(P &&v) noexcept;
  
  bool TryPush(const Elem &v) noexcept;

  template <typename P>
  #ifdef CRYSTAL_NET_CPP20
  requires std::is_constructible<Elem, P &&>::value
  #endif
  bool TryPush(P &&v) noexcept;

  bool TryPop(Elem &elem);
  
  // 只要不改_readIdx那么_readIdx指向的待读取的元素必然是线程安全的
  Elem *Front() noexcept;

  // pop调用之前必须先调用front(),
  void Pop() noexcept;
 

  UInt64 Size() const noexcept;
  bool IsEmpty() const noexcept;

  static constexpr UInt64 Capacity() noexcept;

private:
  Elem *_slots;
  void *_memory;

  // 对齐到缓存行大小以避免伪共享, 使用 readIdxCache_ 和 writeIdxCache_ 来降低缓存一致性流量(先缓存, 当cache与writeIndex/readIndex相等的时候重新更新cache, 减少竞争)
  alignas(SYSTEM_ALIGN_SIZE) std::atomic<UInt64> _writeIdx = {0};
  alignas(SYSTEM_ALIGN_SIZE) UInt64 _readIdxCache = 0;
  alignas(SYSTEM_ALIGN_SIZE) std::atomic<UInt64> _readIdx = {0};
  alignas(SYSTEM_ALIGN_SIZE) UInt64 _writeIdxCache = 0;
};

template <typename Elem, UInt64 CapacitySize>
#ifdef CRYSTAL_NET_CPP20
requires std::is_copy_constructible<Elem>::value && std::movable<Elem> &&  requires
{
  // 需要至少一个元素, 避免溢出
  requires CapacitySize >= 1 && (CapacitySize <= (SIZE_MAX -1 ));
}
#endif
template <typename... Args>
#ifdef CRYSTAL_NET_CPP20
requires std::is_constructible<Elem, Args &&...>::value
#endif
ALWAYS_INLINE void SPSCQueue<Elem, CapacitySize>::Emplace(Args &&...args) noexcept
{
  auto const writeIdx = _writeIdx.load(std::memory_order_relaxed);
  auto nextWriteIdx = writeIdx + 1;
  if (UNLIKELY(nextWriteIdx == CapacityCount))
  {
    nextWriteIdx = 0;
  }

  // 如果 nextWriteIdx == readIdxCache_ 就阻塞等待
  while (nextWriteIdx == _readIdxCache)
  {
    _readIdxCache = _readIdx.load(std::memory_order_acquire);
  }

  // 创建对象 在至少ALIGN_PADDING_COUNT后开始写数据
  new (&_slots[writeIdx + ALIGN_PADDING_COUNT]) Elem(std::forward<Args>(args)...);
  _writeIdx.store(nextWriteIdx, std::memory_order_release);
}

template <typename Elem, UInt64 CapacitySize>
#ifdef CRYSTAL_NET_CPP20
requires std::is_copy_constructible<Elem>::value && std::movable<Elem> &&  requires
{
  // 需要至少一个元素, 避免溢出
  requires CapacitySize >= 1 && (CapacitySize <= (SIZE_MAX -1 ));
}
#endif
template <typename... Args>
#ifdef CRYSTAL_NET_CPP20
requires std::is_constructible<Elem, Args &&...>::value
#endif
ALWAYS_INLINE bool SPSCQueue<Elem, CapacitySize>::TryEmplace(Args &&...args) noexcept
{
  auto const writeIdx = _writeIdx.load(std::memory_order_relaxed);
  auto nextWriteIdx = writeIdx + 1;
  if (UNLIKELY(nextWriteIdx == CapacityCount))
    nextWriteIdx = 0;
    
  if (nextWriteIdx == _readIdxCache)
  {
    _readIdxCache = _readIdx.load(std::memory_order_acquire);
    // 写满, try就由上层决定, 这里先退出
    if (nextWriteIdx == _readIdxCache)
      return false;
  }
  // 在一个block的末尾写入
  new (&_slots[writeIdx + ALIGN_PADDING_COUNT]) Elem(std::forward<Args>(args)...);
  _writeIdx.store(nextWriteIdx, std::memory_order_release);
  return true;
}

template <typename Elem, UInt64 CapacitySize>
#ifdef CRYSTAL_NET_CPP20
requires std::is_copy_constructible<Elem>::value && std::movable<Elem> &&  requires
{
  // 需要至少一个元素, 避免溢出
  requires CapacitySize >= 1 && (CapacitySize <= (SIZE_MAX -1 ));
}
#endif
ALWAYS_INLINE void SPSCQueue<Elem, CapacitySize>::Push(const Elem &v) noexcept
{
  Emplace(v);
}

template <typename Elem, UInt64 CapacitySize>
#ifdef CRYSTAL_NET_CPP20
requires std::is_copy_constructible<Elem>::value && std::movable<Elem> &&  requires
{
  // 需要至少一个元素, 避免溢出
  requires CapacitySize >= 1 && (CapacitySize <= (SIZE_MAX -1 ));
}
#endif
template <typename P>
#ifdef CRYSTAL_NET_CPP20
requires std::is_constructible<Elem, P &&>::value
#endif
ALWAYS_INLINE void  SPSCQueue<Elem, CapacitySize>::Push(P &&v) noexcept
{
  Emplace(std::forward<P>(v));
}

template <typename Elem, UInt64 CapacitySize>
#ifdef CRYSTAL_NET_CPP20
requires std::is_copy_constructible<Elem>::value && std::movable<Elem> &&  requires
{
  // 需要至少一个元素, 避免溢出
  requires CapacitySize >= 1 && (CapacitySize <= (SIZE_MAX -1 ));
}
#endif
ALWAYS_INLINE bool SPSCQueue<Elem, CapacitySize>::TryPush(const Elem &v) noexcept
{
  return TryEmplace(v);
}

template <typename Elem, UInt64 CapacitySize>
#ifdef CRYSTAL_NET_CPP20
requires std::is_copy_constructible<Elem>::value && std::movable<Elem> &&  requires
{
  // 需要至少一个元素, 避免溢出
  requires CapacitySize >= 1 && (CapacitySize <= (SIZE_MAX -1 ));
}
#endif
template <typename P>
#ifdef CRYSTAL_NET_CPP20
requires std::is_constructible<Elem, P &&>::value
#endif
ALWAYS_INLINE bool SPSCQueue<Elem, CapacitySize>::TryPush(P &&v) noexcept
{
  return TryEmplace(std::forward<P>(v));
}


template <typename Elem, UInt64 CapacitySize>
#ifdef CRYSTAL_NET_CPP20
requires std::is_copy_constructible<Elem>::value && std::movable<Elem> &&  requires
{
  // 需要至少一个元素, 避免溢出
  requires CapacitySize >= 1 && (CapacitySize <= (SIZE_MAX -1 ));
}
#endif
ALWAYS_INLINE bool SPSCQueue<Elem, CapacitySize>::TryPop(Elem &elem)
{
  // 先读取, 如果读满了, 先更新下writeCache, 仍然是满的, 返回空
  auto const readIdx = _readIdx.load(std::memory_order_relaxed);
  if (readIdx == _writeIdxCache)
  {
    _writeIdxCache = _writeIdx.load(std::memory_order_acquire);
    // 最新的writeIdx仍然是满的, 退出
    if (_writeIdxCache == readIdx)
    {
      return false;
    }
  }

  // 转移数据, 在至少ALIGN_PADDING_COUNT后开始读数据
  auto &target = _slots[readIdx + ALIGN_PADDING_COUNT];
  elem = std::move(target);

  // 如果是类对象则需要销毁在队列中的对象
  if constexpr (LibTraitsDataType<Elem>::value == LibDataType::CLASS_TYPE)
  {
    target.~T();
  }

  auto nextReadIdx = readIdx + 1;
  if (UNLIKELY(nextReadIdx == CapacityCount))
  {
    nextReadIdx = 0;
  }
    
  _readIdx.store(nextReadIdx, std::memory_order_release);

  return true;
}

template <typename Elem, UInt64 CapacitySize>
#ifdef CRYSTAL_NET_CPP20
requires std::is_copy_constructible<Elem>::value && std::movable<Elem> &&  requires
{
  // 需要至少一个元素, 避免溢出
  requires CapacitySize >= 1 && (CapacitySize <= (SIZE_MAX -1 ));
}
#endif
ALWAYS_INLINE Elem *SPSCQueue<Elem, CapacitySize>::Front() noexcept
{
  // 先读取, 如果读满了, 先更新下writeIdx, 仍然是满的, 返回空
  auto const readIdx = _readIdx.load(std::memory_order_relaxed);
  if (readIdx == _writeIdxCache)
  {
    _writeIdxCache = _writeIdx.load(std::memory_order_acquire);
    if (_writeIdxCache == readIdx)
    {
      return NULL;
    }
  }
    
  return &_slots[readIdx + ALIGN_PADDING_COUNT];
}

template <typename Elem, UInt64 CapacitySize>
#ifdef CRYSTAL_NET_CPP20
requires std::is_copy_constructible<Elem>::value && std::movable<Elem> &&  requires
{
  // 需要至少一个元素, 避免溢出
  requires CapacitySize >= 1 && (CapacitySize <= (SIZE_MAX -1 ));
}
#endif
ALWAYS_INLINE void SPSCQueue<Elem, CapacitySize>::Pop() noexcept
{
  auto const readIdx = _readIdx.load(std::memory_order_relaxed);
  assert(_writeIdx.load(std::memory_order_acquire) != readIdx &&
         "Can only call pop() after front() has returned a non-nullptr");

  if constexpr (LibTraitsDataType<Elem>::value == LibDataType::CLASS_TYPE)
  {
    _slots[readIdx + ALIGN_PADDING_COUNT].~T();
  }

  auto nextReadIdx = readIdx + 1;
  if (nextReadIdx == CapacityCount)
  {
    nextReadIdx = 0;
  }
  _readIdx.store(nextReadIdx, std::memory_order_release);
}

template <typename Elem, UInt64 CapacitySize>
#ifdef CRYSTAL_NET_CPP20
requires std::is_copy_constructible<Elem>::value && std::movable<Elem> &&  requires
{
  // 需要至少一个元素, 避免溢出
  requires CapacitySize >= 1 && (CapacitySize <= (SIZE_MAX -1 ));
}
#endif
ALWAYS_INLINE UInt64 SPSCQueue<Elem, CapacitySize>::Size() const noexcept
{
  Int64 diff = _writeIdx.load(std::memory_order_acquire) -
                        _readIdx.load(std::memory_order_acquire);
  if (UNLIKELY(diff < 0))
  {
    diff += CapacityCount;
  }
  return static_cast<UInt64>(diff);
}

template <typename Elem, UInt64 CapacitySize>
#ifdef CRYSTAL_NET_CPP20
requires std::is_copy_constructible<Elem>::value && std::movable<Elem> &&  requires
{
  // 需要至少一个元素, 避免溢出
  requires CapacitySize >= 1 && (CapacitySize <= (SIZE_MAX -1 ));
}
#endif
ALWAYS_INLINE bool SPSCQueue<Elem, CapacitySize>::IsEmpty() const noexcept
{
  return _writeIdx.load(std::memory_order_acquire) ==
         _readIdx.load(std::memory_order_acquire);
}

template <typename Elem, UInt64 CapacitySize>
#ifdef CRYSTAL_NET_CPP20
requires std::is_copy_constructible<Elem>::value && std::movable<Elem> &&  requires
{
  // 需要至少一个元素, 避免溢出
  requires CapacitySize >= 1 && (CapacitySize <= (SIZE_MAX -1 ));
}
#endif
ALWAYS_INLINE constexpr  UInt64 SPSCQueue<Elem, CapacitySize>::Capacity() noexcept
{
  return CapacityCount - 1;
}

KERNEL_END

#endif