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
 * Date: 2024-08-17 11:55:14
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Tls/TlsCompsOwner.h>
#include <kernel/comp/Log/log.h>
#include <kernel/comp/Poller/Poller.h>
#include <kernel/comp/Poller/PollerFactory.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(TlsCompsOwner);

TlsCompsOwner::TlsCompsOwner()
:_poller(NULL)
{

}

TlsCompsOwner::~TlsCompsOwner()
{
    _poller = NULL;
}

void TlsCompsOwner::Release()
{
    CRYSTAL_DELETE(this);
}

void TlsCompsOwner::OnRegisterComps()
{
    RegisterComp<PollerFactory>();
}

Int32 TlsCompsOwner::_OnCompsCreated()
{
    auto &comps = GetAllComps();
    for(auto &comp : comps)
        g_Log->Info(LOGFMT_OBJ_TAG("tls comps:%s"), comp->ToString().c_str());

    _poller = GetComp<Poller>();
    return Status::Success;
}

void TlsCompsOwner::_OnAttachedComp(CompObject *oldComp, CompObject *newComp)
{
    // 替换poller
    {
        const auto typeId = KERNEL_NS::RttiUtil::GetTypeId<Poller>();
        if(newComp && (newComp->GetObjTypeId() == typeId))
        {
            _poller = newComp->CastTo<Poller>();
            g_Log->Info(LOGFMT_OBJ_TAG("attach poller:%s, typeId:%llu"), _poller->ToString().c_str(), typeId);
        }
    }
}


OBJ_GET_OBJ_TYPEID_IMPL(TlsCompsOwner)


KERNEL_END
