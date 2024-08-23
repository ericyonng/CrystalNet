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
 * Date: 2023-09-16 20:55:00
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <service_common/ServiceCommon.h>
#include <service/common/common.h>
#include <service/TestService/Common/ServiceCommon.h>

#include <Comps/UserSys/Library/impl/LibraryMgr.h>
#include <Comps/UserSys/Library/impl/LibraryMgrFactory.h>
#include <protocols/protocols.h>

SERVICE_BEGIN
POOL_CREATE_OBJ_DEFAULT_IMPL(ILibraryMgr);
POOL_CREATE_OBJ_DEFAULT_IMPL(LibraryMgr);

LibraryMgr::LibraryMgr()
:ILibraryMgr(KERNEL_NS::RttiUtil::GetTypeId<LibraryMgr>())
,_libraryInfo(new UserLibraryInfo)
,_removeLibraryMemberStub(INVALID_LISTENER_STUB)
,_joinLibraryMemberStub(INVALID_LISTENER_STUB)
{

}

LibraryMgr::~LibraryMgr()
{
    _Clear();
}

void LibraryMgr::Release()
{
    LibraryMgr::DeleteByAdapter_LibraryMgr(LibraryMgrFactory::_buildType.V, this);
}

Int32 LibraryMgr::OnLoaded(const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db)
{
    if(UNLIKELY(!_libraryInfo->FromJsonString(db.GetReadBegin(), static_cast<size_t>(db.GetReadableSize()))))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("parse library info fail user:%s"), GetUser()->ToString().c_str());
        return Status::ParseFail;
    }

    return Status::Success;
}

Int32 LibraryMgr::OnSave(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) const
{
    KERNEL_NS::LibString data;
    if(UNLIKELY(!_libraryInfo->ToJsonString(&(data.GetRaw()))))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("serialize as json string fail user:%s"), GetUser()->ToString().c_str());
        return Status::SerializeFail;
    }

    if(UNLIKELY(!db.Write(data.data(), static_cast<Int64>(data.size()))))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("write to stream fail user:%s"), GetUser()->ToString().c_str());
        return Status::SerializeFail;
    }

    return Status::Success;
}

void LibraryMgr::OnRegisterComps()
{

}

void LibraryMgr::OnLogin()
{
    if(g_Log->IsEnable(KERNEL_NS::LogLevel::Debug))
        g_Log->Debug(LOGFMT_OBJ_TAG("_libraryInfo:%s"), _libraryInfo->ToJsonString().c_str());
}

void LibraryMgr::OnLoginFinish()
{
    _SendUserLibraryInfo();
}

void LibraryMgr::OnLogout()
{

}

UInt64 LibraryMgr::GetMyLibraryId() const
{
    return _libraryInfo->libraryid();
}

Int32 LibraryMgr::_OnUserSysInit()
{
    _RegisterEvents();
    return Status::Success;
}

Int32 LibraryMgr::_OnHostStart()
{
    return Status::Success;
}

void LibraryMgr::_OnSysClose()
{
    _Clear();
}

void LibraryMgr::_Clear()
{
    CRYSTAL_RELEASE_SAFE(_libraryInfo);
    _UnRegisterEvents();
}

void LibraryMgr::_RegisterEvents()
{
    if(_removeLibraryMemberStub != INVALID_LISTENER_STUB)
        return;

    auto eventMgr = GetEventMgr();
    _removeLibraryMemberStub = eventMgr->AddListener(EventEnums::REMOVE_LIBRARY_MEMBER, this, &LibraryMgr::_OnRemoveLibraryMember);
    _joinLibraryMemberStub = eventMgr->AddListener(EventEnums::JOIN_LIBRARY_MEMBER, this, &LibraryMgr::_OnJoinLibraryMember);
}

void LibraryMgr::_UnRegisterEvents()
{
    if(_removeLibraryMemberStub == INVALID_LISTENER_STUB)
        return;

    auto eventMgr = GetEventMgr();
    eventMgr->RemoveListenerX(_removeLibraryMemberStub);
    eventMgr->RemoveListenerX(_joinLibraryMemberStub);
}

void LibraryMgr::_SendUserLibraryInfo() const
{
    UserLibraryInfoNty nty;
    *nty.mutable_userlibraryinfo() = *_libraryInfo;
    Send(Opcodes::OpcodeConst::OPCODE_UserLibraryInfoNty, nty);
}

void LibraryMgr::_OnQuitLibrary()
{
    g_Log->Info(LOGFMT_OBJ_TAG("user quit library user:%s, library id:%llu"), GetUser()->ToString().c_str(), static_cast<UInt64>(_libraryInfo->libraryid()));
    _libraryInfo->set_libraryid(0);
    MaskDirty();
    _SendUserLibraryInfo();

}

void LibraryMgr::_OnJoinLibrary(UInt64 libraryId)
{
    if(libraryId == 0)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("join a zero library user:%s"), _userOwner->ToString().c_str());
        return;
    }

    _libraryInfo->set_libraryid(libraryId);
    MaskDirty();
    _SendUserLibraryInfo();

    g_Log->Info(LOGFMT_OBJ_TAG("user join library user:%s, library id:%llu"), GetUser()->ToString().c_str(), libraryId);
}

void LibraryMgr::_OnRemoveLibraryMember(KERNEL_NS::LibEvent *ev)
{
    _OnQuitLibrary();
}

void LibraryMgr::_OnJoinLibraryMember(KERNEL_NS::LibEvent *ev)
{
    auto libraryId = ev->GetParam(Params::LIBRARY_ID).AsUInt64();
    _OnJoinLibrary(libraryId);
}


SERVICE_END