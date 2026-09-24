# AGENTS.md — CrystalNet AI 开发约束

> 本文件是所有 AI 编程助手（CodeBuddy / Cursor / Copilot / Codex / Claude Code 等）在本仓库工作时**必须无条件遵守**的约束。任何涉及 `kernel/` 目录的开发，以及在其他模块中引用 kernel 组件时，一律以本文件为准。约束均来自现有代码实证，新代码必须与存量代码风格保持一致。

## 0. 项目速览

- CrystalNet 是 C++ 跨平台（Windows + Linux）网络框架，`kernel/` 是最底层核心库，CenterServer/Gateway/service/testsuit 等上层模块均依赖它。
- kernel 目录约定（`kernel/README.md`）：头文件在 `kernel/include/`，实现在 `kernel/source/`，公共宏/类型在 `kernel/include/kernel/common/`，全部组件在 `kernel/include/kernel/comp/`。

---

## 1. 架构红线

### 1.1 目录结构

- 头文件一律放 `kernel/include/kernel/`，实现文件（.cpp/.cc/.cxx）一律放 `kernel/source/`。禁止在 include 目录新增 .cpp，禁止在 source 目录新增对外头文件。
- `kernel/include/kernel/common/`：只放基本宏定义、基本函数、基本类型定义（如 `BaseMacro.h`、`BaseType.h`、`status.h`、`func.h`）。公共宏/类型只能放 common/，禁止在组件内定义全局通用宏。
- `kernel/include/kernel/comp/`：所有组件的目录。**新增组件必须在 `comp/` 下建独立子目录**（如 `comp/MyComp/`）并提供聚合入口头文件；禁止塞进既有组件目录或在 comp/ 根目录散放。
- 现有拼写以代码库为准，引用时不得"纠正"：`comp/Pipline/`（非 Pipeline）、`MemoryAlloctor`（非 Allocator）、`comp/memory/`、`comp/thread/`、`comp/params/`（小写目录）。

### 1.2 导出宏（KERNEL_EXPORT）

- **所有需要导出给外部模块使用的类、函数、全局变量必须带 `KERNEL_EXPORT`**，机制见 `kernel/include/kernel/kernel_export.h`。
- 类导出写法：`class KERNEL_EXPORT LibTimer { ... };`；导出全局变量用 `KERNEL_EXTERN_DEFINE` 声明。
- 外部模块定义 `CRYSTAL_NET_IMPORT_KERNEL_LIB` 导入符号；静态库场景定义 `CRYSTAL_NET_STATIC_KERNEL_LIB`。
- ❌ 禁止给仅库内使用的实现类加 `KERNEL_EXPORT`；❌ 禁止绕过导出宏直接写 `_declspec(dllexport)`。

### 1.3 依赖方向

- **kernel 是最底层基础库，严禁反向依赖任何上层业务模块**：CenterServer、Gateway、client、service、service_common、testsuit、XProject、OptionComponent 等。
- kernel 只允许依赖：`3rd/` 下的第三方库、系统库、C++ 标准库。
- 上层需要 kernel 不具备的能力时：能力下沉为 kernel 通用组件，或在业务层实现，**禁止在 kernel 内 #include 业务模块头文件**。

### 1.4 命名空间（红线）

宏定义见 `kernel/include/kernel/common/BaseMacro.h`，实际命名空间为 `::CRYSTAL_NET::kernel`。

- **kernel 内所有类、函数、枚举的定义必须包裹在 `KERNEL_BEGIN` / `KERNEL_END` 中**（头文件和源文件都要）。
- **在命名空间之外使用 kernel 组件时，必须显式带 `KERNEL_NS::` 前缀**，如 `KERNEL_NS::LibTimer`、`KERNEL_NS::SmartPtr<KERNEL_NS::LibThread>`。
- ❌ 禁止 `using namespace KERNEL_NS;` / `using namespace CRYSTAL_NET::kernel;`
- ❌ 禁止在 kernel 头文件中用裸 `namespace xxx { }` 替代宏。

```cpp
// ✅ kernel 头文件中
KERNEL_BEGIN

class KERNEL_EXPORT MyComp : public CompObject
{
};

KERNEL_END

// ✅ 外部模块使用
KERNEL_NS::LibTimer *timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();

// ❌ 裸写无命名空间
LibTimer *timer = ...;

// ❌ using namespace
using namespace KERNEL_NS;
```

### 1.5 日志（红线）

- **日志输出只允许使用 `CLOG_XXX` 系列宏**，定义于 `kernel/include/kernel/comp/Log/LogMacro.h`，包含聚合头文件 `<kernel/comp/Log/log.h>` 即可使用。
- ❌ 禁止使用 `printf`、`std::cout`、`std::cerr`、`OutputDebugString`、`CRYSTAL_TRACE` 输出业务日志。
- 类成员函数内用 `CLOG_INFO(fmt, ...)`（自动以 this 类名做 tag）；静态/全局函数用 `CLOG_INFO_GLOBAL(ClassName, fmt, ...)`。

```cpp
// ✅
CLOG_ERROR("on host init fail:%s, st:%d", GetObjName().c_str(), st);
CLOG_INFO_GLOBAL(ConfigExporter, "init finish.");

// ❌
printf("init fail %d\n", st);
std::cout << "init fail" << std::endl;
```

### 1.6 include 路径

- kernel 库内部 include **统一使用尖括号全路径**：`#include <kernel/comp/Timer/Timer.h>`。
- ❌ 禁止相对路径（`#include "../Timer/Timer.h"`）。
- ⚠️ include 路径**大小写敏感**（Linux 构建）：目录是 `Log/`、`memory/`、`thread/`，写错大小写会导致 Linux 编译失败。

### 1.7 跨平台

- kernel 必须保持 **Windows + Linux 跨平台**：平台相关代码用平台宏隔离（`_WIN32` / `CRYSTAL_TARGET_PLATFORM_WINDOWS`），禁止在公共路径直接调用单平台 API。
- 新增第三方依赖必须放 `3rd/` 目录并提供 Windows(.lib) 与 Linux(.a) 双平台产物。

---

## 2. 编码规范

### 2.1 文件头模板

每个头文件/源文件必须包含统一 MIT License 注释块（含 Date/Author/Description 字段，与现有文件一致），头文件**同时使用 include guard 与 `#pragma once`**：

```cpp
/*!
 * MIT License
 * ...（与现有文件一致的完整许可证文本）
 *
 * Date: 2026-09-24 10:00:00
 * Author: xxx
 * Description: 一句话说明本文件用途
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MYCOMP_MY_COMP_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MYCOMP_MY_COMP_H__

#pragma once

#include <kernel/kernel_export.h>

KERNEL_BEGIN

class KERNEL_EXPORT MyComp
{
};

KERNEL_END

#endif
```

- include guard 命名：`__CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_<路径大写_下划线分隔>_H__`，与文件相对 `include/` 的路径一一对应。
- ❌ 禁止只写 `#pragma once` 不写 guard，或反之。

### 2.2 命名约定

| 对象 | 约定 | 示例 |
|------|------|------|
| 基础库类 | `Lib` 前缀，PascalCase | `LibString`、`LibTimer`、`LibThread`、`LibLog` |
| 接口类 | `I` 前缀 | `IObject`、`ILog`、`ITask`、`IPollerMgr` |
| 文件名 | 与主类同名、大写开头 | `SpinLock.h` ↔ `class SpinLock` |
| 成员变量 | `_` 前缀 | `_ptr`、`_ref`、`_errCode` |
| 私有/保护方法 | `_` 前缀 | `_OnCreated()`、`_InitMemRange()` |
| 宏 | `KERNEL_` / `CRYSTAL_` 前缀（对象池宏除外） | `KERNEL_EXPORT`、`CRYSTAL_DELETE`、`POOL_CREATE_OBJ_DEFAULT` |
| 枚举 | PascalCase，`: Int32` 指定底层类型 | `enum StatusEnum : Int32` |

### 2.3 错误码

- **函数通过 `Int32` 返回值报告错误**，返回值为 `kernel/include/kernel/common/status.h` 中 `Status::StatusEnum` 的常量（`Status::Success = 0` 成功，其余为错误码）。
- 接口注释注明：`// 返回: Status::Success成功, 其他为错误码`。
- 新增错误码：在 `status.h` 对应数值段内追加（通用段 `[-1, 499]`，框架层截止 `FrameStatusEnd = 65535`），禁止与现有值冲突。
- ❌ **业务/组件/网络层禁止抛 C++ 异常**（存量 throw 仅限 Lua 绑定、内存分配器、协程等基础设施内部），错误一律走 Status 码 + `IObject::_errCode`。

```cpp
// ✅
Int32 MyComp::Connect()
{
    if(!_inited)
        return Status::NotInit;
    return Status::Success;
}

// ❌
throw std::runtime_error("not init");
```

### 2.4 日志宏用法细则（CLOG_XXX）

所有宏为 **printf 风格 fmt + 可变参数**，自带级别开关判断：

| 场景 | 宏 | 示例 |
|------|----|------|
| 类成员函数内（自动以 this 类名做 tag） | `CLOG_DEBUG/INFO/WARN/ERROR(fmt, ...)` | `CLOG_ERROR("created fail st:%d", st);` |
| 静态/全局函数内（首参显式给 tag 类名） | `CLOG_XXX_GLOBAL(ClassName, fmt, ...)` | `CLOG_ERROR_GLOBAL(ConfigExporter, "init fail err:%d", err);` |
| 致命错误 | `CLOG_CRASH(fmt, ...)` | 进程无法继续时 |
| 网络收发类 | `CLOG_NET_DEBUG/INFO/WARN/ERROR/TRACE` | 网络层专用 |
| 系统/监控/链路 | `CLOG_SYS` / `CLOG_MONITOR` / `CLOG_TRACE` | 按场景选用 |
| SQL 相关 | `CLOG_FAIL_SQL` / `CLOG_DUMP_SQL` | 存储层 |

- 字符串参数统一用 `LibString::c_str()` 取出后传 `%s`。

### 2.5 既有设施优先复用

**禁止重复造轮子**（组件详单见第 5 节索引）：

| 需求 | 使用 | 禁止使用 |
|------|------|----------|
| 字符串 | `LibString`（`<kernel/comp/LibString.h>`） | 接口/成员中直接用 `std::string` |
| 智能指针 | `SmartPtr<T>`（单线程）、`AutoDel`（RAII 守卫） | 随意引入 `std::shared_ptr` 跨线程语义 |
| 单例 | `Singleton<T>::GetInstance()` | 手写全局静态对象 |
| 委托/回调 | `IDelegate` / `LibDelegate` / `Delegate` | 裸函数指针散播 |
| 拷贝控制 | `DISABLE_COPY_ASSIGN_MOVE` 宏或 `= delete` | 默认拷贝语义泄漏 |
| 释放 | `CRYSTAL_DELETE` / `CRYSTAL_DELETE_SAFE` / `CRYSTAL_RELEASE_SAFE` | 裸 `delete` 后不置空 |
| 分支预测 | `LIKELY(x)` / `UNLIKELY(x)` | `__builtin_expect` 直写 |
| 强制/禁用内联 | `ALWAYS_INLINE` / `FORBID_INLINE` | 平台相关 attribute 直写 |
| 内存分配 | `KERNEL_ALLOC_MEMORY_TL` / 对象池（见第 3 节） | 高频路径裸 `new/malloc` |

---

## 3. 内存管理

涉及内存分配、对象创建/销毁、智能指针、内存池开发时必读。内存池设计细节（GC 机制、中央收集器）见 `kernel/include/kernel/comp/memory/ReadMe.md`。

### 3.1 分配方式选择（按优先级）

| 场景 | 使用 | 定义位置 |
|------|------|----------|
| 高频/线程内小块内存 | `KERNEL_ALLOC_MEMORY_TL(sz)` / `KERNEL_FREE_MEMORY_TL(ptr)`（线程本地池） | `kernel/common/func.h` |
| 跨线程共享内存 | `KERNEL_ALLOC_MEMORY_MT(sz)` / `KERNEL_FREE_MEMORY_MT(ptr)`（全局池） | `kernel/common/func.h` |
| 池化对象（类声明内） | `POOL_CREATE_OBJ_DEFAULT(ObjType)` / `POOL_CREATE_OBJ_P1~P4(...)` | `kernel/comp/memory/ObjPoolMacro.h` |
| 池化对象创建/销毁 | `OBJ_POOL_NEW(ObjType, BuildType, ...)` / `OBJ_POOL_DEL(ObjType, BuildType, ptr)` | `kernel/comp/memory/MemoryAssist.h` |
| 普通堆对象 | `CRYSTAL_NEW(type)` / `CRYSTAL_DELETE(ptr)` / `CRYSTAL_DELETE_SAFE(ptr)` | `kernel/common/macro.h` |

```cpp
// ✅ 类声明池化（参考 kernel/comp/Timer/LibTimer.h）
class KERNEL_EXPORT LibTimer
{
    POOL_CREATE_OBJ_DEFAULT(LibTimer);
};

// ✅ 线程本地内存，分配/释放宏必须配套
auto *buf = reinterpret_cast<char *>(KERNEL_ALLOC_MEMORY_TL(1024));
KERNEL_FREE_MEMORY_TL(buf);

// ✅ 池化组件工厂方法，创建/销毁配对同一线程语义版本
KERNEL_NS::LibTimer *timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(timer);

// ❌ 高频路径裸分配
char *buf = new char[1024];
```

### 3.2 SmartPtr 使用限制（重要陷阱）

定义于 `kernel/include/kernel/comp/SmartPtr.h`：

1. **引用计数线程不安全**：`SmartPtr<T>` 只能在单线程内使用，禁止跨线程传递/并发操作。
2. **循环依赖死循环陷阱**：SmartPtr 的引用计数本身用内存池分配（`KERNEL_ALLOC_MEMORY_TL`），**用 SmartPtr 包装内存池/对象池对象会导致创建依赖内存池、取池对象又创建 SmartPtr 的死循环**，必须避免。
3. `pop()` 弹出所有权：仅当引用计数为 1 时才能弹出成功，否则返回 `NULL` 并打日志。
4. `AutoDel`（闭包守卫）不可拷贝/赋值/转移，要转移必须 `pop()`。

### 3.3 内存池 GC 与性能要点

- 内存池由 `MemoryBuffer`（内存块）→ `MemoryAlloctor`（分配器）→ `MemoryPool` 三层组成；buffer 用光后触发 `NewBuffer`。
- **避免频繁触发 NewBuffer**：预判大量分配需求时预先创建若干 `MemoryBuffer`（不触发 NewBuffer 时分配性能约为系统的 5 倍）。
- buffer GC：使用计数归零后按 flag 移位周期回收，越大的 block 生命周期越短；不希望被 GC 可在分配时 `flag |= 1`。
- 跨线程释放：block 由非创建线程释放时走**中央收集器**定时归并；线程退出时若仍有 block 被持有会死等，超 5 分钟打印 `CRYSTAL_TRACE` 告警——**严禁在线程退出前泄漏该线程分配的内存块**。

### 3.4 常见错误

- ❌ 混用分配/释放宏（`KERNEL_ALLOC_MEMORY_TL` 配 `delete`）。
- ❌ 跨线程使用 `SmartPtr`。
- ❌ `SmartPtr` 包装 `MemoryPool`/`MemoryAlloctor`/`ObjPool` 对象。
- ❌ 用 `std::shared_ptr`/`std::unique_ptr` 管理池化对象。
- ❌ 线程退出前未释放持有的 TL 内存块。

---

## 4. 组件（ECS）开发

新增组件、继承组件基类、开发宿主对象时必读。完整设计文档见 `kernel/include/kernel/comp/CompObject/README.md`。

### 4.1 四个核心对象

| 对象 | 角色 | 头文件 |
|------|------|--------|
| `IObject` | 所有组成 ECS 系统对象的基类（id、生命周期标志、`_errCode`） | `kernel/comp/CompObject/IObject.h` |
| `CompObject` | 组件对象基类（所有接口可缺省，需要时重写） | `kernel/comp/CompObject/CompObject.h` |
| `CompHostObject` | 宿主组件对象（持有并管理一组组件） | `kernel/comp/CompObject/CompHostObject.h` |
| `CompFactory` | 对外提供的组件工厂 | 同目录 |

### 4.2 生命周期

`IObject` 维护原子标志：`_isCreated` → `_isInited` → `_isStarted` →（运行）→ `_isWillClose` → `_isClose`，就绪标志 `_isReady`。对应可重写钩子（返回 `Int32`，默认 `Status::Success`）：`_OnCreated()` / `_OnInit()` / `_OnStart()` 及关闭阶段钩子。

- **必须实现 `virtual void Release() = 0;`**。
- 钩子返回非 `Status::Success` 会中断流程并记录到 `_errCode`；失败时必须同时打 `CLOG_ERROR`。
- **`DefaultMaskReady(bool)` 默认在 start/WillClose 时调用**：内部有线程的组件**必须重写该接口**，改为线程真正就绪后再标记 ready。

### 4.3 开发新组件（继承 CompObject）

```cpp
// ✅ kernel/include/kernel/comp/MyComp/MyComp.h
KERNEL_BEGIN

class KERNEL_EXPORT MyComp : public CompObject
{
    POOL_CREATE_OBJ_DEFAULT(MyComp);

public:
    virtual void Release() override;

protected:
    Int32 _OnInit() override;      // 需要时才重写
    Int32 _OnStart() override;
};

KERNEL_END
```

- 错误通过返回值（`Status::` 码）上报；❌ 禁止抛异常。
- 高频对象用 `POOL_CREATE_OBJ_DEFAULT` 池化（见第 3 节）。

### 4.4 开发宿主对象（继承 CompHostObject）

**两个纯虚接口必须重写**：

```cpp
virtual void OnRegisterComps() = 0;   // 注册本宿主持有的组件
virtual Int32 _OnHostInit() = 0;      // 创建组件之前对宿主自身初始化
```

- 通过 `GetComp<ICompXxx>()` **按接口类**获取组件（依赖接口而非实现）：

```cpp
// ✅
auto *log = GetComp<ILog>();

// ❌ 依赖具体实现类
auto *log = GetComp<LibLog>();
```

- ⚠️ **宿主循环依赖死循环**：HostC 持有 HostA 且 HostA 持有 HostC 会导致注册死循环，内部仅有运行时检测，**设计上必须避免**，宿主间依赖保持单向。
- 禁止直接访问宿主内部的 `_comps` 等容器，统一走 `GetComp<接口类>()`。

### 4.5 常见错误

- ❌ 继承 `CompObject` 不实现 `Release()`。
- ❌ 继承 `CompHostObject` 不重写 `OnRegisterComps()` / `_OnHostInit()`。
- ❌ 内部有线程的组件不重写 `DefaultMaskReady`。
- ❌ 生命周期钩子里抛异常或不返回 Status 码。
- ❌ 宿主间相互持有造成循环注册。

---

## 5. 现有组件索引（开发前先查，禁止重复实现已有能力）

路径前缀均为 `kernel/include/kernel/comp/`；框架总入口为 `comp/comp.h`。

### 框架与应用层

| 组件 | 用途 | 入口头文件 |
|------|------|-----------|
| App | 应用框架基类（IApplication 生命周期、启动选项） | `App/app.h`、`App/IApplication.h` |
| Service | 服务代理框架（IServiceProxy 及工厂） | `Service/Service.h`、`Service/IServiceProxy.h` |
| CompObject | 组件对象模型（框架核心） | `CompObject/CompObject.h`（README: `CompObject/README.md`） |
| ShareLibraryLoader | 动态库（dll/so）加载器 | `ShareLibraryLoader/ShareLibraryLoader.h` |
| Config | 内核配置（KernelConfig 全局配置项） | `Config/Config.h`、`Config/KernelConfig.h` |
| params | 启动参数解析/参数处理器 | `params/params.h` |
| Lua | Lua 脚本绑定与类型注册 | `Lua/Lua.h`、`Lua/KernelLua.h`（README: `Lua/README.md`） |

### 基础容器、字符串与时间

| 组件 | 用途 | 入口头文件 |
|------|------|-----------|
| Variant | 万能类型（算术运算、类型 traits） | `Variant/Variant.h` |
| Endian | 大小端字节序转换 | `Endian/LibEndian.h` |
| Utils | 工具集：字符串/哈希/容器/数学/GUID/IP/Socket/信号/堆栈回溯/Yaml 等 | `Utils/Utils.h`、`Utils/StringUtil.h` |
| Delegate | 委托/回调封装 | `Delegate/LibDelegate.h`、`Delegate/IDelegate.h` |

### 并发、协程与任务

| 组件 | 用途 | 入口头文件 |
|------|------|-----------|
| Lock | 锁体系：自旋锁、互斥锁、进程锁、锁适配器 | `Lock/Lock.h`、`Lock/Impl/SpinLock.h` |
| Coroutines | C++20 协程框架：CoTask、等待器、调度、Gather 并发 | `Coroutines/Coroutines.h`、`Coroutines/CoTask.h`（README: `Coroutines/README.md`） |
| ConcurrentPriorityQueue | 无锁并发队列：MPMC/SPSC 及优先级队列 | `ConcurrentPriorityQueue/ConcurrentPriorityQueue.h`（README 同目录） |
| thread | 线程、线程池、事件循环线程 | `thread/thread.h`、`thread/LibThreadPool.h`、`thread/LibThread.h` |
| Task | 任务抽象（ITask、委托任务、带参任务） | `Task/Task.h`、`Task/ITask.h` |
| MessageQueue | 消息队列与消息块 | `MessageQueue/MessageQueue.h` |
| Tls | 线程本地存储：TLS 智能指针、对象池、类型系统 | `Tls/Tls.h`、`Tls/TlsPtr.h` |
| Poller | 轮询器/事件分发组件 | `Poller/Poller.h`（README: `Poller/README.md`） |

### 定时与时间轮

| 组件 | 用途 | 入口头文件 |
|------|------|-----------|
| Timer | 定时器：时间轮、定时器管理器 | `Timer/Timer.h`、`Timer/TimerMgr.h` |
| Timing | 计时组件（接口 + 工厂实现） | `Timing/Timing.h` |

### 内存

| 组件 | 用途 | 入口头文件 |
|------|------|-----------|
| memory | 内存池/分配器：MemoryPool、MemoryAlloctor、中央收集器、GC 线程 | `memory/memory.h`、`memory/MemoryPool.h`（README: `memory/ReadMe.md`） |
| MemoryMonitor | 内存监控与统计 | `MemoryMonitor/MemoryMonitor.h` |
| Pipline | 内存/文件管道（IPipe、MemoryPipe、FilePipe） | `Pipline/pipeline.h`、`Pipline/IPipe.h` |

### 日志、事件与监控

| 组件 | 用途 | 入口头文件 |
|------|------|-----------|
| Log | 日志系统：ILog 接口、CLOG_XXX 日志宏、配置、全局日志 | `Log/log.h`、`Log/LogMacro.h` |
| Event | 事件管理器（LibEvent 事件定义） | `Event/event_inc.h`、`Event/EventManager.h` |
| FileMonitor | 文件变更监听 | `FileMonitor/FileMonitor.h`（README 同目录） |

### 网络

| 组件 | 用途 | 入口头文件 |
|------|------|-----------|
| NetEngine | 网络引擎：Socket/Epoll/IOCP 封装、LibPacket、TCP/UDP Poller、协议栈、会话管理 | `NetEngine/NetEngine_Inc.h`、`NetEngine/LibSocket.h` |

### 文件、序列化与编解码

| 组件 | 用途 | 入口头文件 |
|------|------|-----------|
| File | 文件操作、INI 文件、日志文件、控制台配置/颜色 | `File/File.h`、`File/LibFile.h`、`File/LibIniFile.h` |
| Archive | 归档文件读写 | `Archive/archive.h` |
| Coder | 编解码：Base64、Base62、URL 编码、短 ID 生成 | `Coder/coder.h`、`Coder/base64.h` |
| xml | XML 解析（tinyxml2 封装） | `xml/xml.h` |
| xlsx | xlsx 表格读写（workbook/worksheet/cell） | `xlsx/xlsx.h` |

### 安全、随机数与系统信息

| 组件 | 用途 | 入口头文件 |
|------|------|-----------|
| Encrypt | 加解密：AES、RSA、摘要、FF1 格式保留加密、Xor | `Encrypt/Encrypt.h`、`Encrypt/LibAes.h`、`Encrypt/LibRsa.h` |
| Random | 随机数：随机源、分布、Int64 随机 | `Random/Random.h`、`Random/LibRandom.h` |
| IdGenerator | ID 生成器 | `IdGenerator/IdGenerator.h` |
| BlackWhiteList | 黑/白名单匹配 | `BlackWhiteList/BlackWhiteList.h` |
| Cpu | CPU 信息、特性检测、性能计数器 | `Cpu/cpu.h`、`Cpu/LibCpuCounter.h` |

### comp/ 根目录散文件（高频基础设施）

| 文件 | 用途 |
|------|------|
| `SmartPtr.h` | 智能指针（单线程引用计数，限制见第 3.2 节） |
| `Singleton.h` | 单例模板 `Singleton<T>::GetInstance()` |
| `AutoDel.h` | 作用域自动释放（RAII 闭包守卫，不可拷贝/转移） |
| `LibString.h` / `LibBasicString.h` / `LibStringOut.h` / `LibStringYaml.h` | 字符串类（定长/栈上）、输出、Yaml 序列化 |
| `LibTime.h` / `TimeSlice.h` | 时间类与时间段及其 Yaml 支持 |
| `LibStream.h` | 字节流（读写序列化流） |
| `LibList.h` / `LibStack.h` | 侵入式链表、栈容器 |
| `BinaryArray.h` | 二进制数组及默认比较组件 |
| `BigNum.h` | 大数运算 |
| `LibDirtyHelper.h` | 脏数据标记辅助 |
| `LibTraceId.h` | 链路追踪 ID |
| `GlobalLifeCtrl.h` / `ObjLife.h` | 全局对象生命周期控制 |
| `KernelFinally.h` | 作用域结束执行的 Finally 守卫 |
| `PerformanceRecord.h` | 性能记录/耗时统计 |
| `TlsMemoryCleanerComp.h` | TLS 内存清理组件 |

---

## 6. 速查清单（每次提交前自检）

1. 头文件在 `kernel/include/`、实现在 `kernel/source/`，新组件在 `comp/` 独立子目录
2. 对外符号带 `KERNEL_EXPORT`
3. 全部代码包裹在 `KERNEL_BEGIN`/`KERNEL_END`，外部引用带 `KERNEL_NS::`
4. 日志只用 `CLOG_XXX`，无 printf/std::cout
5. 错误返回 `Status::` 码，不抛异常
6. include 用 `<kernel/...>` 尖括号全路径，大小写与磁盘一致
7. 高频分配走内存池/对象池宏，SmartPtr 不跨线程
8. 新组件实现 `Release()`，宿主重写 `OnRegisterComps()`/`_OnHostInit()`
9. 功能实现前查第 5 节索引，优先复用既有组件
10. 代码保持 Windows + Linux 双平台可编译
