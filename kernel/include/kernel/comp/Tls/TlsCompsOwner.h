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
 * Description: 注意TlsCompsOwner相关的组件,g_Log需要判空, 因为g_Log可能在TlCompsOwner初始化之后初始化, 详情见线程池的初始化, 线程池的初始化在g_Log之前,线程初始化的时候会初始化TlsCompsOwner
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TLS_TLS_COMPS_OWNER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_TLS_TLS_COMPS_OWNER_H__

#pragma once

#include <kernel/comp/Tls/ITlsObj.h>
#include <kernel/comp/Tls/Defs.h>
#include <kernel/comp/CompObject/CompHostObject.h>

KERNEL_BEGIN

class Poller;

class KERNEL_EXPORT TlsCompsOwner : public CompHostObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, TlsCompsOwner);

public:
    TlsCompsOwner();
    ~TlsCompsOwner();

    virtual void Release() override;

    virtual void OnRegisterComps() override;
    virtual Int32 _OnCompsCreated() override;
    virtual Int32 _OnHostWillStart() override;

    virtual void _OnAttachedComp(CompObject *oldComp, CompObject *newComp) override;

    Poller *GetPoller();    
    const Poller *GetPoller() const;   

private:
    Poller *_poller;
};

ALWAYS_INLINE Poller *TlsCompsOwner::GetPoller()
{
    return _poller;
}

ALWAYS_INLINE const Poller *TlsCompsOwner::GetPoller() const
{
    return _poller;
}   

KERNEL_END

#endif
