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
 *              2.任务可能落在线程池的不同线程: 每个任务内RoInitialize/RoUninitialize成对调用, com对象不跨任务缓存
 *              3.toast上下文与任务共享所有权(shared_ptr), 组件销毁后线程池中残留任务安全降级
*/

#include <pch.h>
#include <OptionComp/WinToast/Impl/WinToastMgr.h>
#include <OptionComp/WinToast/Impl/WinToastMgrFactory.h>
#include <kernel/comp/Utils/RttiUtil.h>
#include <kernel/comp/Log/log.h>
#include <kernel/common/statics.h>
#include <kernel/comp/thread/LibEventLoopThreadPool.h>

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
#include <memory>
#include <wrl.h>
#include <wrl/wrappers/corewrappers.h>
#include <windows.ui.notifications.h>
#include <windows.data.xml.dom.h>
#include <shlobj.h>
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

    // com初始化guard(成对RoInitialize/RoUninitialize, 任意线程安全)
    struct ComInitGuard
    {
        ComInitGuard()
            :_hr(::RoInitialize(RO_INIT_MULTITHREADED))
        {
        }

        ~ComInitGuard()
        {
            if (SUCCEEDED(_hr))
                ::RoUninitialize();
        }

        bool IsOk() const
        {
            // S_FALSE: 当前线程已初始化过com, 同样视为成功
            return SUCCEEDED(_hr);
        }

        HRESULT _hr;
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

    // xml转义
    std::wstring XmlEscape(const std::wstring &raw)
    {
        std::wstring escaped;
        escaped.reserve(raw.size());
        for (auto ch : raw)
        {
            switch (ch)
            {
            case L'&': escaped.append(L"&amp;"); break;
            case L'<': escaped.append(L"&lt;"); break;
            case L'>': escaped.append(L"&gt;"); break;
            case L'\"': escaped.append(L"&quot;"); break;
            case L'\'': escaped.append(L"&apos;"); break;
            default: escaped.push_back(ch); break;
            }
        }

        return escaped;
    }

    // 初始化任务: 设置aumid并验证notifier可用(aumid不被系统接受时提前暴露)
    void InitToastTask(std::shared_ptr<WinToastCtx> ctx)
    {
        ComInitGuard guard;
        if (UNLIKELY(!guard.IsOk()))
        {
            CLOG_ERROR_GLOBAL(WinToastMgr, "WinToast RoInitialize fail, hr:0x%08x", static_cast<UInt32>(guard._hr));
            return;
        }

        // aumid需在使用通知api前设置(未打包程序弹toast的前提)
        ::SetCurrentProcessExplicitAppUserModelID(ctx->aumid.c_str());

        Microsoft::WRL::ComPtr<ABI::Windows::UI::Notifications::IToastNotificationManagerStatics> toastStatics;
        HRESULT hr = ::RoGetActivationFactory(
            Microsoft::WRL::Wrappers::HStringReference(RuntimeClass_Windows_UI_Notifications_ToastNotificationManager).Get(),
            IID_PPV_ARGS(&toastStatics));
        if (UNLIKELY(FAILED(hr)))
        {
            CLOG_ERROR_GLOBAL(WinToastMgr, "WinToast get ToastNotificationManager statics fail, hr:0x%08x", static_cast<UInt32>(hr));
            return;
        }

        Microsoft::WRL::ComPtr<ABI::Windows::UI::Notifications::IToastNotifier> notifier;
        hr = toastStatics->CreateToastNotifierWithId(
            Microsoft::WRL::Wrappers::HStringReference(ctx->aumid.c_str()).Get(), &notifier);
        if (UNLIKELY(FAILED(hr)))
        {
            CLOG_ERROR_GLOBAL(WinToastMgr, "WinToast create toast notifier fail, hr:0x%08x", static_cast<UInt32>(hr));
            return;
        }

        ctx->ready.store(true, std::memory_order_release);
    }

    // 弹通知任务: com对象均在本任务内创建与释放, 不跨任务缓存
    void NotifyTask(std::shared_ptr<WinToastCtx> ctx, const KERNEL_NS::LibString &content, const KERNEL_NS::LibString &title)
    {
        if (UNLIKELY(!ctx->ready.load(std::memory_order_acquire)))
        {
            CLOG_WARN_GLOBAL(WinToastMgr, "WinToast is not ready, notify dropped");
            return;
        }

        ComInitGuard guard;
        if (UNLIKELY(!guard.IsOk()))
        {
            CLOG_ERROR_GLOBAL(WinToastMgr, "WinToast RoInitialize fail, hr:0x%08x", static_cast<UInt32>(guard._hr));
            return;
        }

        // 组装toast xml(标题 + 正文), 内容需要转义
        std::wstring xml = L"<toast><visual><binding template=\"ToastText02\"><text id=\"1\">";
        xml += XmlEscape(Utf8ToWide(title));
        xml += L"</text><text id=\"2\">";
        xml += XmlEscape(Utf8ToWide(content));
        xml += L"</text></binding></visual></toast>";

        Microsoft::WRL::Wrappers::HString xmlHString;
        HRESULT hr = xmlHString.Set(xml.c_str(), static_cast<UINT32>(xml.size()));
        if (UNLIKELY(FAILED(hr)))
        {
            CLOG_ERROR_GLOBAL(WinToastMgr, "WinToast create toast xml hstring fail, hr:0x%08x", static_cast<UInt32>(hr));
            return;
        }

        Microsoft::WRL::ComPtr<ABI::Windows::UI::Notifications::IToastNotificationManagerStatics> toastStatics;
        hr = ::RoGetActivationFactory(
            Microsoft::WRL::Wrappers::HStringReference(RuntimeClass_Windows_UI_Notifications_ToastNotificationManager).Get(),
            IID_PPV_ARGS(&toastStatics));
        if (UNLIKELY(FAILED(hr)))
        {
            CLOG_ERROR_GLOBAL(WinToastMgr, "WinToast get ToastNotificationManager statics fail, hr:0x%08x", static_cast<UInt32>(hr));
            return;
        }

        Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlDocument> xmlDoc;
        hr = toastStatics->GetTemplateContent(ABI::Windows::UI::Notifications::ToastTemplateType_ToastText02, &xmlDoc);
        if (UNLIKELY(FAILED(hr)))
        {
            CLOG_ERROR_GLOBAL(WinToastMgr, "WinToast get template content fail, hr:0x%08x", static_cast<UInt32>(hr));
            return;
        }

        Microsoft::WRL::ComPtr<ABI::Windows::Data::Xml::Dom::IXmlDocumentIO> xmlDocIo;
        hr = xmlDoc.As(&xmlDocIo);
        if (UNLIKELY(FAILED(hr)))
        {
            CLOG_ERROR_GLOBAL(WinToastMgr, "WinToast query IXmlDocumentIO fail, hr:0x%08x", static_cast<UInt32>(hr));
            return;
        }

        hr = xmlDocIo->LoadXml(xmlHString.Get());
        if (UNLIKELY(FAILED(hr)))
        {
            CLOG_ERROR_GLOBAL(WinToastMgr, "WinToast load toast xml fail, hr:0x%08x", static_cast<UInt32>(hr));
            return;
        }

        Microsoft::WRL::ComPtr<ABI::Windows::UI::Notifications::IToastNotificationFactory> toastFactory;
        hr = ::RoGetActivationFactory(
            Microsoft::WRL::Wrappers::HStringReference(RuntimeClass_Windows_UI_Notifications_ToastNotification).Get(),
            IID_PPV_ARGS(&toastFactory));
        if (UNLIKELY(FAILED(hr)))
        {
            CLOG_ERROR_GLOBAL(WinToastMgr, "WinToast get ToastNotification factory fail, hr:0x%08x", static_cast<UInt32>(hr));
            return;
        }

        Microsoft::WRL::ComPtr<ABI::Windows::UI::Notifications::IToastNotification> toast;
        hr = toastFactory->CreateToastNotification(xmlDoc.Get(), &toast);
        if (UNLIKELY(FAILED(hr)))
        {
            CLOG_ERROR_GLOBAL(WinToastMgr, "WinToast create toast notification fail, hr:0x%08x", static_cast<UInt32>(hr));
            return;
        }

        Microsoft::WRL::ComPtr<ABI::Windows::UI::Notifications::IToastNotifier> notifier;
        hr = toastStatics->CreateToastNotifierWithId(
            Microsoft::WRL::Wrappers::HStringReference(ctx->aumid.c_str()).Get(), &notifier);
        if (UNLIKELY(FAILED(hr)))
        {
            CLOG_ERROR_GLOBAL(WinToastMgr, "WinToast create toast notifier fail, hr:0x%08x", static_cast<UInt32>(hr));
            return;
        }

        hr = notifier->Show(toast.Get());
        if (UNLIKELY(FAILED(hr)))
            CLOG_ERROR_GLOBAL(WinToastMgr, "WinToast show toast fail, hr:0x%08x", static_cast<UInt32>(hr));
    }
}
#endif

WinToastMgr::WinToastMgr()
    :IWinToastMgr(RttiUtil::GetTypeId<WinToastMgr>())
    , _defaultTitle("CrystalNet")
    , _aumid("CrystalNet.WinToast")
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
    auto ctx = *static_cast<std::shared_ptr<WinToastCtx> *>(_toastCtx);
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
    auto ctxHolder = new std::shared_ptr<WinToastCtx>(std::make_shared<WinToastCtx>());
    (*ctxHolder)->aumid = Utf8ToWide(_aumid);
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

    // 初始化任务投递到线程池(设置aumid并验证notifier可用)
    auto ctx = *static_cast<std::shared_ptr<WinToastCtx> *>(_toastCtx);
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

    // ready置否, 线程池中未执行的notify任务快速丢弃(com对象不跨任务缓存, 无需等待残留任务)
    if (_toastCtx)
        (*static_cast<std::shared_ptr<WinToastCtx> *>(_toastCtx))->ready.store(false, std::memory_order_release);
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
    if (_toastCtx)
    {
        delete static_cast<std::shared_ptr<WinToastCtx> *>(_toastCtx);
        _toastCtx = NULL;
    }
#endif
}

KERNEL_END
