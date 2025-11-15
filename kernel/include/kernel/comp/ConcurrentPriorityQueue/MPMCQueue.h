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
 * Date: 2025-11-13 23:47:58
 * Author: Eric Yonng
 * Description: 
 * 1. 使用环形缓冲区实现FIFO
 * 2. 可以用环形缓冲区实现高效的消息队列, 当缓冲区满那么生产者等待, 如果缓冲区没有可读的则消费者等待
 * 测试：多个线程操作测试Pop出来的元素地址不冲突
 *
 * 入队操作：
 * 从头部获取下一个写入票据。
 * 等待轮次（2 * (票据 / 容量)）以写入槽位（票据 % 容量）。
 * 设置轮次 = 轮次 + 1，通知读取者写入完成。
 *
 * 出队操作：
 * 从尾部获取下一个读取票据。
 * 等待轮次（2 * (票据 / 容量) + 1）以读取槽位（票据 % 容量）。
 * 设置轮次 = 轮次 + 1，通知写入者读取完成。
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_CONCURRENT_PRIORITY_QUEUE_RING_BUFFER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_CONCURRENT_PRIORITY_QUEUE_RING_BUFFER_H__

#pragma once

#include <kernel/comp/memory/ObjPoolMacro.h>

#include "kernel/comp/Utils/MathUtil.h"

KERNEL_BEGIN

// 2的整数倍约束,如果Capacity是2的整数倍，那么, 任意数x对Capacity求余, 等价于 x & (Capacity - 1)
template<Int64 N>
concept IsPowerOfTwo = (N > 1) && ((N & (N - 1)) == 0);

template <typename Elem>
requires std::movable<Elem>
struct QueueSlot
{
  // 释放Elem资源
  ~QueueSlot() noexcept
  {
    if constexpr (LibTraitsDataType<Elem>::value == LibDataType::CLASS_TYPE)
    {
      if (_turn.load(std::memory_order_acquire) & 1)
        Destroy();
    }
  }

  // 构建Elem
  template <typename... Args>
  void Construct(Args &&...args) noexcept;

  // 销毁Elem
  void Destroy() noexcept;

  // 移动Elem
  Elem &&Move() noexcept;

  // Align to avoid false sharing between adjacent slots
  alignas(SYSTEM_ALIGN_SIZE) std::atomic<size_t> _turn  = {0};
  typename AlignedStorage<sizeof(Elem), alignof(Elem)>::Type _storage;
};

template <typename Elem>
requires std::movable<Elem>
template <typename... Args>
ALWAYS_INLINE void QueueSlot<Elem>::Construct(Args &&...args) noexcept
{
  new (&_storage) Elem(std::forward<Args>(args)...);
}

template <typename Elem>
requires std::movable<Elem>
ALWAYS_INLINE void QueueSlot<Elem>::Destroy() noexcept
{
  // 类类型的才需要析构
  if constexpr (LibTraitsDataType<Elem>::value == LibDataType::CLASS_TYPE)
  {
    reinterpret_cast<Elem *>(&_storage)->~Elem();
  }
}

template <typename Elem>
requires std::movable<Elem>
ALWAYS_INLINE Elem &&QueueSlot<Elem>::Move() noexcept
{
  return reinterpret_cast<Elem &&>(_storage);
}

/// 多生产者多消费者无锁队列, mpmcqueue 初始化的时候会初始化slots，如果CapacitySize比较大那么会比较耗时,默认16KB个slots
template <typename Elem, size_t CapacitySize = 16 * 1024>
requires std::movable<Elem> && requires
{
    // 2的整数倍
    requires IsPowerOfTwo<CapacitySize>;

    // QueueSlot<Elem> 必须与 SYSTEM_ALIGN_SIZE对齐, QueueSlot 必须按缓存行边界对齐，以防止伪共享。
    requires alignof(QueueSlot<Elem>) == SYSTEM_ALIGN_SIZE;

    // 需要保证 QueueSlot<Elem>大小是SYSTEM_ALIGN_SIZE的整数倍：槽位大小必须是缓存行大小的整数倍，以防止相邻槽位间的伪共享。
    requires (sizeof(QueueSlot<Elem>) % SYSTEM_ALIGN_SIZE) == 0;
}
class MPMCQueue
{
  POOL_CREATE_TEMPLATE_OBJ_DEFAULT(MPMCQueue, Elem, CapacitySize)

public:
  explicit MPMCQueue()
    : _head{0}
    , _tail{0}
  {
        // CapacitySize + 1 多分配一个槽位避免伪共享(false sharing)
        _memory = KERNEL_ALLOC_MEMORY_TL((CapacitySize + 1) *sizeof(QueueSlot<Elem>));
      
        // 超对齐( over-aligned types)需要验证 见:http://eel.is/c++draft/allocator.requirements#10
        // _slots的地址必须与alignof(QueueSlot<Elem>)对齐
        // 一定能在 [_memory, _memory + alignof(QueueSlot<Elem>]之间找到一个与alignof(QueueSlot<Elem>对齐的内存地址
        Byte8 *endAddr = reinterpret_cast<Byte8 *>(_memory) + alignof(QueueSlot<Elem>);
        for (Byte8 *addr = reinterpret_cast<Byte8 *>(_memory); addr <= endAddr; ++addr)
        {
          if (size_t(addr) % alignof(QueueSlot<Elem>) == 0)
          {
              _slots = KERNEL_NS::KernelCastTo<QueueSlot<Elem>>(addr);
              break;
          }
        }

        if (UNLIKELY(_slots == NULL))
        {
          auto addr = _memory;
          if (_memory)
              KERNEL_FREE_MEMORY_TL(_memory);

          _memory = NULL;
          throw std::logic_error(KERNEL_NS::LibString().AppendFormat("_memory:%p cant find alignof(QueueSlot<%s>) = %lld"
              ,addr, KERNEL_NS::RttiUtil::GetByType<Elem>().c_str(), static_cast<Int64>(alignof(QueueSlot<Elem>))).c_str());
        }

        // 初始化 _slots
        for (size_t i = 0; i < CapacitySize; ++i)
          new (&_slots[i]) QueueSlot<Elem>();

          // Queue大小必须与SYSTEM_ALIGN_SIZE对齐, 保证Queue之间避免false sharing 队列大小必须是缓存行大小的整数倍，以防止相邻队列间的伪共享
        static_assert(sizeof(MPMCQueue) % SYSTEM_ALIGN_SIZE == 0,
                      "MPMCQueue size must be a multiple of cache line size to prevent false sharing between adjacent queues");
          
          // tail 与 head 也必须严格间隔一个CACHE_LINE
        static_assert(
            static_cast<Int64>(offsetof(MPMCQueue, _tail) - offsetof(MPMCQueue, _head)) == static_cast<Int64>(SYSTEM_ALIGN_SIZE),
            "_head and _tail must be a cache line apart to prevent false sharing");
  }

  ~MPMCQueue() noexcept
  {
    // 只有类类型才需要销毁
    if constexpr (LibTraitsDataType<Elem>::value == LibDataType::CLASS_TYPE)
    {
      for (size_t i = 0; i < CapacitySize; ++i)
      {
        _slots[i].~QueueSlot();
      }
    }
      
    if (LIKELY(_memory))
      KERNEL_FREE_MEMORY_TL(_memory);

    _memory = NULL;
  }

  // 不可拷贝, 不可移动
  MPMCQueue(const MPMCQueue &) = delete;
  MPMCQueue &operator=(const MPMCQueue &) = delete;

  // 会忙等待, 如果竞争比较激烈, 原理是通过递增_turn来保证线程安全, 如果_turn不满足条件则会忙等待, 直到满足条件
  template <typename... Args>
  void Emplace(Args &&...args) noexcept;

  /// 如果所有写线程无法推动_head说明消费不动了, 此时会退出，不必浪费时间
  template <typename... Args>
  bool TryEmplace(Args &&...args) noexcept;

  void Push(const Elem &v) noexcept;
  template <typename P>
  void Push(P &&v) noexcept;

  bool TryPush(const Elem &v) noexcept;
  template <typename P>
  bool TryPush(P &&v) noexcept;

  // 如果生产过慢, 则会忙等待, 通过匹配_turn实现
  void Pop(Elem &v) noexcept;
  // 如果生产过慢其他读线程也读不到数据就会return false,说明生产过慢, 没必要无意义的尝试
  bool TryPop(Elem &v) noexcept;
 
  /// 返回队列中元素的数量
  /// 当队列为空且至少有一个读者在等待时，可能为负
  /// 由于是并发队列, 在所有读写线程joined之前，这个大小只是尽力而为的推测
  Int64 Size() const noexcept;

  // 这是一个并发队列，所以在没有同步的情况下，判断为空只是尽力而为的猜测，直到所有的读写线程都被汇合（joined）之后才能确定
  bool Empty() const noexcept;

private:
  // 取模, 计算索引
  ALWAYS_INLINE constexpr size_t _Mod(size_t i) const noexcept { return i & (CapacitySize - 1); }
  // i除以Capacity 等价于右移log2 capacity, 计算轮次, 第几轮
  ALWAYS_INLINE constexpr size_t _Turn(size_t i) const noexcept { return i >> Log2OfCapacity; }

private:
    // 编译期计算CapacitySize 对数
    static constexpr size_t Log2OfCapacity = MathUtil::log(2, CapacitySize);

    QueueSlot<Elem> *_slots;
    void *_memory;
    
    // 内存对齐避免, 在_head 与 _tail之间false sharing
    alignas(SYSTEM_ALIGN_SIZE) std::atomic<size_t> _head;
    alignas(SYSTEM_ALIGN_SIZE) std::atomic<size_t> _tail;
};

template <typename Elem, size_t CapacitySize = 16 * 1024>
requires std::movable<Elem> && requires
{
  // 2的整数倍
  requires IsPowerOfTwo<CapacitySize>;

  // QueueSlot<Elem> 必须与 SYSTEM_ALIGN_SIZE对齐, QueueSlot 必须按缓存行边界对齐，以防止伪共享。
  requires alignof(QueueSlot<Elem>) == SYSTEM_ALIGN_SIZE;

  // 需要保证 QueueSlot<Elem>大小是SYSTEM_ALIGN_SIZE的整数倍：槽位大小必须是缓存行大小的整数倍，以防止相邻槽位间的伪共享。
  requires (sizeof(QueueSlot<Elem>) % SYSTEM_ALIGN_SIZE) == 0;
}
POOL_CREATE_TEMPLATE_OBJ_DEFAULT_IMPL(MPMCQueue, Elem, CapacitySize);

template <typename Elem, size_t CapacitySize>
requires std::movable<Elem> && requires
{
  // 2的整数倍
  requires IsPowerOfTwo<CapacitySize>;

  // QueueSlot<Elem> 必须与 SYSTEM_ALIGN_SIZE对齐, QueueSlot 必须按缓存行边界对齐，以防止伪共享。
  requires alignof(QueueSlot<Elem>) == SYSTEM_ALIGN_SIZE;

  // 需要保证 QueueSlot<Elem>大小是SYSTEM_ALIGN_SIZE的整数倍：槽位大小必须是缓存行大小的整数倍，以防止相邻槽位间的伪共享。
  requires (sizeof(QueueSlot<Elem>) % SYSTEM_ALIGN_SIZE) == 0;
}
template <typename... Args>
ALWAYS_INLINE void MPMCQueue<Elem, CapacitySize>::Emplace(Args &&...args) noexcept
{
  auto const head = _head.fetch_add(1);
  auto &slot = _slots[_Mod(head)];

    // 如果读过数据, 那么turn == _Turn(head) * 2, 没读过则一直忙等, 对于其他写线程, 也会等待其他写线程推动slot._turn递增或者读线程推动_turn递增, 通过_turn来保证多生产者, 多消费者线程安全, 如果不想忙等待, 可以使用TryEmplace接口
  while (_Turn(head) * 2 != slot._turn.load(std::memory_order_acquire))
    ;
  slot.Construct(std::forward<Args>(args)...);
  slot._turn.store(_Turn(head) * 2 + 1, std::memory_order_release);
}


template <typename Elem, size_t CapacitySize>
requires std::movable<Elem> && requires
{
  // 2的整数倍
  requires IsPowerOfTwo<CapacitySize>;

  // QueueSlot<Elem> 必须与 SYSTEM_ALIGN_SIZE对齐, QueueSlot 必须按缓存行边界对齐，以防止伪共享。
  requires alignof(QueueSlot<Elem>) == SYSTEM_ALIGN_SIZE;

  // 需要保证 QueueSlot<Elem>大小是SYSTEM_ALIGN_SIZE的整数倍：槽位大小必须是缓存行大小的整数倍，以防止相邻槽位间的伪共享。
  requires (sizeof(QueueSlot<Elem>) % SYSTEM_ALIGN_SIZE) == 0;
}
template <typename... Args>
ALWAYS_INLINE bool MPMCQueue<Elem, CapacitySize>::TryEmplace(Args &&...args) noexcept
{
  auto head = _head.load(std::memory_order_acquire);
  for (;;)
  {
    // 1. 计算槽位索引（环形缓冲区）
    auto &slot = _slots[_Mod(head)];

    // 2. 检查槽位是否可用, 偶数表示可写
    if (_Turn(head) * 2 == slot._turn.load(std::memory_order_acquire))
    {
      // 递增计数
      if (_head.compare_exchange_weak(head, head + 1, std::memory_order_acq_rel))
      {
        // 写入数据
        slot.Construct(std::forward<Args>(args)...);

        // 写入完成后变奇数, 表示可读, _Turn计算轮次
        slot._turn.store(_Turn(head) * 2 + 1, std::memory_order_release);
        return true;
      }
    }
    else
    {
        // 如果 prevHead仍然==head说明_turn不满足条件，可能没消费比较慢，此时退出, 避免无意义的忙等待，如果不相等, 说明是有其他线程改变了_head, 消费得动, 此时再尝试写入
      auto const prevHead = head;
      head = _head.load(std::memory_order_acquire);
      if (head == prevHead)
      {
        return false;
      }
    }
  }
}


template <typename Elem, size_t CapacitySize>
requires std::movable<Elem> && requires
{
  // 2的整数倍
  requires IsPowerOfTwo<CapacitySize>;

  // QueueSlot<Elem> 必须与 SYSTEM_ALIGN_SIZE对齐, QueueSlot 必须按缓存行边界对齐，以防止伪共享。
  requires alignof(QueueSlot<Elem>) == SYSTEM_ALIGN_SIZE;

  // 需要保证 QueueSlot<Elem>大小是SYSTEM_ALIGN_SIZE的整数倍：槽位大小必须是缓存行大小的整数倍，以防止相邻槽位间的伪共享。
  requires (sizeof(QueueSlot<Elem>) % SYSTEM_ALIGN_SIZE) == 0;
}
ALWAYS_INLINE void MPMCQueue<Elem, CapacitySize>::Push(const Elem &v) noexcept
{
  Emplace(v);
}

template <typename Elem, size_t CapacitySize>
requires std::movable<Elem> && requires
{
  // 2的整数倍
  requires IsPowerOfTwo<CapacitySize>;

  // QueueSlot<Elem> 必须与 SYSTEM_ALIGN_SIZE对齐, QueueSlot 必须按缓存行边界对齐，以防止伪共享。
  requires alignof(QueueSlot<Elem>) == SYSTEM_ALIGN_SIZE;

  // 需要保证 QueueSlot<Elem>大小是SYSTEM_ALIGN_SIZE的整数倍：槽位大小必须是缓存行大小的整数倍，以防止相邻槽位间的伪共享。
  requires (sizeof(QueueSlot<Elem>) % SYSTEM_ALIGN_SIZE) == 0;
}
template <typename P>
ALWAYS_INLINE void MPMCQueue<Elem, CapacitySize>::Push(P &&v) noexcept
{
  Emplace(std::forward<P>(v));
}

template <typename Elem, size_t CapacitySize>
requires std::movable<Elem> && requires
{
  // 2的整数倍
  requires IsPowerOfTwo<CapacitySize>;

  // QueueSlot<Elem> 必须与 SYSTEM_ALIGN_SIZE对齐, QueueSlot 必须按缓存行边界对齐，以防止伪共享。
  requires alignof(QueueSlot<Elem>) == SYSTEM_ALIGN_SIZE;

  // 需要保证 QueueSlot<Elem>大小是SYSTEM_ALIGN_SIZE的整数倍：槽位大小必须是缓存行大小的整数倍，以防止相邻槽位间的伪共享。
  requires (sizeof(QueueSlot<Elem>) % SYSTEM_ALIGN_SIZE) == 0;
}
ALWAYS_INLINE bool MPMCQueue<Elem, CapacitySize>::TryPush(const Elem &v) noexcept
{
  return TryEmplace(v);
}


template <typename Elem, size_t CapacitySize>
requires std::movable<Elem> && requires
{
  // 2的整数倍
  requires IsPowerOfTwo<CapacitySize>;

  // QueueSlot<Elem> 必须与 SYSTEM_ALIGN_SIZE对齐, QueueSlot 必须按缓存行边界对齐，以防止伪共享。
  requires alignof(QueueSlot<Elem>) == SYSTEM_ALIGN_SIZE;

  // 需要保证 QueueSlot<Elem>大小是SYSTEM_ALIGN_SIZE的整数倍：槽位大小必须是缓存行大小的整数倍，以防止相邻槽位间的伪共享。
  requires (sizeof(QueueSlot<Elem>) % SYSTEM_ALIGN_SIZE) == 0;
}
template <typename P>
ALWAYS_INLINE bool MPMCQueue<Elem, CapacitySize>::TryPush(P &&v) noexcept
{
  return TryEmplace(std::forward<P>(v));
}


template <typename Elem, size_t CapacitySize>
requires std::movable<Elem> && requires
{
  // 2的整数倍
  requires IsPowerOfTwo<CapacitySize>;

  // QueueSlot<Elem> 必须与 SYSTEM_ALIGN_SIZE对齐, QueueSlot 必须按缓存行边界对齐，以防止伪共享。
  requires alignof(QueueSlot<Elem>) == SYSTEM_ALIGN_SIZE;

  // 需要保证 QueueSlot<Elem>大小是SYSTEM_ALIGN_SIZE的整数倍：槽位大小必须是缓存行大小的整数倍，以防止相邻槽位间的伪共享。
  requires (sizeof(QueueSlot<Elem>) % SYSTEM_ALIGN_SIZE) == 0;
}
ALWAYS_INLINE void MPMCQueue<Elem, CapacitySize>::Pop(Elem &v) noexcept
{
  auto const tail = _tail.fetch_add(1);
  auto &slot = _slots[_Mod(tail)];

    // 如果生产过慢, 则会忙等待
  while (_Turn(tail) * 2 + 1 != slot._turn.load(std::memory_order_acquire))
    ;
      
  v = slot.Move();

  // 非类类型不需要调用Destroy(编译期不会编入)
  if constexpr (LibTraitsDataType<Elem>::value == LibDataType::CLASS_TYPE)
  {
    slot.Destroy();
  }
    
  slot._turn.store(_Turn(tail) * 2 + 2, std::memory_order_release);
}


template <typename Elem, size_t CapacitySize>
requires std::movable<Elem> && requires
{
  // 2的整数倍
  requires IsPowerOfTwo<CapacitySize>;

  // QueueSlot<Elem> 必须与 SYSTEM_ALIGN_SIZE对齐, QueueSlot 必须按缓存行边界对齐，以防止伪共享。
  requires alignof(QueueSlot<Elem>) == SYSTEM_ALIGN_SIZE;

  // 需要保证 QueueSlot<Elem>大小是SYSTEM_ALIGN_SIZE的整数倍：槽位大小必须是缓存行大小的整数倍，以防止相邻槽位间的伪共享。
  requires (sizeof(QueueSlot<Elem>) % SYSTEM_ALIGN_SIZE) == 0;
}
ALWAYS_INLINE bool MPMCQueue<Elem, CapacitySize>::TryPop(Elem &v) noexcept
{
  auto tail = _tail.load(std::memory_order_acquire);
    
  for (;;)
  {
    auto &slot = _slots[_Mod(tail)];
      
    if (_Turn(tail) * 2 + 1 == slot._turn.load(std::memory_order_acquire))
    {
      if (_tail.compare_exchange_weak(tail, tail + 1, std::memory_order_acq_rel))
      {
        v = slot.Move();
        if constexpr (LibTraitsDataType<Elem>::value == LibDataType::CLASS_TYPE)
        {
          slot.Destroy();
        }

        slot._turn.store(_Turn(tail) * 2 + 2, std::memory_order_release);
        return true;
      }
    }
    else
    {
        // prevTail == tail说明所有读线程都没推动_tail改变, 说明生产不动了, 此时没必要尝试直接退出, 如果prevTail != tail说明其他读线程还可以读到数据, 说明还有机会读到数据，则继续尝试
      auto const prevTail = tail;
      tail = _tail.load(std::memory_order_acquire);
      if (tail == prevTail)
      {
        return false;
      }
    }
  }
}


template <typename Elem, size_t CapacitySize>
requires std::movable<Elem> && requires
{
  // 2的整数倍
  requires IsPowerOfTwo<CapacitySize>;

  // QueueSlot<Elem> 必须与 SYSTEM_ALIGN_SIZE对齐, QueueSlot 必须按缓存行边界对齐，以防止伪共享。
  requires alignof(QueueSlot<Elem>) == SYSTEM_ALIGN_SIZE;

  // 需要保证 QueueSlot<Elem>大小是SYSTEM_ALIGN_SIZE的整数倍：槽位大小必须是缓存行大小的整数倍，以防止相邻槽位间的伪共享。
  requires (sizeof(QueueSlot<Elem>) % SYSTEM_ALIGN_SIZE) == 0;
}
ALWAYS_INLINE Int64 MPMCQueue<Elem, CapacitySize>::Size() const noexcept
{
  return static_cast<Int64>(_head.load(std::memory_order_relaxed) -
                                _tail.load(std::memory_order_relaxed));
}

template <typename Elem, size_t CapacitySize>
requires std::movable<Elem> && requires
{
  // 2的整数倍
  requires IsPowerOfTwo<CapacitySize>;

  // QueueSlot<Elem> 必须与 SYSTEM_ALIGN_SIZE对齐, QueueSlot 必须按缓存行边界对齐，以防止伪共享。
  requires alignof(QueueSlot<Elem>) == SYSTEM_ALIGN_SIZE;

  // 需要保证 QueueSlot<Elem>大小是SYSTEM_ALIGN_SIZE的整数倍：槽位大小必须是缓存行大小的整数倍，以防止相邻槽位间的伪共享。
  requires (sizeof(QueueSlot<Elem>) % SYSTEM_ALIGN_SIZE) == 0;
}
ALWAYS_INLINE bool MPMCQueue<Elem, CapacitySize>::Empty() const noexcept
{
  return Size() <= 0;
}


KERNEL_END

#endif
