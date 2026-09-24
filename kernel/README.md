# 贡献指南

使用 AI 辅助开发时请遵守根目录 AGENTS.md 中的约束

# 核心库

* 头文件目录: kernel/include/
* cpp/cxx/.cc 等实现文件目录: kernel/source/
* kernel/kernel_export.h 是库导出的宏文件，所有要导出给外部使用的都需要带上KERNEL_EXPORT
* kernel/common/目录是基本的公共目录, 包含基本的宏定义, 基本的函数, 基本的类型定义等
* kernel/comp/ 目录是核心库所有组件的目录, 新增的组件都放这里

