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
 * Date: 2022-03-11 12:56:42
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Log/log.h>
#include <kernel/comp/Utils/RttiUtil.h>
#include <kernel/comp/CompObject/IObject.h>
#include <kernel/comp/Utils/SystemUtil.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(IObject);

IObject::IObject()
    :_id(IObject::NewId())
    ,_entityId(0)
    ,_type(0)
    ,_kernelObjType(KernelObjectType::UNKNOWN)
    ,_isCreated{false}
    ,_isInited{false}
    ,_isStarted{false}
    ,_isReady{false}
    ,_owner(NULL)
    ,_maxFocusTypeEnumEnd(ObjFocusInterfaceFlag::END)
    ,_errCode{Status::Success}
{
}

IObject::~IObject()
{
    IObject::_Clear();
}

Int32 IObject::OnCreated()
{
    if(UNLIKELY(_isCreated.exchange(true)))
        return Status::Success;

    _objName = RttiUtil::GetByObj(this);
    if(UNLIKELY(GetType() == 0))
        g_Log->Debug(LOGFMT_OBJ_TAG("have no type this comp comp obj name:%s"), GetObjName().c_str());

    auto st = _OnCreated();
    if(UNLIKELY(st != Status::Success))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("created fail st:%d"), st);
        return st;
    }

    return Status::Success;
}

Int32 IObject::Init()
{
    if(UNLIKELY(_isInited))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("repeat init"));
        return Status::Repeat;
    }

    auto st = OnCreated();
    if(st != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("on created fail st:%d, obj name: %s"), st, GetObjName().c_str());
        return st;
    }

    st = _OnInit();
    if(st != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("init fail st:%d"), st);
        return st;
    }

    _isInited = true;
    _isStarted = false;
    _isReady = false;

    g_Log->Debug(LOGFMT_OBJ_TAG("init obj success."));
    return Status::Success;
}

Int32 IObject::Start()
{
    if(UNLIKELY(_isStarted))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("repeat start"));
        return Status::Repeat;
    }

    auto st = _OnStart();
    if(UNLIKELY(st != Status::Success))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("start fail st:%d"), st);
        return st;
    }

    DefaultMaskReady(true);
    _isStarted = true;

    return _errCode;
}

void IObject::WillClose()
{
    if(!_isStarted)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("obj not started."));
        return;
    }

    _OnWillClose();

    DefaultMaskReady(false);

    g_Log->Debug(LOGFMT_OBJ_TAG("will close object."));
}

void IObject::Close()
{
    if(!_isStarted)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("obj not started."));
        return;
    }

    _OnClose();
    _Clear();
    _isStarted = false;
    _isInited = false;
    _isCreated = false;

    g_Log->Debug(LOGFMT_OBJ_TAG("close object."));
}

void IObject::SetErrCode(const IObject *obj, Int32 errCode)
{
    Int32 oldCode = Status::Success;
    Int32 countLoop = 0;
    while(!_errCode.compare_exchange_weak(oldCode, errCode))
    {
        // 已经有错误了保留
        if(oldCode != Status::Success)
            break;

        ++countLoop;

        if(countLoop > 1000)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("set err code perhaps in dead loop countLoop:%d, oldCode:%d, errCode:%d")
                        , countLoop, oldCode, errCode);
        }
    }

    // 传递给owner
    if(_owner)
    {
        _owner->SetErrCode(this, errCode);
    }

    if(LIKELY(errCode != Status::Success))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("err happen errCode:%d, owner:%s, obj:%s")
                    , errCode, ToString().c_str(), obj?obj->ToString().c_str():"null obj");
    }
}

LibString IObject::ToString() const
{
    LibString info;
    info.AppendFormat("comp info: obj name:%s, id:%llu, type:%d, isInited:%s, isStarted:%s, isReady:%s, errCode:%d"
                    , _objName.c_str(), _id, _type, _isInited ? "true" : "false"
                    , _isStarted ? "true" : "false", _isReady ? "true" : "false", _errCode.load());

    // 接口
    info.AppendFormat("focuse interface: ");
    const Int32 maxFocosEnum = GetMaxFocusEnd();
    for(Int32 idx = ObjFocusInterfaceFlag::BEGIN; idx < maxFocosEnum;)
    {
        if(IsFocus(idx))
            info.AppendFormat("%s", TurnFocusToString(idx).c_str());

        if(++idx == maxFocosEnum)
        {
            info.AppendFormat(".");
        }
        else
        {
            info.AppendFormat(", ");
        }
    }

    return info;
}

LibString IObject::TurnFocusToString(Int32 focusEnum) const
{
    // 默认
    return ObjFocusInterfaceFlag::ToString(focusEnum);
}

void IObject::Clear()
{
    _Clear();
}

void IObject::MaskReady(bool isReady)
{
    _isReady = isReady;
}

void IObject::DefaultMaskReady(bool isReady)
{
    MaskReady(isReady);
}

UInt64 IObject::NewId()
{
    static std::atomic<UInt64> s_maxId = {0};
    return ++s_maxId;
}

void IObject::_Clear()
{
    g_Log->Debug(LOGFMT_OBJ_TAG("_Clear IObject"));
}

// void IObject::OnCheck()
// {
//     // 校验对象名提醒并设置成类型名
//     if(GetObjName().empty())
//     {
//         const LibString objName = RttiUtil::GetByObj(this);
//         g_Log->Warn(LOGFMT_OBJ_TAG("have no obj name and will set comp type name as obj name :%s")
//             , objName.c_str());
//         _SetObjName(objName);
//     }

//     // 校验类型并提醒
//     if(!GetType())
//     {
//         g_Log->Warn(LOGFMT_OBJ_TAG("have no type and it will cant use any api with type operation comp name:%s")
//                 , GetObjName().c_str());
//     }
// }

KERNEL_END
