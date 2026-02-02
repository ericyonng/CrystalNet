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
 * Date: 2024-11-04 12:43:13
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>

#ifdef CRYSTAL_NET_CPP20

#include <kernel/comp/Coroutines/CoTaskParam.h>

#include "kernel/comp/Timer/LibTimer.h"
#include "kernel/comp/Utils/TlsUtil.h"
#include "kernel/comp/LibTraceId.h"

KERNEL_BEGIN

CoTaskParam::~CoTaskParam()
{
    if(_timeout)
    {
        LibTimer::DeleteThreadLocal_LibTimer(_timeout);
        _timeout = NULL;
    }

    _handle = NULL;

    // 如果当前协程参数是自己, 则设置空,协程销毁的时候置空
    auto currentCoPtr = TlsUtil::GetOrCreateTargetPtr<CoCurrentTaskParam>();
    if(currentCoPtr->Get()->_coParam == this)
        CoTaskParam::SetCurrentCoParam(NULL);

    if(_trace)
    {
        LibTraceId::DeleteThreadLocal_LibTraceId(_trace);
        _trace = NULL;
    }

    if (_releaseSource)
    {
        _releaseSource->Invoke();
    }
    CRYSTAL_RELEASE_SAFE(_releaseSource);
}

void CoTaskParam::Release()
{
  CoTaskParam::DeleteThreadLocal_CoTaskParam(this);
}

CoTaskParam *CoTaskParam::GetCurrentCoParam()
{
    auto currentCoPtr = TlsUtil::GetOrCreateTargetPtr<CoCurrentTaskParam>();
    return currentCoPtr->Get()->_coParam;
}

void CoTaskParam::SetCurrentCoParam(CoTaskParam *param)
{
    auto currentCoPtr = TlsUtil::GetOrCreateTargetPtr<CoCurrentTaskParam>();
    currentCoPtr->Get()->_coParam = param;
}

void TaskParamRefWrapper::Release()
{
    TaskParamRefWrapper::DeleteThreadLocal_TaskParamRefWrapper(this);
}



KERNEL_END

#endif