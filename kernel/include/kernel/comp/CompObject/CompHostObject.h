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

#include <unordered_map>
#include <vector>
#include <map>
#include <unordered_set>

#include <kernel/common/status.h>
#include <kernel/comp/CompObject/CompFactory.h>
#include <kernel/comp/CompObject/CompObject.h>

KERNEL_BEGIN

// 类型id:KERNEL_NS::RttiUtil::GetTypeId<XXX>()
class KERNEL_EXPORT CompHostObject : public CompObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, CompHostObject);

public:
    static std::vector<CompObject *> _emptyComps;

public:

    // 类型id:KERNEL_NS::RttiUtil::GetTypeId<XXX>()
    CompHostObject(UInt64 objTypeId);
    virtual ~CompHostObject();

// // 基本api
protected:
    virtual Int32 _OnCreated() final;
    virtual Int32 _OnInit() final;
    virtual Int32 _OnStart() final;
    virtual void _OnWillClose() final;
    virtual void _OnClose() final;

    virtual Int32 _OnHostCreated();
    // 在组件初始化前 必须重写
    virtual Int32 _OnHostInit() { return Status::Success; };
    // 带优先级的组件创建完成
    virtual Int32 _OnPriorityLevelCompsCreated();
    // 所有组件创建完成
    virtual Int32 _OnCompsCreated();
    // 在组件启动之前 请勿在WillStart及之前的接口启动线程，此时都是线程不安全的状态
    virtual Int32 _OnHostWillStart(){ return Status::Success; }
    // 组件启动之后 此时可以启动线程 必须重写
    virtual Int32 _OnHostStart() { return Status::Success; };
    // 在组件willclose之前 host若内部有多线程,应该要在组件的willclose之前结束掉线程,避免资源的竞争
    virtual void _OnHostBeforeCompsWillClose() {}
    // 在组件willclose之后
    virtual void _OnHostWillClose(){}
    // 在组件close之前
    virtual void _OnHostBeforeCompsClose() {}
    // 在组件Close之后 必须重写
    virtual void _OnHostClose() {};
    // 在组件更新之前
    virtual void _OnWillHostUpdate() {}
    // 在组件更新之后
    virtual void _OnHostUpdate() {}

    // 动态添加组件Created
    virtual Int32 _OnDynamicAddCompCreated(CompObject *newComp) { return Status::Success;}
    virtual Int32 _OnDynamicAddCompInited(CompObject *newComp) {return Status::Success;}
    virtual Int32 _OnDynamicAddCompStarted(CompObject *newComp) {return Status::Success;}
    virtual Int32 _OnAfterDynamicAddComp(CompObject *newComp) {return Status::Success;}
    
    virtual void _OnWillDynamicPopComp(CompObject *comp) {}
    virtual void _OnDynamicPopCompFinish(CompObject *comp) {}

    // attach
    virtual void _OnAttachedComp(CompObject *oldComp, CompObject *newComp) {}

public:
    virtual void Clear() override;
    virtual void OnUpdate() final;

    // 宿主所有组件以及自身准备就绪 注意多线程安全可能是从其他组件触发,组件可能不在一个线程
    virtual LibString ToString() const override;

    bool IsAllCompsReady(CompObject *&notReadyComp) const;
    bool IsAllCompsDown(CompObject *&notDownComp) const;
    bool IsAttached(CompObject *comp) const;

// // 功能api
public:
    // 注册组件 // 注意递归死循环,若HostC中有HostA, HostA中也有HostC那么将导致死循环
    virtual void OnRegisterComps() {};  

    // 注册
    template<typename CompFactoryType>
    Int32 RegisterComp();
    Int32 RegisterComp(CompFactory *factory);
    // 一次性注册多个
    Int32 RegisterComp(const std::vector<CompFactory *> &factory);
    Int32 RegisterComp(CompObject *comp);

    template<typename CompType>
    void RemoveComp()
    {
        // 替换已有组件 TODO:找到组件, 存在的要对组件执行Close以及移除操作
        const UInt64 typeId = RttiUtil::GetTypeId<CompType>();
        auto comp = GetCompByTypeId(typeId);
        if(UNLIKELY(!comp))
        {
            // 找不到有可能还没注册
            const Int32 count = static_cast<Int32>(_willRegComps.size());
            for(Int32 idx = count - 1; idx >= 0; --idx)
            {
                // 如果是Factory,这时候就移除不了,需要从设计上考虑
                auto &regComp = _willRegComps[idx];
                if(!regComp._comp)
                    continue;
                
                const auto willRegTypeId = regComp._comp->GetObjTypeId();
                const auto iWillRegTypeId = regComp._comp->GetInterfaceTypeId();
                if((typeId != willRegTypeId) && (typeId != iWillRegTypeId))
                    continue;

                regComp._comp->WillClose();
                regComp._comp->Close();
                regComp._comp->Release();

                _willRegComps.erase(_willRegComps.begin() + idx);
            }

            return;
        }

        // 移除所有
        do
        {
            _RemoveComp(comp);
        } while ((comp = GetCompByTypeId(typeId)) != NULL);
    }

    void RemoveComp(CompObject *comp);
    template<typename CompFactoryType>
    void RemoveCompFactory()
    {
        const auto &typeName = RttiUtil::GetByType<CompFactoryType>();
        const Int32 count = static_cast<Int32>(_willRegComps.size());
        for(Int32 idx = count -1; idx >= 0; --idx)
        {
            auto &regInfo = _willRegComps[idx];
            if(!regInfo._factory)
                continue;

            const auto &factoryTypeName = RttiUtil::GetByObj(regInfo._factory);
            if(typeName != factoryTypeName)
                continue;

            regInfo._factory->Release();
            _willRegComps.erase(_willRegComps.begin() + idx);
        }
    }

    // 通过objectid（除了DynamicComp）
    template<typename ObjType>
    ObjType *GetComp(UInt64 objectId);
    CompObject *GetComp(UInt64 objectId);

    // 通过运行时类型识别（除了DynamicComp）
    template<typename ObjType>
    ObjType *GetComp();
    template<typename ObjType>
    const ObjType *GetComp() const;
    CompObject *GetCompByTypeId(UInt64 typeId);
    const CompObject *GetCompByTypeId(UInt64 typeId) const;
    std::vector<CompObject *> &GetCompsByTypeId(UInt64 typeId);
    const std::vector<CompObject *> &GetCompsByTypeId(UInt64 typeId) const;

    CompObject *GetCompByName(const LibString &objName);
    const CompObject *GetCompByName(const LibString &objName) const;
    std::vector<CompObject *> &GetCompsByName(const LibString &objName);
    const std::vector<CompObject *> &GetCompsByName(const LibString &objName) const;

    // 替换已有组件 同时必须校验循环依赖 TODO:找到组件, 存在的要对组件执行Close以及移除操作, 用于比如热更配置（在另外一个线程创建ConfigLoader并加载好配置后替换掉原有的ConfigLoader）
    template<typename ObjType>
    bool ReplaceComp(ObjType *comp)
    {
        // 必须没有owner
        if(UNLIKELY(comp->GetOwner()))
            return false;

        // 必须没有错误
        if(UNLIKELY(comp->GetErrCode() != Status::Success))
            return false;

        // 替换已有组件 TODO:找到组件, 存在的要对组件执行Close以及移除操作
        const auto typeId = comp->GetObjTypeId();
        auto oldComp = GetCompByTypeId(typeId);
        if(oldComp == comp)
            return true;

        if(UNLIKELY(!oldComp))
        {// 旧的组件不存在, 那么执行的是新增
            if(!AddComp(comp))
            {
                return false;
            }

            return true;
        }

        // 替换
        _ReplaceComp(oldComp, comp);

        if(LIKELY(!IsAttached(oldComp)))
        {
            oldComp->WillClose();
            oldComp->Close();
            oldComp->Release();
        }

        comp->OnBindNewHost(this);

        return true;
    }

    // 借用对象, 不会bindowner,它的owner仍然是原来的, 只是做了dict的映射, 也不会调用生命周期函数和它关forcus的接口, 避免和原owner冲突
    bool AttachComp(CompObject *comp);

    // 批量替换相同类型组件 TODO:找到组件, 存在的要对组件进行Close和移除操作
    // TODO:批量替换多个同类型组件

    // 提供给有设置了Type的对象（除了DynamicComp）
    CompObject *GetCompByType(Int32 type);
    const CompObject *GetCompByType(Int32 type) const;
    std::vector<CompObject *> &GetCompsByType(Int32 type);
    const std::vector<CompObject *> &GetCompsByType(Int32 type) const;

    // 所有组件（除了DynamicComp）
    std::vector<CompObject *> &GetAllComps();
    const std::vector<CompObject *> &GetAllComps() const;

    // 检查host 类型组件避免循环依赖 dependingShip:依赖关系
    bool CheckCircleDepending(const LibString &compName, KERNEL_NS::LibString &dependingShip) const;

    // 添加组件
    bool AddComp(CompObject *comp);

    // 弹出组件
    template<typename ObjType>
    ObjType *PopComp();

    bool PopComp(CompObject *comp);    

private:
    // 在MaxFocusEnum变化时需要调用该方法
    void _ResizeFocusDict();
    Int32 _InitComps();
    Int32 _StartComps();
    void _OnWillCloseComps();
    void _CloseComps();
    void _DestroyWillRegComps();
    void _DestroyComps();
    void _AddComp(CompObject *comp, bool isAttach = false);
    void _ReplaceComp(CompObject *oldComp, CompObject *comp);
    Int32 _MakeCompStarted(CompObject *comp);
    void _RemoveComp(CompObject *comp);

    void _MaskIfFocus(CompObject *comp);
    void _RemoveFromFocus(CompObject *comp);
    void _Clear();

    void _OnUpdateComps();

    void _Release(CompFactory *factory);
    void _Release(const std::vector<CompFactory *> &factorys);

private:
    // 委托注册
    class _WillRegComp
    {
    public:
        _WillRegComp(CompFactory *factory):_comp(NULL), _factory(factory){}
        _WillRegComp(CompObject *comp):_comp(comp), _factory(NULL){}

        CompObject *_comp;
        CompFactory *_factory;

    };
    std::vector<_WillRegComp> _willRegComps;
    std::map<Int32, std::vector<_WillRegComp>> _priorityLevelRefWillRegComp;

    std::vector<CompObject *> _comps;
    // std::unordered_map<LibString, std::vector<CompObject *>> _icompNameRefComps;

    std::unordered_map<UInt64, std::vector<CompObject *>> _compTypeIdRefComps;
    std::unordered_map<LibString, std::vector<CompObject *>> _compObjNameRefComps;

    std::unordered_map<UInt64, CompObject *> _compIdRefComp;
    std::unordered_map<Int32, std::vector<CompObject *>> _compTypeRefComps;

    // 关注的接口
    std::vector<std::vector<CompObject *>> _focusTypeRefComps;

    // 被当前宿主借用的组件, 不进行生命周期管理,只能在当前宿主started之后, 且还没close才能借用, 且只做映射, 不执行forcus等接口调用, owner也不替换成当前Host
    std::unordered_set<CompObject *> _attachedComps;
};

ALWAYS_INLINE bool CompHostObject::IsAttached(CompObject *comp) const
{
    return _attachedComps.find(comp) != _attachedComps.end();
}

template<typename CompFactoryType>
ALWAYS_INLINE Int32 CompHostObject::RegisterComp()
{
    auto newFactory = CompFactoryType::FactoryCreate();
    Int32 ret = RegisterComp(newFactory);
    if(ret != Status::Success)
    {
        _Release(newFactory);
        return ret;
    }

    return Status::Success;    
}

ALWAYS_INLINE void CompHostObject::RemoveComp(CompObject *comp)
{
    if(UNLIKELY(!comp))
        return;

    _RemoveComp(comp);
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
    const auto typeId = RttiUtil::GetTypeId<ObjType>();
    if(UNLIKELY(typeId == 0))
        return NULL;

    return KernelCastTo<ObjType>(GetCompByTypeId(typeId));
}

template<typename ObjType>
ALWAYS_INLINE const ObjType *CompHostObject::GetComp() const
{
    const auto typeId = RttiUtil::GetTypeId<ObjType>();
    if(UNLIKELY(typeId == 0))
        return NULL;

    return KernelCastTo<ObjType>(GetCompByTypeId(typeId));
}

ALWAYS_INLINE CompObject *CompHostObject::GetCompByTypeId(UInt64 typeId)
{
    auto iter = _compTypeIdRefComps.find(typeId);
    if(iter == _compTypeIdRefComps.end() || iter->second.empty())
        return NULL;

    return iter->second[0];
}

ALWAYS_INLINE const CompObject *CompHostObject::GetCompByTypeId(UInt64 typeId) const
{
    auto iter = _compTypeIdRefComps.find(typeId);
    if(iter == _compTypeIdRefComps.end() || iter->second.empty())
        return NULL;

    return iter->second[0];
}

ALWAYS_INLINE std::vector<CompObject *> &CompHostObject::GetCompsByTypeId(UInt64 typeId)
{
    auto iter = _compTypeIdRefComps.find(typeId);
    return iter == _compTypeIdRefComps.end() ? CompHostObject::_emptyComps : iter->second;
}

ALWAYS_INLINE const std::vector<CompObject *> &CompHostObject::GetCompsByTypeId(UInt64 typeId) const
{
    auto iter = _compTypeIdRefComps.find(typeId);
    return iter == _compTypeIdRefComps.end() ? CompHostObject::_emptyComps : iter->second;
}

ALWAYS_INLINE CompObject *CompHostObject::GetCompByName(const LibString &objName)
{
    auto iter = _compObjNameRefComps.find(objName);
    return iter == _compObjNameRefComps.end() ? NULL : iter->second[0];
}

ALWAYS_INLINE const CompObject *CompHostObject::GetCompByName(const LibString &objName) const
{
    auto iter = _compObjNameRefComps.find(objName);
    return iter == _compObjNameRefComps.end() ? NULL : iter->second[0];
}

ALWAYS_INLINE std::vector<CompObject *> &CompHostObject::GetCompsByName(const LibString &objName)
{
    auto iter = _compObjNameRefComps.find(objName);
    return iter == _compObjNameRefComps.end() ? CompHostObject::_emptyComps : iter->second;
}

ALWAYS_INLINE const std::vector<CompObject *> &CompHostObject::GetCompsByName(const LibString &objName) const
{
    auto iter = _compObjNameRefComps.find(objName);
    return iter == _compObjNameRefComps.end() ? CompHostObject::_emptyComps : iter->second;
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

template<typename ObjType>
ALWAYS_INLINE ObjType *CompHostObject::PopComp()
{
    auto comp = GetComp<ObjType>();

    if(UNLIKELY(!comp))
        return NULL;

    PopComp(comp);
    return comp;
}

ALWAYS_INLINE void CompHostObject::_ResizeFocusDict()
{
    _focusTypeRefComps.resize(GetMaxFocusEnd());
}

ALWAYS_INLINE void CompHostObject::_Release(CompFactory *factory)
{
    factory->Release();
}

ALWAYS_INLINE void CompHostObject::_Release(const std::vector<CompFactory *> &factorys)
{
    for(auto factory : factorys)
        factory->Release();
}

KERNEL_END

#endif
