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
 * Date: 2022-04-16 22:43:20
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Log/log.h>
#include <kernel/comp/NetEngine/Poller/impl/IpRule/IpRuleMgrFactory.h>
#include <kernel/comp/NetEngine/Poller/impl/IpRule/IpRuleMgr.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(IpRuleMgr);

IpRuleMgr::IpRuleMgr()
{

}

IpRuleMgr::~IpRuleMgr()
{
    _Clear();
}

void IpRuleMgr::Release()
{
    IpRuleMgr::DeleteByAdapter_IpRuleMgr(IpRuleMgrFactory::_buildType.V, this);
}

Int32 IpRuleMgr::_OnInit() 
{
    auto ret = CompObject::_OnInit();
    if(ret != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("comp object init fail ret:%d"), ret);
        return ret;
    }

    g_Log->Debug(LOGFMT_OBJ_TAG("ip rule mgr inited."));
    return Status::Success;
}

Int32 IpRuleMgr::_OnStart()
{
    auto ret = CompObject::_OnStart();
    if(ret != Status::Success)
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("comp object start fail ret:%d"), ret);
        return ret;
    }

    return Status::Success;
}

void IpRuleMgr::_OnWillClose()
{
    CompObject::_OnWillClose();
}

void IpRuleMgr::_OnClose()
{
    _Clear();
    CompObject::_OnClose();
}

void IpRuleMgr::Clear()
{
    _Clear();
    CompObject::Clear();
}

LibString IpRuleMgr::ToString() const
{
    LibString info;
    info.AppendFormat("ip black list:");

    auto &blackList = _ipBlackWhiteList.GetBlackList();
    for(auto &ip:blackList)
        info.AppendFormat("%s;", ip.c_str());

    info.AppendFormat("\nip white list:");

    auto &whiteList = _ipBlackWhiteList.GetWhiteList();
    for(auto &ip:whiteList)
        info.AppendFormat("%s;", ip.c_str());   

    return info;
}

bool IpRuleMgr::SetBlackWhiteListFlag(UInt32 blackWhiteListFlag)
{
    if(!_ipBlackWhiteList.SetMode(blackWhiteListFlag))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("black white list set mode fail blackWhiteListFlag:%u"), blackWhiteListFlag);
        return false;
    }

    return true;
}

void IpRuleMgr::_Clear()
{
    _ipBlackWhiteList.Clear();
}

KERNEL_END
