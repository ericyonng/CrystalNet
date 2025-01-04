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

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(LibTraceId);
POOL_CREATE_OBJ_DEFAULT_IMPL(TlsLibTrace);

LibTraceId::LibTraceId()
{
    UpdateMain();
    UpdateSub();
}

void LibTraceId::UpdateMain()
{
    // TODO 全球唯一id
    _mainTraceId = ++GetAtomicMaxId();
}

void LibTraceId::UpdateSub()
{
    // 全球唯一id
    _subTraceId = ++GetAtomicMaxId();
}

std::atomic<UInt64> &LibTraceId::GetAtomicMaxId()
{
    static std::atomic<UInt64> s_AtomicMaxId(0);
    return s_AtomicMaxId;
}

LibTraceId *LibTraceId::GetCurrentTrace()
{     
  if(auto currentCo = CoTaskParam::GetCurrentCoParam())
        return currentCo->_trace;

  auto tlsTrace = TlsUtil::GetOrCreateTargetPtr<TlsLibTrace>();
  if(!tlsTrace->_ptr->_trace)
      tlsTrace->_ptr->_trace = LibTraceId::NewThreadLocal_LibTraceId();

  return tlsTrace->_ptr->_trace;
}

TlsLibTrace::~TlsLibTrace()
{
    if(_trace)
    {
        LibTraceId::DeleteThreadLocal_LibTraceId(_trace);
        _trace = NULL;
    }
}


KERNEL_END