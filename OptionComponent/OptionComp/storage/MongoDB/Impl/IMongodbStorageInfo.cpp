// MIT License
// 
// Copyright (c) 2020 ericyonng<120453674@qq.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// Date: 2026-06-28 22:06:41
// Author: Eric Yonng
// Description:
#include <pch.h>
#include <OptionComp/storage/MongoDB/Impl/IMongodbStorageInfo.h>
#include <kernel/comp/Utils/RttiUtil.h>

KERNEL_BEGIN

IMongodbStorageInfo::IMongodbStorageInfo(UInt64 objTypeId, const KERNEL_NS::LibString &rttiObjName)
    :CompHostObject(objTypeId)
    ,_systemName(KERNEL_NS::StringUtil::RemoveNameSpace(rttiObjName))
    ,_storageType(0)
    ,_flags(0)
    ,_numberSaveCb(NULL)
    ,_numberSteamSaveCb(NULL)
    ,_stringSaveCb(NULL)
    ,_stringSteamSaveCb(NULL)
    ,_asFieldSystemSaveCb(NULL)
{
    // 设置接口id
    SetInterfaceTypeId(KERNEL_NS::RttiUtil::GetTypeId<IMongodbStorageInfo>());
}

IMongodbStorageInfo::~IMongodbStorageInfo()
{
    CRYSTAL_RELEASE_SAFE(_numberSaveCb);
    CRYSTAL_RELEASE_SAFE(_numberSteamSaveCb);
    CRYSTAL_RELEASE_SAFE(_stringSaveCb);
    CRYSTAL_RELEASE_SAFE(_stringSteamSaveCb);
    CRYSTAL_RELEASE_SAFE(_asFieldSystemSaveCb);
}

Int32 IMongodbStorageInfo::_OnHostStart()
{
    // 作为其他系统的字段, 由其他系统的OnSave中去持久化
    if((!_numberSaveCb) && (!_stringSaveCb) && (!_numberSteamSaveCb) && (!_stringSteamSaveCb) && (!_asFieldSystemSaveCb))
    {
        CLOG_ERROR("storage have no onsave callback please check : %s", GetObjName().c_str());
        return Status::Failed;
    }
    
    return Status::Success;
}

KERNEL_END