# WinToast 组件

windows 右下角 toast 通知组件（基于 WinRT `Windows.UI.Notifications`，WRL/ABI 方式实现，无第三方依赖）。

## 特性

* `Notify` 为**异步非阻塞**接口：调用线程只把任务投递到内核全局重任务线程池 `g_EventLoopHeavyTaskThreadPool`（`Send`）即返回，弹窗在线程池中执行，组件自身不创建任何线程
* 支持标题 + 正文两行文本（ToastText02 模板）
* 控制台程序无需打包即可使用（组件内部自动设置默认 AppUserModelId：`CrystalNet.WinToast`）
* 仅 windows 平台生效，其他平台为空实现，可安全跨平台编译
* 初始化/弹窗失败仅打日志降级，不抛异常、不影响宿主启动

## 使用

```c++
// 1. 宿主(CompHostObject派生类)中注册组件
void MyHost::OnRegisterComps()
{
    RegisterComp<WinToastMgrFactory>();
}

// 2. (可选) 组件Init前自定义AppUserModelId
auto winToast = GetComp<KERNEL_NS::IWinToastMgr>();
winToast->SetAppUserModelId("MyCompany.MyApp");

// 3. 弹通知(任意线程调用, 不阻塞)
winToast->Notify(KERNEL_NS::LibString("服务器已启动"));
winToast->Notify(KERNEL_NS::LibString("收到新订单"), KERNEL_NS::LibString("订单提醒"));
```

## 注意事项

* 依赖内核全局线程池 `g_EventLoopHeavyTaskThreadPool`，**内核初始化后必然存在**，因此组件需在内核初始化之后启动使用
* 组件未 Init/Start 或已关闭时 `Notify` 会丢弃并打日志；组件启动后初始化任务在线程池中异步执行，极短的启动窗口内 `Notify` 会丢弃（ready 未置位）
* 未打包的进程弹 toast 依赖 AppUserModelId；部分系统策略下（如未注册开始菜单快捷方式）系统可能拒绝弹窗，组件只会记录错误日志
* windows 系统设置中需开启通知（关闭勿扰/专注助手），否则系统不显示 toast
* 链接依赖 `windowsapp`（premake 脚本中已添加）
