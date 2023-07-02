# 内存管理

* 内存块MemoryBuffer

  1. 对内存块进行切割并串成MemoryBlock链表

  2. 优先从Free链表分配内存

  3. gc规则：

     ```c++
         /* gc重要标记
         *  1. 初始值是1
            2. 调用Alloc 当 _usedBlockCnt 为 _blockCnt 时向左移位一次（表示缓存用光）
            3. 当 _notEnableGcFlag 为0时，表示下次当 _usedBlockCnt 为0时会把MemoryBuffer gc掉，也就是说，当缓存被用光64次后，缓存中没有对象时候就会被gc掉
            4.若不希望buffer被gc掉，可以每次分配的时候手动将flag|=1，这样可以保证buffer永不会被gc掉
         */
         UInt64                  _notEnableGcFlag;
     
     ```
     
     ```c++
     ALWAYS_INLINE UInt64 MemoryBuffer::GetShiftNum(UInt64 memBytes)
     {
         // 1024 被定义为小内存 有64次被用光的机会
         if(LIKELY(memBytes <= 1024))
             return 1;
     
         // 4096内的 被定义为中等内存 有16次被用光的机会
         if(LIKELY(memBytes <= 4096))
             return 4;
     
         // 1MB 内的 被定义为中等内存 有4次被用光的机会
         if(LIKELY(memBytes <= 1048576))
             return 16;
     
         // 1MB以上的内存只有 1 次被用光的机会,用光后会被gc掉
         return 64;
     }
     ```
     
     

* 内存分配器MemoryAlloctor

  1. 将MemoryBuffer串成链表 _activeHead：是激活的链表，总是从这里分配内存

     _busyHead:是MemoryBuffer把所有内存块分配完之后会放到_  _busyHead链表中,只要下次还有可分配的内存块就会放会  _activeHead链表

  2. 性能测试结果：

     * 不加锁
       1. 分配
          * 在不触发NewBuffer情况下alloctor分配的性能是系统的5倍左右
          * 在频繁触发NewBuffer情况下 系统的性能是alloctor的1.3倍
       2. 释放
          * alloctor 的性能是系统的1.5倍
       3. 分配同时释放（大部分情形）
          * alloctor的性能是系统的5倍左右
     * 加锁
       1. 分配
          * 在不触发NewBuffer情况下alloctor分配的性能是系统的5.5倍左右
          * 在频繁触发NewBuffer情况下系统的性能是alloctor的2.2倍左右
       2. 释放
          * alloctor性能是系统的2倍左右
       3. 分配同时释放（大部分情形）
          * alloctor的性能是系统的3倍左右

* 内存池

  性能和MemoryAlloctor类似

* 对象池

  性能和MemoryAlloctor类似
  
* 最新版本已支持了不同MemoryAlloctor回收其他Alloctor时会转到中央收集器，中央收集器再定时定量的合并到创建block的MemoryAlloctor,

* 中央收集器需要打印信息: block创建时的线程, block释放时的线程，block的数量等信息

* 中央收集器以MemoryAlloctor创建的线程id为单位为每个线程创建跨线程合并block区

  * 跨线程合并区数据：以memoryalloctor作为key，value是block数量，block链表
  * 中央收集器当动态时间到时将待分类区域进行归并分类，并合并到每个MemoryAlloctor的待合并区
  * 中央收集器当待合并区block数量达到13w个时立即启动合并
  * 动态时间计算: 当剩余数量为1000个以内时， 则时间设置为100ms, 当数量在 1000 - 10000时时间设置为25ms, 当数量在10000 - 13w之间时，时间设置为1ms，当数量在13w以上时时间设置成0，表示不休眠立即执行、
  * 每个LibThread线程都需要向中央收集器注册属于自己的内存分配器等 的数据
  * 当线程结束的时候如果还有内存块被持有(buffer中的使用数量不为0，则要死等，5分钟过后需要打印日志CRYSTAL_TRACE， 来提醒有内存块被阻塞，输出线程id等信息)



# 总结

不管是使用内存分配器还是内存池对象池，应尽量避免在分配与释放时频繁触发NewBuffer，触发NewBuffer是因为当前Buffer没有可分配的内存了，需要添加新的内存块，所以若预判到有大量的内存分配需求，可以预先指定创建若干的MemoryBuffer在那边准备着，这样可以提升内存分配的性能