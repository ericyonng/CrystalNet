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

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TASK_DELEGATE_TASK_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TASK_DELEGATE_TASK_H__

#pragma once

#include <kernel/comp/Task/ITask.h>
#include <kernel/comp/Delegate/IDelegate.h>
#include <kernel/common/macro.h>

KERNEL_BEGIN

template<typename ObjType>
class DelegateTask : public ITask
{
    NO_COPY(DelegateTask);

    POOL_CREATE_TEMPLATE_OBJ_DEFAULT_P1(ITask, DelegateTask, ObjType);

public:
    DelegateTask(ObjType *obj, IDelegate<void, ObjType *> *callback);
    virtual ~DelegateTask();

    virtual void Run();
    virtual void Release();
    IDelegate<void, ObjType *> *PopCallback();

private:
    ObjType *_obj;
    IDelegate<void, ObjType *> *_callback;
};

template<typename ObjType>
inline DelegateTask<ObjType>::DelegateTask(ObjType *obj, IDelegate<void, ObjType *> *callback)
    :_obj(obj)
    ,_callback(callback)
{
    
}

template<typename ObjType>
inline DelegateTask<ObjType>::~DelegateTask()
{
    CRYSTAL_RELEASE_SAFE(_callback);
}

template<typename ObjType>
inline void DelegateTask<ObjType>::Run()
{
    _callback->Invoke(_obj);
}

template<typename ObjType>
inline void DelegateTask<ObjType>::Release()
{
    DelegateTask<ObjType>::Delete_DelegateTask(this);
    // CRYSTAL_DELETE(this);
}

template<typename ObjType>
inline IDelegate<void, ObjType *> *DelegateTask<ObjType>::PopCallback()
{
    auto cb = _callback;
    _callback = NULL;
    return cb;
}

KERNEL_END

#endif