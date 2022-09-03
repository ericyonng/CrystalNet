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

std::vector<CompObject *> CompHostObject::_emptyComps;

CompHostObject::CompHostObject()
:CompObject()
{

}

CompHostObject::~CompHostObject()
{
    _Clear();
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

    g_Log->Info(LOGFMT_OBJ_TAG("_OnInit CompHostObject suc"));
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
        return ret;
    }

    g_Log->Info(LOGFMT_OBJ_TAG("_OnStart suc host object"));
    return Status::Success;
}

void CompHostObject::_OnWillClose() 
{
    _OnHostBeforeCompsWillClose();
    _OnWillCloseComps();
    _OnHostWillClose();

    g_Log->Info(LOGFMT_OBJ_TAG("_OnWillClose CompHostObject suc"));
}

void CompHostObject::_OnClose()
{
    _OnHostBeforeCompsClose();
    _CloseComps();
    _OnHostClose();

    _Clear();
    CompObject::_OnClose();

    g_Log->Info(LOGFMT_OBJ_TAG("_OnClose CompHostObject suc"));
}

void CompHostObject::Clear()
{
    _Clear();
    CompObject::Clear();
}

void CompHostObject::OnUpdate()
{
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
    for(auto iter = _willRegComps.begin(); iter != _willRegComps.end(); ++iter)
    {
        if(iter->_factory == factory)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("repeat factory:%s, already register in will reg comps queue."), RttiUtil::GetByObj(factory));
            return Status::Repeat;
        }
    }

    _willRegComps.push_back(_WillRegComp(factory));
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
            g_Log->Warn(LOGFMT_OBJ_TAG("repeat comp:%s, already register in will reg comps queue."), RttiUtil::GetByObj(comp));
            return Status::Repeat;
        }
    }

    // 是否被注册过
    for(auto iter = _comps.begin(); iter != _comps.end(); ++iter)
    {
        if(*iter == comp)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("repeat comp:%s, already register in comps queue."), RttiUtil::GetByObj(comp));
            return Status::Repeat;
        }
    }

    _willRegComps.push_back(_WillRegComp(comp));
    return Status::Success;
}

Int32 CompHostObject::_OnCompsCreated()
{
    g_Log->Info(LOGFMT_OBJ_TAG("_OnCompsCreated host comp name:%s"), GetObjName().c_str());
    return Status::Success;
}

Int32 CompHostObject::_InitComps()
{
    bool initSuccess = true;
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
            g_Log->Error(LOGFMT_OBJ_TAG("comp created fail ret:%d, comp name:%s, host comp name:%s"), ret, RttiUtil::GetByObj(newComp), GetObjName().c_str());
            
            initSuccess = false;
            newComp->WillClose();
            newComp->Close();
            newComp->Release();
            newComp = NULL;

            _OnWillCloseComps();
            _CloseComps();
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
    Int32 ret = _OnCompsCreated();
    if(ret != Status::Success)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("comps created fail host comp name:%s, ret:%d"), GetObjName().c_str(), ret);
        _OnWillCloseComps();
        _CloseComps();
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
            return Status::Failed;
        }
    }

    g_Log->Info(LOGFMT_OBJ_TAG("init comps suc."));
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
            return Status::Failed;
        }
    }

    g_Log->Info(LOGFMT_OBJ_TAG("start comps suc."));
    return Status::Success;
}

void CompHostObject::_OnWillCloseComps()
{
    for(auto iter = _comps.rbegin(); iter != _comps.rend(); ++iter)
    {
        auto comp = *iter;
        if(!comp->IsStarted())
            continue;

        comp->WillClose();
    }

    // 等待所有组件结束
    CompObject *comp = NULL;
    for(;!IsAllCompsDown(comp);)
    {
        SystemUtil::ThreadSleep(1);
        g_Log->Warn(LOGFMT_OBJ_TAG("wait for all comps down current not down comp:%s"), comp->GetObjName().c_str());
    }
}

void CompHostObject::_CloseComps()
{
    for(auto iter = _comps.rbegin(); iter != _comps.rend(); ++iter)
    {
        auto comp = *iter;
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
}

void CompHostObject::_AddComp(CompObject *comp)
{
    _comps.push_back(comp);
    _compIdRefComp.insert(std::make_pair(comp->GetId(), comp));

    // 名字映射
    const auto &compName = comp->GetObjName();
    auto iterComps = _compNameRefComps.find(compName);
    if(iterComps == _compNameRefComps.end())
        iterComps = _compNameRefComps.insert(std::make_pair(compName, std::vector<CompObject *>())).first;
    auto &comps = iterComps->second;
    comps.push_back(comp);

    // 接口名字映射
    const LibString icompName = ConstantGather::interfacePrefix + compName.GetRaw();
    auto iterIComps = _icompNameRefComps.find(icompName);
    if(iterIComps == _icompNameRefComps.end())
        iterIComps = _icompNameRefComps.insert(std::make_pair(icompName, std::vector<CompObject *>())).first;
    auto &icomps = iterIComps->second;
    icomps.push_back(comp);

    // 类型映射
    if(comp->GetType())
    {
        auto iterTypeComps = _compTypeRefComps.find(comp->GetType());
        if(iterTypeComps == _compTypeRefComps.end())
            iterTypeComps = _compTypeRefComps.insert(std::make_pair(comp->GetType(), std::vector<CompObject *>())).first;
        iterTypeComps->second.push_back(comp);
    }
}

void CompHostObject::_MaskIfFocus(CompObject *comp)
{
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
    for(auto iter = _comps.rbegin(); iter != _comps.rend(); ++iter)
    {
        auto comp = *iter;
        comp->Clear();
    }

    // 释放
    ContainerUtil::DelContainer(_comps, [](CompObject *comp){
        comp->Release();
    });

    _compNameRefComps.clear();
    _icompNameRefComps.clear();
    _compIdRefComp.clear();

    _focusTypeRefComps.clear();
    _DestroyWillRegComps();
    g_Log->Info(LOGFMT_OBJ_TAG("_Clear CompHostObject"));
}

void CompHostObject::_OnUpdateComps()
{
    auto &updateComps = _focusTypeRefComps[ObjFocusInterfaceFlag::ON_UPDATE];
    for(auto comp : updateComps)
        comp->OnUpdate();

    g_Log->Info(LOGFMT_OBJ_TAG("_OnUpdateComps finish"));
}

KERNEL_END