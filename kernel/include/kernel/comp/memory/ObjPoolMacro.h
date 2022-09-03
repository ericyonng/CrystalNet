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
 * Date: 2022-01-23 12:48:39
 * Author: Eric Yonng
 * Description: 
 * 1. 对象池会初始化一个buffer, 连续内存, 这个buffer大小会根据initBlockNumPerBuffer来定,
 * 2. 对象池buffer对象都分配完之后会进行扩容,扩容的新buffer的block个数是上一个bufferblock数量的2倍....
 * 3. 对象池支持垃圾回收,当对象free后会出现n个空闲的没有分配任何对象的buffer，这时候当空闲buffer达到buffer限制数量的一半以上的时候会进行回收
 * 4. 回收策略：会把空的buffer释放到只有bufferlimit数量的一半以下
 * 5. 回收是在垃圾回收线程里做的,而对象池中只是把要回收的buffer swap到垃圾回收线程的队列中，几乎没有什么消耗
 * 6. 对象池对象创建请使用对象池创建宏,便于追溯对象分配信息
 * 7. MEMORY_BUFFER_BLOCK_INIT 默认的对象池初始化对象个数
 * 8.区分普通类宏与泛型宏
 *                     非泛型宏:POOL_CREATE_OBJ, POOL_CREATE_OBJ_P1, POOL_CREATE_OBJ_P2, ...
 *                     非泛型默认宏:POOL_CREATE_OBJ_DEFAULT,POOL_CREATE_OBJ_DEFAULT_P1,POOL_CREATE_OBJ_DEFAULT_P2,...
 *                     非泛型默认宏初始化时不创建buffer(节省内存资源):POOL_CREATE_OBJ_DEFAULT_NO_BUFFER_INIT,POOL_CREATE_OBJ_DEFAULT_NO_BUFFER_INIT_P1,POOL_CREATE_OBJ_DEFAULT_NO_BUFFER_INIT_P2,...
 * 
 *                     泛型宏:POOL_CREATE_TEMPLATE_OBJ, POOL_CREATE_TEMPLATE_OBJ_P1, POOL_CREATE_TEMPLATE_OBJ_P2, ...
 *                     泛型默认宏:POOL_CREATE_TEMPLATE_OBJ_DEFAULT, POOL_CREATE_TEMPLATE_OBJ_DEFAULT_P1, POOL_CREATE_TEMPLATE_OBJ_DEFAULT_P2, ...
 *                     泛型默认宏初始化不创建buffer（节省内存资源）:POOL_CREATE_TEMPLATE_OBJ_DEFAULT_NO_BUFFER_INIT, POOL_CREATE_TEMPLATE_OBJ_DEFAULT_NO_BUFFER_INIT_P1, POOL_CREATE_TEMPLATE_OBJ_DEFAULT_NO_BUFFER_INIT_P2, ...
 *                              
 * Attention:
 *          不支持多于4个继承,多的要考虑是不是设计有问题
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_OBJ_POOL_MACRO_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_OBJ_POOL_MACRO_H__

#pragma once

// // 非泛型宏

#include <kernel/comp/memory/Defs/Defs.h>

#undef POOL_CREATE_OBJ
#define POOL_CREATE_OBJ(createBufferWhenInit, initBlockNumPerBuffer, ObjType)   \
__OBJ_POOL_CREATE_OBJ(createBufferWhenInit, initBlockNumPerBuffer, ObjType)\
__OBJ_POOL_CREATE_GEN_BEGIN(ObjType)\
__OBJ_POOL_CREATE_GEN_END()\
__OBJ_POOL_CREATE_THREAD_LOCAL_BEGIN(ObjType)\
__OBJ_POOL_CREATE_THREAD_LOCAL_END()

// 单继承
#undef POOL_CREATE_OBJ_P1
#define POOL_CREATE_OBJ_P1(createBufferWhenInit, initBlockNumPerBuffer, ParentClass, ObjType)\
__OBJ_POOL_CREATE_OBJ(createBufferWhenInit, initBlockNumPerBuffer, ObjType)\
__OBJ_POOL_CREATE_GEN_BEGIN(ObjType)\
__OBJ_POOL_CREATE_GEN_ADD_PARANT(ParentClass)\
__OBJ_POOL_CREATE_GEN_END()\
__OBJ_POOL_CREATE_THREAD_LOCAL_BEGIN(ObjType)\
__OBJ_POOL_CREATE_THREAD_LOCAL_ADD_PARANT(ParentClass)\
__OBJ_POOL_CREATE_THREAD_LOCAL_END()

// 双继承
#undef POOL_CREATE_OBJ_P2
#define POOL_CREATE_OBJ_P2(createBufferWhenInit, initBlockNumPerBuffer, ParentClass1, ParentClass2, ObjType)\
__OBJ_POOL_CREATE_OBJ(createBufferWhenInit, initBlockNumPerBuffer, ObjType)\
__OBJ_POOL_CREATE_GEN_BEGIN(ObjType)\
__OBJ_POOL_CREATE_GEN_ADD_PARANT(ParentClass1)\
__OBJ_POOL_CREATE_GEN_ADD_PARANT(ParentClass2)\
__OBJ_POOL_CREATE_GEN_END()\
__OBJ_POOL_CREATE_THREAD_LOCAL_BEGIN(ObjType)\
__OBJ_POOL_CREATE_THREAD_LOCAL_ADD_PARANT(ParentClass1)\
__OBJ_POOL_CREATE_THREAD_LOCAL_ADD_PARANT(ParentClass2)\
__OBJ_POOL_CREATE_THREAD_LOCAL_END()

// 三继承
#undef POOL_CREATE_OBJ_P3
#define POOL_CREATE_OBJ_P3(createBufferWhenInit, initBlockNumPerBuffer, ParentClass1, ParentClass2, ParentClass3, ObjType)\
__OBJ_POOL_CREATE_OBJ(createBufferWhenInit, initBlockNumPerBuffer, ObjType)\
__OBJ_POOL_CREATE_GEN_BEGIN(ObjType)\
__OBJ_POOL_CREATE_GEN_ADD_PARANT(ParentClass1)\
__OBJ_POOL_CREATE_GEN_ADD_PARANT(ParentClass2)\
__OBJ_POOL_CREATE_GEN_ADD_PARANT(ParentClass3)\
__OBJ_POOL_CREATE_GEN_END()\
__OBJ_POOL_CREATE_THREAD_LOCAL_BEGIN(ObjType)\
__OBJ_POOL_CREATE_THREAD_LOCAL_ADD_PARANT(ParentClass1)\
__OBJ_POOL_CREATE_THREAD_LOCAL_ADD_PARANT(ParentClass2)\
__OBJ_POOL_CREATE_THREAD_LOCAL_ADD_PARANT(ParentClass3)\
__OBJ_POOL_CREATE_THREAD_LOCAL_END()

// 四继承
#undef POOL_CREATE_OBJ_P4
#define POOL_CREATE_OBJ_P4(createBufferWhenInit, initBlockNumPerBuffer, ParentClass1, ParentClass2, ParentClass3, ParentClass4, ObjType)\
__OBJ_POOL_CREATE_OBJ(createBufferWhenInit, initBlockNumPerBuffer, ObjType)\
__OBJ_POOL_CREATE_GEN_BEGIN(ObjType)\
__OBJ_POOL_CREATE_GEN_ADD_PARANT(ParentClass1)\
__OBJ_POOL_CREATE_GEN_ADD_PARANT(ParentClass2)\
__OBJ_POOL_CREATE_GEN_ADD_PARANT(ParentClass3)\
__OBJ_POOL_CREATE_GEN_ADD_PARANT(ParentClass4)\
__OBJ_POOL_CREATE_GEN_END()\
__OBJ_POOL_CREATE_THREAD_LOCAL_BEGIN(ObjType)\
__OBJ_POOL_CREATE_THREAD_LOCAL_ADD_PARANT(ParentClass1)\
__OBJ_POOL_CREATE_THREAD_LOCAL_ADD_PARANT(ParentClass2)\
__OBJ_POOL_CREATE_THREAD_LOCAL_ADD_PARANT(ParentClass3)\
__OBJ_POOL_CREATE_THREAD_LOCAL_ADD_PARANT(ParentClass4)\
__OBJ_POOL_CREATE_THREAD_LOCAL_END()

// 默认宏
#undef POOL_CREATE_OBJ_DEFAULT
#define POOL_CREATE_OBJ_DEFAULT(ObjType) POOL_CREATE_OBJ(true, MEMORY_BUFFER_BLOCK_INIT, ObjType)
#undef POOL_CREATE_OBJ_DEFAULT_P1
#define POOL_CREATE_OBJ_DEFAULT_P1(ParentClass, ObjType) POOL_CREATE_OBJ_P1(true, MEMORY_BUFFER_BLOCK_INIT, ParentClass, ObjType)
#undef POOL_CREATE_OBJ_DEFAULT_P2
#define POOL_CREATE_OBJ_DEFAULT_P2(ParentClass1, ParentClass2, ObjType) POOL_CREATE_OBJ_P2(true, MEMORY_BUFFER_BLOCK_INIT, ParentClass1, ParentClass2, ObjType)
#undef POOL_CREATE_OBJ_DEFAULT_P3
#define POOL_CREATE_OBJ_DEFAULT_P3(ParentClass1, ParentClass2, ParentClass3, ObjType) POOL_CREATE_OBJ_P3(true, MEMORY_BUFFER_BLOCK_INIT, ParentClass1, ParentClass2, ParentClass3, ObjType)
#undef POOL_CREATE_OBJ_DEFAULT_P4
#define POOL_CREATE_OBJ_DEFAULT_P4(ParentClass1, ParentClass2, ParentClass3, ParentClass4, ObjType) POOL_CREATE_OBJ_P4(true, MEMORY_BUFFER_BLOCK_INIT, ParentClass1, ParentClass2, ParentClass3, ParentClass4, ObjType)

// 默认宏初始化时候不创建buffer（避免内存吃尽） 业务层应该用此宏,初始化时不会立马分配一块内存区,避免内存暴涨
#undef POOL_CREATE_OBJ_DEFAULT_NO_BUFFER_INIT
#define POOL_CREATE_OBJ_DEFAULT_NO_BUFFER_INIT(ObjType) POOL_CREATE_OBJ(false, MEMORY_BUFFER_BLOCK_INIT, ObjType)
#undef POOL_CREATE_OBJ_DEFAULT_NO_BUFFER_INIT_P1
#define POOL_CREATE_OBJ_DEFAULT_NO_BUFFER_INIT_P1(ParentClass, ObjType) POOL_CREATE_OBJ_P1(false, MEMORY_BUFFER_BLOCK_INIT, ParentClass, ObjType)
#undef POOL_CREATE_OBJ_DEFAULT_NO_BUFFER_INIT_P2
#define POOL_CREATE_OBJ_DEFAULT_NO_BUFFER_INIT_P2(ParentClass1, ParentClass2, ObjType) POOL_CREATE_OBJ_P2(false, MEMORY_BUFFER_BLOCK_INIT, ParentClass1, ParentClass2, ObjType)
#undef POOL_CREATE_OBJ_DEFAULT_NO_BUFFER_INIT_P3
#define POOL_CREATE_OBJ_DEFAULT_NO_BUFFER_INIT_P3(ParentClass1, ParentClass2, ParentClass3, ObjType) POOL_CREATE_OBJ_P3(false, MEMORY_BUFFER_BLOCK_INIT, ParentClass1, ParentClass2, ParentClass3, ObjType)
#undef POOL_CREATE_OBJ_DEFAULT_NO_BUFFER_INIT_P4
#define POOL_CREATE_OBJ_DEFAULT_NO_BUFFER_INIT_P4(ParentClass1, ParentClass2, ParentClass3, ParentClass4, ObjType) POOL_CREATE_OBJ_P4(false, MEMORY_BUFFER_BLOCK_INIT, ParentClass1, ParentClass2, ParentClass3, ParentClass4, ObjType)



// // 泛型宏

// 无继承 
// param(ObjType):泛型类型,
// param(...):泛型模版参数
#undef POOL_CREATE_TEMPLATE_OBJ
#define POOL_CREATE_TEMPLATE_OBJ(createBufferWhenInit, initBlockNumPerBuffer, ObjType, ...)   \
__OBJ_POOL_CREATE_TEMPLATE_OBJ(createBufferWhenInit, initBlockNumPerBuffer, ObjType, ##__VA_ARGS__)\
__OBJ_POOL_CREATE_GEN_TEMPLATE_BEGIN(ObjType)\
__OBJ_POOL_CREATE_GEN_TEMPLATE_END()\
__OBJ_POOL_CREATE_THREAD_LOCAL_TEMPLATE_BEGIN(ObjType)\
__OBJ_POOL_CREATE_THREAD_LOCAL_TEMPLATE_END()

// 单继承
// param(ObjType):泛型类型,
// param(...):泛型模版参数
// param(ParentClass):泛型父类类型,
#undef POOL_CREATE_TEMPLATE_OBJ_P1
#define POOL_CREATE_TEMPLATE_OBJ_P1(createBufferWhenInit, initBlockNumPerBuffer, ParentClass, ObjType, ...)\
__OBJ_POOL_CREATE_TEMPLATE_OBJ(createBufferWhenInit, initBlockNumPerBuffer, ObjType, ##__VA_ARGS__)\
__OBJ_POOL_CREATE_GEN_TEMPLATE_BEGIN(ObjType)\
__OBJ_POOL_CREATE_GEN_TEMPLATE_ADD_PARANT(ParentClass)\
__OBJ_POOL_CREATE_GEN_TEMPLATE_END()\
__OBJ_POOL_CREATE_THREAD_LOCAL_TEMPLATE_BEGIN(ObjType)\
__OBJ_POOL_CREATE_THREAD_LOCAL_TEMPLATE_ADD_PARANT(ParentClass)\
__OBJ_POOL_CREATE_THREAD_LOCAL_TEMPLATE_END()

// 双继承
#undef POOL_CREATE_TEMPLATE_OBJ_P2
#define POOL_CREATE_TEMPLATE_OBJ_P2(createBufferWhenInit, initBlockNumPerBuffer, ParentClass1, ParentClass2, ObjType, ...)\
__OBJ_POOL_CREATE_TEMPLATE_OBJ(createBufferWhenInit, initBlockNumPerBuffer, ObjType, ##__VA_ARGS__)\
__OBJ_POOL_CREATE_GEN_TEMPLATE_BEGIN(ObjType)\
__OBJ_POOL_CREATE_GEN_TEMPLATE_ADD_PARANT(ParentClass1)\
__OBJ_POOL_CREATE_GEN_TEMPLATE_ADD_PARANT(ParentClass2)\
__OBJ_POOL_CREATE_GEN_TEMPLATE_END()\
__OBJ_POOL_CREATE_THREAD_LOCAL_TEMPLATE_BEGIN(ObjType)\
__OBJ_POOL_CREATE_THREAD_LOCAL_TEMPLATE_ADD_PARANT(ParentClass1)\
__OBJ_POOL_CREATE_THREAD_LOCAL_TEMPLATE_ADD_PARANT(ParentClass2)\
__OBJ_POOL_CREATE_THREAD_LOCAL_TEMPLATE_END()

// 三继承
#undef POOL_CREATE_TEMPLATE_OBJ_P3
#define POOL_CREATE_TEMPLATE_OBJ_P3(createBufferWhenInit, initBlockNumPerBuffer, ParentClass1, ParentClass2, ParentClass3, ObjType, ...)\
__OBJ_POOL_CREATE_TEMPLATE_OBJ(createBufferWhenInit, initBlockNumPerBuffer, ObjType, ##__VA_ARGS__)\
__OBJ_POOL_CREATE_GEN_TEMPLATE_BEGIN(ObjType)\
__OBJ_POOL_CREATE_GEN_TEMPLATE_ADD_PARANT(ParentClass1)\
__OBJ_POOL_CREATE_GEN_TEMPLATE_ADD_PARANT(ParentClass2)\
__OBJ_POOL_CREATE_GEN_TEMPLATE_ADD_PARANT(ParentClass3)\
__OBJ_POOL_CREATE_GEN_TEMPLATE_END()\
__OBJ_POOL_CREATE_THREAD_LOCAL_TEMPLATE_BEGIN(ObjType)\
__OBJ_POOL_CREATE_THREAD_LOCAL_TEMPLATE_ADD_PARANT(ParentClass1)\
__OBJ_POOL_CREATE_THREAD_LOCAL_TEMPLATE_ADD_PARANT(ParentClass2)\
__OBJ_POOL_CREATE_THREAD_LOCAL_TEMPLATE_ADD_PARANT(ParentClass3)\
__OBJ_POOL_CREATE_THREAD_LOCAL_TEMPLATE_END()

// 四继承
#undef POOL_CREATE_TEMPLATE_OBJ_P4
#define POOL_CREATE_TEMPLATE_OBJ_P4(createBufferWhenInit, initBlockNumPerBuffer, ParentClass1, ParentClass2, ParentClass3, ParentClass4, ObjType, ...)\
__OBJ_POOL_CREATE_TEMPLATE_OBJ(createBufferWhenInit, initBlockNumPerBuffer, ObjType, ##__VA_ARGS__)\
__OBJ_POOL_CREATE_GEN_TEMPLATE_BEGIN(ObjType)\
__OBJ_POOL_CREATE_GEN_TEMPLATE_ADD_PARANT(ParentClass1)\
__OBJ_POOL_CREATE_GEN_TEMPLATE_ADD_PARANT(ParentClass2)\
__OBJ_POOL_CREATE_GEN_TEMPLATE_ADD_PARANT(ParentClass3)\
__OBJ_POOL_CREATE_GEN_TEMPLATE_ADD_PARANT(ParentClass4)\
__OBJ_POOL_CREATE_GEN_TEMPLATE_END()\
__OBJ_POOL_CREATE_THREAD_LOCAL_TEMPLATE_BEGIN(ObjType)\
__OBJ_POOL_CREATE_THREAD_LOCAL_TEMPLATE_ADD_PARANT(ParentClass1)\
__OBJ_POOL_CREATE_THREAD_LOCAL_TEMPLATE_ADD_PARANT(ParentClass2)\
__OBJ_POOL_CREATE_THREAD_LOCAL_TEMPLATE_ADD_PARANT(ParentClass3)\
__OBJ_POOL_CREATE_THREAD_LOCAL_TEMPLATE_ADD_PARANT(ParentClass4)\
__OBJ_POOL_CREATE_THREAD_LOCAL_TEMPLATE_END()

// 默认宏
#undef POOL_CREATE_TEMPLATE_OBJ_DEFAULT
#define POOL_CREATE_TEMPLATE_OBJ_DEFAULT(ObjType, ...) POOL_CREATE_TEMPLATE_OBJ(true, MEMORY_BUFFER_BLOCK_INIT, ObjType, ##__VA_ARGS__)
#undef POOL_CREATE_TEMPLATE_OBJ_DEFAULT_P1
#define POOL_CREATE_TEMPLATE_OBJ_DEFAULT_P1(ParentClass, ObjType, ...) POOL_CREATE_TEMPLATE_OBJ_P1(true, MEMORY_BUFFER_BLOCK_INIT, ParentClass, ObjType, ##__VA_ARGS__)
#undef POOL_CREATE_TEMPLATE_OBJ_DEFAULT_P2
#define POOL_CREATE_TEMPLATE_OBJ_DEFAULT_P2(ParentClass1, ParentClass2, ObjType, ...) POOL_CREATE_TEMPLATE_OBJ_P2(true, MEMORY_BUFFER_BLOCK_INIT, ParentClass1, ParentClass2, ObjType, ##__VA_ARGS__)
#undef POOL_CREATE_TEMPLATE_OBJ_DEFAULT_P3
#define POOL_CREATE_TEMPLATE_OBJ_DEFAULT_P3(ParentClass1, ParentClass2, ParentClass3, ObjType, ...) POOL_CREATE_TEMPLATE_OBJ_P3(true, MEMORY_BUFFER_BLOCK_INIT, ParentClass1, ParentClass2, ParentClass3, ObjType, ##__VA_ARGS__)
#undef POOL_CREATE_TEMPLATE_OBJ_DEFAULT_P4
#define POOL_CREATE_TEMPLATE_OBJ_DEFAULT_P4(ParentClass1, ParentClass2, ParentClass3, ParentClass4, ObjType, ...) POOL_CREATE_TEMPLATE_OBJ_P4(true, MEMORY_BUFFER_BLOCK_INIT, ParentClass1, ParentClass2, ParentClass3, ParentClass4, ObjType, ##__VA_ARGS__)

// 默认宏初始化时候不创建buffer（避免内存吃尽） 业务层应该用此宏,初始化时不会立马分配一块内存区,避免内存暴涨
#undef POOL_CREATE_TEMPLATE_OBJ_DEFAULT_NO_BUFFER_INIT
#define POOL_CREATE_TEMPLATE_OBJ_DEFAULT_NO_BUFFER_INIT(ObjType, ...) POOL_CREATE_TEMPLATE_OBJ(false, MEMORY_BUFFER_BLOCK_INIT, ObjType, ##__VA_ARGS__)
#undef POOL_CREATE_TEMPLATE_OBJ_DEFAULT_NO_BUFFER_INIT_P1
#define POOL_CREATE_TEMPLATE_OBJ_DEFAULT_NO_BUFFER_INIT_P1(ParentClass, ObjType, ...) POOL_CREATE_TEMPLATE_OBJ_P1(false, MEMORY_BUFFER_BLOCK_INIT, ParentClass, ObjType, ##__VA_ARGS__)
#undef POOL_CREATE_TEMPLATE_OBJ_DEFAULT_NO_BUFFER_INIT_P2
#define POOL_CREATE_TEMPLATE_OBJ_DEFAULT_NO_BUFFER_INIT_P2(ParentClass1, ParentClass2, ObjType, ...) POOL_CREATE_TEMPLATE_OBJ_P2(false, MEMORY_BUFFER_BLOCK_INIT, ParentClass1, ParentClass2, ObjType, ##__VA_ARGS__)
#undef POOL_CREATE_TEMPLATE_OBJ_DEFAULT_NO_BUFFER_INIT_P3
#define POOL_CREATE_TEMPLATE_OBJ_DEFAULT_NO_BUFFER_INIT_P3(ParentClass1, ParentClass2, ParentClass3, ObjType, ...) POOL_CREATE_TEMPLATE_OBJ_P3(false, MEMORY_BUFFER_BLOCK_INIT, ParentClass1, ParentClass2, ParentClass3, ObjType, ##__VA_ARGS__)
#undef POOL_CREATE_TEMPLATE_OBJ_DEFAULT_NO_BUFFER_INIT_P4
#define POOL_CREATE_TEMPLATE_OBJ_DEFAULT_NO_BUFFER_INIT_P4(ParentClass1, ParentClass2, ParentClass3, ParentClass4, ObjType, ...) POOL_CREATE_TEMPLATE_OBJ_P4(false, MEMORY_BUFFER_BLOCK_INIT, ParentClass1, ParentClass2, ParentClass3, ParentClass4, ObjType, ##__VA_ARGS__)



// // 没有构造参数的创建
#undef POOL_OBJ_NEW_BY_ADAPT
#define POOL_OBJ_NEW_BY_ADAPT(ObjType, BuildType)                                \
ObjType::NewByAdapter_##ObjType(BuildType)

#undef POOL_TEMP_OBJ_NEW_BY_ADAPT
#define POOL_TEMP_OBJ_NEW_BY_ADAPT(TempClassType, BuildType, ...)                          \
TempClassType<##__VA_ARGS__>::NewByAdapter_##TempClassType(BuildType)


#endif
