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
 * Date: 2023-09-14 16:09:11
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <Comps/Library/impl/LibraryGlobal.h>
#include <Comps/Library/impl/LibraryGlobalStorageFactory.h>
#include <Comps/Library/impl/LibraryGlobalFactory.h>
#include <protocols/protocols.h>
#include <Comps/User/User.h>
#include <Comps/config/config.h>
#include <Comps/UserSys/UserSys.h>
#include <Comps/InviteCode/InviteCode.h>
#include <Comps/NickName/nickname.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(ILibraryGlobal);
POOL_CREATE_OBJ_DEFAULT_IMPL(LibraryGlobal);

LibraryGlobal::LibraryGlobal()
{

}

LibraryGlobal::~LibraryGlobal()
{
    _Clear();
}

void LibraryGlobal::Release()
{
    LibraryGlobal::DeleteByAdapter_LibraryGlobal(LibraryGlobalFactory::_buildType.V, this);
}

void LibraryGlobal::OnRegisterComps()
{
    RegisterComp<LibraryGlobalStorageFactory>();
}

Int32 LibraryGlobal::OnLoaded(UInt64 key, const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db)
{
    auto iter = _idRefLibraryInfo.find(key);
    if(iter != _idRefLibraryInfo.end())
    {
        g_Log->Error(LOGFMT_OBJ_TAG("repeated key:%llu"), key);
        return Status::Failed;
    }

    auto libraryInfo = CRYSTAL_NEW(LibraryInfo);
    if(UNLIKELY(!libraryInfo->Decode(db)))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("parse library info fail key:%llu"), key);
        return Status::ParseFail;
    }

    _idRefLibraryInfo.insert(std::make_pair(key, libraryInfo));

    // 成员信息字典
    {
        auto iterMemberDict = _libraryIdRefUserRefMember.insert(std::make_pair(libraryInfo->id(), std::map<UInt64, MemberInfo *>())).first;
        const Int32 memberCount = static_cast<Int32>(libraryInfo->memberlist_size());
        for(Int32 idx = 0; idx < memberCount; ++idx)
        {
            auto memberInfo = libraryInfo->mutable_memberlist(idx);
            iterMemberDict->second.insert(std::make_pair(memberInfo->userid(), memberInfo));
        }
    }

    return Status::Success;
}

Int32 LibraryGlobal::OnSave(UInt64 key, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) const
{
    auto iter = _idRefLibraryInfo.find(key);
    if(iter == _idRefLibraryInfo.end())
    {
        g_Log->Error(LOGFMT_OBJ_TAG("serialize library info fail key:%llu not found"), key);
        return Status::SerializeFail;
    }

    KERNEL_NS::LibString data;
    if(UNLIKELY(!iter->second->Encode(db)))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("serialize fail key:%llu"), key);
        return Status::SerializeFail;
    } 

    return Status::Success;
}

const LibraryInfo *LibraryGlobal::GetLibraryInfo(UInt64 libraryId) const
{
    auto iter = _idRefLibraryInfo.find(libraryId);
    return iter == _idRefLibraryInfo.end() ? NULL : iter->second;
}

LibraryInfo *LibraryGlobal::GetLibraryInfo(UInt64 libraryId)
{
    auto iter = _idRefLibraryInfo.find(libraryId);
    return iter == _idRefLibraryInfo.end() ? NULL : iter->second;
}

const MemberInfo *LibraryGlobal::GetMemberInfo(UInt64 libraryId, UInt64 userId) const
{
    auto iter = _libraryIdRefUserRefMember.find(libraryId);
    if(iter == _libraryIdRefUserRefMember.end())
        return NULL;

    auto iterMember = iter->second.find(userId);
    if(iterMember == iter->second.end())
        return NULL;

    return iterMember->second;
}

MemberInfo *LibraryGlobal::GetMemberInfo(UInt64 libraryId, UInt64 userId)
{
    auto iter = _libraryIdRefUserRefMember.find(libraryId);
    if(iter == _libraryIdRefUserRefMember.end())
        return NULL;

    auto iterMember = iter->second.find(userId);
    if(iterMember == iter->second.end())
        return NULL;

    return iterMember->second;
}

KERNEL_NS::LibString LibraryGlobal::LibraryToString(const LibraryInfo *libraryInfo) const
{
    if(UNLIKELY(!libraryInfo))
        return "";

    KERNEL_NS::LibString info;
    info.AppendFormat("library id:%llu, name:", libraryInfo->id())
        .AppendData(libraryInfo->name().data(), static_cast<Int64>(libraryInfo->name().size()))
        .AppendFormat(", member count:%d, manager count:%d"
        , libraryInfo->memberlist_size(), libraryInfo->managerinfolist_size());

    return info;
}

KERNEL_NS::LibString LibraryGlobal::LibraryToString(UInt64 libraryId) const
{
    auto libraryInfo = GetLibraryInfo(libraryId);
    return LibraryToString(libraryInfo);
}

Int32 LibraryGlobal::_OnGlobalSysInit()
{
    GetService()->Subscribe(Opcodes::OpcodeConst::OPCODE_GetLibraryInfoReq, this, &LibraryGlobal::_OnGetLibraryInfoReq);
    GetService()->Subscribe(Opcodes::OpcodeConst::OPCODE_GetLibraryListReq, this, &LibraryGlobal::_OnGetLibraryListReq);
    GetService()->Subscribe(Opcodes::OpcodeConst::OPCODE_CreateLibraryReq, this, &LibraryGlobal::_OnCreateLibraryReq);
    GetService()->Subscribe(Opcodes::OpcodeConst::OPCODE_JoinLibraryReq, this, &LibraryGlobal::_OnJoinLibraryReq);
    GetService()->Subscribe(Opcodes::OpcodeConst::OPCODE_QuitLibraryReq, this, &LibraryGlobal::_OnQuitLibraryReq);
    GetService()->Subscribe(Opcodes::OpcodeConst::OPCODE_TransferLibraianReq, this, &LibraryGlobal::_OnTransferLibraianReq);
    GetService()->Subscribe(Opcodes::OpcodeConst::OPCODE_ModifyMemberInfoReq, this, &LibraryGlobal::_OnModifyMemberInfoReq);

    return Status::Success;
}

Int32 LibraryGlobal::_OnGlobalSysCompsCreated()
{
    return Status::Success;
}

void LibraryGlobal::_OnGlobalSysClose()
{
    _Clear();
}

void LibraryGlobal::_OnGetLibraryInfoReq(KERNEL_NS::LibPacket *&packet)
{
    auto user = GetGlobalSys<IUserMgr>()->GetUserBySessionId(packet->GetSessionId());
    if(UNLIKELY(!user))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user not online packet:%s"), packet->ToString().c_str());
        return;
    }

    auto req = packet->GetCoder<GetLibraryInfoReq>();
    auto libraryMgr = user->GetSys<ILibraryMgr>();
    const auto myLibraryId = libraryMgr->GetMyLibraryId();
    Int32 errCode = Status::Success;
    if(myLibraryId != 0)
    {
        auto libraryInfo = GetLibraryInfo(myLibraryId);
        if(libraryInfo)
        {
            _SendLibraryInfoNty(user, libraryInfo);
        }
        else
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("librarty not found myLibraryId:%llu, user:%s"), myLibraryId, user->ToString().c_str());
            errCode = Status::NotFound;
        }
    }

    GetLibraryInfoRes res;
    res.set_errcode(errCode);
    user->Send(Opcodes::OpcodeConst::OPCODE_GetLibraryInfoRes, res, packet->GetPacketId());
}

void LibraryGlobal::_OnGetLibraryListReq(KERNEL_NS::LibPacket *&packet)
{
    auto user = GetGlobalSys<IUserMgr>()->GetUserBySessionId(packet->GetSessionId());
    if(UNLIKELY(!user))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user not online packet:%s"), packet->ToString().c_str());
        return;
    }

    auto req = packet->GetCoder<GetLibraryListReq>();

    GetLibraryListRes res;
    for(auto &iter : _idRefLibraryInfo)
    {
        auto libraryInfo = iter.second;
        auto newPreviewInfo = res.mutable_librarypreviewinfolist()->Add();
        _BuildPreviewInfo(newPreviewInfo, libraryInfo);
    }

    user->Send(Opcodes::OpcodeConst::OPCODE_GetLibraryListRes, res, packet->GetPacketId());
}

void LibraryGlobal::_OnCreateLibraryReq(KERNEL_NS::LibPacket *&packet)
{
    auto user = GetGlobalSys<IUserMgr>()->GetUserBySessionId(packet->GetSessionId());
    if(UNLIKELY(!user))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user not online packet:%s"), packet->ToString().c_str());
        return;
    }

    auto req = packet->GetCoder<CreateLibraryReq>();
    auto inviteCodeGlobal = GetGlobalSys<IInviteCodeGlobal>();
    const CommonConfig *isNeedInviteCodeConfig = GetGlobalSys<ConfigLoader>()->GetComp<CommonConfigMgr>()->GetConfigById(CommonConfigIdEnums::CREATE_LIBRARY_NEED_INVITE_CODE);
    auto isNeedInviteCode = (!isNeedInviteCodeConfig || (isNeedInviteCodeConfig->_value > 0));
    CreateLibraryRes res;
    Int32 errCode = Status::Success;
    res.set_errcode(errCode);
    do
    {
        // 是否已经有图书馆了
        auto libraryMgr = user->GetSys<ILibraryMgr>();
        if(libraryMgr->GetMyLibraryId() > 0)
        {
            errCode = Status::Repeat;
            g_Log->Warn(LOGFMT_OBJ_TAG("user has already joined library user:%s, packet:%s, library id:%llu")
                , user->ToString().c_str(), packet->ToString().c_str(), libraryMgr->GetMyLibraryId());
            break;
        }

        if(req->invitecode().empty())
        {
            errCode = Status::ParamError;
            g_Log->Warn(LOGFMT_OBJ_TAG("invite code cant be empty user:%s, packet:%s")
                    , user->ToString().c_str(), packet->ToString().c_str());
            break;
        }

        if(isNeedInviteCode)
        {
            if(inviteCodeGlobal->IsUsed(req->invitecode()))
            {
                errCode = Status::InvalidInviteCode;
                g_Log->Warn(LOGFMT_OBJ_TAG("invite code is used before user:%s, packet:%s, invitecode:%s")
                        , user->ToString().c_str(), packet->ToString().c_str(), req->invitecode().c_str());
                break;
            }

            if(!inviteCodeGlobal->IsValidCode(req->invitecode()))
            {
                errCode = Status::InvalidInviteCode;
                g_Log->Warn(LOGFMT_OBJ_TAG("invite code is invalid, user:%s, packet:%s, invitecode:%s")
                        , user->ToString().c_str(), packet->ToString().c_str(), req->invitecode().c_str());
                break;
            }
        }

        // 校验名字
        auto nicknameGlobal = GetGlobalSys<INicknameGlobal>();
        if(!req->name().empty())
        {
            if(!nicknameGlobal->CheckNickname(req->name()))
            {
                errCode = Status::InvalidName;
                g_Log->Warn(LOGFMT_OBJ_TAG("invalid library name user:%s, packet:%s, name:%s")
                    , user->ToString().c_str(), packet->ToString().c_str(), req->name().c_str());
                break;
            }
        }

        // 其他内容长度限制
        auto contentConfig = GetGlobalSys<ConfigLoader>()->GetComp<CommonConfigMgr>()->GetConfigById(CommonConfigIdEnums::CONTENT_LIMIT);
        if(static_cast<Int32>(req->address().size()) > contentConfig->_value)
        {
                errCode = Status::InvalidContent;
                g_Log->Warn(LOGFMT_OBJ_TAG("invalid library addr user:%s, packet:%s, address:%s")
                    , user->ToString().c_str(), packet->ToString().c_str(), req->address().c_str());
            break;
        }

        if(static_cast<Int32>(req->opentime().size()) > contentConfig->_value)
        {
                errCode = Status::InvalidContent;
                g_Log->Warn(LOGFMT_OBJ_TAG("invalid library addr user:%s, packet:%s, opentime:%s")
                    , user->ToString().c_str(), packet->ToString().c_str(), req->address().c_str());
            break;
        }

        if(static_cast<Int32>(req->telphonenumber().size()) > contentConfig->_value)
        {
            errCode = Status::InvalidContent;
            g_Log->Warn(LOGFMT_OBJ_TAG("invalid library addr user:%s, packet:%s, telphonenumber:%s")
                , user->ToString().c_str(), packet->ToString().c_str(), req->address().c_str());
            break;
        }

        // 馆长必须有手机号
        if(req->bindphone() == 0)
        {
            errCode = Status::NeedBindPhone;
            g_Log->Warn(LOGFMT_OBJ_TAG("user has already joined library user:%s, packet:%s, library id:%llu")
                , user->ToString().c_str(), packet->ToString().c_str(), libraryMgr->GetMyLibraryId());
            break;
        }

        // 手机必须是11位数
        KERNEL_NS::LibString phoneNumber;
        phoneNumber.AppendFormat("%llu", req->bindphone());
        if(phoneNumber.size() < 11)
        {
            errCode = Status::InvalidPhoneNubmer;
            g_Log->Warn(LOGFMT_OBJ_TAG("phone invalid:%llu, library user:%s, packet:%s, library id:%llu")
                , req->bindphone(), user->ToString().c_str(), packet->ToString().c_str(), libraryMgr->GetMyLibraryId());
            break;
        }

    } while (false);

    if(errCode == Status::Success)
    {
        if(isNeedInviteCode)
            inviteCodeGlobal->AddUsedInviteCode(req->invitecode());

        auto library = _CreateLibrary(user, req->name(), req->address(), req->opentime(), req->telphonenumber(), req->bindphone());
        if(!library)
        {
            errCode = Status::Failed;
            g_Log->Warn(LOGFMT_OBJ_TAG("create library fail user:%s, packet:%s"), user->ToString().c_str(), packet->ToString().c_str());
        }
    }

    res.set_errcode(errCode);
    user->Send(Opcodes::OpcodeConst::OPCODE_CreateLibraryRes, res, packet->GetPacketId());
}

void LibraryGlobal::_OnJoinLibraryReq(KERNEL_NS::LibPacket *&packet)
{
    auto user = GetGlobalSys<IUserMgr>()->GetUserBySessionId(packet->GetSessionId());
    if(UNLIKELY(!user))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user not online packet:%s"), packet->ToString().c_str());
        return;
    }

    auto req = packet->GetCoder<JoinLibraryReq>();
    Int32 errCode = Status::Success;
    const LibraryInfo *library = NULL;
    do
    {
        if(req->libraryid() == 0)
        {
            errCode = Status::ParamError;
            g_Log->Warn(LOGFMT_OBJ_TAG("lack library id param user:%s, packet:%s"), user->ToString().c_str(), packet->ToString().c_str());
            break;
        }

        // 是否已经是某个图书馆成员
        auto libraryMgr = user->GetSys<ILibraryMgr>();
        if(libraryMgr->GetMyLibraryId() > 0)
        {
            errCode = Status::AlreadyMemberOfLibrary;
            g_Log->Warn(LOGFMT_OBJ_TAG("already a member of library id:%llu cant join library:%llu param user:%s, packet:%s")
            , libraryMgr->GetMyLibraryId(), req->libraryid(), user->ToString().c_str(), packet->ToString().c_str());
            break;
        }

        // 图书馆是否存在
        auto libraryInfo = GetLibraryInfo(req->libraryid());
        if(!libraryInfo)
        {
            errCode = Status::LibraryNotFound;
            g_Log->Warn(LOGFMT_OBJ_TAG("library not found, library id:%llu, param user:%s, packet:%s")
            , req->libraryid(), user->ToString().c_str(), packet->ToString().c_str());
            break;
        }

        // 是不是已经在图书馆中
        auto memberInfo = GetMemberInfo(req->libraryid(), user->GetUserId());
        if(memberInfo)
        {
            errCode = Status::AlreadyMemberOfLibrary;
            g_Log->Warn(LOGFMT_OBJ_TAG("already a member in library but my library id is zero, please check, library id:%llu, param user:%s, packet:%s")
            , req->libraryid(), user->ToString().c_str(), packet->ToString().c_str());
            break;
        }

        _JoinMember(libraryInfo, user, RoleType_ENUMS_NoAuth);

        library = libraryInfo;

    } while (false);

    // 同步图书馆数据
    if(errCode == Status::Success)
        _SendLibraryInfoNty(user, library);

    JoinLibraryRes res;
    res.set_errcode(errCode);
    user->Send(Opcodes::OpcodeConst::OPCODE_JoinLibraryRes, res, packet->GetPacketId());
}

void LibraryGlobal::_OnQuitLibraryReq(KERNEL_NS::LibPacket *&packet)
{
    auto user = GetGlobalSys<IUserMgr>()->GetUserBySessionId(packet->GetSessionId());
    if(UNLIKELY(!user))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user not online packet:%s"), packet->ToString().c_str());
        return;
    }

    auto req = packet->GetCoder<QuitLibraryReq>();
    QuitLibraryRes res;

    Int32 errCode = Status::Success;
    do
    {
        // 1.需要有加入图书馆
        auto libraryMgr = user->GetSys<ILibraryMgr>();
        if(libraryMgr->GetMyLibraryId() == 0)
        {
            errCode = Status::NotJoinAnyLibrary;
            g_Log->Warn(LOGFMT_OBJ_TAG("not join any library user:%s"), user->ToString().c_str());
            break;
        }

        auto libraryInfo = GetLibraryInfo(libraryMgr->GetMyLibraryId());
        if(!libraryInfo)
        {
            errCode = Status::LibraryNotFound;
            g_Log->Warn(LOGFMT_OBJ_TAG("library not found, library id:%llu, param user:%s, packet:%s")
                , libraryMgr->GetMyLibraryId(), user->ToString().c_str(), packet->ToString().c_str());
            break;
        }

        // 2.馆长不可以退出, 只能转让
        if(libraryInfo->librarianuserid() == user->GetUserId())
        {
            errCode = Status::CantQuitLibrary;
            g_Log->Warn(LOGFMT_OBJ_TAG("libraian cant quit, library id:%llu, param user:%s, packet:%s")
                , libraryMgr->GetMyLibraryId(), user->ToString().c_str(), packet->ToString().c_str());
            break;
        }

        // 3.能不能操作
        if(!_CanHandle(libraryInfo->id(), user->GetUserId()))
        {
            errCode = Status::MemberIsLocked;
            g_Log->Warn(LOGFMT_OBJ_TAG("user is locked cant quit, library id:%llu, param user:%s, packet:%s")
                , libraryMgr->GetMyLibraryId(), user->ToString().c_str(), packet->ToString().c_str());
            break;
        }

        // 3.书没还完不可以退出
        auto memberInfo = GetMemberInfo(libraryInfo->id(), user->GetUserId());
        if(memberInfo)
        {
            if(!_IsReturnBackAllBook(memberInfo))
            {
                errCode = Status::HaveBookBorrowedNotReturnBack;
                g_Log->Warn(LOGFMT_OBJ_TAG("have book not return back user:%s, library info:%s"), LibraryToString(libraryInfo).c_str());
                break;
            }
        }

        // 4.删除成员数据
        _RemoveMember(libraryInfo, user);

        // 5.更新数据
        _SendLibraryInfoNty(user, libraryInfo);
    } while (false);

    res.set_errcode(errCode);
    user->Send(Opcodes::OpcodeConst::OPCODE_QuitLibraryRes, res, packet->GetPacketId());
}

void LibraryGlobal::_OnTransferLibraianReq(KERNEL_NS::LibPacket *&packet)
{
    auto user = GetGlobalSys<IUserMgr>()->GetUserBySessionId(packet->GetSessionId());
    if(UNLIKELY(!user))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user not online packet:%s"), packet->ToString().c_str());
        return;
    }

    const auto reqUserId = user->GetUserId();
    const auto packetId = packet->GetPacketId();

    auto req = packet->GetCoder<TransferLibraianReq>();  
    Int32 errCode = Status::Success;
    do
    {
        if((req->targetuserid() == 0) || (req->targetuserid() == user->GetUserId()))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("bad target user id:%llu, user:%s"), req->targetuserid(), user->ToString().c_str());
            errCode = Status::ParamError;
            break;
        } 

        // 得有图书馆
        auto libraryMgr = user->GetSys<ILibraryMgr>();
        if(libraryMgr->GetMyLibraryId() == 0)
        {
            errCode = Status::NotJoinAnyLibrary;
            g_Log->Warn(LOGFMT_OBJ_TAG("not join any library user:%s"), user->ToString().c_str());
            break;
        }

        auto libraryInfo = GetLibraryInfo(libraryMgr->GetMyLibraryId());
        if(!libraryInfo)
        {
            errCode = Status::LibraryNotFound;
            g_Log->Warn(LOGFMT_OBJ_TAG("library not found, library id:%llu, user:%s, packet:%s")
            , libraryMgr->GetMyLibraryId(), user->ToString().c_str(), packet->ToString().c_str());
            break;
        }

        // 自己是否被锁定
        if(!_CanHandle(libraryInfo->id(), user->GetUserId()))
        {
            errCode = Status::MemberIsLocked;
            g_Log->Warn(LOGFMT_OBJ_TAG("user is locked cant handle, library id:%llu, param user:%s, packet:%s")
                , libraryMgr->GetMyLibraryId(), user->ToString().c_str(), packet->ToString().c_str());
            break;
        }

        // 目标是否被锁定
        if(!_CanHandle(libraryInfo->id(), req->targetuserid()))
        {
            errCode = Status::MemberIsLocked;
            g_Log->Warn(LOGFMT_OBJ_TAG("target user is locked cant handle, library id:%llu, target user:%llu, user:%s, packet:%s")
                , libraryMgr->GetMyLibraryId(), req->targetuserid(), user->ToString().c_str(), packet->ToString().c_str());
            break;
        }

        // 得是馆长
        if(libraryInfo->librarianuserid() != user->GetUserId())
        {
            errCode = Status::NotLibrarian;
            g_Log->Warn(LOGFMT_OBJ_TAG("user not libarian, library id:%llu, param user:%s, packet:%s")
                , libraryMgr->GetMyLibraryId(), user->ToString().c_str(), packet->ToString().c_str());
            break;
        }

        // 目标玩家得是管理员
        auto targetmember = GetMemberInfo(libraryInfo->id(), req->targetuserid());
        if(!targetmember)
        {
            errCode = Status::NotMember;
            g_Log->Warn(LOGFMT_OBJ_TAG("target user not member, library id:%llu, param user:%s, packet:%s")
                , libraryMgr->GetMyLibraryId(), user->ToString().c_str(), packet->ToString().c_str());
            break;
        }

        // 不能是自己
        if(targetmember->userid() == user->GetUserId())
        {
            errCode = Status::ParamError;
            g_Log->Warn(LOGFMT_OBJ_TAG("target user cant be self, library id:%llu, user:%s, target user id:%llu packet:%s")
                , libraryMgr->GetMyLibraryId(), user->ToString().c_str(), targetmember->userid(), packet->ToString().c_str());
            break;
        }

        if(!_IsManager(targetmember->role()))
        {
            errCode = Status::NotManager;
            g_Log->Warn(LOGFMT_OBJ_TAG("target user not manager, library id:%llu, user:%s, target user role:%d,%s packet:%s")
                , libraryMgr->GetMyLibraryId(), user->ToString().c_str(), targetmember->role(), RoleType::ENUMS_Name(targetmember->role()).c_str()
                , packet->ToString().c_str());
            break;
        }

        // 转换
        auto memberInfo = GetMemberInfo(libraryInfo->id(), reqUserId);
        auto targetMemberInfo = GetMemberInfo(libraryInfo->id(), req->targetuserid());
        _TransferMember(libraryInfo, memberInfo, targetMemberInfo);
    } while (false);
    
    TransferLibraianRes res;
    res.set_errcode(errCode);
    user->Send(Opcodes::OpcodeConst::OPCODE_TransferLibraianRes, res, packetId);
}

void LibraryGlobal::_OnModifyMemberInfoReq(KERNEL_NS::LibPacket *&packet)
{
    auto userMgr = GetGlobalSys<IUserMgr>();
    auto user = userMgr->GetUserBySessionId(packet->GetSessionId());
    if(UNLIKELY(!user))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user not online packet:%s"), packet->ToString().c_str());
        return;
    }

    auto req = packet->GetCoder<ModifyMemberInfoReq>();  
    Int32 errCode = Status::Success;
    const auto packetId = packet->GetPacketId();
    
    do
    {
        auto libraryMgr = user->GetSys<ILibraryMgr>();
        if(libraryMgr->GetMyLibraryId() == 0)
        {
            errCode = Status::NotJoinAnyLibrary;
            g_Log->Warn(LOGFMT_OBJ_TAG("not join any library user:%s"), user->ToString().c_str());
            break;
        }

        auto libraryInfo = GetLibraryInfo(libraryMgr->GetMyLibraryId());
        if(!libraryInfo)
        {
            errCode = Status::LibraryNotFound;
            g_Log->Warn(LOGFMT_OBJ_TAG("not join any library user:%s"), user->ToString().c_str());
            break;
        }

        auto targetMember = GetMemberInfo(libraryInfo->id(), req->memberuserid());
        if(!targetMember)
        {
            errCode = Status::NotMember;
            g_Log->Warn(LOGFMT_OBJ_TAG("target not member target user id:%llu user:%s"), req->memberuserid(), user->ToString().c_str());
            break;
        }

        auto memberInfo = GetMemberInfo(libraryInfo->id(), user->GetUserId());
        if(!memberInfo)
        {
            errCode = Status::NotMember;
            g_Log->Warn(LOGFMT_OBJ_TAG("user not member library:%s, user:%s"), LibraryToString(libraryInfo).c_str(), user->ToString().c_str());
            break;
        }

        if(!_CanHandle(libraryInfo->id(), memberInfo->userid()))
        {
            errCode = Status::MemberIsLocked;
            g_Log->Warn(LOGFMT_OBJ_TAG("user is locked cant modify member, library id:%llu, user:%s, packet:%s")
                , libraryMgr->GetMyLibraryId(), user->ToString().c_str(), packet->ToString().c_str());
            break;
        }

        if(!_CanHandle(libraryInfo->id(), targetMember->userid()))
        {
            errCode = Status::MemberIsLocked;
            g_Log->Warn(LOGFMT_OBJ_TAG("target user is locked cant modify, target user id:%llu library id:%llu, user:%s, packet:%s")
                , req->memberuserid(), libraryMgr->GetMyLibraryId(), user->ToString().c_str(), packet->ToString().c_str());
            break;
        }

        if(targetMember->role() == RoleType_ENUMS_Librarian)
        {
            errCode = Status::AuthNotEnough;
            g_Log->Warn(LOGFMT_OBJ_TAG("target not member target user id:%llu user:%s"), req->memberuserid(), user->ToString().c_str());
            break;
        }

        if(!_IsManager(memberInfo->role()))
        {
            errCode = Status::NotManager;
            g_Log->Warn(LOGFMT_OBJ_TAG("user not manager library:%s, user:%s"), LibraryToString(libraryInfo).c_str(), user->ToString().c_str());
            break;
        }

        _LockMember(libraryInfo->id(), user->GetUserId());
        _LockMember(libraryInfo->id(), req->memberuserid());

        if(memberInfo->role() < targetMember->role())
        {
            errCode = Status::AuthNotEnough;
            g_Log->Warn(LOGFMT_OBJ_TAG("user auth not enough library:%s, member role:%d,%s, target member role:%d,%s user:%s")
            , LibraryToString(libraryInfo).c_str(), memberInfo->role(), RoleType::ENUMS_Name(memberInfo->role()).c_str()
            , targetMember->role(), RoleType::ENUMS_Name(targetMember->role()).c_str(), user->ToString().c_str());
            
            _UnlockMember(libraryInfo->id(), user->GetUserId());
            _UnlockMember(libraryInfo->id(), req->memberuserid());
            break;
        }

        // 目标用户不在就加载
        auto targetUser = userMgr->GetUser(req->memberuserid());
        if(!targetUser)
        {
            const auto libraryId = libraryInfo->id();
            const auto reqUserId = user->GetUserId();
            const auto targetUserId = targetMember->userid();

            const auto newReq = *req;
            errCode = userMgr->LoadUserBy(req->memberuserid(), [userMgr, libraryId, reqUserId, targetUserId, newReq, packetId, this](Int32 errCode, PendingUser *pending, IUser *targetUser, KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> &param){
                
                _UnlockMember(libraryId, reqUserId);
                _UnlockMember(libraryId, targetUserId);

                auto user = userMgr->GetUser(reqUserId);
                auto libraryInfo = GetLibraryInfo(libraryId);
                if((errCode != Status::Success) || !targetUser || !libraryInfo)     
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("load target user fail or have no library info, libraryId:%llu target user id:%llu reqUserId:%llu")
                    , libraryId, targetUserId, reqUserId);
                    if(user)
                    {
                        ModifyMemberInfoRes res;
                        res.set_errcode(errCode);
                        user->Send(Opcodes::OpcodeConst::OPCODE_ModifyMemberInfoRes, res, packetId);
                    }

                    return;
                }     

                errCode = _ContinueModifyMember(libraryInfo, reqUserId, targetUser, newReq);
                if(user)
                {
                    ModifyMemberInfoRes res;
                    res.set_errcode(errCode);
                    user->Send(Opcodes::OpcodeConst::OPCODE_ModifyMemberInfoRes, res, packetId);
                }
            });

            if(errCode != Status::Success)
            {
                errCode = Status::LoadUserFail;
                g_Log->Warn(LOGFMT_OBJ_TAG("user not manager library:%s, user:%s"), LibraryToString(libraryInfo).c_str(), user->ToString().c_str());
                break;
            }

            return;
        }

        _UnlockMember(libraryInfo->id(), user->GetUserId());
        _UnlockMember(libraryInfo->id(), req->memberuserid());

        errCode = _ContinueModifyMember(libraryInfo, user->GetUserId(), targetUser, *req);

    } while (false);
    
    ModifyMemberInfoRes res;
    res.set_errcode(errCode);
    user->Send(Opcodes::OpcodeConst::OPCODE_ModifyMemberInfoRes, res, packetId);
}

Int32 LibraryGlobal::_ContinueModifyMember(LibraryInfo *libraryInfo, UInt64 reqUserId, IUser *targetUser, const ModifyMemberInfoReq &req)
{
    auto memberInfo = GetMemberInfo(libraryInfo->id(), reqUserId);
    auto targetMember = GetMemberInfo(libraryInfo->id(), targetUser->GetUserId());

    // 修改角色
    if(req.has_newrole())
    {
        if(req.newrole() == RoleType_ENUMS_Librarian)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("user auth not enough library:%s, member role:%d,%s, target member role:%d,%s new role is librarian req user id:%llu")
            , LibraryToString(libraryInfo).c_str(), memberInfo->role(), RoleType::ENUMS_Name(memberInfo->role()).c_str()
            , targetMember->role(), RoleType::ENUMS_Name(targetMember->role()).c_str(), reqUserId);

            return Status::AuthNotEnough;
        }

        if(req.newrole() > memberInfo->role())
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("user auth not enough library:%s, member role:%d,%s, target member role:%d,%s new role is%d,%s req user id:%llu")
            , LibraryToString(libraryInfo).c_str(), memberInfo->role(), RoleType::ENUMS_Name(memberInfo->role()).c_str()
            , targetMember->role(), RoleType::ENUMS_Name(targetMember->role()).c_str(), req.newrole(), RoleType::ENUMS_Name(req.newrole()).c_str()
            , reqUserId);
            return Status::AuthNotEnough;
        }

        if(targetMember->role() == RoleType_ENUMS_NoAuth)
        {// 必须检查手机
            if(!_IsValidPhone(req.newmemberphone()))
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("user bind a invalid phone from no auth to new role:%d, phone number:%llu library:%s, member role:%d,%s, target member role:%d,%s req user id:%llu")
                ,req.newrole(), req.newmemberphone(), LibraryToString(libraryInfo).c_str(), memberInfo->role(), RoleType::ENUMS_Name(memberInfo->role()).c_str()
                , targetMember->role(), RoleType::ENUMS_Name(targetMember->role()).c_str(), reqUserId);
                return Status::InvalidPhoneNubmer;
            }
        }

        const auto oldRole = targetMember->role();
        targetMember->set_role(req.newrole());

        if(oldRole == RoleType_ENUMS_NoAuth)
        {// TODO:绑定手机
            targetUser->BindPhone(req.newmemberphone());
        }

        MaskNumberKeyModifyDirty(libraryInfo->id());
    }

    if(req.newmemberphone() > 0)
    {
        if(!_IsValidPhone(req.newmemberphone()))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("user bind a invalid phone from no auth to new role:%d, phone number:%llu library:%s, member role:%d,%s, target member role:%d,%s req user id:%llu")
            ,req.newrole(), req.newmemberphone(), LibraryToString(libraryInfo).c_str(), memberInfo->role(), RoleType::ENUMS_Name(memberInfo->role()).c_str()
            , targetMember->role(), RoleType::ENUMS_Name(targetMember->role()).c_str(), reqUserId);
            return Status::InvalidPhoneNubmer;
        }

        targetUser->BindPhone(req.newmemberphone());
        MaskNumberKeyModifyDirty(libraryInfo->id());
    }

    _SendLibraryInfoNty(reqUserId, libraryInfo);

    g_Log->Info(LOGFMT_OBJ_TAG("target member info is modified by user id:%llu, new target member:%s"), reqUserId, _MemberToString(targetMember).c_str());

    // TODO:系统日志

    return Status::Success;
}

KERNEL_NS::LibString LibraryGlobal::_MemberToString(const MemberInfo *memberInfo) const
{
    KERNEL_NS::LibString info;
    info.AppendFormat("user id:%llu, nickname:%s, role:%d,%s, borrow list:%d, locktime ms:%lld, cur time:%lld"
    , memberInfo->userid()
    , memberInfo->nickname().c_str()
    , memberInfo->role()
    , RoleType::ENUMS_Name(memberInfo->role()).c_str()
    , memberInfo->borrowlist_size()
    , memberInfo->locktimestampms()
    , KERNEL_NS::LibTime::NowMilliTimestamp()
    );

    return info;
}

LibraryInfo *LibraryGlobal::_CreateLibrary(IUser *user, const KERNEL_NS::LibString &libraryName, const KERNEL_NS::LibString &address, const KERNEL_NS::LibString &openTime, const KERNEL_NS::LibString &telphoneNumber, UInt64 bindPhone)
{
    auto newLibrary = new LibraryInfo;
    const auto id = GetGlobalSys<IGlobalUidMgr>()->NewGuid();
    newLibrary->set_id(id);

    auto nicknameGlobal = GetGlobalSys<INicknameGlobal>();
    if(libraryName.empty())
    {
        KERNEL_NS::LibString name;
        nicknameGlobal->GenRandNickname(name);
        newLibrary->set_name(name.GetRaw());
    }
    else
    {
        nicknameGlobal->AddUsedNickname(libraryName);
        newLibrary->set_name(libraryName.GetRaw());
    }

    // 基本信息
    newLibrary->set_address(address.GetRaw());
    newLibrary->set_opentime(openTime.GetRaw());
    newLibrary->set_telphonenumber(telphoneNumber.GetRaw());
    newLibrary->set_librarianuserid(user->GetUserId());
    newLibrary->set_librarianusernickname(user->GetUserBaseInfo()->nickname());
    _idRefLibraryInfo.insert(std::make_pair(newLibrary->id(), newLibrary));
    MaskNumberKeyAddDirty(newLibrary->id());

    // 绑定手机
    user->BindPhone(bindPhone);

    // 加入成员
    _JoinMember(newLibrary, user, RoleType_ENUMS_Librarian);

    // 图书馆信息
    this->_SendLibraryInfoNty(user, newLibrary);

    // TODO:系统日志
    g_Log->Info(LOGFMT_OBJ_TAG("create new library, create user:%s, new library:%s"), user->ToString().c_str(), newLibrary->ToJsonString().c_str());
    return newLibrary;
}

void LibraryGlobal::_JoinMember(LibraryInfo *libraryInfo, IUser *user, Int32 roleType)
{
    // 成员
    auto newMember = libraryInfo->add_memberlist();
    newMember->set_userid(user->GetUserId());
    newMember->set_role(roleType);
    newMember->set_nickname(user->GetNickname());
    auto iter = _libraryIdRefUserRefMember.find(libraryInfo->id());
    if(iter == _libraryIdRefUserRefMember.end())
        iter = _libraryIdRefUserRefMember.insert(std::make_pair(libraryInfo->id(), std::map<UInt64, MemberInfo *>())).first;
    iter->second.insert(std::make_pair(user->GetUserId(), newMember));

    // 管理员
    if(_IsManager(roleType))
    {
        // 管理员列表
        auto newManagerInfo = libraryInfo->mutable_managerinfolist()->Add();
        newManagerInfo->set_userid(user->GetUserId());
    }

    auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::JOIN_LIBRARY_MEMBER);
    ev->SetParam(Params::USER_ID, user->GetUserId());
    ev->SetParam(Params::USER_OBJ, user);
    ev->SetParam(Params::LIBRARY_ID, libraryInfo->id());
    GetEventMgr()->FireEvent(ev);
    
    MaskNumberKeyModifyDirty(libraryInfo->id());

    // TODO:系统日志
    g_Log->Info(LOGFMT_OBJ_TAG("join new member user:%s, roleType:%d,%s library:%s")
    , user->ToString().c_str()
    , roleType
    , RoleType::ENUMS_Name(roleType).c_str()
    , LibraryToString(libraryInfo).c_str());
}

bool LibraryGlobal::_IsReturnBackAllBook(const MemberInfo *memberInfo) const
{
    const Int32 len = memberInfo->borrowlist_size();
    for(Int32 idx = 0; idx < len; ++idx)
    {
        auto &orderInfo = memberInfo->borrowlist(idx);
        const Int32 bookListSize = orderInfo.borrowbooklist_size();
        for(Int32 bookIdx = 0; bookIdx < bookListSize; ++bookIdx)
        {
            auto &borrowBookInfo = orderInfo.borrowbooklist(bookIdx);
            if(borrowBookInfo.realgivebacktime() < borrowBookInfo.borrowtime())
                return false;
        }
    }

    return true;
}

bool LibraryGlobal::_RemoveMember(LibraryInfo *libraryInfo, UInt64 userId)
{
    if(libraryInfo->librarianuserid() == userId)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("librarian cant be removed userId:%llu, library:%s"), userId, LibraryToString(libraryInfo).c_str());
        return false;
    }

    auto iterMembers = _libraryIdRefUserRefMember.find(libraryInfo->id());
    if(iterMembers != _libraryIdRefUserRefMember.end())
        iterMembers->second.erase(userId);

    {
        const Int32 len = libraryInfo->memberlist_size();
        for(Int32 idx = 0; idx < len; ++idx)
        {
            if(libraryInfo->memberlist(idx).userid() == userId)
            {
                g_Log->Info(LOGFMT_OBJ_TAG("library will remove member library info:%s, member info:%s"), LibraryToString(libraryInfo).c_str(), libraryInfo->memberlist(idx).ToJsonString().c_str());
                libraryInfo->mutable_memberlist()->DeleteSubrange(idx, 1);
                break;
            }
        }
    }

    {
        const Int32 len = libraryInfo->managerinfolist_size();
        for(Int32 idx = 0; idx < len; ++idx)
        {
            if(libraryInfo->managerinfolist(idx).userid() == userId)
            {
                libraryInfo->mutable_managerinfolist()->DeleteSubrange(idx, 1);
                break;
            }
        } 
    }

    // TODO:系统日志
    MaskNumberKeyModifyDirty(libraryInfo->id());

    // 事件
    auto ev = KERNEL_NS::LibEvent::NewThreadLocal_LibEvent(EventEnums::REMOVE_LIBRARY_MEMBER);
    ev->SetParam(Params::USER_ID, userId);
    GetEventMgr()->FireEvent(ev);

    return true;
}

bool LibraryGlobal::_RemoveMember(LibraryInfo *libraryInfo, IUser *user)
{
    return _RemoveMember(libraryInfo, user->GetUserId());
}

bool LibraryGlobal::_IsManager(Int32 roleType) const
{
    switch (roleType)
    {
    case RoleType_ENUMS_Librarian:
    case RoleType_ENUMS_Manager:
    {
        return true;
    }
    default:
        break;
    }

    return false;
}

bool LibraryGlobal::_CanHandle(UInt64 libraryId, UInt64 userId) const
{
    auto memberInfo = GetMemberInfo(libraryId, userId);
    if(!memberInfo)
        return true;

    return memberInfo->locktimestampms() < KERNEL_NS::LibTime::Now().GetMilliTimestamp();
}

void LibraryGlobal::_LockMember(UInt64 libraryId, UInt64 userId, Int64 timeoutMs)
{
    auto memberInfo = GetMemberInfo(libraryId, userId);
    if(!memberInfo)
        return;

    memberInfo->set_locktimestampms(KERNEL_NS::LibTime::NowMilliTimestamp() + timeoutMs);
    MaskNumberKeyModifyDirty(libraryId);
}

void LibraryGlobal::_UnlockMember(UInt64 libraryId, UInt64 userId)
{
    auto memberInfo = GetMemberInfo(libraryId, userId);
    if(!memberInfo)
        return;

    memberInfo->set_locktimestampms(0);
    MaskNumberKeyModifyDirty(libraryId);
}

void LibraryGlobal::_TransferMember(LibraryInfo *libraryInfo, MemberInfo *memberInfo, MemberInfo *targetMember)
{
    // 只有管理员才能交换
    if(!_IsManager(memberInfo->role()))
        return;

    // 只有管理员才能交换
    if(!_IsManager(targetMember->role()))
        return;

    const auto role = memberInfo->role();
    const auto targetRole = targetMember->role();
    memberInfo->set_role(targetMember->role());
    targetMember->set_role(role);

    if(role == RoleType_ENUMS_Librarian)
    {
        libraryInfo->set_librarianuserid(targetMember->userid());
        libraryInfo->set_librarianusernickname(targetMember->nickname());
    }

    MaskNumberKeyModifyDirty(libraryInfo->id());

    g_Log->Info(LOGFMT_OBJ_TAG("library:%s, member user id:%llu transfer %d => %d"), LibraryToString(libraryInfo), memberInfo->userid(), role, targetRole);
    g_Log->Info(LOGFMT_OBJ_TAG("library:%s, member user id:%llu transfer %d => %d"), LibraryToString(libraryInfo), targetMember->userid(), targetRole, role);

    // TODO:系统日志

    _SendLibraryInfoNty(memberInfo->userid(), libraryInfo);
}

Int32 LibraryGlobal::_CheckTurnNormalMember(const LibraryInfo *libraryInfo, const IUser *user) const
{
    // 必须绑定手机
    if(user->GetUserBaseInfo()->bindphone() == 0)
        return Status::InvalidPhoneNubmer;

    // 手机是否合法
    if(!_IsValidPhone(user->GetUserBaseInfo()->bindphone()))
        return Status::InvalidPhoneNubmer;

    // 是否成员
    auto memberInfo = GetMemberInfo(libraryInfo->id(), user->GetUserId());
    if(!memberInfo)
        return Status::NotMember;

    return Status::Success;
}

bool LibraryGlobal::_IsValidPhone(UInt64 phoneNumber) const
{
    KERNEL_NS::LibString phone;
    phone.AppendFormat("%llu", phoneNumber);
    if(phone.length() < 11)
        return false;

    return true;
}

void LibraryGlobal::_BuildPreviewInfo(LibraryPreviewInfo *previewInfo, const LibraryInfo *libraryInfo) const
{
    previewInfo->set_id(libraryInfo->id());
    previewInfo->set_name(libraryInfo->name());
    previewInfo->set_librarianuserid(libraryInfo->librarianuserid());
    previewInfo->set_librariannickname(libraryInfo->librarianusernickname());
}

void LibraryGlobal::_SendLibraryInfoNty(const IUser *user, const LibraryInfo *libraryInfo) const
{
    auto libraryMgr = user->GetSys<ILibraryMgr>();

    LibraryInfoNty nty;
    auto newLibraryInfo = nty.mutable_libraryinfo();
    *newLibraryInfo = *libraryInfo;
    newLibraryInfo->clear_memberlist();

    // 获取角色
    auto memberInfo = GetMemberInfo(libraryInfo->id(), user->GetUserId());
    if(memberInfo)
    {
        switch(memberInfo->role())
        {
            case RoleType::ENUMS::RoleType_ENUMS_Librarian:
            {
                *newLibraryInfo->mutable_memberlist() = libraryInfo->memberlist();
            }break;
            case RoleType::ENUMS::RoleType_ENUMS_Manager:
            {
                *newLibraryInfo->mutable_memberlist() = libraryInfo->memberlist();
            }break;
            case RoleType::ENUMS::RoleType_ENUMS_NormalMember:
            {
                *newLibraryInfo->mutable_memberlist()->Add() = *memberInfo;
            }break;
            default:
            {
                *newLibraryInfo->mutable_memberlist()->Add() = *memberInfo;
            }break;
        }
    }

    user->Send(Opcodes::OpcodeConst::OPCODE_LibraryInfoNty, nty);
}

void LibraryGlobal::_SendLibraryInfoNty(UInt64 userId, const LibraryInfo *libraryInfo) const
{
    auto user = GetGlobalSys<IUserMgr>()->GetUser(userId);
    if(!user)
        return;

    _SendLibraryInfoNty(user, libraryInfo);
}

void LibraryGlobal::_Clear()
{
    KERNEL_NS::ContainerUtil::DelContainer2(_idRefLibraryInfo);
    _libraryIdRefUserRefMember.clear();
}

SERVICE_END
