/*!
 * MIT License
 *  
 * Copyright (c) 2020 Eric Yonng<120453674@qq.com>
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *  
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *  
 * 
 * Date: 2020-10-06 18:57:40
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_MACRO_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMMON_MACRO_H__

#pragma once

#include <kernel/common/compile.h>

#ifndef CRYSTAL_DEBUG_ENABLE
    #define CRYSTAL_DEBUG_ENABLE 1
#endif

// 使用系统的自选所
#ifndef USE_SYSTEM_SPIN_LOCK
    #define USE_SYSTEM_SPIN_LOCK 1
#endif

#undef NULL
#define NULL nullptr

// 自旋锁，自选轮询次数
#undef SPINNING_COUNT
#define SPINNING_COUNT 8000

// 快照帧数
#undef SYMBOL_MAX_CAPTURE_FRAMES
#define SYMBOL_MAX_CAPTURE_FRAMES 100

// 符号最大长度
#undef SYMBOL_MAX_SYMBOL_NAME
#define SYMBOL_MAX_SYMBOL_NAME 63

// 命名空间
#undef CRYSTAL_NET_BEGIN
#define CRYSTAL_NET_BEGIN namespace CRYSTAL_NET {

#undef CRYSTAL_NET_END
#define CRYSTAL_NET_END }

#undef KERNEL_BEGIN
#define KERNEL_BEGIN                                    \
CRYSTAL_NET_BEGIN                                       \
    namespace kernel {

#undef KERNEL_END
#define KERNEL_END } CRYSTAL_NET_END

// CRYSTAL_NET命名空间
#undef CRYSTAL_NET_NS
#define CRYSTAL_NET_NS ::CRYSTAL_NET

// kernel命名空间
#undef KERNEL_NS
#define KERNEL_NS   CRYSTAL_NET_NS::kernel

#undef DISABLE_COPY_ASSIGN_MOVE
#define DISABLE_COPY_ASSIGN_MOVE(cls)                   \
cls &operator = (const cls &obj) = delete;              \
cls(const cls &obj) = delete;                           \
cls &operator = (cls &&r) = delete;                     \
cls(cls &&r) = delete;                                  

#undef DISABLE_COPY
#define DISABLE_COPY(cls)                               \
cls(const cls &) = delete;

// 请注意windows下一个进程至少有一个heap,原则上从哪个heap申请的内存就必须在哪个heap上释放,否则会crash
// windows下不同dll的crtHeap地址不一样,会造成free失败
// linux下每个进程只有唯一的heap所以不存在这个问题
// 解决方案：windows下new/delete/malloc/free都只调用主heap来申请和释放内存,或者每个对象抽象一个Release接口来delete掉自己
#undef CRYSTAL_DELETE
#define CRYSTAL_DELETE(p)                                  \
delete p

#undef CRYSTAL_MULTI_DELETE
#define CRYSTAL_MULTI_DELETE(p)                            \
delete [] p

#undef CRYSTAL_DELETE_SAFE                                 
#define CRYSTAL_DELETE_SAFE(p)                             \
(p)&&(CRYSTAL_DELETE(p), p=NULL)

#undef CRYSTAL_MULTI_DELETE_SAFE
#define CRYSTAL_MULTI_DELETE_SAFE(p)                       \
(p)&&(CRYSTAL_MULTI_DELETE(p), p=NULL)

#undef CRYSTAL_RELEASE
#define CRYSTAL_RELEASE(p)                                 \
(p) && (p->Release(), true)

#undef CRYSTAL_RELEASE_SAFE
#define CRYSTAL_RELEASE_SAFE(p)                             \
(p)&&(p->Release(), p=NULL)

#undef CRYSTAL_MALLOC
#define CRYSTAL_MALLOC(type, size)                     (reinterpret_cast<type *>(::malloc(size)))
#define CRYSTAL_CALLOC(type, size)                     (reinterpret_cast<type *>(::calloc(size, 1)))
#define CRYSTAL_REALLOC(type, memblock, size)          (reinterpret_cast<type *>(::realloc((memblock), (size))))

#undef CRYSTAL_NEW
#define CRYSTAL_NEW(type)                              new type
#undef CRYSTAL_NEW_MULTI                       
#define CRYSTAL_NEW_MULTI(type, cnt)                   (new type[cnt])                         

// #undef CRYSTAL_NEW_OBJ
// #define CRYSTAL_NEW_OBJ(o, ...)                              (new o(##__VA_ARGS__))  
// #undef CRYSTAL_NEW_OBJ_MULTI                       
// #define CRYSTAL_NEW_OBJ_MULTI(o, cnt, ...)                   (new o(##__VA_ARGS__)[cnt])    

#ifndef INFINITE
#define INFINITE        0xFFFFFFFF
#endif

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
#define CRYSTAL_INFINITE INFINITE
#else
#define CRYSTAL_INFINITE ((Int32)0xFFFFFFFF)
#endif

#undef MAX_NAME_LEN
#define MAX_NAME_LEN 32

#undef MAX_PWD_LEN
#define MAX_PWD_LEN 32

#undef MAX_CEIL_WIDE
#define MAX_CEIL_WIDE 16

#undef DOUBLE_FMT_STR
#define DOUBLE_FMT_STR "%.16lf"

#undef FLOAT_FMT_STR
#define FLOAT_FMT_STR "%.8lf"

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    #ifndef LIKELY
        #define LIKELY(x) (x)
    #endif
    #ifndef UNLIKELY
        #define UNLIKELY(x) (x)
    #endif
#else
    #ifndef LIKELY
        #define LIKELY(x) __builtin_expect(!!(x), 1)
    #endif
    #ifndef UNLIKELY
        #define UNLIKELY(x) __builtin_expect(!!(x), 0)
    #endif
#endif

#if CRYSTAL_TARGET_PLATFORM_WINDOWS

// BuildType: 使用_Build::MT / _Build::TL
#define __CRYSTAL_FmtArgs_WIN32(fmt, buf, len, pool, BuildType)                                         \
    do {                                                                                                \
        int &___len = (len);                                                                            \
        char *&___buf = (buf);                                                                          \
                                                                                                        \
        if (UNLIKELY(!(fmt))) {                                                                         \
            ___len = 0; ___buf = NULL;                                                                  \
            break;                                                                                      \
        }                                                                                               \
                                                                                                        \
        va_list ___ap;                                                                                  \
                                                                                                        \
        int ___bufSize = 1024; ___len = 0;                                                              \
        ___buf = reinterpret_cast<char *>(pool->AllocAdapter<BuildType>(___bufSize + 1));               \
        while (true) {                                                                                  \
            va_start(___ap, fmt);                                                                       \
            ___len = ::vsnprintf_s(___buf, ___bufSize, _TRUNCATE, (fmt), ___ap);                        \
            va_end(___ap);                                                                              \
                                                                                                        \
            if (___len >= 0)                                                                            \
                break;                                                                                  \
                                                                                                        \
            ___bufSize <<= 1;                                                                           \
            ___buf = reinterpret_cast<char *>(pool->ReallocAdapter<BuildType>(___buf, ___bufSize + 1)); \
        }                                                                                               \
        ___buf[___len] = '\0';                                                                          \
    } while (0)                                                                     

#else

#define __CRYSTAL_FmtArgs_NOWIN32(fmt, buf, len, pool, BuildType)                                       \
    do {                                                                                                \
        int &___len = (len);                                                                            \
        char *&___buf = (buf);                                                                          \
                                                                                                        \
        if (UNLIKELY(!(fmt))) {                                                                         \
            ___len = 0; ___buf = NULL;                                                                  \
            break;                                                                                      \
        }                                                                                               \
                                                                                                        \
        va_list ___ap;                                                                                  \
                                                                                                        \
        int ___bufSize = 1024; ___len = 0;                                                              \
        ___buf = reinterpret_cast<char *>(pool->AllocAdapter<BuildType>(___bufSize));                   \
        while (true) {                                                                                  \
            va_start(___ap, (fmt));                                                                     \
            ___len = ::vsnprintf(___buf, ___bufSize, (fmt), ___ap);                                     \
            va_end(___ap);                                                                              \
                                                                                                        \
            /* Workded, break */                                                                        \
            if (___len > -1 && ___len < ___bufSize)                                                     \
                break;                                                                                  \
                                                                                                        \
            /* Try again with more space */                                                             \
            if (LIKELY(___len > -1)) /* glibc 2.1 and later */                                          \
                ___bufSize = ___len + 1;                                                                \
            else /* glibc 2.0 */                                                                        \
                ___bufSize <<= 1;                                                                       \
                                                                                                        \
            ___buf = reinterpret_cast<char *>(pool->ReallocAdapter<BuildType>(___buf, ___bufSize));     \
        }                                                                                               \
        ___buf[___len] = '\0';                                                                          \
    } while(0)                                                                      
#endif


// sprintf 返回值：len(格式化后的字符串长度)
#undef CRYSTAL_SPRINTF
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    #define CRYSTAL_SPRINTF(buffer, bufferSize, fmt, ap)  ::vsnprintf_s(buffer, bufferSize, _TRUNCATE, (fmt), ap);
#else
    #define CRYSTAL_SPRINTF(buffer, bufferSize, fmt, ap)  ::vsnprintf(buffer, bufferSize, (fmt), ap);
#endif

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    #define __CRYSTAL_BuildFormatStr_  __CRYSTAL_FmtArgs_WIN32
#else
    #define __CRYSTAL_BuildFormatStr_  __CRYSTAL_FmtArgs_NOWIN32
#endif

#undef NO_COPY
#define NO_COPY(x)                                                                  \
private:                                                                            \
        x(const x&);                                                                \
        x& operator =(const x&);                                              

#undef TIME_WHEEL_RESOLUTION_DEF
#define TIME_WHEEL_RESOLUTION_DEF 1LL     // 时间轮盘默认精度(毫秒)

// 内存对齐
#undef MEM_ALIGN_BEGIN
#define MEM_ALIGN_BEGIN(n)  pack(push, n)
#undef MEM_ALIGN_END
#define MEM_ALIGN_END       pack(pop)

#undef __MEMORY_ALIGN_BYTES__
#define __MEMORY_ALIGN_BYTES__      (sizeof(void *)<<1)     // 默认16字节对齐 涉及跨cache line 开销

// 面向__MEMORY_ALIGN_BYTES__字节内存对齐
#undef __MEMORY_ALIGN__
#define __MEMORY_ALIGN__(bytes)                                                     \
((bytes) / __MEMORY_ALIGN_BYTES__ * __MEMORY_ALIGN_BYTES__ + ((bytes)%__MEMORY_ALIGN_BYTES__?__MEMORY_ALIGN_BYTES__:0))

// 内存块大小
#undef __MEMORY_ALIGN_BLOCK_SIZE__
#define __MEMORY_ALIGN_BLOCK_SIZE__(ObjBytes) __MEMORY_ALIGN__(ObjBytes + sizeof(KERNEL_NS::MemoryBlock))

// assert接口
#undef POP_BOX
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    #define POP_BOX(str) assert(str)
#else
    #define POP_BOX(str)
#endif

// release下被ASSERT包裹的代码将不会被编译进去
#undef ASSERT
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    #ifdef _DEBUG
        #define ASSERT(x) (x)?true:(POP_BOX(#x))
    #else
        #define ASSERT(x)
    #endif
#endif

#if CRYSTAL_TARGET_PLATFORM_LINUX
 #if defined(_DEBUG)
    #define ASSERT(x) LIKELY(x)?assert(x):(assert(!"check assert fail!!"#x))
 #else
    #define ASSERT(x)
 #endif
#endif


#undef ARRAY_ELEM_COUNT
#define ARRAY_ELEM_COUNT(x) sizeof(x)/sizeof(x[0])

// memcpy
#undef KERNEL_MEMCPY
#define KERNEL_MEMCPY(dest, src, bytes)                                             \
{                                                                                   \
    size_t chg = bytes;                                                             \
    for(;chg--;)                                                                    \
        *(dest + chg) = *(src + chg);                                               \
}                                               

// 普通打印
#if CRYSTAL_DEBUG_ENABLE

    #undef CRYSTAL_TRACE
    #define CRYSTAL_TRACE(fmt, ...)                                                         \
    {KERNEL_NS::LockConsole();                                                   \
    printf("file[%s],line[%d]:" fmt "\n", __FILE__, __LINE__,  ##__VA_ARGS__);              \
    KERNEL_NS::UnlockConsole();}

#else
    #undef CRYSTAL_TRACE
    #ifdef _DEBUG
        #define CRYSTAL_TRACE(fmt, ...)                                                         \
        {KERNEL_NS::LockConsole();                                                   \
        printf("file[%s],line[%d]:" fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);               \
        KERNEL_NS::UnlockConsole();}

    #else
        #define CRYSTAL_TRACE(fmt, ...)  {KERNEL_NS::LockConsole(); printf("file[%s],line[%d]:" fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); KERNEL_NS::UnlockConsole();}

    #endif

#endif

// 清零
#undef ZERO_RESET
#define ZERO_RESET(ptr)                                                             \
::memset(ptr, 0, sizeof(*ptr))

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
// 是否因为信号被唤醒 windows event组件配合wait
#undef CRYSTAL_IS_EVENT_SINAL_WAKE_UP
#define CRYSTAL_IS_EVENT_SINAL_WAKE_UP(waitRet)                                      \
(static_cast<Int64>(WAIT_OBJECT_0) <= (waitRet)) &&                                  \
((waitRet) <= static_cast<Int64>(MAXIMUM_WAIT_OBJECTS + WAIT_OBJECT_0))
#endif

// 默认需要剔除的符号
#undef DEF_STRIP_CHARS
#define DEF_STRIP_CHARS   " \t\v\r\n\f"       

// 不同平台下文本行结束字符
#undef LINE_END_CHARS
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    #define LINE_END_CHARS "\r\n"
#else
    #define LINE_END_CHARS "\n"
#endif

#undef FMT_FLAGS
#define FMT_FLAGS    "-+ #0"

// 浮点数精度
#define FLOAT_NUM_PRECISION     DBL_EPSILON
// 浮点数0值比较
#define COMP_FLOAT_WITH_ZERO(floatNum)                  \
( ((floatNum) < FLOAT_NUM_PRECISION) &&  ((floatNum) > -FLOAT_NUM_PRECISION) )
// 浮点数相等
#define IS_DOUBLE_EQUAL(a, b)                           \
COMP_FLOAT_WITH_ZERO(a-b)

// 浮点数比较大小
#define IS_DOUBLE_BIGGER(a, b)                          \
!IS_DOUBLE_EQUAL(a, b) && ((a-b)>0)


// 估计的要关闭的文件描述符最大值
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    #undef BD_MAX_CLOSE
    #define BD_MAX_CLOSE 8192
#endif

#undef unreachable
#define unreachable() ASSERT(!"unreachable")
#undef not_reached
#define not_reached() unreachable()


#undef set_errno
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    #define set_errno(errNum)    SetLastError(errNum)
#else
    #define set_errno(errNum)    errno = (errNum)
#endif

#undef get_errno
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    #define get_errno()    GetLastError()
#else
    #define get_errno()    errno
#endif

#undef LG_SIZEOF_INTMAX_T
#define LG_SIZEOF_INTMAX_T 4

#undef U2S_BUFSIZE
#define U2S_BUFSIZE ((1U << (LG_SIZEOF_INTMAX_T + 3)) + 1)

#undef D2S_BUFSIZE
#define D2S_BUFSIZE (1 + U2S_BUFSIZE)

#undef O2S_BUFSIZE
#define O2S_BUFSIZE (1 + U2S_BUFSIZE)

#undef X2S_BUFSIZE
#define X2S_BUFSIZE (2 + U2S_BUFSIZE)


// 提升循环性能
#undef CRYSTAL_OPTIMIZE_LOOP
#define CRYSTAL_OPTIMIZE_LOOP(loopCounter, LOOP_LIMIT)                  \
            SystemUtil::RelaxCpu();                                     \
                                                                        \
            if (LIKELY(--loopCounter))                                  \
                continue;                                               \
                                                                        \
            std::this_thread::yield();                                  \
            loopCounter = LOOP_LIMIT


// 行结束
#undef CRYSTAL_NET_LINE_END
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    #define CRYSTAL_NET_LINE_END    "\r\n"
#else
    #define CRYSTAL_NET_LINE_END    "\n"
#endif

/*
 * Number of milli-seconds/micro-seconds/100-nano seconds between the beginning of the
 * Windows epoch (Jan. 1, 1601) and the Unix epoch (Jan. 1, 1970).
 *
 * This assumes all Win32 compilers have 64-bit support.
 */
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
 #if defined(_MSC_VER) || defined(_MSC_EXTENSIONS) || defined(__WATCOMC__)
    #define CRYSTAL_EPOCH_IN_MSEC    11644473600000LLU
    #define CRYSTAL_EPOCH_IN_USEC    11644473600000000LLU
    #define CRYSTAL_EPOCH_IN_100NSEC 116444736000000000LLU
 #else
    #define CRYSTAL_EPOCH_IN_MSEC    11644473600000LLU
    #define CRYSTAL_EPOCH_IN_USEC    11644473600000000LLU
    #define CRYSTAL_EPOCH_IN_100NSEC 116444736000000000LLU
 #endif
#endif

#ifndef LIB_LOG_FMT
#define LIB_LOG_FMT(...) (##__VA_ARGS__)
#endif

// ini 统一放在程序根目录下的ini目录
#define ROOT_DIR_INI_SUB_DIR    "ini/"
#define ROOT_DIR_LOG_SUB_DIR    "Log/"

// socket
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    #ifdef __APPLE__
        #define _DARWIN_UNLIMITED_SELECT
    #endif // !__APPLE__

    #define SOCKET Int32
    #define INVALID_SOCKET          -1
    #define SOCKET_ERROR            (-1)
#endif

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    #ifndef DEF_THREAD_LOCAL_DECLEAR
        #define DEF_THREAD_LOCAL_DECLEAR thread_local
    #endif
#else
    #ifndef DEF_THREAD_LOCAL_DECLEAR
        #define DEF_THREAD_LOCAL_DECLEAR __thread
    #endif
#endif

#ifndef DEF_STATIC_THREAD_LOCAL_DECLEAR
    #define DEF_STATIC_THREAD_LOCAL_DECLEAR static DEF_THREAD_LOCAL_DECLEAR 
#endif


// 获取cpuid
#if CRYSTAL_TARGET_PLATFORM_WINDOWS

// 支持cpu主功能号(cpuEABCDXInfoArray:int array[4])
// #include <intrin.h>
#undef CRYSTAL_CPUID
#define CRYSTAL_CPUID(cpuMainFunctionId, cpuEABCDXInfoArray)                            \
__cpuid(cpuEABCDXInfoArray, cpuMainFunctionId)

// 支持cpu主功能号, 子功能号(cpuEABCDXInfoArray:int array[4])
#undef CRYSTAL_CPUID_EX
#define CRYSTAL_CPUID_EX(cpuMainFunctionId, subFunctionId, cpuEABCDXInfoArray)          \
__cpuidex(cpuEABCDXInfoArray, cpuMainFunctionId, subFunctionId)

#else

// 非windows平台采用内联汇编 #include <cpuid.h>  // 获取cpuid
// 支持cpu主功能号(cpuEABCDXInfoArray:int array[4])
// 支持cpu主功能号
#undef CRYSTAL_CPUID
#define CRYSTAL_CPUID(cpuMainFunctionId, cpuEABCDXInfoArray)                                                                                \
  __asm__("xchg{l}\t{%%}ebx, %1\n\t"                                                                                                        \
       "cpuid\n\t"                                                                                                                          \
       "xchg{l}\t{%%}ebx, %1\n\t"                                                                                                           \
       : "=a" (cpuEABCDXInfoArray[0]), "=r" (cpuEABCDXInfoArray[1]), "=c" (cpuEABCDXInfoArray[2]), "=d" (cpuEABCDXInfoArray[3])             \
       : "0" (cpuMainFunctionId))

// 支持cpu主功能号, 子功能号
#undef CRYSTAL_CPUID_EX
#define CRYSTAL_CPUID_EX(cpuMainFunctionId, subFunctionId, cpuEABCDXInfoArray)                                                                          \
  __asm__ ("xchg{l}\t{%%}ebx, %1\n\t"                                                                                                                   \
       "cpuid\n\t"                                                                                                                                      \
       "xchg{l}\t{%%}ebx, %1\n\t"                                                                                                                       \
       : "=a" (cpuEABCDXInfoArray[0]), "=r" (cpuEABCDXInfoArray[1]), "=c" (cpuEABCDXInfoArray[2]), "=d" (cpuEABCDXInfoArray[3])                         \
       : "0" (cpuMainFunctionId), "2" (subFunctionId))

#endif

// tsc高精度计时器支持
#undef CRYSTAL_RDTSC
#define CRYSTAL_RDTSC()                 \
__rdtsc()

// cpuCoreId:UInt32
#undef CRYSTAL_RDTSCP
#define CRYSTAL_RDTSCP(cpuCoreId)      \
__rdtscp(&cpuCoreId)

#undef CRYSTAL_ORDERED_RDTSC_START
#define CRYSTAL_ORDERED_RDTSC_START

// Force inline macro define. __forceinline
#undef CRYSTAL_FORCE_INLINE
#if defined(_MSC_VER)
 #define CRYSTAL_FORCE_INLINE inline
#elif defined(__GNUC__) || defined(__clang__)
 #define CRYSTAL_FORCE_INLINE __inline__ __attribute__((always_inline))
#else
 #define CRYSTAL_FORCE_INLINE inline
#endif

/**
 * Function string format arguments check macro define.
 * format特性 参数隐藏this指针（第一个参数）,所以fmtIdx需要偏移一位
 */
#undef LIB_KERNEL_FORMAT_CHECK
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
 #define LIB_KERNEL_FORMAT_CHECK(fmtIdx, fmtArgsBegIdx)
#else // Non-Win32
 #define LIB_KERNEL_FORMAT_CHECK(fmtIdx, fmtArgsBegIdx) __attribute__((format(printf, fmtIdx, fmtArgsBegIdx)))
#endif // LLBC_TARGET_PLATFORM_WIN32

// 内联属性
#undef ALWAYS_INLINE
#define ALWAYS_INLINE CRYSTAL_FORCE_INLINE

// 标准库名字空间
#undef FOR_STD_LIB_BEGIN
#define FOR_STD_LIB_BEGIN namespace std { 
#undef FOR_STD_LIB_END
#define FOR_STD_LIB_END }

// SOCKET_ERROR
#undef KERNEL_SOCKET_ERROR
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
#define KERNEL_SOCKET_ERROR SOCKET_ERROR
#else
#define KERNEL_SOCKET_ERROR -1
#endif

#undef DEF_AGAINST_TEMPLATE_LAZY
#define DEF_AGAINST_TEMPLATE_LAZY()     \
public:                                 \
Int32 _againstLazy = 0

// 不使用的变量
#ifndef UNUSED
 #define UNUSED(x) (void)(x)
#endif

#endif
