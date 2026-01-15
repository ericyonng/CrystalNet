#pragma once

#include <kernel/common/common.h>
#include <atomic>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

class LibThread;
class IApplication;

KERNEL_END

struct AccountInfo
{
    KERNEL_NS::LibString ip;
    Int32 port;
    KERNEL_NS::LibString Account;
    KERNEL_NS::LibString Pwd;
};

class Entry
{
public:
    static  KERNEL_NS::LibThread *EntryThread;
    static std::atomic<KERNEL_NS::IApplication *> Application;
    static std::atomic<SERVICE_COMMON_NS::IService *> Service;
    static AccountInfo AccountInfo;
    static bool Run();
};