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
 * Date: 2022-03-10 21:45:09
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Log/log.h>
#include <kernel/common/statics.h>

#include <kernel/comp/CompObject/CompObject.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(CompObject);

CompObject::CompObject()
:IObject()
{
    SetKernelObjType(KernelObjectType::COMP);
}

CompObject::~CompObject()
{
    _Clear();
}

Int32 CompObject::_OnCreated()
{
    g_Log->Debug(LOGFMT_OBJ_TAG("created suc comp info obj name:%s."), GetObjName().c_str());
    return Status::Success;
}

Int32 CompObject::_OnInit()
{
    g_Log->Debug(LOGFMT_OBJ_TAG("init suc comp obj name:%s."), GetObjName().c_str());
    return Status::Success;
}

Int32 CompObject::_OnStart()
{
    g_Log->Debug(LOGFMT_OBJ_TAG("_OnStart suc comp obj name:%s."), GetObjName().c_str());
    return Status::Success;
}

void CompObject::_OnWillClose()
{
    g_Log->Debug(LOGFMT_OBJ_TAG("_OnWillClose suc comp obj name:%s."), GetObjName().c_str());
}

void CompObject::_OnClose()
{
    _Clear();
    g_Log->Debug(LOGFMT_OBJ_TAG("_OnClose suc comp obj name:%s."), GetObjName().c_str());
}

LibString CompObject::ToString() const
{
    return IObject::ToString();
}

void CompObject::OnUpdate()
{
    // g_Log->Debug(LOGFMT_OBJ_TAG("CompObject OnUpdate comp obj name:%s."), GetObjName().c_str());
}

void CompObject::Clear()
{
    _Clear();
    IObject::Clear();
    g_Log->Debug(LOGFMT_OBJ_TAG("Clear comp obj name:%s."), GetObjName().c_str());
}

void CompObject::_Clear()
{
    g_Log->Debug(LOGFMT_OBJ_TAG("_Clear comp obj name:%s."), GetObjName().c_str());
}

KERNEL_END