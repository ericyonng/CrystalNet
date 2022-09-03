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
 * Date: 2020-12-07 01:28:10
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TASK_DELEGATE_WITH_PARAMS_TASK_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TASK_DELEGATE_WITH_PARAMS_TASK_H__

#pragma once


#include <kernel/kernel_inc.h>
#include <kernel/comp/Task/ITask.h>
#include <kernel/comp/Delegate/Delegate.h>
#include <kernel/comp/Variant/Variant.h>

KERNEL_BEGIN

template<typename ObjType>
class DelegateWithParamsTask : public ITask
{
    NO_COPY(DelegateWithParamsTask);

    POOL_CREATE_TEMPLATE_OBJ_DEFAULT_P1(ITask, DelegateWithParamsTask, ObjType);

public:
    DelegateWithParamsTask(ObjType *obj, IDelegate<void, ObjType *, Variant *> *callback, Variant *params = NULL);
    virtual ~DelegateWithParamsTask();

    virtual void Run();
    virtual void Release();
    IDelegate<void, ObjType *, Variant *> *PopCallback();

private:
    ObjType *_obj;
    IDelegate<void, ObjType *, Variant *> *_callback;
    Variant *_params;
};

template<typename ObjType>
inline DelegateWithParamsTask<ObjType>::DelegateWithParamsTask(ObjType *obj, IDelegate<void, ObjType *, Variant *> *callback, Variant *params)
    :_obj(obj)
    ,_callback(callback)
    ,_params(params)
{
    
}

template<typename ObjType>
inline DelegateWithParamsTask<ObjType>::~DelegateWithParamsTask()
{
    CRYSTAL_RELEASE_SAFE(_callback);
    if (UNLIKELY(_params))
        Variant::Delete_Variant(_params);
    _params = NULL;
}

template<typename ObjType>
inline void DelegateWithParamsTask<ObjType>::Run()
{
    _callback->Invoke(_obj, _params);
}

template<typename ObjType>
inline void DelegateWithParamsTask<ObjType>::Release()
{
    DelegateWithParamsTask<ObjType>::Delete_DelegateWithParamsTask(this);
}

template<typename ObjType>
inline IDelegate<void, ObjType *, Variant *> *DelegateWithParamsTask<ObjType>::PopCallback()
{
    auto cb = _callback;
    _callback = NULL;
    return cb;
}

KERNEL_END

#endif
