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
 * Date: 2023-09-04 13:32:11
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <Comps/NickName/impl/NicknameGlobal.h>
#include <Comps/NickName/impl/NicknameGlobalStorageFactory.h>
#include <Comps/NickName/impl/NicknameGlobalFactory.h>
#include <Comps/config/config.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(INicknameGlobal);
POOL_CREATE_OBJ_DEFAULT_IMPL(NicknameGlobal);

NicknameGlobal::NicknameGlobal()
{

}

NicknameGlobal::~NicknameGlobal()
{

}

void NicknameGlobal::Release()
{
    NicknameGlobal::DeleteByAdapter_NicknameGlobal(NicknameGlobalFactory::_buildType.V, this);
}

void NicknameGlobal::OnRegisterComps()
{
    RegisterComp<NicknameGlobalStorageFactory>();
}

Int32 NicknameGlobal::OnLoaded(UInt64 key, const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db)
{
    KERNEL_NS::LibString nickname;
    const auto len = db.GetReadableSize();
    nickname.AppendData(db.GetReadBegin(), len);

    _AddName(key, nickname);

    return Status::Success;
}

Int32 NicknameGlobal::OnSave(UInt64 key, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) const
{
    auto iter = _idRefNickname.find(key);
    if(iter == _idRefNickname.end())
    {
        g_Log->Error(LOGFMT_OBJ_TAG("cant find key:%llu"), key);
        return Status::Failed;
    }

    db.Write(iter->second.data(), static_cast<Int64>(iter->second.length()));
    return Status::Success;
}

bool NicknameGlobal::CheckNickname(const KERNEL_NS::LibString &nickname) const
{
    // 名字长度限制
    auto maxlenConfig = GetService()->GetComp<ConfigLoader>()->GetComp<CommonConfigMgr>()->GetConfigById(CommonConfigIdEnums::NAME_MAX_LEN);
    if(static_cast<Int32>(nickname.length_with_utf8()) > maxlenConfig->_value)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("nickname too long:%s, utf8 len:%d"), nickname.c_str(),  static_cast<Int32>(nickname.length_with_utf8()));
        return false;
    }

    // 已经存在
    if(_IsExists(nickname))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("nickname already exists name:%s"), nickname.c_str());
        return false;
    }
    
    return true;
}
    
void NicknameGlobal::GenRandNickname(KERNEL_NS::LibString &newName)
{
    const auto id = GetGlobalSys<IGlobalUidMgr>()->NewGuid();
    newName.AppendFormat("nick_%llu", id);
    auto maxlenConfig = GetService()->GetComp<ConfigLoader>()->GetComp<CommonConfigMgr>()->GetConfigById(CommonConfigIdEnums::NAME_MAX_LEN);

    const Int32 utf8Len = static_cast<Int32>(newName.length_with_utf8());
    if(utf8Len >  maxlenConfig->_value)
        newName = newName.substr_with_utf8(0, maxlenConfig->_value);

    _AddName(id, newName);
    MaskNumberKeyAddDirty(id);
}

void NicknameGlobal::AddUsedNickname(const KERNEL_NS::LibString &nickname)
{
    if(_IsExists(nickname))
        return;

    const auto id = GetGlobalSys<IGlobalUidMgr>()->NewGuid();
    _AddName(id, nickname);
    MaskNumberKeyAddDirty(id);
}

void NicknameGlobal::_AddName(UInt64 id, const KERNEL_NS::LibString &name)
{
    _historyNicknames.insert(name);
    _idRefNickname.insert(std::make_pair(id, name));
}

bool NicknameGlobal::_IsExists(const KERNEL_NS::LibString &name) const
{
    return _historyNicknames.find(name) != _historyNicknames.end();
}

SERVICE_END
