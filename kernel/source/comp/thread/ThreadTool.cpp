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
#include <kernel/comp/thread/ThreadTool.h>
#include <kernel/comp/Utils/TlsUtil.h>
#include <kernel/comp/Tls/Tls.h>

KERNEL_BEGIN

void ThreadTool::OnInit(LibThread *thread, LibThreadPool *pool, UInt64 threadId, const Byte8 *tlsMemPoolReason)
{
    // tls
    auto tlsStack = TlsUtil::GetTlsStack();
    auto defTls = tlsStack->GetDef();
    defTls->_threadId = threadId;
    defTls->_thread = thread;
    defTls->_threadPool = pool;

    BUFFER256 reason;
    reason[0] = 0;
    sprintf(reason, "%s thread id = [%llu]", tlsMemPoolReason, defTls->_threadId);
    TlsUtil::CreateMemoryPool(reason);
}

void ThreadTool::OnDestroy()
{
    // tls 资源清理
    TlsUtil::ClearTlsResource();

    // 释放线程局部存储资源
    TlsUtil::DestroyTlsStack();
}

KERNEL_END