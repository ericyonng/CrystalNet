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
 * Date: 2025-01-04 21:29:21
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/LibTraceId.h>
#include <kernel/comp/Coroutines/CoTaskParam.h>
#include <kernel/comp/Utils/TlsUtil.h>

#include "kernel/comp/IdGenerator/Impl/IdGenerator.h"

KERNEL_BEGIN


LibTraceId::LibTraceId()
{
    UpdateMain();
    UpdateSub();
}

void LibTraceId::UpdateMain()
{
    DEF_STATIC_THREAD_LOCAL_DECLEAR IdGenerator *s_IdGen = NULL;
    if(LIKELY(s_IdGen))
    {
        _mainTraceId = s_IdGen->NewId();
        return;
    }

    s_IdGen = TlsUtil::GetIdGenerator();
    if(LIKELY(s_IdGen))
    {
        _mainTraceId = s_IdGen->NewId();
        return;
    }

    _mainTraceId = (1LLU << 63) | (GetAtomicMaxId().fetch_add(1, std::memory_order_acq_rel) + 1);
}

void LibTraceId::UpdateSub()
{
    DEF_STATIC_THREAD_LOCAL_DECLEAR IdGenerator *s_IdGen = NULL;
    if(LIKELY(s_IdGen))
    {
        _subTraceId = s_IdGen->NewId();
        return;
    }

    s_IdGen = TlsUtil::GetIdGenerator();
    if(LIKELY(s_IdGen))
    {
        _subTraceId = s_IdGen->NewId();
        return;
    }

    _subTraceId = (1LLU << 63) | (GetAtomicMaxId().fetch_add(1, std::memory_order_acq_rel) + 1);
}

std::atomic<UInt64> &LibTraceId::GetAtomicMaxId()
{
    static std::atomic<UInt64> s_AtomicMaxId(0);
    return s_AtomicMaxId;
}

LibTraceId *LibTraceId::GetCurrentTrace()
{
#ifdef CRYSTAL_NET_CPP20
  if(auto currentCo = CoTaskParam::GetCurrentCoParam())
        return currentCo->_trace;
#endif
    

  auto tlsTrace = TlsUtil::GetOrCreateTargetPtr<TlsLibTrace>();
  if(UNLIKELY(!tlsTrace->_ptr->_trace))
      tlsTrace->_ptr->_trace = CRYSTAL_NEW(LibTraceId);

  return tlsTrace->_ptr->_trace;
}

TlsLibTrace::~TlsLibTrace()
{
   CRYSTAL_DELETE_SAFE(_trace);
}


KERNEL_END