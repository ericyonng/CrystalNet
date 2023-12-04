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
 * Date: 2021-09-11 18:53:15
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/common/Buffer.h>
#include <kernel/comp/thread/ThreadTool.h>
#include <kernel/comp/Utils/TlsUtil.h>
#include <kernel/comp/Tls/Tls.h>
#include <kernel/comp/Log/log.h>
#include <kernel/comp/memory/CenterMemoryCollector.h>

KERNEL_BEGIN

void ThreadTool::OnInit(LibThread *thread, LibThreadPool *pool, UInt64 threadId, UInt64 threadGlobalId, const Byte8 *tlsMemPoolReason)
{
    // tls
    auto tlsStack = TlsUtil::GetTlsStack();
    auto defTls = tlsStack->GetDef();
    defTls->_threadId = threadId;
    defTls->_thread = thread;
    defTls->_threadPool = pool;
    defTls->_threadGlobalId = threadGlobalId;

    // 注册到内存中央收集器
    auto centerMemroyCollector = CenterMemoryCollector::GetInstance();
    centerMemroyCollector->RegisterThreadInfo(threadId, tlsStack);

    BUFFER256 reason;
    reason[0] = 0;
    sprintf(reason, "%s thread id = [%llu]", tlsMemPoolReason, defTls->_threadId);
    TlsUtil::CreateMemoryPool(reason);

    if(LIKELY(g_Log))
        g_Log->Sys(LOGFMT_NON_OBJ_TAG(ThreadTool, "thread init suc thread id:[%llu], tlsMemPoolReason:%s."), threadId, tlsMemPoolReason ? tlsMemPoolReason : "None");
}

void ThreadTool::OnDestroy()
{
    const auto currentThreadId = SystemUtil::GetCurrentThreadId();
    // 得等收集器关闭
    auto centerMemroyCollector = CenterMemoryCollector::GetInstance();
    if(centerMemroyCollector->GetWorkerThreadId() != currentThreadId)
        centerMemroyCollector->OnThreadQuit(currentThreadId);

    // tls 资源清理 主线程不释放资源, 因为全局对象可能在程序结束时候需要使用到内存释放
    if(currentThreadId != SystemUtil::GetCurProcessMainThreadId())
        TlsUtil::ClearTlsResource();

    // 释放线程局部存储资源
    if(centerMemroyCollector->GetWorkerThreadId() == currentThreadId)
        TlsUtil::DestroyTlsStack();
}

void ThreadTool::Destroy(TlsStack<TlsStackSize::SIZE_1MB> *tlsTask)
{
    // tls 资源清理
    TlsUtil::ClearTlsResource();

    // 释放线程局部存储资源
    TlsUtil::DestroyTlsStack();
}

KERNEL_END