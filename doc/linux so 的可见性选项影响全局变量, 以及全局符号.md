The user is making an important point about Linux shared library behavior with static variables. They're saying that even with a new dlopen handle, static local variables and static global variables in the .so might be shared/persistent.

Let me think about this carefully:

On Linux, when you dlopen a .so, the static variables are allocated in the .data and .bss sections of the loaded library. Each dlopen creates a new memory mapping, so in theory, each handle would have its own copy of static variables.

But wait - the user might be talking about a specific case. If two dlopen handles refer to the SAME underlying library (same path), Linux might share the .data/.bss sections? Or there might be some copy-on-write behavior?

Actually, no. Let me reconsider. With dlopen, each call that returns a new handle creates entirely new mappings. The .data and .bss sections are per-handle.

But wait, there's a subtlety. On Linux with `RTLD_GLOBAL`, when you dlopen the same .so twice (same path), the first dlopen returns handle A, and the second dlopen returns the same handle A (ref count +1). So static variables ARE shared in that case.

But in this user's log, the handles are DIFFERENT (0x7f5d6003d910 vs 0x36881c0). So different handles mean... the second dlopen must have been from a different path, OR the first one was already dlclose'd.

Actually wait, I'm wrong. On Linux, `dlopen()` maps new copies of the library each time you call it, even with the same path. Each handle gets its own copy of static variables. That's the whole point of dlopen handles being separate.

Hmm, but actually... let me think again. The user seems knowledgeable about Linux shared libraries. They're saying static variables are shared. Maybe they're right in some specific scenario?

Actually on second thought, on Linux:
- If you dlopen the SAME exact path twice: the second call returns the SAME handle (ref count +1), so static variables ARE shared
- If the paths are different (even if they resolve to the same file): you get DIFFERENT handles with independent static variables

So in the user's case, since the log shows different handles, the paths must be different, and static variables should be independent.

But the user seems to be asserting that even with different handles, static local variables persist. Maybe there's some ELF-level sharing I'm not aware of?

Actually wait, there IS something specific about static local variables in .so files on Linux. When the linker creates position-independent code (PIC), static locals are typically accessed via the GOT (Global Offset Table). Each dlopen'd copy has its own GOT, so they SHOULD be independent.

But actually, I think the user might be confusing this with a specific case: when the .so is loaded with RTLD_GLOBAL and there are symbol conflicts. If old .so and new .so both export the same symbol, the GOT entry in the new .so might be pointing to the old .so's variable.

That's actually the crux of the issue! With RTLD_GLOBAL and default visibility, the GOT entries for global variables in the new .so could be resolved to the OLD .so's variables. This is the same PLT/symbol interposition problem, but for data symbols.

So the user is right! Even with different dlopen handles, if there are exported symbols with the same name and RTLD_GLOBAL is used, the dynamic linker will resolve GOT entries in the new .so to point to the OLD .so's variables.

This means:
- `g_PluginMgr` in the new .so might actually point to the OLD .so's `g_PluginMgr`! 
- `GetCrystalModuleId()` in the new .so might actually access the OLD .so's module ID!

But wait, actually in C++, static member functions don't go through PLT - they're direct calls within the same translation unit. And class static functions resolved via a function pointer... let me think.

Actually in PIC code, even intra-DSO calls to global functions go through PLT (unless compiled with `-fvisibility=hidden` or `-fno-semantic-interposition`). This is because on ELF platforms, global symbols can be interposed.

So the user is pointing out that the issue is even broader than just timer callbacks - it affects ALL global symbols: functions, variables, static locals, etc.

And `-fvisibility=hidden` fixes ALL of this by telling the compiler: "these symbols are local, don't go through PLT/GOT for them, and don't export them to the global symbol table."

Let me acknowledge the user's correction and provide a clearer explanation.
你说得对，我之前说得不够准确。对于 **default visibility** 的 ELF so，情况确实是：

## 静态变量也会被符号介入

```
旧 so (RTLD_GLOBAL):
    g_PluginMgr 存入 GOT → 全局符号表注册 "g_PluginMgr"

新 so (dlopen, RTLD_GLOBAL):
    编译器生成 PIC 代码:
    g_PluginMgr 访问方式: mov rax, [rip + GOT_g_PluginMgr]
    
    动态链接器解析 GOT_g_PluginMgr 时:
    → 查全局符号表 → 旧 so 的 g_PluginMgr 已注册
    → GOT 条目指向旧 so 的地址！ ❌
```

所以你说得完全对：即使是不同 dlopen 句柄、独立内存映射，**default visibility 的全局/静态数据符号的 GOT 条目也会被旧 so 抢占**。新 so 代码里读写的 `g_PluginMgr`、`GetCrystalModuleId()`、甚至函数内的 `static` 局部变量，实际上都操作的是**旧 so 的数据**。

## 但 `-fvisibility=hidden` 正好解决这一切

```
hidden 后:
    函数 → 编译为 local, 不通过 PLT, 直接调用 ✅
    数据 → 编译为 local, 不通过 GOT, 直接寻址 ✅  
    static 局部 → local linkage, 不受外部影响 ✅
    所有符号 → 不进全局符号表, 不抢占也不被抢占 ✅
```

这就是为什么 `-fvisibility=hidden` 是完整的解决方案——它不光解决函数符号冲突，**数据符号冲突也一并解决**。新 so 中所有内部符号（函数、全局变量、静态变量、thread_local）都完全独立，不与旧 so 产生任何符号冲突。



所有的程序都要默认隐藏符号, 避免污染全局符号表 （避免走 PLT/GOT）， 因为同一个符号可能有不同版本的实现，建议如果没有导出符号的需求就默认隐藏不要默认导出

  -- hidden是隐藏符号，符号默认不会导出

  filter { "system:not windows" }

​    buildoptions { "-fvisibility=hidden" }

  filter {}



hidden的话就是默认不导出，如果so中需要使用到其他模块的符号，那么不建议默认hidden而是对一些不想导出的函数hidden

链接是 加 -rdynamic, -rdynamic会把可执行文件中所有非hidden符号放入动态符号表