# WinToast 组件

windows 右下角 toast 通知组件（启动隐藏 `powershell.exe` 进程弹 toast，无第三方依赖）。

## 实现原理

`Notify` 把任务投递到内核全局重任务线程池，池线程内拼装 PowerShell 脚本（ToastText02 模板，标题 + 正文），UTF-16LE 经 Base64 编码后以 `-EncodedCommand` 启动**隐藏窗口**的 `powershell.exe` 进程弹 toast。PowerShell 是系统已注册应用，以其身份弹出的 toast **不受 windows 对未注册 AUMID 的节流/静默**，发送者显示为 `Windows PowerShell`。

## 特性

* `Notify` 为**异步非阻塞**接口：调用线程只把任务投递到 `g_EventLoopHeavyTaskThreadPool`（`Send`）即返回，进程创建在池线程执行，组件自身不创建任何线程
* 支持标题 + 正文两行文本（ToastText02 模板），标题/正文超长自动截断（64/256 字符）
* 控制台程序无需打包、无需注册 AUMID 即可稳定弹窗
* 仅 windows 平台生效（依赖系统自带 `powershell.exe`，Windows PowerShell 5.1），其他平台为空实现
* 初始化/弹窗失败仅打日志降级，不抛异常、不影响宿主启动

## 使用

```c++
// 1. 宿主(CompHostObject派生类)中注册组件
void MyHost::OnRegisterComps()
{
    RegisterComp<WinToastMgrFactory>();
}

// 2. (可选) 组件Init前自定义AppUserModelId(作为powershell脚本中CreateToastNotifier的appId)
auto winToast = GetComp<KERNEL_NS::IWinToastMgr>();
winToast->SetAppUserModelId("MyCompany.MyApp");

// 3. 弹通知(任意线程调用, 不阻塞)
winToast->Notify(KERNEL_NS::LibString("服务器已启动"));
winToast->Notify(KERNEL_NS::LibString("收到新订单"), KERNEL_NS::LibString("订单提醒"));
```

## 注意事项

* 依赖内核全局线程池 `g_EventLoopHeavyTaskThreadPool`，**内核初始化后必然存在**，因此组件需在内核初始化之后启动使用
* 每次通知会启动一个 `powershell.exe` 进程（约几十~几百 ms 进程启动开销，发生在池线程），**适用低频通知场景**（告警、状态变化等），请勿在热路径高频调用
* 组件未 Init/Start 或已关闭时 `Notify` 会丢弃并打日志；启动后初始化任务（检查 powershell.exe 存在性）异步完成前的极短窗口内 `Notify` 会丢弃
* windows 系统设置中需开启通知（关闭勿扰/专注助手），否则系统不显示 toast；通知中发送者显示为 `Windows PowerShell`
