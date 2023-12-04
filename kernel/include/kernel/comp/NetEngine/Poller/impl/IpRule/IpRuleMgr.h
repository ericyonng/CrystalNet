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
 * Date: 2022-04-16 22:08:14
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_IMPL_IP_RULE_IP_RULE_MGR_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_POLLER_IMPL_IP_RULE_IP_RULE_MGR_H__

#pragma once

#include <kernel/comp/CompObject/CompObjectInc.h>
#include <kernel/comp/BlackWhiteList/BlackWhiteList.h>

KERNEL_BEGIN

class KERNEL_EXPORT IpRuleMgr : public CompObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, IpRuleMgr);

public:
    IpRuleMgr();
    ~IpRuleMgr();
    virtual void Release() override;

    void Clear() override;
    LibString ToString() const override;

    bool SetBlackWhiteListFlag(UInt32 blackWhiteListFlag);
    bool Check(const LibString &ip) const;
    void PushWhite(const LibString &ip);
    void PushBlack(const LibString &ip);
    void EraseWhite(const LibString &ip);
    void EraseBlack(const LibString &ip);
    void Lock();
    void Unlock();

protected:
    Int32 _OnInit() override;
    Int32 _OnStart() override;
    void _OnWillClose() override;
    void _OnClose() override;
    
private:
    void _Clear();

    BlackWhiteList<LibString> _ipBlackWhiteList;
};

ALWAYS_INLINE bool IpRuleMgr::Check(const LibString &ip) const
{
    return _ipBlackWhiteList.Check(ip);
}

ALWAYS_INLINE void IpRuleMgr::PushWhite(const LibString &ip)
{
    _ipBlackWhiteList.PushWhite(ip);
}

ALWAYS_INLINE void IpRuleMgr::PushBlack(const LibString &ip)
{
    _ipBlackWhiteList.PushBlack(ip);
}

ALWAYS_INLINE void IpRuleMgr::EraseWhite(const LibString &ip)
{
    _ipBlackWhiteList.EraseWhite(ip);
}

ALWAYS_INLINE void IpRuleMgr::EraseBlack(const LibString &ip)
{
    _ipBlackWhiteList.EraseBlack(ip);
}

ALWAYS_INLINE void IpRuleMgr::Lock()
{
    _ipBlackWhiteList.Lock();
}

ALWAYS_INLINE void IpRuleMgr::Unlock()
{
    _ipBlackWhiteList.Unlock();
}

KERNEL_END

#endif
