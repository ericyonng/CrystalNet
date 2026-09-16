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
 * Date: 2026-09-15 10:00:00
 * Author: Eric Yonng
 * Description: windows toast通知组件实现(仅windows平台生效)
 *              1.Notify为异步非阻塞接口: 调用线程只做任务投递(g_EventLoopHeavyTaskThreadPool->Send), toast在线程池中弹出
 *              2.弹窗方式: 线程池任务中拼装powershell脚本(-EncodedCommand Base64传参), 启动隐藏powershell.exe进程弹toast
 *                powershell是系统已注册应用, 以其身份弹toast不受windows对未注册aumid的节流/静默
 *              3.toast上下文与任务共享所有权(shared_ptr), 组件销毁后线程池中残留任务安全降级
*/

#include <pch.h>
#include <OptionComp/WinToast/Impl/WinToastMgr.h>
#include <OptionComp/WinToast/Impl/WinToastMgrFactory.h>
#include <kernel/comp/Utils/RttiUtil.h>
#include <kernel/comp/Log/log.h>
#include <kernel/common/statics.h>
#include <kernel/comp/thread/LibEventLoopThreadPool.h>
#include <kernel/comp/Coder/base64.h>

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
#include <memory>
#include <vector>
#endif

KERNEL_BEGIN

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
namespace
{
    // toast上下文(与线程池任务共享所有权, aumid在组件init时设置后只读)
    struct WinToastCtx
    {
        std::wstring aumid;
        std::atomic_bool ready{false};
    };

    // utf8 => wide
    std::wstring Utf8ToWide(const KERNEL_NS::LibString &str)
    {
        if (str.empty())
            return std::wstring();

        const int len = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), NULL, 0);
        if (len <= 0)
            return std::wstring();

        std::wstring wide;
        wide.resize(static_cast<size_t>(len));
        ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), &wide[0], len);
        return wide;
    }

    // powershell单引号字面量转义(' => '')
    std::wstring PsEscape(const std::wstring &raw)
    {
        std::wstring escaped;
        escaped.reserve(raw.size());
        for (auto ch : raw)
        {
            if (ch == L'\'')
                escaped.append(L"''");
            else
                escaped.push_back(ch);
        }

        return escaped;
    }

    // 系统自带powershell路径(windows powershell 5.1)
    std::wstring GetPowerShellPath()
    {
        wchar_t sysDir[MAX_PATH] = {0};
        const UINT len = ::GetSystemDirectoryW(sysDir, MAX_PATH);

        std::wstring path(sysDir, len);
        path += L"\\WindowsPowerShell\\v1.0\\powershell.exe";
        return path;
    }

    // 初始化任务: 检查powershell.exe是否存在
    void InitToastTask(WinToastCtx *ctx)
    {
        const auto psPath = GetPowerShellPath();
        if (UNLIKELY(::GetFileAttributesW(psPath.c_str()) == INVALID_FILE_ATTRIBUTES))
        {
            CLOG_ERROR_GLOBAL(WinToastMgr, "WinToast cannot find powershell.exe");
            return;
        }

        ctx->ready.store(true, std::memory_order_release);
    }

    // 弹通知任务: 拼装powershell脚本, Base64编码后启动隐藏powershell.exe进程弹toast
    void NotifyTask(WinToastCtx *ctx, const KERNEL_NS::LibString &content, const KERNEL_NS::LibString &title)
    {
        if (UNLIKELY(!ctx->ready.load(std::memory_order_acquire)))
        {
            CLOG_WARN_GLOBAL(WinToastMgr, "WinToast is not ready, notify dropped");
            return;
        }

        // 截断保护命令行长度(命令行上限32767, base64与utf16有膨胀)
        const auto titleWide = PsEscape(Utf8ToWide(title).substr(0, 64));
        const auto contentWide = PsEscape(Utf8ToWide(content).substr(0, 256));

        // toast脚本: ToastText02模板(标题+正文), CreateTextNode天然处理xml转义
        std::wstring script =
            L"[Windows.UI.Notifications.ToastNotificationManager,Windows.UI.Notifications,ContentType=WindowsRuntime]|Out-Null;"
            L"$t=[Windows.UI.Notifications.ToastNotificationManager]::GetTemplateContent([Windows.UI.Notifications.ToastTemplateType]::ToastText02);"
            L"$n=$t.GetElementsByTagName('text');"
            L"$n.Item(0).AppendChild($t.CreateTextNode('";
        script += titleWide;
        script += L"'))|Out-Null;$n.Item(1).AppendChild($t.CreateTextNode('";
        script += contentWide;
        script += L"'))|Out-Null;"
            L"[Windows.UI.Notifications.ToastNotificationManager]::CreateToastNotifier('";
        script += ctx->aumid;
        script += L"').Show([Windows.UI.Notifications.ToastNotification]::new($t))";

        // -EncodedCommand要求utf16le字节的base64
        const auto encoded = KERNEL_NS::LibBase64::Encode(
            reinterpret_cast<const Byte8 *>(script.c_str()), script.size() * sizeof(wchar_t));

        // 命令行
        std::wstring cmdLine = L"\"";
        cmdLine += GetPowerShellPath();
        cmdLine += L"\" -NoProfile -NonInteractive -WindowStyle Hidden -EncodedCommand ";
        cmdLine += std::wstring(encoded.begin(), encoded.end());

        // CreateProcessW要求可写缓冲区
        std::vector<wchar_t> cmdBuf(cmdLine.begin(), cmdLine.end());
        cmdBuf.push_back(L'\0');

        STARTUPINFOW startInfo;
        ::memset(&startInfo, 0, sizeof(startInfo));
        startInfo.cb = sizeof(startInfo);
        startInfo.dwFlags = STARTF_USESHOWWINDOW;
        startInfo.wShowWindow = SW_HIDE;

        PROCESS_INFORMATION procInfo;
        ::memset(&procInfo, 0, sizeof(procInfo));

        const auto ok = ::CreateProcessW(NULL, cmdBuf.data(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &startInfo, &procInfo);
        if (UNLIKELY(!ok))
        {
            CLOG_ERROR_GLOBAL(WinToastMgr, "WinToast create powershell process fail, err:%u", static_cast<UInt32>(::GetLastError()));
            return;
        }

        // 不等待powershell退出, 句柄直接关闭, toast由powershell进程独立弹出
        ::CloseHandle(procInfo.hProcess);
        ::CloseHandle(procInfo.hThread);
    }
}
#endif

WinToastMgr::WinToastMgr()
    :IWinToastMgr(RttiUtil::GetTypeId<WinToastMgr>())
    , _defaultTitle("CrystalNet")
    // 默认借用powershell已注册的aumid(发送者显示为Windows PowerShell)
    , _aumid("{1AC14E77-02E7-4E5D-B744-2EB1AE5198B7}\\WindowsPowerShell\\v1.0\\powershell.exe")
    , _closed{false}
    , _toastCtx(NULL)
{
}

WinToastMgr::~WinToastMgr()
{
    _Clear();
}

void WinToastMgr::Release()
{
    WinToastMgr::DeleteByAdapter_WinToastMgr(WinToastMgrFactory::_buildType.V, this);
}

void WinToastMgr::SetAppUserModelId(const KERNEL_NS::LibString &aumid)
{
    _aumid = aumid;
}

void WinToastMgr::Notify(const KERNEL_NS::LibString &content) const
{
    Notify(content, _defaultTitle);
}

void WinToastMgr::Notify(const KERNEL_NS::LibString &content, const KERNEL_NS::LibString &title) const
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    if (UNLIKELY(_closed.load(std::memory_order_acquire)))
        return;

    if (UNLIKELY(!_toastCtx))
    {
        CLOG_WARN("WinToastMgr is not init, notify dropped");
        return;
    }

    if (UNLIKELY(!g_EventLoopHeavyTaskThreadPool))
    {
        CLOG_ERROR("have no EventLoopHeavyTaskThreadPool, notify dropped");
        return;
    }

    // 仅投递任务, 调用线程不阻塞, toast在线程池中弹出(捕获ctx共享所有权副本, 不捕获this)
    auto ctx = reinterpret_cast<WinToastCtx *>(_toastCtx);
    g_EventLoopHeavyTaskThreadPool->Send([ctx, content, title]()
    {
        NotifyTask(ctx, content, title);
    });
#else
    (void)content;
    (void)title;
#endif
}

Int32 WinToastMgr::_OnInit()
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    auto ctxHolder = new WinToastCtx;
    ctxHolder->aumid = Utf8ToWide(_aumid);
    _toastCtx = ctxHolder;
    _closed.store(false, std::memory_order_release);
#endif

    return Status::Success;
}

Int32 WinToastMgr::_OnStart()
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    if (UNLIKELY(!g_EventLoopHeavyTaskThreadPool))
    {
        CLOG_ERROR("have no EventLoopHeavyTaskThreadPool, WinToastMgr will not work");
        return Status::Success;
    }

    // 初始化任务投递到线程池(检查powershell.exe是否存在)
    auto ctx = reinterpret_cast<WinToastCtx *>(_toastCtx);
    g_EventLoopHeavyTaskThreadPool->Send([ctx]()
    {
        InitToastTask(ctx);
    });
#endif

    return Status::Success;
}

void WinToastMgr::_OnWillClose()
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    // 先置关闭标志, Notify直接丢弃
    _closed.store(true, std::memory_order_release);

    // ready置否, 线程池中未执行的notify任务快速丢弃
    if (_toastCtx)
        (reinterpret_cast<WinToastCtx *>(_toastCtx))->ready.store(false, std::memory_order_release);
    
#endif
}

void WinToastMgr::_OnClose()
{
}

void WinToastMgr::_Clear()
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    _closed.store(true, std::memory_order_release);

    // 仅释放本组件持有的ctx引用, 线程池中残留任务持有自己的副本, 执行时ready已为false安全降级
    CRYSTAL_DELETE_SAFE(_toastCtx);
    
#endif
}

KERNEL_END
