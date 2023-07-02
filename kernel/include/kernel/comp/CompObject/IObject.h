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
 * Date: 2022-03-11 12:47:52
 * Author: Eric Yonng
 * Description: 
 *              // 必要执行时序:Init/Start/Close, 其他可选
 *              // OnCreate:告知组件在创建时设置参数,在init之前
 *              时序:OnCreated-Init-Start-WillClose-Close
 *              派生类需要重写的函数：Init/Start/Close/Clear/Release
 *              派生类可选重写的函数：
 *                          OnUpdate/ToString/TurnFocusToString/
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COMP_OBJECT_IOBJECT_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COMP_OBJECT_IOBJECT_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/memory.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Utils/BitUtil.h>

KERNEL_BEGIN


// 组件关注的接口 外部可以调用SetFocus扩展,关注的接口可以不仅限于框架提供的接口
class KERNEL_EXPORT ObjFocusInterfaceFlag
{
public:
    enum ENUMS
    {
        BEGIN = 1,                  // 开始
        ON_UPDATE = BEGIN,          // 组件更新
        END,                        // 结束
    };

    static const Byte8 *ToString(Int32 focusFlag)
    {
        switch (focusFlag)
        {
        case ObjFocusInterfaceFlag::ON_UPDATE: return "ON_UPDATE";
        default:
            break;
        }

        return "Unknown Interface";
    }
};

class KERNEL_EXPORT KernelObjectType
{
public:
    enum ENUMS
    {
        UNKNOWN = 0,    // 未知
        COMP = 1,       // 普通的组件
        HOST_COMP = 2,  // 宿主组件,可挂载多种组件
    };
};

class KERNEL_EXPORT IObject
{
    POOL_CREATE_OBJ_DEFAULT(IObject);

public:
    IObject();
    virtual ~IObject();
    // 必须重写
    virtual void Release() = 0;

    // obj创建成功,在 _OnInit 之前
    virtual Int32 OnCreated() final;
    // 初始化数据
    virtual Int32 Init() final;
    // 启动 依赖可以采用顺序启动来保证
    virtual Int32 Start() final;
    // 即将关闭
    virtual void WillClose() final;
    // 关闭
    virtual void Close() final;
    // 错误码
    virtual void SetErrCode(const IObject *obj, Int32 errCode) final;
    // 获取错误码
    Int32 GetErrCode() const;

    // 用于组件的刷新
    virtual void OnUpdate() {}

    // 派生类统一重载接口
protected:
    virtual Int32 _OnCreated() { return Status::Success; }
    virtual Int32 _OnInit() { return Status::Success; }
    virtual Int32 _OnStart() { return Status::Success; }
    virtual void _OnWillClose() {}
    virtual void _OnClose() {}

    // 启动完毕（比较特殊的函数,它不应该被外部直接调用（除了CompHostObject））
    virtual Int32 _Startup() = delete;

    // 属性
public:
    // 对象信息
    virtual LibString ToString() const;

    // 关注的接口字符串 可以派生扩展
    virtual LibString TurnFocusToString(Int32 focusEnum) const;   

    // 清理
    virtual void Clear();

    // 属性
public:
    UInt64 GetId() const;
    Int32 GetType() const;
    Int32 GetKernelObjType() const;
    void SetKernelObjType(Int32 type);
    UInt64 GetEntityId() const;
    void SetEntityId(UInt64 entityId);

    // 关注接口最大值
    Int32 GetMaxFocusEnd() const;
    void SetMaxFocusEnd(Int32 maxFocusEnd); // 取 maxFocusEnd 与 ObjFocusInterfaceFlag::END 最大值
    
    const LibString &GetObjName() const;
    bool IsReady() const;
    bool IsInited() const;
    bool IsStarted() const;

    virtual void MaskReady(bool isReady) final;
    // 默认在start/WillClose调用,若觉得这个时候不合适则请重写成空函数(一般组件内部有线程的都需要重写该接口,因为此时组件还没准备好只有线程也准备好了才是准备好的状态)
    virtual void DefaultMaskReady(bool isReady);

    IObject *GetOwner();
    const IObject *GetOwner() const;
    void BindOwner(IObject *owner);
    static UInt64 NewId();
    template<typename ObjType>
    ObjType *CastTo();
    template<typename ObjType>
    const ObjType *CastTo() const;

    // // 接口关注，便于宿主调用各自组件关心的接口
public:
    void SetFocus(Int32 focusEnum);
    void ClearFocus(Int32 focusEnum);
    bool IsFocus(Int32 focusEnum) const;
    

protected:
    void _SetType(Int32 type);

private:
    void _Clear();

private:
    friend class CompHostObject;

    const UInt64 _id;
    UInt64 _entityId;
    LibString _objName;
    Int32 _type;
    Int32 _kernelObjType;
    std::atomic_bool _isCreated;
    std::atomic_bool _isInited;
    std::atomic_bool _isStarted;
    std::atomic_bool _isWillClose;
    std::atomic_bool _isClose;
    std::atomic_bool _isReady;

    IObject *_owner;
    Int32 _maxFocusTypeEnumEnd;
    std::unordered_map<Int32, UInt64> _focusInterfaceBitmapFlag;
    std::atomic<Int32> _errCode;
};

ALWAYS_INLINE Int32 IObject::GetErrCode() const
{
    return _errCode;
}

ALWAYS_INLINE UInt64 IObject::GetId() const
{
    return _id;
}

ALWAYS_INLINE Int32 IObject::GetType() const
{
    return _type;
}

ALWAYS_INLINE Int32 IObject::GetKernelObjType() const
{
    return _kernelObjType;   
}

ALWAYS_INLINE void IObject::SetKernelObjType(Int32 type)
{
    _kernelObjType = type;
}

ALWAYS_INLINE UInt64 IObject::GetEntityId() const
{
    return _entityId;
}

ALWAYS_INLINE void IObject::SetEntityId(UInt64 entityId)
{
    _entityId = entityId;
}

ALWAYS_INLINE Int32 IObject::GetMaxFocusEnd() const
{
    return _maxFocusTypeEnumEnd;
}

ALWAYS_INLINE void IObject::SetMaxFocusEnd(Int32 maxFocusEnd)
{
    _maxFocusTypeEnumEnd = std::max<Int32>(maxFocusEnd, ObjFocusInterfaceFlag::END);
}

ALWAYS_INLINE const LibString &IObject::GetObjName() const
{
    return _objName;
}

ALWAYS_INLINE bool IObject::IsReady() const
{
    return _isReady;
}

ALWAYS_INLINE bool IObject::IsInited() const
{
    return _isInited;
}

ALWAYS_INLINE bool IObject::IsStarted() const
{
    return _isStarted;
}

ALWAYS_INLINE IObject *IObject::GetOwner()
{
    return _owner;
}

ALWAYS_INLINE void IObject::BindOwner(IObject *owner)
{
    _owner = owner;
}

template<typename ObjType>
ALWAYS_INLINE ObjType *IObject::CastTo()
{
    return reinterpret_cast<ObjType *>(this);
}

template<typename ObjType>
ALWAYS_INLINE const ObjType *IObject::CastTo() const
{
    return reinterpret_cast<const ObjType *>(this);
}

ALWAYS_INLINE const IObject *IObject::GetOwner() const
{
    return _owner;
}

ALWAYS_INLINE void IObject::SetFocus(Int32 focusEnum)
{
    SimpleBitmapUtil::Set(_focusInterfaceBitmapFlag, focusEnum);
}

ALWAYS_INLINE void IObject::ClearFocus(Int32 focusEnum)
{
    SimpleBitmapUtil::Clear(_focusInterfaceBitmapFlag, focusEnum);
}

ALWAYS_INLINE bool IObject::IsFocus(Int32 focusEnum) const
{
    return SimpleBitmapUtil::IsSet(_focusInterfaceBitmapFlag, focusEnum);
}

ALWAYS_INLINE void IObject::_SetType(Int32 type)
{
    _type = type;
}

KERNEL_END

#endif
