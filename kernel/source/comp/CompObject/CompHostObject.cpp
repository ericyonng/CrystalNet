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
 * Date: 2022-03-10 16:09:21
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/CompObject/CompFactory.h>
#include <kernel/comp/CompObject/CompObject.h>
#include <kernel/comp/Log/log.h>
#include <kernel/comp/Utils/RttiUtil.h>
#include <kernel/comp/Utils/ContainerUtil.h>

#include <kernel/comp/CompObject/CompHostObject.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(CompHostObject);

std::vector<CompObject *> CompHostObject::_emptyComps;

CompHostObject::CompHostObject(UInt64 objTypeId)
:CompObject(objTypeId)
{
    SetKernelObjType(KernelObjectType::HOST_COMP);
}

CompHostObject::~CompHostObject()
{
    _Clear();
}

Int32 CompHostObject::_OnCreated()
{
    const LibString &objName = RttiUtil::GetByObj(this);
    LibString dependingShip = objName + " ";
    if(!CheckCircleDepending(objName, dependingShip))
    {
        auto owner = GetOwner();
        g_Log->Warn(LOGFMT_OBJ_TAG("circle depending... %s, owner::%s, dependingShip:%s"), objName.c_str(), owner ? owner->GetObjName().c_str() : "", dependingShip.c_str());
        return Status::Repeat;
    }

    auto st = _OnHostCreated();
    if(st != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("on host _OnCreated fail:%s, st:%d"), GetObjName().c_str(), st);
        return st;
    }

    if(g_Log->IsEnable(LogLevel::Debug))
        g_Log->Debug(LOGFMT_OBJ_TAG("on host _OnCreated obj:%s"), GetObjName().c_str());

    return Status::Success;
}

Int32 CompHostObject::_OnInit()
{    
    Int32 ret = _OnHostInit();
    if(ret != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("on host _OnInit fail ret:%d"), ret);
        return ret;
    }

    // 重新调整关注接口字典
    _ResizeFocusDict();

    // 注册组件
    OnRegisterComps();
    
    // 初始化
    ret = _InitComps();
    if(ret != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("init comps fail ret:%d"), ret);
        return ret;
    }

    if(g_Log->IsEnable(LogLevel::Debug))
        g_Log->Debug(LOGFMT_OBJ_TAG("_OnInit CompHostObject %s suc"), GetObjName().c_str());
    return Status::Success;
}

Int32 CompHostObject::_OnStart()
{
    Int32 ret = _OnHostWillStart();
    if(ret != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("CompHostObject _OnHostWillStart fail ret:%d"), ret);
        return ret;
    }

    ret = _StartComps();
    if(ret != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("start comps fail ret:%d"), ret);
        return ret;
    }

    // 自己完成了
    ret = _OnHostStart();
    if(ret != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("_OnHostStart host comp fail ret:%d"), ret);
        
        _OnWillClose();
        _OnClose();
        
        return ret;
    }

    if(g_Log->IsEnable(LogLevel::Debug))
        g_Log->Debug(LOGFMT_OBJ_TAG("_OnStart suc host %s object"), GetObjName().c_str());
    return Status::Success;
}

void CompHostObject::_OnWillClose() 
{
    _OnHostBeforeCompsWillClose();
    _OnWillCloseComps();
    _OnHostWillClose();

    if(g_Log->IsEnable(LogLevel::Debug))
        g_Log->Debug(LOGFMT_OBJ_TAG("_OnWillClose CompHostObject %s suc"), GetObjName().c_str());
}

void CompHostObject::_OnClose()
{
    _OnHostBeforeCompsClose();
    _CloseComps();
    _OnHostClose();

    _Clear();
    CompObject::_OnClose();

    if(g_Log->IsEnable(LogLevel::Debug))
        g_Log->Debug(LOGFMT_OBJ_TAG("_OnClose CompHostObject suc host:%s"), _objName.c_str());
}

void CompHostObject::Clear()
{
    _Clear();
    CompObject::Clear();
}

void CompHostObject::OnUpdate()
{
    _OnWillHostUpdate();
    _OnUpdateComps();
    _OnHostUpdate();
}

LibString CompHostObject::ToString() const
{
    LibString info;
    info.AppendFormat("host comp base info:%s\n", IObject::ToString().c_str())
        .AppendFormat("comps count:%llu, comps type quantity:%llu info:\n"
                    , static_cast<UInt64>(_comps.size()), static_cast<UInt64>(_compTypeRefComps.size()));

    for(auto comp : _comps)
        info.AppendFormat("%s\n", comp->ToString().c_str());

    return info;   
}

bool CompHostObject::IsAllCompsReady(CompObject *&notReadyComp) const
{
    for(auto comp : _comps)
    {
        if(!comp->IsReady())
        {
            notReadyComp = comp;
            return false;
        }
    }

    return true;
}

bool CompHostObject::IsAllCompsDown(CompObject *&notDownComp) const
{
    for(auto comp: _comps)
    {
        if(comp->IsReady())
        {
            notDownComp = comp;
            return false;
        }
    }

    return true;
}

Int32 CompHostObject::RegisterComp(CompFactory *factory)
{
    if(UNLIKELY(!factory))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("factory is nil please check"));
        return Status::Failed;
    }

    // 是否被注册过
    for(auto iter : _priorityLevelRefWillRegComp)
    {
        for(auto iterRegComp : iter.second)
        {
            if(iterRegComp._factory == factory)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("repeat factory:%s, already register in will reg comps queue."), RttiUtil::GetByObj(factory).c_str());
                return Status::Repeat;
            }
        }
    }

    // 是否被注册过
    for(auto iter = _willRegComps.begin(); iter != _willRegComps.end(); ++iter)
    {
        if(iter->_factory == factory)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("repeat factory:%s, already register in will reg comps queue."), RttiUtil::GetByObj(factory).c_str());
            return Status::Repeat;
        }
    }

    if(LIKELY(factory->GetPriorityLevel() == CompPriorityLevel::NONE))
    {
        _willRegComps.push_back(_WillRegComp(factory));
    }
    else
    {
        const auto priorityLevel = factory->GetPriorityLevel();
        auto iter = _priorityLevelRefWillRegComp.find(priorityLevel);
        if(iter == _priorityLevelRefWillRegComp.end())
            iter = _priorityLevelRefWillRegComp.insert(std::make_pair(priorityLevel, std::vector<_WillRegComp>())).first;
        iter->second.push_back(_WillRegComp(factory));
    }

    return Status::Success;
}

Int32 CompHostObject::RegisterComp(const std::vector<CompFactory *> &factorys)
{
    if(UNLIKELY(factorys.empty()))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("have no factorys please check"));
        return Status::Failed;
    }

    std::vector<CompFactory *> toRegister;
    for(auto factory : factorys)
    {
        // 是否被注册过
        bool isRegister = false;
        for(auto iter = _willRegComps.begin(); iter != _willRegComps.end(); ++iter)
        {
            if(iter->_factory == factory)
            {
                isRegister = true;
                g_Log->Warn(LOGFMT_OBJ_TAG("repeat factory:%s, already register in will reg comps queue."), RttiUtil::GetByObj(factory).c_str());
                break;
            }
        }

        if(UNLIKELY(isRegister))
            continue;

        // 是否被注册过
        isRegister = false;
        for(auto iter : _priorityLevelRefWillRegComp)
        {
            for(auto iterRegComp : iter.second)
            {
                if(iterRegComp._factory == factory)
                {
                    isRegister = true;
                    g_Log->Warn(LOGFMT_OBJ_TAG("repeat factory:%s, already register in will reg comps queue."), RttiUtil::GetByObj(factory).c_str());
                    break;
                }
            }
        }

        if(UNLIKELY(isRegister))
            continue;

        toRegister.push_back(factory);
    }

    if(toRegister.empty())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("have no any comp to register."));
        return Status::Success;
    }

    for(auto factory : toRegister)
    {
        if(LIKELY(factory->GetPriorityLevel() == CompPriorityLevel::NONE))
        {
            _willRegComps.push_back(_WillRegComp(factory));
        }
        else
        {
            const auto priorityLevel = factory->GetPriorityLevel();
            auto iter = _priorityLevelRefWillRegComp.find(priorityLevel);
            if(iter == _priorityLevelRefWillRegComp.end())
                iter = _priorityLevelRefWillRegComp.insert(std::make_pair(priorityLevel, std::vector<_WillRegComp>())).first;
            iter->second.push_back(_WillRegComp(factory));
        }
    }

    return Status::Success;
}

Int32 CompHostObject::RegisterComp(CompObject *comp)
{
    if(UNLIKELY(!comp))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("comp is nil please check"));
        return Status::Failed;
    }

    // 是否被注册过
    for(auto iter = _willRegComps.begin(); iter != _willRegComps.end(); ++iter)
    {
        if(iter->_comp == comp)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("repeat comp:%s, already register in will reg comps queue."), RttiUtil::GetByObj(comp).c_str());
            return Status::Repeat;
        }
    }

    
    // 是否被注册过
    for(auto iter = _comps.begin(); iter != _comps.end(); ++iter)
    {
        if(*iter == comp)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("repeat comp:%s, already register in comps queue."), RttiUtil::GetByObj(comp).c_str());
            return Status::Repeat;
        }
    }

    _willRegComps.push_back(_WillRegComp(comp));
    return Status::Success;
}

bool CompHostObject::AttachComp(CompObject *comp)
{
    if(UNLIKELY(!comp))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("comp is zero current:%s"), GetObjName().c_str());
        return false;
    }

    // 必须没有错误
    if(UNLIKELY(comp->GetErrCode() != Status::Success))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("comp has error, comp:%s current:%s"), comp->ToString().c_str(), GetObjName().c_str());
        return false;
    }

    // 替换已有组件 TODO:找到组件, 存在的要对组件执行Close以及移除操作
    const auto typeId = comp->GetObjTypeId();
    auto oldComp = GetCompByTypeId(typeId);
    if(oldComp == comp)
        return true;

    if(UNLIKELY(!oldComp))
    {// 旧的组件不存在, 那么执行的是新增

        //是否存在循环依赖
        LibString info;
        if(UNLIKELY(!CheckCircleDepending(comp->GetObjName(), info)))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("exists circle :%s, comp:%s, current host:%s"), info.c_str(), comp->GetObjName().c_str(), GetObjName().c_str());
            return false;
        }

        _AddComp(comp, true);

        _OnAttachedComp(NULL, comp);

        g_Log->Info(LOGFMT_OBJ_TAG("host:%s, attach comp:%s"), GetObjName().c_str(), comp->GetObjName().c_str());

        return true;
    }

    // 替换的必须comp是已启动的, 因为执行Start/等的不确定性
    if(UNLIKELY(!comp->IsStarted()))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("comp not started, comp:%s, current host:%s"), comp->GetObjName().c_str(), GetObjName().c_str());
        return false;
    }

    _attachedComps.insert(comp);

    // 替换
    _ReplaceComp(oldComp, comp);

    g_Log->Info(LOGFMT_OBJ_TAG("host:%s, attatch oldcomp:%s => new comp:%s"), ToString().c_str(), oldComp->ToString().c_str(), comp->ToString().c_str());

    _OnAttachedComp(oldComp, comp);

    if(LIKELY(!IsAttached(oldComp)))
    {
        g_Log->Info(LOGFMT_OBJ_TAG("attch comp:%s, remove old comp:%s, host:%s")
        , comp->ToString().c_str(), oldComp->ToString().c_str(), ToString().c_str());

        oldComp->WillClose();
        oldComp->Close();
        oldComp->Release();
    }

    _attachedComps.erase(oldComp);

    return true;
}


bool CompHostObject::CheckCircleDepending(const LibString &compName, KERNEL_NS::LibString &dependingShip) const
{
    // 循环依赖定义:A <= B <= A, 即A往上找到宿主B，再往上找如果找到的是自己那么说明存在循环依赖
    auto owner = GetOwner();
    if(LIKELY(owner))
    {
        dependingShip.AppendFormat("=> %s ", owner->GetObjName().c_str());
        if(owner->GetObjName() == compName)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("circle depending please check compName:%s, dependingShip:%s")
                        , compName.c_str(), dependingShip.c_str());
            return false;
        }

        return owner->CastTo<CompHostObject>()->CheckCircleDepending(compName, dependingShip);
    }

    return true;
}


bool CompHostObject::AddComp(CompObject *comp)
{
    if(UNLIKELY(!comp))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("comp is nil please check"));
        return false;
    }

    if(UNLIKELY(comp->GetErrCode() != Status::Success))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("comp has error, comp:%s, current:%s"), comp->ToString().c_str(), GetObjName().c_str());
        return false;
    }

    // 是否被注册过
    for(auto iter = _willRegComps.begin(); iter != _willRegComps.end(); ++iter)
    {
        if(iter->_comp == comp)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("repeat comp:%s, already register in will reg comps queue."), RttiUtil::GetByObj(comp).c_str());
            return false;
        }
    }

    // 是否被注册过
    for(auto iter = _comps.begin(); iter != _comps.end(); ++iter)
    {
        if(*iter == comp)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("repeat comp:%s, already register in comps queue."), RttiUtil::GetByObj(comp).c_str());
            return false;
        }
    }

    //是否存在循环依赖
    LibString info;
    if(UNLIKELY(!CheckCircleDepending(comp->GetObjName(), info)))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("exists circle :%s, comp:%s, current host:%s"), info.c_str(), comp->GetObjName().c_str(), GetObjName().c_str());
        return false;
    }

    auto oldOwner = comp->GetOwner();
    comp->BindOwner(this);

    auto st = comp->OnCreated();
    if(st != Status::Success)
    {
        comp->BindOwner(oldOwner);
        g_Log->Warn(LOGFMT_OBJ_TAG("oncreate fail comp:%s, st:%d"), comp->GetObjName().c_str(), st);
        return false;
    }

    _AddComp(comp);
    _MaskIfFocus(comp);

    st = _OnDynamicAddCompCreated(comp);
    if(st != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("_OnDynamicAddCompCreated fail comp:%s, st:%d")
        , comp->GetObjName().c_str(), st);
        PopComp(comp);
        comp->BindOwner(oldOwner);

        return false;
    }

    if(!comp->IsInited())
    {
        st = comp->Init();
        if(st != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("Init fail comp:%s st:%d")
            , comp->GetObjName().c_str(), st);
            PopComp(comp);
            comp->BindOwner(oldOwner);
            return false;
        }

        st = _OnDynamicAddCompInited(comp);
        if(st != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("_OnDynamicAddCompInited fail comp:%s st:%d")
            , comp->GetObjName().c_str(), st);
            PopComp(comp);
            comp->BindOwner(oldOwner);
            return false;
        }
    }

    if(!comp->IsStarted())
    {
        st = comp->Start();
        if(st != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("Start fail comp:%s st:%d")
            , comp->GetObjName().c_str(), st);
            PopComp(comp);
            comp->BindOwner(oldOwner);
            return false;
        }

        st = _OnDynamicAddCompStarted(comp);
        if(st != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("_OnDynamicAddCompStarted fail comp:%s st:%d")
            , comp->GetObjName().c_str(), st);
            PopComp(comp);
            comp->BindOwner(oldOwner);
            return false;
        }
    }

    st = _OnAfterDynamicAddComp(comp);
    if(st != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("_OnAfterDynamicAddComp fail comp:%s st:%d")
        , comp->GetObjName().c_str(), st);
        PopComp(comp);
        comp->BindOwner(oldOwner);
        return false;
    }

    comp->OnBindNewHost(this);

    return true;
}

bool CompHostObject::PopComp(CompObject *comp)
{
    if(UNLIKELY(!comp))
        return true;

    _OnWillDynamicPopComp(comp);

    // TODO:
    {// 容器中替代
        const Int32 compAmount = static_cast<Int32>(_comps.size());
        for(Int32 idx = 0; idx < compAmount; ++idx)
        {
            if(_comps[idx] == comp)
            {
                _comps.erase(_comps.begin() + idx);
                break;
            }
        }

        _compIdRefComp.erase(comp->GetId());
    }

    {// 类型名映射
        const auto &objName = comp->GetObjName();
        auto iterComps = _compObjNameRefComps.find(objName);
        if(iterComps != _compObjNameRefComps.end())
        {
            auto &comps = iterComps->second;
            {
                const Int32 compAmount = static_cast<Int32>(comps.size());
                for(Int32 idx = 0; idx < compAmount; ++idx)
                {
                    if(comps[idx] == comp)
                    {
                        comps.erase(comps.begin() + idx);
                        break;
                    }
                }

                if(comps.empty())
                    _compObjNameRefComps.erase(iterComps);
            }
        }
    }

    // 类型映射
    {
        const auto typeId = comp->GetObjTypeId();
        auto iterComps = _compTypeIdRefComps.find(typeId);
        if(iterComps != _compTypeIdRefComps.end())
        {
            auto &comps = iterComps->second;
            {
                const Int32 compAmount = static_cast<Int32>(comps.size());
                for(Int32 idx = 0; idx < compAmount; ++idx)
                {
                    if(comps[idx] == comp)
                    {
                        comps.erase(comps.begin() + idx);
                        break;
                    }
                }

                if(comps.empty())
                    _compTypeIdRefComps.erase(iterComps);
            }
        }
    }

    const Int32 maxFocusEnum = GetMaxFocusEnd();
    for(Int32 idx = ObjFocusInterfaceFlag::BEGIN; idx < maxFocusEnum; ++idx)
    {
        auto &focusComps = _focusTypeRefComps[idx];
        const auto cn = static_cast<Int32>(focusComps.size());
        for(Int32 compIdx = 0; compIdx < cn; ++compIdx)
        {
            auto item = focusComps[compIdx];
            if(item == comp)
            {
                focusComps.erase(focusComps.begin() + compIdx);
                break;
            }
        }
    }

    // 旧的接口类型id移除
    const auto oldInterfaceTypeId = comp->GetInterfaceTypeId();
    if(oldInterfaceTypeId)
    {
        auto iterOldComps = _compTypeIdRefComps.find(oldInterfaceTypeId);
        if(iterOldComps != _compTypeIdRefComps.end())
        {
            auto &comps = iterOldComps->second;
            {
                const Int32 compAmount = static_cast<Int32>(comps.size());
                for(Int32 idx = 0; idx < compAmount; ++idx)
                {
                    if(comps[idx] == comp)
                    {
                        comps.erase(comps.begin() + idx);
                        break;
                    }
                }

                if(comps.empty())
                    _compTypeIdRefComps.erase(iterOldComps);
            }
        }
    }
    
    // 类型映射
    if(comp->GetType())
    {
        auto iterTypeComps = _compTypeRefComps.find(comp->GetType());
        if(iterTypeComps != _compTypeRefComps.end())
        {
            auto &typeComps = iterTypeComps->second;
            const Int32 compAmount = static_cast<Int32>(typeComps.size());
            for(Int32 idx = 0; idx < compAmount; ++idx)
            {
                if(typeComps[idx] == comp)
                {
                    typeComps.erase(typeComps.begin() + idx);
                    break;
                }
            }

            if(typeComps.empty())
                _compTypeRefComps.erase(iterTypeComps);
        }
    }

    comp->OnPop(this);

    _OnDynamicPopCompFinish(comp);

    _attachedComps.erase(comp);
    return true;
}

Int32 CompHostObject::_OnHostCreated()
{
    if(g_Log->IsEnable(LogLevel::Debug))
        g_Log->Debug(LOGFMT_OBJ_TAG("_OnHostCreated host comp name:%s"), GetObjName().c_str());

    return Status::Success;
}

Int32 CompHostObject::_OnPriorityLevelCompsCreated()
{
    if(g_Log->IsEnable(LogLevel::Debug))
        g_Log->Debug(LOGFMT_OBJ_TAG("_OnPriorityLevelCompsCreated host comp name:%s"), GetObjName().c_str());

    return Status::Success;
}

Int32 CompHostObject::_OnCompsCreated()
{
    if(g_Log->IsEnable(LogLevel::Debug))
        g_Log->Debug(LOGFMT_OBJ_TAG("_OnCompsCreated host comp name:%s"), GetObjName().c_str());

    return Status::Success;
}

Int32 CompHostObject::_InitComps()
{
    // 优先级的先处理
    bool initSuccess = true;
    for(auto iter = _priorityLevelRefWillRegComp.begin(); iter != _priorityLevelRefWillRegComp.end(); ++iter)
    {
        auto &regComps = iter->second;
        for(auto &willRegComp : regComps)
        {
            // 取得comp
            CompObject *newComp = NULL;
            if(willRegComp._factory)
            {
                newComp = willRegComp._factory->Create();
                willRegComp._factory->Release();
                willRegComp._factory = NULL;
            }
            else
            {
                newComp = willRegComp._comp;
                willRegComp._comp = NULL;
            }

            ASSERT(newComp);
            newComp->BindOwner(this);

            Int32 ret = newComp->OnCreated();
            if(ret != Status::Success)
            {
                g_Log->Error(LOGFMT_OBJ_TAG("comp created fail ret:%d, comp name:%s, host comp name:%s"), ret, RttiUtil::GetByObj(newComp).c_str(), GetObjName().c_str());
                
                initSuccess = false;
                newComp->WillClose();
                newComp->Close();
                newComp->Release();
                newComp = NULL;

                _OnWillCloseComps();
                _CloseComps();
                _DestroyComps();
                _DestroyWillRegComps();
                break;
            }

            _AddComp(newComp);

            // 设置关注接口
            _MaskIfFocus(newComp);
        }
    }

    // 失败与否
    if(!initSuccess)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("create priority level comps fail this host comp name:%s."), GetObjName().c_str());
        return Status::Failed;
    }

    _priorityLevelRefWillRegComp.clear();
    Int32 ret = _OnPriorityLevelCompsCreated();
    if(ret != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("priority level comps created fail host comp name:%s, ret:%d"), GetObjName().c_str(), ret);
        _OnWillCloseComps();
        _CloseComps();
        _DestroyComps();
        _DestroyWillRegComps();
        return ret;
    }

    initSuccess = true;
    for(auto iter = this->_willRegComps.begin(); iter != _willRegComps.end(); ++iter)
    {
        // 取得comp
        auto &willRegComp = *iter;
        CompObject *newComp = NULL;
        if(willRegComp._factory)
        {
            newComp = willRegComp._factory->Create();
            willRegComp._factory->Release();
            willRegComp._factory = NULL;
        }
        else
        {
            newComp = willRegComp._comp;
            willRegComp._comp = NULL;
        }

        ASSERT(newComp);
        newComp->BindOwner(this);

        Int32 ret = newComp->OnCreated();
        if(ret != Status::Success)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("comp created fail ret:%d, comp name:%s, host comp name:%s"), ret, RttiUtil::GetByObj(newComp).c_str(), GetObjName().c_str());
            
            initSuccess = false;
            newComp->WillClose();
            newComp->Close();
            newComp->Release();
            newComp = NULL;

            _OnWillCloseComps();
            _CloseComps();
            _DestroyComps();
            _DestroyWillRegComps();
            break;
        }

        _AddComp(newComp);

        // 设置关注接口
        _MaskIfFocus(newComp);
    }

    // 失败与否
    if(!initSuccess)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("create comps fail this host comp name:%s."), GetObjName().c_str());
        return Status::Failed;
    }

    _willRegComps.clear();

    // 组件创建成功
    ret = _OnCompsCreated();
    if(ret != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("comps created fail host comp name:%s, ret:%d"), GetObjName().c_str(), ret);
        _OnWillCloseComps();
        _CloseComps();
        _DestroyComps();
        return ret;
    }

    // 初始化组件
    for(auto comp : _comps)
    {
        // init是初始化自身不可调用依赖
        Int32 ret = comp->Init();
        if(ret != Status::Success)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("init comp obj name:%s, fail ret:%d"), comp->GetObjName().c_str(), ret);
            _OnWillCloseComps();
            _CloseComps();
            _DestroyComps();
            return Status::Failed;
        }
    }

    if(g_Log->IsEnable(LogLevel::Debug))
        g_Log->Debug(LOGFMT_OBJ_TAG("init comps %s suc."), GetObjName().c_str());
    return Status::Success;
}

Int32 CompHostObject::_StartComps()
{
    {// start
        UInt64 startIdx = 0;
        const UInt64 quantity = static_cast<UInt64>(_comps.size());
        for(; startIdx < quantity; ++startIdx)
        {
            CompObject *comp = _comps[startIdx];
            if(comp->IsStarted())
                continue;

            Int32 ret = comp->Start();
            if(ret != Status::Success)
            {
                g_Log->Error(LOGFMT_OBJ_TAG("start comp fail ret:%d, comp name:%s comp info:%s"), ret, comp->GetObjName().c_str(), comp->ToString().c_str());
                break;
            }
        }
        if(startIdx != quantity)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("has some unhandled comps when do start startIdx:%llu, quantity:%llu"), startIdx, quantity);

            _OnWillCloseComps();
            _CloseComps();
            _DestroyComps();
            return Status::Failed;
        }
    }

    if(g_Log->IsEnable(LogLevel::Debug))
        g_Log->Debug(LOGFMT_OBJ_TAG("start comps %s suc."), GetObjName().c_str());
    return Status::Success;
}

void CompHostObject::_OnWillCloseComps()
{
    for(auto iter = _comps.rbegin(); iter != _comps.rend(); ++iter)
    {
        auto comp = *iter;
        if(UNLIKELY(IsAttached(comp)))
            continue;

        if(!comp->IsStarted())
            continue;
        
        comp->WillClose();
    }

    // 等待所有组件结束 TODO:有些组件需要等待有些组件不需要等待，由具体的逻辑自行阻塞
    // CompObject *comp = NULL;
    // for(;!IsAllCompsDown(comp);)
    // {
    //     SystemUtil::ThreadSleep(1);
    //     g_Log->Warn(LOGFMT_OBJ_TAG("wait for all comps down current not down comp:%s"), comp->GetObjName().c_str());
    // }
}

void CompHostObject::_CloseComps()
{
    for(auto iter = _comps.rbegin(); iter != _comps.rend(); ++iter)
    {
        auto comp = *iter;
        if(UNLIKELY(IsAttached(comp)))
            continue;

        if(!comp->IsStarted())
            continue;

        comp->Close();
    }
}

void CompHostObject::_DestroyWillRegComps()
{
    ContainerUtil::DelContainer(_willRegComps, [](_WillRegComp &regComp){
        if(regComp._factory)
        {
            regComp._factory->Release();
            regComp._factory = NULL;
        }
        else if(regComp._comp)
        {
            regComp._comp->Release();
            regComp._comp = NULL;
        }
    });

    for(auto iter = _priorityLevelRefWillRegComp.begin(); iter != _priorityLevelRefWillRegComp.end();)
    {
        ContainerUtil::DelContainer(iter->second, [](_WillRegComp &regComp){
            if(regComp._factory)
            {
                regComp._factory->Release();
                regComp._factory = NULL;
            }
            else if(regComp._comp)
            {
                regComp._comp->Release();
                regComp._comp = NULL;
            }
        });

        iter = _priorityLevelRefWillRegComp.erase(iter);
    }
}

void CompHostObject::_DestroyComps()
{
    ContainerUtil::DelContainer(_comps, [this](CompObject *comp){
        if(UNLIKELY(IsAttached(comp)))
            return;

        const auto compName = comp->GetObjName();
        comp->Release();
    });

    _compTypeIdRefComps.clear();
    _compIdRefComp.clear();
    _compObjNameRefComps.clear();
    _compTypeRefComps.clear();

    if(g_Log->IsEnable(LogLevel::Info)) 
       g_Log->Info(LOGFMT_OBJ_TAG("destroyed comps over of host:%s"), _objName.c_str());
}

void CompHostObject::_AddComp(CompObject *comp, bool isAttach)
{
    _comps.push_back(comp);
    _compIdRefComp.insert(std::make_pair(comp->GetId(), comp));

    // 类型id映射
    {
        const auto typeId = comp->GetObjTypeId();
        auto iterComps = _compTypeIdRefComps.find(typeId);
        if(iterComps == _compTypeIdRefComps.end())
            iterComps = _compTypeIdRefComps.insert(std::make_pair(typeId, std::vector<CompObject *>())).first;
        auto &comps = iterComps->second;
        comps.push_back(comp);
    }

    // 接口类型id映射
    const auto interfaceTypeId = comp->GetInterfaceTypeId();
    if(LIKELY(interfaceTypeId))
    {
        auto iterComps = _compTypeIdRefComps.find(interfaceTypeId);
        if(iterComps == _compTypeIdRefComps.end())
            iterComps = _compTypeIdRefComps.insert(std::make_pair(interfaceTypeId, std::vector<CompObject *>())).first;
        auto &comps = iterComps->second;
        comps.push_back(comp);
    }

    // 类型映射
    if(comp->GetType())
    {
        auto iterTypeComps = _compTypeRefComps.find(comp->GetType());
        if(iterTypeComps == _compTypeRefComps.end())
            iterTypeComps = _compTypeRefComps.insert(std::make_pair(comp->GetType(), std::vector<CompObject *>())).first;
        iterTypeComps->second.push_back(comp);
    }

    // 类型名映射
    const auto &objName = comp->GetObjName();
    {
        auto iterComps = _compObjNameRefComps.find(objName);
        if(iterComps == _compObjNameRefComps.end())
            iterComps = _compObjNameRefComps.insert(std::make_pair(objName, std::vector<CompObject *>())).first;

        iterComps->second.push_back(comp);
    }

    if(isAttach)
        _attachedComps.insert(comp);
}

void CompHostObject::_ReplaceComp(CompObject *oldComp, CompObject *comp)
{
    if(LIKELY(!IsAttached(comp)))
        comp->BindOwner(this);

    // TODO:
    {// 容器中替代
        const Int32 compAmount = static_cast<Int32>(_comps.size());
        for(Int32 idx = 0; idx < compAmount; ++idx)
        {
            if(_comps[idx] == oldComp)
            {
                _comps[idx] = comp;
                break;
            }
        }

        _compIdRefComp.erase(oldComp->GetId());
        _compIdRefComp.insert(std::make_pair(comp->GetId(), comp));
    }

    {// 类型名映射
        const auto &objName = oldComp->GetObjName();
        auto iterComps = _compObjNameRefComps.find(objName);
        if(iterComps == _compObjNameRefComps.end())
            iterComps = _compObjNameRefComps.insert(std::make_pair(objName, std::vector<CompObject *>())).first;

        auto &comps = iterComps->second;
        {
            const Int32 compAmount = static_cast<Int32>(comps.size());
            for(Int32 idx = 0; idx < compAmount; ++idx)
            {
                if(comps[idx] == oldComp)
                {
                    comps[idx] = comp;
                    break;
                }
            }
        }
    }

    // 类型映射
    {
        const auto typeId = comp->GetObjTypeId();
        auto iterComps = _compTypeIdRefComps.find(typeId);
        if(iterComps == _compTypeIdRefComps.end())
            iterComps = _compTypeIdRefComps.insert(std::make_pair(typeId, std::vector<CompObject *>())).first;
        auto &comps = iterComps->second;
        {
            const Int32 compAmount = static_cast<Int32>(comps.size());
            for(Int32 idx = 0; idx < compAmount; ++idx)
            {
                if(comps[idx] == oldComp)
                {
                    comps[idx] = comp;
                    break;
                }
            }
        }
    }

    // 旧的接口类型id移除
    bool isInterfaceReplace = false;
    const auto oldInterfaceTypeId = oldComp->GetInterfaceTypeId();
    if(oldInterfaceTypeId)
    {
        auto iterOldComps = _compTypeIdRefComps.find(oldInterfaceTypeId);
        if(iterOldComps != _compTypeIdRefComps.end())
        {
            auto &comps = iterOldComps->second;
            {
                const Int32 compAmount = static_cast<Int32>(comps.size());
                for(Int32 idx = 0; idx < compAmount; ++idx)
                {
                    if(comps[idx] == oldComp)
                    {
                        comps[idx] = comp;
                        isInterfaceReplace = true;
                        break;
                    }
                }
            }
        }
    }

    // 新组件的接口映射
    {
        const auto newInterfaceTypeId = comp->GetInterfaceTypeId();
        if(!isInterfaceReplace && newInterfaceTypeId)
        {
            auto iterComps = _compTypeIdRefComps.find(newInterfaceTypeId);
            if(iterComps == _compTypeIdRefComps.end())
                iterComps = _compTypeIdRefComps.insert(std::make_pair(newInterfaceTypeId, std::vector<CompObject *>())).first;
            auto &comps = iterComps->second;
            comps.push_back(comp);
        }
    }
    
    // 类型映射
    if(comp->GetType())
    {
        auto iterTypeComps = _compTypeRefComps.find(comp->GetType());
        if(iterTypeComps == _compTypeRefComps.end())
            iterTypeComps = _compTypeRefComps.insert(std::make_pair(comp->GetType(), std::vector<CompObject *>())).first;
        
        auto &typeComps = iterTypeComps->second;
        const Int32 compAmount = static_cast<Int32>(typeComps.size());
        for(Int32 idx = 0; idx < compAmount; ++idx)
        {
            if(typeComps[idx] == oldComp)
            {
                typeComps[idx] = comp;
                break;
            }
        }
    }

    // 关注的接口
    const Int32 maxFocusEnum = GetMaxFocusEnd();
    for(Int32 idx = ObjFocusInterfaceFlag::BEGIN; idx < maxFocusEnum; ++idx)
    {
        auto &focusComps = _focusTypeRefComps[idx];
        auto replaceComp = comp->IsFocus(idx) ? comp : NULL;
        if(UNLIKELY(IsAttached(comp)))
            replaceComp = NULL;

        if(oldComp->IsFocus(idx))
        {
            const Int32 cn = static_cast<Int32>(focusComps.size());
            bool isReplaceSuc = false;
            for(Int32 compIdx = 0; compIdx < cn; ++compIdx)
            {
                auto item = focusComps[compIdx];
                if(item == oldComp)
                {
                    if(replaceComp)
                       focusComps[compIdx] = replaceComp;
                    else
                        focusComps.erase(focusComps.begin() + compIdx);

                    isReplaceSuc = true;
                    break;
                }
            }

            // 没替换成功, 则新增到focus中
            if(!isReplaceSuc)
            {
                if(replaceComp)
                    focusComps.push_back(replaceComp);
            }
        }
        else
        {
            if(replaceComp)
                focusComps.push_back(replaceComp);
        }
    }
}

Int32 CompHostObject::_MakeCompStarted(CompObject *comp)
{
    auto st = comp->OnCreated();
    if(st != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("OnCreated fail comp:%s, st:%d"), comp->ToString().c_str(), st);
        return st;
    }

    st = _OnDynamicAddCompCreated(comp);
    if(st != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("_OnDynamicAddCompCreated fail comp:%s, st:%d"), comp->ToString().c_str(), st);
        return st;
    }

    if(!comp->IsInited())
    {
        st = comp->Init();
        if(st != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("Init fail comp:%s, st:%d"), comp->ToString().c_str(), st);
            return st;
        }

        st = _OnDynamicAddCompInited(comp);
        if(st != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("_OnDynamicAddCompInited fail comp:%s, st:%d"), comp->ToString().c_str(), st);
            return st;
        }
    }

    if(!comp->IsStarted())
    {
        st = comp->Start();
        if(st != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("Start fail comp:%s, st:%d"), comp->ToString().c_str(), st);
            return st;
        }

        st = _OnDynamicAddCompStarted(comp);
        if(st != Status::Success)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("_OnDynamicAddCompStarted fail comp:%s, st:%d"), comp->ToString().c_str(), st);
            return st;
        }
    }

    st = _OnAfterDynamicAddComp(comp);
    if(st != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("_OnAfterDynamicAddComp fail comp:%s, st:%d"), comp->ToString().c_str(), st);
        return st;
    }

    return Status::Success;
}


void CompHostObject::_RemoveComp(CompObject *comp)
{
    {// will reg, 无法移除Factory
        const Int32 count = static_cast<Int32>(_willRegComps.size());
        for(Int32 idx = count - 1; idx >= 0; --idx)
        {
            auto &regComp = _willRegComps[idx];
            if(regComp._comp != comp)
                continue;

            _willRegComps.erase(_willRegComps.begin() + idx);
        }
    }

    {// priority level dict
        for(auto iter = _priorityLevelRefWillRegComp.begin(); iter != _priorityLevelRefWillRegComp.end();)
        {
            auto &regs = iter->second;
            const Int32 cn = static_cast<Int32>(regs.size());
            for(Int32 idx = cn -1; idx >= 0; --idx)
            {
                auto &reg = regs[idx];
                if(reg._comp == comp)
                    continue;

                regs.erase(regs.begin() + idx);
            }

            if(regs.empty())
            {
                iter = _priorityLevelRefWillRegComp.erase(iter);
            }
            else
            {
                ++iter;
            }
        }
        
    }

    {
        const Int32 count = static_cast<Int32>(_comps.size());
        for(Int32 idx = count - 1; idx >= 0; --idx)
        {
            if(_comps[idx] != comp)
                continue;

            _comps.erase(_comps.begin() + idx);
        }

        _compIdRefComp.erase(comp->GetId());

        // 组件名容器中移除
        const auto typeId = comp->GetObjTypeId();
        {
            auto iterComps = _compTypeIdRefComps.find(typeId);
            if(iterComps != _compTypeIdRefComps.end())
            {
                auto &comps = iterComps->second;
                const Int32 countComp = static_cast<Int32>(comps.size());
                for(Int32 idxComp = countComp - 1; idxComp >= 0; --idxComp)
                {
                    if(comps[idxComp] != comp)
                        continue;

                    comps.erase(comps.begin() + idxComp);
                }

                if(comps.empty())
                    _compTypeIdRefComps.erase(iterComps);
            }
        }

        // 接口名容器中移除
        const auto interfaceTypeId = comp->GetInterfaceTypeId();
        {
            auto iterComps = _compTypeIdRefComps.find(interfaceTypeId);
            if(iterComps != _compTypeIdRefComps.end())
            {
                auto &comps = iterComps->second;
                const Int32 countComp = static_cast<Int32>(comps.size());
                for(Int32 idxComp = countComp - 1; idxComp >= 0; --idxComp)
                {
                    if(comps[idxComp] != comp)
                        continue;

                    comps.erase(comps.begin() + idxComp);
                }

                if(comps.empty())
                    _compTypeIdRefComps.erase(iterComps);
            }
        }

        // 从名字容器中移除
        {
            auto iterComps = _compObjNameRefComps.find(comp->GetObjName());
            if(iterComps != _compObjNameRefComps.end())
            {
                auto &comps = iterComps->second;
                const Int32 countComp = static_cast<Int32>(comps.size());
                for(Int32 idxComp = countComp - 1; idxComp >= 0; --idxComp)
                {
                    if(comps[idxComp] != comp)
                        continue;

                    comps.erase(comps.begin() + idxComp);
                }

                if(comps.empty())
                    _compObjNameRefComps.erase(iterComps);
            }
        }

        if(comp->GetType())
        {
            auto iterComps = _compTypeRefComps.find(comp->GetType());
            if(iterComps != _compTypeRefComps.end())
            {
                auto &comps = iterComps->second;
                const Int32 countComp = static_cast<Int32>(comps.size());
                for(Int32 idxComp = countComp - 1; idxComp >= 0; --idxComp)
                {
                    if(comps[idxComp] != comp)
                        continue;

                    comps.erase(comps.begin() + idxComp);
                }

                if(comps.empty())
                    _compTypeRefComps.erase(iterComps);
            }
        }
    }

    {// focus
        const Int32 cn = static_cast<Int32>(_focusTypeRefComps.size());

        for(Int32 idx = cn - 1; idx >= 0; --idx)
        {
            auto &item = _focusTypeRefComps[idx];
            const Int32 cnItem = static_cast<Int32>(item.size());
            for(Int32 idxItem = cnItem - 1; idxItem >= 0; --idxItem)
            {
                auto o = item[idxItem];
                if(!o)
                    continue;

                if(o == comp)
                {
                    item.erase(item.begin() + idxItem);
                }
            }
        }
    }

    if(!IsAttached(comp))
    {
        comp->WillClose();
        comp->Close();
        comp->Release();
    }

    _attachedComps.erase(comp);
}

void CompHostObject::_MaskIfFocus(CompObject *comp)
{
    // attach 的组件不进行生命周期管理, 由comp的owner进行管理
    if(UNLIKELY(IsAttached(comp)))
        return;

    const Int32 maxFocusEnum = GetMaxFocusEnd();
    for(Int32 idx = ObjFocusInterfaceFlag::BEGIN; idx < maxFocusEnum; ++idx)
    {
        auto &focusComps = _focusTypeRefComps[idx];
        if(comp->IsFocus(idx))
            focusComps.push_back(comp);
    }
}

void CompHostObject::_Clear()
{
//     for(auto iter = _comps.rbegin(); iter != _comps.rend(); ++iter)
//     {
//         auto comp = *iter;
//         comp->Clear();
//     }

    // 释放
    _DestroyComps();
    _DestroyWillRegComps();

    _focusTypeRefComps.clear();
    _attachedComps.clear();

    if(g_Log->IsEnable(LogLevel::Info))
        g_Log->Info(LOGFMT_OBJ_TAG("_Clear CompHostObject host:%s over"), _objName.c_str());
}

void CompHostObject::_OnUpdateComps()
{
    auto &updateComps = _focusTypeRefComps[ObjFocusInterfaceFlag::ON_UPDATE];
    for(auto comp : updateComps)
        comp->OnUpdate();
}

KERNEL_END