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
 * Date: 2022-03-10 13:46:13
 * Author: Eric Yonng
 * Description:  
 *              // 必要执行时序:Init/Start/Close, 其他可选
 *              // 建议执行时序:Init/Start/WillClose/Close
 *              派生类需要重写的函数：OnRegisterComps/_OnInit/_OnStart/_OnClose/Release
 *              派生类可选重写的函数：
 *                          Clear/ToString/
 *                          _OnCompsCreated/_OnWillStart/_OnWillClose
 *                          OnCreated/OnUpdate/ToString/TurnFocusToString
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COMP_OBJECT_COMP_HOST_OBJECT_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COMP_OBJECT_COMP_HOST_OBJECT_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/CompObject/CompObject.h>

KERNEL_BEGIN

class CompFactory;

class KERNEL_EXPORT CompHostObject : public CompObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, CompHostObject);

public:
    static std::vector<CompObject *> _emptyComps;

public:
    CompHostObject();
    virtual ~CompHostObject();

// // 基本api
protected:
    virtual Int32 _OnInit() final;
    virtual Int32 _OnStart() final;
    virtual void _OnWillClose() final;
    virtual void _OnClose() final;

    // 在组件初始化前
    virtual Int32 _OnHostInit() = 0;
    // 所有组件创建完成
    virtual Int32 _OnCompsCreated();
    // 在组件启动之前 请勿在WillStart及之前的接口启动线程，此时都是线程不安全的状态
    virtual Int32 _OnHostWillStart(){ return Status::Success; }
    // 组件启动之后 此时可以启动线程
    virtual Int32 _OnHostStart() = 0;
    // 在组件willclose之前 host若内部有多线程,应该要在组件的willclose之前结束掉线程,避免资源的竞争
    virtual void _OnHostBeforeCompsWillClose() {}
    // 在组件willclose之后
    virtual void _OnHostWillClose(){}
    // 在组件close之前
    virtual void _OnHostBeforeCompsClose() {}
    // 在组件Close之后
    virtual void _OnHostClose() = 0;
    // 在组件更新之后
    virtual void _OnHostUpdate() {}

public:
    virtual void Clear() override;
    virtual void OnUpdate() final;
    // 宿主所有组件以及自身准备就绪 注意多线程安全可能是从其他组件触发,组件可能不在一个线程
    virtual LibString ToString() const override;

    bool IsAllCompsReady(CompObject *&notReadyComp) const;
    bool IsAllCompsDown(CompObject *&notDownComp) const;


// // 功能api
public:
    // 注册组件
    virtual void OnRegisterComps() = 0;  

    // 注册
    template<typename CompFactoryType>
    Int32 RegisterComp();
    Int32 RegisterComp(CompFactory *factory);
    Int32 RegisterComp(CompObject *comp);

    // 通过objectid（除了DynamicComp）
    template<typename ObjType>
    ObjType *GetComp(UInt64 objectId);
    CompObject *GetComp(UInt64 objectId);

    // 通过运行时类型识别（除了DynamicComp）
    template<typename ObjType>
    ObjType *GetComp();
    template<typename ObjType>
    const ObjType *GetComp() const;
    CompObject *GetComp(const LibString &compRttiName);
    const CompObject *GetComp(const LibString &compRttiName) const;
    std::vector<CompObject *> &GetComps(const LibString &compRttiName);
    const std::vector<CompObject *> &GetComps(const LibString &compRttiName) const;

    // 提供给有设置了Type的对象（除了DynamicComp）
    CompObject *GetCompByType(Int32 type);
    const CompObject *GetCompByType(Int32 type) const;
    std::vector<CompObject *> &GetCompsByType(Int32 type);
    const std::vector<CompObject *> &GetCompsByType(Int32 type) const;

    // 所有组件（除了DynamicComp）
    std::vector<CompObject *> &GetAllComps();
    const std::vector<CompObject *> &GetAllComps() const;


private:
    // 在MaxFocusEnum变化时需要调用该方法
    void _ResizeFocusDict();
    Int32 _InitComps();
    Int32 _StartComps();
    void _OnWillCloseComps();
    void _CloseComps();
    void _DestroyWillRegComps();
    void _AddComp(CompObject *comp);

    void _MaskIfFocus(CompObject *comp);
    void _Clear();

    void _OnUpdateComps();

private:
    // 委托注册
    class _WillRegComp
    {
    public:
        _WillRegComp(CompFactory *factory):_comp(NULL), _factory(factory){}
        _WillRegComp(CompObject *comp):_comp(comp), _factory(NULL){}

        CompObject *_comp;
        CompFactory *_factory;

        // TODO:动态组件
        bool _isDynamicComp = false;    
    };
    std::vector<_WillRegComp> _willRegComps;

    std::vector<CompObject *> _comps;
    std::unordered_map<LibString, std::vector<CompObject *>> _compNameRefComps;
    std::unordered_map<LibString, std::vector<CompObject *>> _icompNameRefComps;
    std::unordered_map<UInt64, CompObject *> _compIdRefComp;
    std::unordered_map<Int32, std::vector<CompObject *>> _compTypeRefComps;

    // 关注的接口
    std::vector<std::vector<CompObject *>> _focusTypeRefComps;
};

template<typename CompFactoryType>
ALWAYS_INLINE Int32 CompHostObject::RegisterComp()
{
    auto newFactory = CRYSTAL_NEW(CompFactoryType);
    Int32 ret = RegisterComp(newFactory);
    if(ret != Status::Success)
    {
        newFactory->Release();
        newFactory = NULL;
        return ret;
    }

    return Status::Success;    
}


template<typename ObjType>
ALWAYS_INLINE ObjType *CompHostObject::GetComp(UInt64 objectId)
{
    return KernelCastTo<ObjType>(GetComp(objectId));
}

ALWAYS_INLINE CompObject *CompHostObject::GetComp(UInt64 objectId)
{
    auto iter = _compIdRefComp.find(objectId);
    return iter == _compIdRefComp.end() ? NULL : iter->second;
}

template<typename ObjType>
ALWAYS_INLINE ObjType *CompHostObject::GetComp()
{
    auto compName = RttiUtil::GetByType<ObjType>();
    if(UNLIKELY(KERNEL_NS::LibString(compName) == ""))
        return NULL;

    return KernelCastTo<ObjType>(GetComp(compName));
}

template<typename ObjType>
ALWAYS_INLINE const ObjType *CompHostObject::GetComp() const
{
    auto compName = RttiUtil::GetByType<ObjType>();
    if(UNLIKELY(compName == ""))
        return NULL;

    return reinterpret_cast<const ObjType *>(GetComp(compName));
}

ALWAYS_INLINE CompObject *CompHostObject::GetComp(const LibString &compRttiName)
{
    // 先从接口中获得
    auto iterIComp = _icompNameRefComps.find(compRttiName);
    if(iterIComp != _icompNameRefComps.end() && !iterIComp->second.empty())
        return iterIComp->second[0];

    auto iter = _compNameRefComps.find(compRttiName);
    if(iter == _compNameRefComps.end() || iter->second.empty())
        return NULL;

    return iter->second[0];
}

ALWAYS_INLINE const CompObject *CompHostObject::GetComp(const LibString &compRttiName) const
{
    // 先从接口中获得
    auto iterIComp = _icompNameRefComps.find(compRttiName);
    if(iterIComp != _icompNameRefComps.end() && !iterIComp->second.empty())
        return iterIComp->second[0];

    auto iter = _compNameRefComps.find(compRttiName);
    if(iter == _compNameRefComps.end() || iter->second.empty())
        return NULL;

    return iter->second[0];
}

ALWAYS_INLINE std::vector<CompObject *> &CompHostObject::GetComps(const LibString &compRttiName)
{
    // 先从接口中获得
    auto iterIComp = _icompNameRefComps.find(compRttiName);
    if(iterIComp != _icompNameRefComps.end() && !iterIComp->second.empty())
        return iterIComp->second;

    auto iter = _compNameRefComps.find(compRttiName);
    return iter == _compNameRefComps.end() ? CompHostObject::_emptyComps : iter->second;
}

ALWAYS_INLINE const std::vector<CompObject *> &CompHostObject::GetComps(const LibString &compRttiName) const
{
    // 先从接口中获得
    auto iterIComp = _icompNameRefComps.find(compRttiName);
    if(iterIComp != _icompNameRefComps.end() && !iterIComp->second.empty())
        return iterIComp->second;

    auto iter = _compNameRefComps.find(compRttiName);
    return iter == _compNameRefComps.end() ? CompHostObject::_emptyComps : iter->second;
}

ALWAYS_INLINE CompObject *CompHostObject::GetCompByType(Int32 type)
{
    auto iter = _compTypeRefComps.find(type);
    if(iter == _compTypeRefComps.end() || iter->second.empty())
        return NULL;

    return iter->second[0];
}

ALWAYS_INLINE const CompObject *CompHostObject::GetCompByType(Int32 type) const
{
    auto iter = _compTypeRefComps.find(type);
    if(iter == _compTypeRefComps.end() || iter->second.empty())
        return NULL;

    return iter->second[0];  
}

ALWAYS_INLINE std::vector<CompObject *> &CompHostObject::GetCompsByType(Int32 type)
{
    auto iter = _compTypeRefComps.find(type);
    return iter == _compTypeRefComps.end() ? _emptyComps : iter->second;
}

ALWAYS_INLINE const std::vector<CompObject *> &CompHostObject::GetCompsByType(Int32 type) const
{
    auto iter = _compTypeRefComps.find(type);
    return iter == _compTypeRefComps.end() ? _emptyComps : iter->second;
}

ALWAYS_INLINE std::vector<CompObject *> &CompHostObject::GetAllComps()
{
    return _comps;
}

ALWAYS_INLINE const std::vector<CompObject *> &CompHostObject::GetAllComps() const
{
    return _comps;
}

ALWAYS_INLINE void CompHostObject::_ResizeFocusDict()
{
    _focusTypeRefComps.resize(GetMaxFocusEnd());
}

KERNEL_END

#endif
