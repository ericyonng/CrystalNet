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
#include <kernel/kernel.h>
#include <service_common/ServiceCommon.h>
#include <service/common/common.h>
#include <service/TestService/Common/ServiceCommon.h>

#include <Comps/Library/impl/LibraryGlobal.h>
#include <Comps/Library/impl/LibraryGlobalStorageFactory.h>
#include <Comps/Library/impl/LibraryGlobalFactory.h>
#include <protocols/protocols.h>
#include <Comps/User/User.h>
#include <Comps/config/config.h>
#include <Comps/UserSys/UserSys.h>
#include <Comps/InviteCode/InviteCode.h>
#include <Comps/NickName/nickname.h>
#include <Comps/Notify/Notify.h>
#include <Comps/SystemLog/SystemLog.h>

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

    const auto &nowTime = KERNEL_NS::LibTime::Now();
    _idRefLibraryInfo.insert(std::make_pair(key, libraryInfo));

    // 成员信息字典
    {
        auto iterMemberDict = _libraryIdRefUserRefMember.insert(std::make_pair(libraryInfo->id(), std::map<UInt64, MemberInfo *>())).first;
        const Int32 memberCount = static_cast<Int32>(libraryInfo->memberlist_size());
        for(Int32 idx = 0; idx < memberCount; ++idx)
        {
            auto memberInfo = libraryInfo->mutable_memberlist(idx);
            iterMemberDict->second.insert(std::make_pair(memberInfo->userid(), memberInfo));

            // 订单
            const Int32 orderCount = memberInfo->borrowlist_size();
            for(Int32 orderIdx = 0; orderIdx < orderCount; ++orderIdx)
            {
                auto orderInfo = memberInfo->mutable_borrowlist(orderIdx);
                _MakeOrderDict(libraryInfo->id(), orderInfo);

                if(orderInfo->orderstate() == BorrowOrderState_ENUMS_WAIT_USER_RECEIVE)
                {
                    const auto &getOverTime = KERNEL_NS::LibTime::FromMilliSeconds(orderInfo->getovertime());
                    _StartCacelOrderTimer(libraryInfo->id(), orderInfo->orderid(), (getOverTime - nowTime).GetTotalMilliSeconds());
                }
            }
        }
    }

    // 图书字典
    const Int32 bookCount = libraryInfo->booklist_size();
    for(Int32 idx = 0; idx < bookCount; ++idx)
    {
        auto bookInfo = libraryInfo->mutable_booklist(idx);
        _MakeBookDict(libraryInfo->id(), bookInfo);
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
    info.AppendFormat("library id:%llu, name:", static_cast<UInt64>(libraryInfo->id()))
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

const BookInfo *LibraryGlobal::GetBookInfo(UInt64 libraryId, UInt64 bookId) const
{
    auto iterLibrary = _libraryIdRefIdRefBookInfo.find(libraryId);
    if(iterLibrary == _libraryIdRefIdRefBookInfo.end())
        return NULL;

    auto iterBook = iterLibrary->second.find(bookId);
    return iterBook == iterLibrary->second.end() ? NULL : iterBook->second;
}

BookInfo *LibraryGlobal::GetBookInfo(UInt64 libraryId, UInt64 bookId)
{
    auto iterLibrary = _libraryIdRefIdRefBookInfo.find(libraryId);
    if(iterLibrary == _libraryIdRefIdRefBookInfo.end())
        return NULL;

    auto iterBook = iterLibrary->second.find(bookId);
    return iterBook == iterLibrary->second.end() ? NULL : iterBook->second;
}

Int32 LibraryGlobal::CreateBorrowOrder(UInt64 libraryId, const IUser *user, const BookBagInfo &bookBagInfo, const KERNEL_NS::LibString &remark)
{
    if(!remark.empty())
    {
        auto contentConfig = GetGlobalSys<ConfigLoader>()->GetComp<CommonConfigMgr>()->GetConfigById(CommonConfigIdEnums::CONTENT_LIMIT);
        if(!remark.IsUtf8())
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("bad remark content user:%s"), user->ToString().c_str());
            return Status::InvalidContent;
        }

        const Int32 remarkCount = static_cast<Int32>(remark.length_with_utf8());
        if((remarkCount < 0) || (remarkCount > contentConfig->_value))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("remark count too long, remarkCount:%d limit:%d, user:%s")
            , remarkCount, contentConfig->_value, user->ToString().c_str());
            return Status::ContentTooLong;
        }
    }

    // TODO
    auto memberUserId = user->GetUserId();
    auto memberInfo = GetMemberInfo(libraryId, memberUserId);
    if(!memberInfo)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("member info not found libraryId:%llu, user id:%llu"), libraryId, memberUserId);
        return Status::NotFound;
    }

    // 必须绑定手机
    if(!user->HasBindPhone())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("member not bind phone libraryId:%llu, user id:%llu"), libraryId, memberUserId);
        return Status::AuthNotEnough;
    }

    // 有借书权限
    if(!_DoesRoleHaveAuthToBorrow(memberInfo->role()))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("member have no auto phone libraryId:%llu, user id:%llu, member role:%d, %s"), libraryId, memberUserId, memberInfo->role(), RoleType_ENUMS_Name(memberInfo->role()).c_str());
        return Status::AuthNotEnough;
    }

    if(bookBagInfo.bookinfoitemlist_size() == 0)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("bookbag is empty libraryId:%llu, user id:%llu"), libraryId, memberUserId);
        return Status::Failed;
    }

    // 图书校验
    auto maxBorrowDaysConfig = GetService()->GetComp<ConfigLoader>()->GetComp<CommonConfigMgr>()->GetConfigById(CommonConfigIdEnums::MAX_BORROW_DAYS);
    
    std::map<UInt64, Int64> bookIdRefCount;
    for(auto &item : bookBagInfo.bookinfoitemlist())
    {
        // 是否存在
        auto bookInfo = GetBookInfo(libraryId, item.bookid());
        if(!bookInfo)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("book is not found book id:%llu libraryId:%llu, user id:%llu"), static_cast<UInt64>(item.bookid()), libraryId, memberUserId);
            return Status::BookNotFound;
        }

        // 是否超库存
        if((item.bookcount() == 0) || bookInfo->variantinfo().count() < static_cast<Int64>(item.bookcount()))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("book count is empty or over capacity,book id:%llu book count:%lld, will borrow count:%d libraryId:%llu, user id:%llu")
            , static_cast<UInt64>(item.bookid()), static_cast<Int64>(bookInfo->variantinfo().count()), item.bookcount(), libraryId, memberUserId);
            return Status::BookCountOverCapacity;
        }

        // 时间校验(是否为0, 是否超过最大时间)
        if(item.borrowdays() <= 0 || item.borrowdays() > maxBorrowDaysConfig->_value)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("borrowdays:[%d] over limit:[%d] book id:%llu, libraryId:%llu memberUserId:%llu")
            , item.borrowdays(), maxBorrowDaysConfig->_value
            ,  static_cast<UInt64>(item.bookid())
            , libraryId, memberUserId);
            return Status::ParamError;
        }

        auto iter = bookIdRefCount.find(bookInfo->id());
        if(iter == bookIdRefCount.end())
            iter = bookIdRefCount.insert(std::make_pair(bookInfo->id(), 0)).first;
        iter->second += static_cast<Int64>(item.bookcount());
    }

    for(auto iter : bookIdRefCount)
    {
        // 是否存在
        auto bookInfo = GetBookInfo(libraryId, iter.first);
        if(!bookInfo)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("book is not found book id:%llu libraryId:%llu, user id:%llu")
            , iter.first, libraryId, memberUserId);
            return Status::BookNotFound;
        }

        // 是否超库存
        if((iter.second == 0) || bookInfo->variantinfo().count() < static_cast<Int64>(iter.second))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("book count is empty or over capacity,book id:%llu book count:%lld, will borrow count:%lld libraryId:%lld, user id:%llu")
            , iter.first, static_cast<Int64>(bookInfo->variantinfo().count()), iter.second, libraryId, memberUserId);
            return Status::BookCountOverCapacity;
        }
    }

    // 有逾期不可继续借
    const auto &nowTime = KERNEL_NS::LibTime::Now();
    if(_HasOverDeadlineOrder(memberInfo, nowTime))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("hase over dead line order memver info:%s, now time:%s, library id:%llu, member user id:%llu")
        , memberInfo->ToJsonString().c_str(), nowTime.ToStringOfMillSecondPrecision().c_str(), libraryId,  memberUserId);
        return Status::HaveBookOverDeadline;
    }
    
    auto newOrderInfo = memberInfo->add_borrowlist();

    auto guidMgr = GetGlobalSys<IGlobalUidMgr>();
    auto orderId = guidMgr->NewGuid();
    newOrderInfo->set_orderid(orderId);
    newOrderInfo->set_createordertime(nowTime.GetMilliTimestamp());
    newOrderInfo->set_orderstate(BorrowOrderState_ENUMS_WAITING_OUT_OF_WAREHOUSE);

    if(!remark.empty())
        newOrderInfo->set_remark(remark.data(), remark.size());

    newOrderInfo->set_userid(user->GetUserId());

    for(auto &item : bookBagInfo.bookinfoitemlist())
    {
        auto bookInfo = GetBookInfo(libraryId, item.bookid());

        const auto bookCount = static_cast<Int64>(item.bookcount());
        auto newSubOrder = newOrderInfo->add_borrowbooklist();
        newSubOrder->set_bookid(item.bookid());
        newSubOrder->set_isbncode(bookInfo->isbncode());
        newSubOrder->set_borrowcount(item.bookcount());
        newSubOrder->set_borrowtime(nowTime.GetMilliTimestamp());
        newSubOrder->set_borrowdays(item.borrowdays());

        // const auto &planGiveBackTime = nowTime + KERNEL_NS::TimeSlice::FromSeconds(24 * 3600 * static_cast<Int64>(item.borrowdays()));
        // newSubOrder->set_plangivebacktime(planGiveBackTime.GetMilliTimestamp());

        newSubOrder->set_suborderid(guidMgr->NewGuid());

        // 扣库存
        const auto oldCount = bookInfo->mutable_variantinfo()->count();
        bookInfo->mutable_variantinfo()->set_count(oldCount > bookCount ? (oldCount - bookCount) : (bookCount - oldCount));
        bookInfo->set_borrowedcount(bookInfo->borrowedcount() + bookCount);
    }

    _MakeOrderDict(libraryId, newOrderInfo);

    MaskNumberKeyModifyDirty(libraryId);

    auto libarayrInfo = GetLibraryInfo(libraryId);
    _SendLibraryInfoNty(memberUserId, libarayrInfo);

    // 通知管理员有订单需要处理
    auto notifyGlobal = GetGlobalSys<INotifyGlobal>();

    for(auto &manager:libarayrInfo->managerinfolist())
    {
        // "{}申请借书, 用户id:{}, 订单号:{},请即时处理出库."
        std::vector<VariantParam> params;
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_STRING);
            param.set_strvalue(user->GetNickname());
            params.push_back(param);
        }
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
            param.set_unsignedvalue(user->GetUserId());
            params.push_back(param);
        }
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
            param.set_unsignedvalue(newOrderInfo->orderid());
            params.push_back(param);
        }

        notifyGlobal->SendNotify(manager.userid(), "HAVE_NEW_BORROW_APPLY_ORDER", {}, "BORROW_ORDER_CONTENT"
        , params);
    }

    // "用户:【{0}】, id:{1},角色:{2}({3}), 创建借书订单, 订单id:{4}, 订单状态:{5}({6}) 借图书列表:\n{7}"
    // 借书列表:BookName:%s, ISBN:%s, Count x %d, Days x %d
    auto sysLog = GetGlobalSys<ISystemLogGlobal>();
    std::vector<VariantParam> params;
    {
        VariantParam param;
        param.set_varianttype(VariantParamType_ENUMS_STRING);
        param.set_strvalue(user->GetNickname());
        params.push_back(param);
    }
    {
        VariantParam param;
        param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
        param.set_unsignedvalue(user->GetUserId());
        params.push_back(param);
    }
    {
        VariantParam param;
        param.set_varianttype(VariantParamType_ENUMS_VALUE);
        param.set_intvalue(static_cast<Int64>(memberInfo->role()));
        params.push_back(param);
    }
    {
        VariantParam param;
        param.set_varianttype(VariantParamType_ENUMS_STRING);
        param.set_strvalue(RoleType_ENUMS_Name(memberInfo->role()));
        params.push_back(param);
    }
    {
        VariantParam param;
        param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
        param.set_unsignedvalue(newOrderInfo->orderid());
        params.push_back(param);
    }
    {
        VariantParam param;
        param.set_varianttype(VariantParamType_ENUMS_VALUE);
        param.set_intvalue(static_cast<Int64>(newOrderInfo->orderstate()));
        params.push_back(param);
    }
    {
        VariantParam param;
        param.set_varianttype(VariantParamType_ENUMS_STRING);
        param.set_strvalue(BorrowOrderState_ENUMS_Name(newOrderInfo->orderstate()));
        params.push_back(param);
    }

    // 借书列表:BookName:%s, ISBN:%s, Count x %d, Days x %d
    KERNEL_NS::LibString borrowInfo;
    for(auto &subOrder : newOrderInfo->borrowbooklist())
    {
        auto bookInfo = GetBookInfo(libraryId, subOrder.bookid());
        borrowInfo.AppendFormat("BookName:");
        borrowInfo.AppendData(bookInfo->bookname());

        borrowInfo.AppendFormat(", ISBN:%s, Count x %d, BorrowDays x %d\n"
        , bookInfo->isbncode().c_str()
        , subOrder.borrowcount(), subOrder.borrowdays());
    }
    {
        VariantParam param;
        param.set_varianttype(VariantParamType_ENUMS_STRING);
        param.set_strvalue(borrowInfo.GetRaw());
        params.push_back(param);
    }
    sysLog->AddLog(libraryId, "CREATE_BORROW_ORDER_TITLE", {}, "CREATE_BORROW_ORDER_CONTENT", params);

    g_Log->Info(LOGFMT_OBJ_TAG("create order success, library id:%llu, borrow user:%llu, order info:%s"), libraryId, user->GetUserId(), newOrderInfo->ToJsonString().c_str());

    return Status::Success;
}

bool LibraryGlobal::IsManager(UInt64 libraryId, UInt64 userId) const
{
    return _IsManager(libraryId, userId);
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
    GetService()->Subscribe(Opcodes::OpcodeConst::OPCODE_GetLibraryMemberSimpleInfoReq, this, &LibraryGlobal::_OnGetLibraryMemberSimpleInfoReq);
    GetService()->Subscribe(Opcodes::OpcodeConst::OPCODE_AddLibraryBookReq, this, &LibraryGlobal::_OnAddLibraryBookReq);
    GetService()->Subscribe(Opcodes::OpcodeConst::OPCODE_GetBookInfoReq, this, &LibraryGlobal::_OnGetBookInfoReq);
    GetService()->Subscribe(Opcodes::OpcodeConst::OPCODE_GetBookByBookNameReq, this, &LibraryGlobal::_OnGetBookByBookNameReq);
    GetService()->Subscribe(Opcodes::OpcodeConst::OPCODE_GetBookInfoListReq, this, &LibraryGlobal::_OnGetBookInfoListReq);
    
    Subscribe(Opcodes::OpcodeConst::OPCODE_GetBookOrderDetailInfoReq, this, &LibraryGlobal::_OnGetBookOrderDetailInfoReq);
    Subscribe(Opcodes::OpcodeConst::OPCODE_OutStoreOrderReq, this, &LibraryGlobal::_OnOutStoreOrderReq);
    Subscribe(Opcodes::OpcodeConst::OPCODE_ManagerScanOrderForUserGettingBooksReq, this, &LibraryGlobal::_OnManagerScanOrderForUserGettingBooksReq);
    Subscribe(Opcodes::OpcodeConst::OPCODE_UserGetBooksOrderConfirmReq, this, &LibraryGlobal::_OnUserGetBooksOrderConfirmReq);
    Subscribe(Opcodes::OpcodeConst::OPCODE_CancelOrderReq, this, &LibraryGlobal::_OnCancelOrderReq);
    Subscribe(Opcodes::OpcodeConst::OPCODE_ReturnBackReq, this, &LibraryGlobal::_OnReturnBackReq);
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
    req->set_name(req->name());
    req->set_address(req->address());
    req->set_opentime(req->opentime());
    req->set_telphonenumber(req->telphonenumber());

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
        if((!req->address().empty()) && (!KERNEL_NS::StringUtil::IsUtf8String(req->address())))
        {
            errCode = Status::InvalidContent;
            g_Log->Warn(LOGFMT_OBJ_TAG("invalid library addr not utf8 user:%s, packet:%s, address:%s")
                , user->ToString().c_str(), packet->ToString().c_str(), req->address().c_str());
            break;
        }

        if(static_cast<Int32>(req->address().size()) > contentConfig->_value)
        {
                errCode = Status::InvalidContent;
                g_Log->Warn(LOGFMT_OBJ_TAG("invalid library addr user:%s, packet:%s, address:%s")
                    , user->ToString().c_str(), packet->ToString().c_str(), req->address().c_str());
            break;
        }

        if((!req->opentime().empty()) && (!KERNEL_NS::StringUtil::IsUtf8String(req->opentime())))
        {
            errCode = Status::InvalidContent;
            g_Log->Warn(LOGFMT_OBJ_TAG("invalid library opentime not utf8 user:%s, packet:%s")
                , user->ToString().c_str(), packet->ToString().c_str());
            break;
        }

        if(static_cast<Int32>(req->opentime().size()) > contentConfig->_value)
        {
                errCode = Status::InvalidContent;
                g_Log->Warn(LOGFMT_OBJ_TAG("invalid library addr user:%s, packet:%s, opentime:%s")
                    , user->ToString().c_str(), packet->ToString().c_str(), req->address().c_str());
            break;
        }

        if((!req->telphonenumber().empty()) && (!KERNEL_NS::StringUtil::IsUtf8String(req->telphonenumber())))
        {
            errCode = Status::InvalidContent;
            g_Log->Warn(LOGFMT_OBJ_TAG("invalid library telphonenumber not utf8 user:%s, packet:%s")
                , user->ToString().c_str(), packet->ToString().c_str());
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
        phoneNumber.AppendFormat("%llu", static_cast<UInt64>(req->bindphone()));
        if(phoneNumber.size() < 11)
        {
            errCode = Status::InvalidPhoneNubmer;
            g_Log->Warn(LOGFMT_OBJ_TAG("phone invalid:%llu, library user:%s, packet:%s, library id:%llu")
                , static_cast<UInt64>(req->bindphone()), user->ToString().c_str(), packet->ToString().c_str(), libraryMgr->GetMyLibraryId());
            break;
        }

        // 手机号必须没绑定过的
        auto userMgr = GetGlobalSys<IUserMgr>();
        bool hasBinded = false;
        if(!userMgr->IsPhoneNumberBinded(user, req->bindphone(), {user->GetUserId()}, hasBinded))
        {
            errCode = Status::DBError;
            g_Log->Warn(LOGFMT_OBJ_TAG("invoke IsPhoneNumberBinded error library user:%s, packet:%s, library id:%llu")
                , user->ToString().c_str(), packet->ToString().c_str(), libraryMgr->GetMyLibraryId());
            break;
        }

        if(hasBinded)
        {
            errCode = Status::NewPhoneIsBindedByOtherUser;
            g_Log->Warn(LOGFMT_OBJ_TAG("phone is binded by other user, phone:%llu, library user:%s, packet:%s, library id:%llu")
                , static_cast<UInt64>(req->bindphone()), user->ToString().c_str(), packet->ToString().c_str(), libraryMgr->GetMyLibraryId());
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
            , libraryMgr->GetMyLibraryId(), static_cast<UInt64>(req->libraryid()), user->ToString().c_str(), packet->ToString().c_str());
            break;
        }

        // 图书馆是否存在
        auto libraryInfo = GetLibraryInfo(req->libraryid());
        if(!libraryInfo)
        {
            errCode = Status::LibraryNotFound;
            g_Log->Warn(LOGFMT_OBJ_TAG("library not found, library id:%llu, param user:%s, packet:%s")
            , static_cast<UInt64>(req->libraryid()), user->ToString().c_str(), packet->ToString().c_str());
            break;
        }

        // 是不是已经在图书馆中
        auto memberInfo = GetMemberInfo(req->libraryid(), user->GetUserId());
        if(memberInfo)
        {
            errCode = Status::AlreadyMemberOfLibrary;
            g_Log->Warn(LOGFMT_OBJ_TAG("already a member in library but my library id is zero, please check, library id:%llu, param user:%s, packet:%s")
            , static_cast<UInt64>(req->libraryid()), user->ToString().c_str(), packet->ToString().c_str());
            break;
        }

        _JoinMember(libraryInfo, user, RoleType_ENUMS_NoAuth);

        library = libraryInfo;

        // "{0}加入图书馆, 用户id:{1}"
        auto sysLog = GetGlobalSys<ISystemLogGlobal>();
        std::vector<VariantParam> params;
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_STRING);
            param.set_strvalue(user->GetNickname());
            params.push_back(param);
        }
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
            param.set_unsignedvalue(user->GetUserId());
            params.push_back(param);
        }
        sysLog->AddLog(libraryInfo->id(), "JOIN_LIBRARY_LOG_TITLE", {}, "JOIN_LIBRARY_LOG_CONTENT", params);
    
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
                g_Log->Warn(LOGFMT_OBJ_TAG("have book not return back user:%s, library info:%s")
                , user->ToString().c_str(), LibraryToString(libraryInfo).c_str());
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
            g_Log->Warn(LOGFMT_OBJ_TAG("bad target user id:%llu, user:%s"), static_cast<UInt64>(req->targetuserid()), user->ToString().c_str());
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
                , libraryMgr->GetMyLibraryId(), static_cast<UInt64>(req->targetuserid()), user->ToString().c_str(), packet->ToString().c_str());
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
                , libraryMgr->GetMyLibraryId(), user->ToString().c_str(), static_cast<UInt64>(targetmember->userid()), packet->ToString().c_str());
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
            g_Log->Warn(LOGFMT_OBJ_TAG("target not member target user id:%llu user:%s"), static_cast<UInt64>(req->memberuserid()), user->ToString().c_str());
            break;
        }

        auto memberInfo = GetMemberInfo(libraryInfo->id(), user->GetUserId());
        if(!memberInfo)
        {
            errCode = Status::NotMember;
            g_Log->Warn(LOGFMT_OBJ_TAG("user not member library:%s, user:%s"), LibraryToString(libraryInfo).c_str(), user->ToString().c_str());
            break;
        }

        if(memberInfo->userid() == targetMember->userid())
        {
            if(req->has_newrole())
            {
                errCode = Status::AuthNotEnough;
                g_Log->Warn(LOGFMT_OBJ_TAG("cant modify self role target user id:%llu user:%s"), static_cast<UInt64>(req->memberuserid()), user->ToString().c_str());
                break;
            }
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
                , static_cast<UInt64>(req->memberuserid()), libraryMgr->GetMyLibraryId(), user->ToString().c_str(), packet->ToString().c_str());
            break;
        }

        if((targetMember->role() == RoleType_ENUMS_Librarian) &&
         (memberInfo->role() != RoleType_ENUMS_Librarian))
        {
            errCode = Status::AuthNotEnough;
            g_Log->Warn(LOGFMT_OBJ_TAG("target not member target user id:%llu user:%s"), static_cast<UInt64>(req->memberuserid()), user->ToString().c_str());
            break;
        }

        if(!_IsManager(memberInfo->role()))
        {
            errCode = Status::NotManager;
            g_Log->Warn(LOGFMT_OBJ_TAG("user not manager library:%s, user:%s"), LibraryToString(libraryInfo).c_str(), user->ToString().c_str());
            break;
        }

        if(req->has_newrole())
        {
            if(!RoleType_ENUMS_IsValid(req->newrole()))
            {
                errCode = Status::ParamError;
                g_Log->Warn(LOGFMT_OBJ_TAG("invalid role:%d, user:%s"), req->newrole(), user->ToString().c_str());
                break;
            }

            // 书还了么?
            if(req->newrole() == RoleType_ENUMS_NoAuth)
            {
                if(!_IsReturnBackAllBook(targetMember))
                {
                    errCode = Status::HaveBookBorrowedNotReturnBack;
                    g_Log->Warn(LOGFMT_OBJ_TAG("have book not return back user:%s, library info:%s")
                        , user->ToString().c_str(), LibraryToString(libraryInfo).c_str());
                    break;
                }
            }
        }

        if(req->has_newmemberphone())
        {
            if(!_IsValidPhone(req->newmemberphone()))
            {
                errCode = Status::InvalidPhoneNubmer;
                g_Log->Warn(LOGFMT_OBJ_TAG("invalid phone:%llu, user:%s"), static_cast<UInt64>(req->newmemberphone()), user->ToString().c_str());
                break;
            }

            bool newPhoneIsBinded = false;
            if(!userMgr->IsPhoneNumberBinded(user, req->newmemberphone(), {req->memberuserid()}, newPhoneIsBinded))
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("db error phone:%llu, user:%s"), static_cast<UInt64>(req->newmemberphone()), user->ToString().c_str());
                errCode = Status::DBError;
                break;
            }

            if(newPhoneIsBinded)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("new phone:%llu is binded by other user, user:%s"), static_cast<UInt64>(req->newmemberphone()), user->ToString().c_str());
                errCode = Status::NewPhoneIsBindedByOtherUser;
                break;
            }
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
            const auto libraryId = static_cast<UInt64>(libraryInfo->id());
            const auto reqUserId = user->GetUserId();
            const auto targetUserId = static_cast<UInt64>(targetMember->userid());

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

void LibraryGlobal::_OnGetLibraryMemberSimpleInfoReq(KERNEL_NS::LibPacket *&packet)
{
    auto userMgr = GetGlobalSys<IUserMgr>();
    auto user = userMgr->GetUserBySessionId(packet->GetSessionId());
    if(UNLIKELY(!user))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user not online packet:%s"), packet->ToString().c_str());
        return;
    }

    Int32 errCode = Status::Success;
    const auto packetId = packet->GetPacketId();

    KERNEL_NS::SmartPtr<std::set<UInt64>> memberUserIdsCopy = new std::set<UInt64>();
    KERNEL_NS::SmartPtr<::google::protobuf::RepeatedPtrField<SimpleUserInfo>> userInfos = new ::google::protobuf::RepeatedPtrField<SimpleUserInfo>();

    do
    {
        auto libraryMgr = user->GetSys<ILibraryMgr>();
        if(libraryMgr->GetMyLibraryId() == 0)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("have no library user:%s"), user->ToString().c_str());
            errCode = Status::NotJoinAnyLibrary;
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

        auto myMemberInfo = GetMemberInfo(libraryInfo->id(), user->GetUserId());
        if(!myMemberInfo)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("not library member user:%s, library id:%llu")
            , user->ToString().c_str(), static_cast<UInt64>(libraryInfo->id()));
            errCode = Status::NotJoinAnyLibrary;
            break;
        }

        const Int32 myRole = myMemberInfo->role();

        auto &memberList = libraryInfo->memberlist();
        std::set<UInt64> memberUserIds;
        for(auto &member : memberList)
        {
            memberUserIds.insert(member.userid());
            memberUserIdsCopy->insert(member.userid());
        }

        auto userMgr = GetGlobalSys<IUserMgr>();
        const auto reqUserId = user->GetUserId();
        for(auto userId : memberUserIds)
        {
            auto memberUser = userMgr->GetUser(userId);
            if(!memberUser)
            {
                userMgr->LoadUserBy(userId, [userMgr, packetId, memberUserIdsCopy, userInfos, myRole, reqUserId, userId, this](Int32 errCode, PendingUser *pending, IUser *targetUser, KERNEL_NS::SmartPtr<KERNEL_NS::Variant, KERNEL_NS::AutoDelMethods::CustomDelete> &param) mutable
                {
                    memberUserIdsCopy->erase(userId);
                    do
                    {
                        if(!targetUser)
                        {
                            g_Log->Warn(LOGFMT_OBJ_TAG("load user fail user id:%llu"), userId);
                            break;
                        }

                        auto userInfo = userInfos->Add();
                        userInfo->set_userid(userId);
                        userInfo->set_nickname(targetUser->GetNickname());

                        if(_IsManager(myRole) || (reqUserId == userId))
                            userInfo->set_bindphone(targetUser->GetUserBaseInfo()->bindphone());
                        
                    } while (false);

                    if(memberUserIdsCopy->empty())
                    {
                        auto user = userMgr->GetUser(reqUserId);
                        if(user)
                        {
                            GetLibraryMemberSimpleInfoRes res;
                            res.set_errcode(Status::Success);
                            *res.mutable_simpleuserinfolist() = *userInfos.AsSelf();
                            user->Send(Opcodes::OpcodeConst::OPCODE_GetLibraryMemberSimpleInfoRes, res, packetId);
                        }
                    }
                });
                continue;
            }

            auto userInfo = userInfos->Add();
            userInfo->set_userid(userId);
            userInfo->set_nickname(memberUser->GetNickname());

            if(_IsManager(myMemberInfo->role()) || (reqUserId == memberUser->GetUserId()))
                userInfo->set_bindphone(memberUser->GetUserBaseInfo()->bindphone());
            memberUserIdsCopy->erase(userId);
        }

    }while(false);


    GetLibraryMemberSimpleInfoRes res;
    res.set_errcode(errCode);
    if(errCode != Status::Success)
    {
        user->Send(Opcodes::OpcodeConst::OPCODE_GetLibraryMemberSimpleInfoRes, res, packetId);
        return;
    }

    if(memberUserIdsCopy->empty())
    {
        *res.mutable_simpleuserinfolist() = *userInfos.AsSelf();
        user->Send(Opcodes::OpcodeConst::OPCODE_GetLibraryMemberSimpleInfoRes, res, packetId);
    }
}

void LibraryGlobal::_OnAddLibraryBookReq(KERNEL_NS::LibPacket *&packet)
{
    auto userMgr = GetGlobalSys<IUserMgr>();
    auto user = userMgr->GetUserBySessionId(packet->GetSessionId());
    if(UNLIKELY(!user))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user not online packet:%s"), packet->ToString().c_str());
        return;
    }

    auto contentConfig = GetGlobalSys<ConfigLoader>()->GetComp<CommonConfigMgr>()->GetConfigById(CommonConfigIdEnums::CONTENT_LIMIT);
    auto imageSizeLimitConfig = GetGlobalSys<ConfigLoader>()->GetComp<CommonConfigMgr>()->GetConfigById(CommonConfigIdEnums::MAX_IMAGE_SIZE);
    const size_t contentMaxLen = static_cast<size_t>(contentConfig->_int64Value);
    const size_t imageMaxSize = static_cast<size_t>(imageSizeLimitConfig->_int64Value);
    auto bookContentConfig = GetGlobalSys<ConfigLoader>()->GetComp<CommonConfigMgr>()->GetConfigById(CommonConfigIdEnums::BOOK_CONTENT_LIMIT);
    const size_t maxBookContentLen = static_cast<size_t>(bookContentConfig->_int64Value);
    auto keyWordsConfig = GetGlobalSys<ConfigLoader>()->GetComp<CommonConfigMgr>()->GetConfigById(CommonConfigIdEnums::KEYWORDS_LIMIT);
    auto snapshotConfig = GetGlobalSys<ConfigLoader>()->GetComp<CommonConfigMgr>()->GetConfigById(CommonConfigIdEnums::BOOK_SNAP_SHOT_MAX_LIMIT);
    Int32 err = Status::Success;

    do
    {
        auto req = packet->GetCoder<AddLibraryBookReq>();
        if((req->isbncode().size() >= contentMaxLen) || req->isbncode().empty())
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("isbn too long:%llu user:%s"), static_cast<UInt64>(req->isbncode().size()), user->ToString().c_str());
            err = Status::ParamError;
            break;
        }
        auto myLibraryMgr = user->GetSys<ILibraryMgr>();

        // 用户有没权限
        auto memberInfo = GetMemberInfo(myLibraryMgr->GetMyLibraryId(), user->GetUserId());
        if(!memberInfo || !_IsManager(memberInfo->role()))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("auth not enough role:%d, user:%s"), memberInfo ? memberInfo->role() : -1, user->ToString().c_str());
            err = Status::AuthNotEnough;
            break;
        }

        // 数量
        if(req->modifycount() > static_cast<Int64>((std::numeric_limits<Int32>::max)()) || 
        req->modifycount() < static_cast<Int64>((std::numeric_limits<Int32>::min)()))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("bad modify count:%lld user:%s"), static_cast<Int64>(req->modifycount()), user->ToString().c_str());
            err = Status::AuthNotEnough;
            break;
        }

        auto libarayInfo = GetLibraryInfo(myLibraryMgr->GetMyLibraryId());

        // 是不是已经存在了
        auto bookInfo = _GetBookInfo(libarayInfo->id(), req->isbncode());
        KERNEL_NS::LibString oldBookName;
        Int64 oldPrice = 0;
        if(bookInfo)
        {
            oldBookName = bookInfo->bookname();
            oldPrice = bookInfo->variantinfo().price();

            if(!req->bookname().empty())
            {
                if(!KERNEL_NS::StringUtil::IsUtf8String(req->bookname()))
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("not utf8 user:%s"), user->ToString().c_str());
                    err = Status::ParamError;
                    break;
                }

                if((req->bookname().size() >= contentMaxLen) || req->bookname().empty())
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("book name too long:%llu user:%s"), static_cast<UInt64>(req->bookname().size()), user->ToString().c_str());
                    err = Status::ParamError;
                    break;
                }
            }

            if(req->has_bookcoverimage())
            {
                {
                    if((req->bookcoverimage().size() * 3 / 4) >= imageMaxSize)
                    {
                        g_Log->Warn(LOGFMT_OBJ_TAG("cover image too large :%llu user:%s"), static_cast<UInt64>(req->bookcoverimage().size()), user->ToString().c_str());
                        err = Status::ImageTooLarge;
                        break;
                    }
                }
            }

            if(req->has_content())
            {
                const KERNEL_NS::LibString &copyContent = req->content();
                const auto currentLen = copyContent.length_with_utf8();
                if((currentLen * 3 / 4) > maxBookContentLen)
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("content too long :%llu limit:%llu user:%s")
                    , static_cast<UInt64>(currentLen), static_cast<UInt64>(maxBookContentLen), user->ToString().c_str());
                    err = Status::ContentTooLong;
                    break;
                }
            }

            if(req->has_keywords())
            {
                if(req->keywords().keywords_size() > keyWordsConfig->_value)
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("keywords too much :%d limit:%d user:%s")
                    , req->keywords().keywords_size(), keyWordsConfig->_value, user->ToString().c_str());
                    err = Status::KeyWordsTooMuch;
                    break;
                }

                for(auto &keyword : req->keywords().keywords())
                {
                    if(keyword.size() > contentMaxLen)
                    {
                        g_Log->Warn(LOGFMT_OBJ_TAG("keyword too long :%llu limit:%llu user:%s")
                        , static_cast<UInt64>(keyword.size()), static_cast<UInt64>(contentMaxLen), user->ToString().c_str());
                        err = Status::ContentTooLong;
                        break;
                    }
                }

                if(err != Status::Success)
                {
                    break;
                }
            }

            if(req->has_snapshot())
            {
                if(req->snapshot().snapshots_size() > snapshotConfig->_value)
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("snapshot too much :%d limit:%d user:%s")
                    , req->snapshot().snapshots_size(), snapshotConfig->_value, user->ToString().c_str());
                    err = Status::KeyWordsTooMuch;
                    break;
                }

                for(auto &snapshot : req->snapshot().snapshots())
                {
                    if((snapshot.size() * 3 / 4) >= imageMaxSize)
                    {
                        g_Log->Warn(LOGFMT_OBJ_TAG("snapshot image too large :%llu user:%s"), static_cast<UInt64>(snapshot.size()), user->ToString().c_str());
                        err = Status::ImageTooLarge;
                        break;
                    }
                }

                if(err != Status::Success)
                {
                    break;
                }
            }

            if(!req->bookname().empty())
                bookInfo->set_bookname(req->bookname());

            if(req->has_bookcoverimage())
                bookInfo->set_bookcoverimage(req->bookcoverimage());

            if(req->price() > 0)
                bookInfo->mutable_variantinfo()->set_price(req->price());

            if(req->has_keywords())
                *bookInfo->mutable_keywords() = req->keywords().keywords();

            if(req->has_content())
                bookInfo->set_content(req->content());

            if(req->has_snapshot())
                *bookInfo->mutable_snapshot() = req->snapshot().snapshots();
        }
        else
        {
            if(!KERNEL_NS::StringUtil::IsUtf8String(req->bookname()))
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("not utf8 user:%s"), user->ToString().c_str());
                err = Status::ParamError;
                break;
            }

            if((req->bookname().size() >= contentMaxLen) || req->bookname().empty())
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("book name too long:%llu user:%s"), static_cast<UInt64>(req->bookname().size()), user->ToString().c_str());
                err = Status::ParamError;
                break;
            }

            if(req->has_bookcoverimage())
            {
                if((req->bookcoverimage().size() * 3 / 4) >= imageMaxSize)
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("cover image too large :%llu user:%s"), static_cast<UInt64>(req->bookcoverimage().size()), user->ToString().c_str());
                    err = Status::ImageTooLarge;
                    break;
                }
            }

            if(req->price() <= 0)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("price error :%lld user:%s"), static_cast<Int64>(req->price()), user->ToString().c_str());
                err = Status::ParamError;
                break;
            }
            
            if(req->has_content())
            {
                const KERNEL_NS::LibString &copyContent = req->content();
                const auto currentLen = copyContent.length_with_utf8();
                if((currentLen * 3 / 4) > maxBookContentLen)
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("content too long :%llu limit:%llu user:%s")
                    , static_cast<UInt64>(currentLen), static_cast<UInt64>(maxBookContentLen), user->ToString().c_str());
                    err = Status::ContentTooLong;
                    break;
                }
            }

            if(req->has_keywords())
            {
                if(req->keywords().keywords_size() > keyWordsConfig->_value)
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("keywords too much :%d limit:%d user:%s")
                    , req->keywords().keywords_size(), keyWordsConfig->_value, user->ToString().c_str());
                    err = Status::KeyWordsTooMuch;
                    break;
                }

                for(auto &keyword : req->keywords().keywords())
                {
                    if(keyword.size() > contentMaxLen)
                    {
                        g_Log->Warn(LOGFMT_OBJ_TAG("keyword too long :%llu limit:%llu user:%s")
                        , static_cast<UInt64>(keyword.size()), static_cast<UInt64>(contentMaxLen), user->ToString().c_str());
                        err = Status::ContentTooLong;
                        break;
                    }
                }
                if(err != Status::Success)
                {
                    break;
                }
            }

            if(req->has_snapshot())
            {
                if(req->snapshot().snapshots_size() > snapshotConfig->_value)
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("snapshot too much :%d limit:%d user:%s")
                    , req->snapshot().snapshots_size(), snapshotConfig->_value, user->ToString().c_str());
                    err = Status::KeyWordsTooMuch;
                    break;
                }

                for(auto &snapshot : req->snapshot().snapshots())
                {
                    if((snapshot.size() * 3 / 4) >= imageMaxSize)
                    {
                        g_Log->Warn(LOGFMT_OBJ_TAG("snapshot image too large :%llu user:%s"), static_cast<UInt64>(snapshot.size()), user->ToString().c_str());
                        err = Status::ImageTooLarge;
                        break;
                    }
                }

                if(err != Status::Success)
                {
                    break;
                }
            }

            auto newBook = libarayInfo->add_booklist();
            auto guidMgr = GetGlobalSys<IGlobalUidMgr>();
            newBook->set_id(guidMgr->NewGuid());
            newBook->set_bookname(req->bookname());
            newBook->set_isbncode(req->isbncode());

            if(req->has_bookcoverimage())
                newBook->set_bookcoverimage(req->bookcoverimage());
            newBook->mutable_variantinfo()->set_price(req->price());

            if(req->has_keywords())
                *newBook->mutable_keywords() = req->keywords().keywords();

            if(req->has_content())
                newBook->set_content(req->content());

            if(req->has_snapshot())
                *newBook->mutable_snapshot() = req->snapshot().snapshots();

            _MakeBookDict(libarayInfo->id(), newBook);
            bookInfo = newBook;
        }

        const auto originalCount = bookInfo->mutable_variantinfo()->count();
        bookInfo->mutable_variantinfo()->set_count(originalCount + req->modifycount());

        MaskNumberKeyModifyDirty(libarayInfo->id());
        
        // "操作人id:{0}, 昵称:【{1}】 [录书] 图书id:{2}, ISBN:{3}, 书名:【{4}】=>【{5}】,, 库存:{6}=>{7}, 价格:{8}.{9}=>{10}.{11}(单位:元)"
        auto sysLog = GetGlobalSys<ISystemLogGlobal>();
        std::vector<VariantParam> params;
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
            param.set_unsignedvalue(user->GetUserId());
            params.push_back(param);
        }
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_STRING);
            param.set_strvalue(user->GetNickname());
            params.push_back(param);
        }
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
            param.set_unsignedvalue(bookInfo->id());
            params.push_back(param);
        }
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_STRING);
            param.set_strvalue(bookInfo->isbncode());
            params.push_back(param);
        }
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_STRING);
            param.set_strvalue(oldBookName.GetRaw());
            params.push_back(param);
        }
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_STRING);
            param.set_strvalue(bookInfo->bookname());
            params.push_back(param);
        }
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_VALUE);
            param.set_intvalue(originalCount);
            params.push_back(param);
        }
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_VALUE);
            param.set_intvalue(bookInfo->variantinfo().count());
            params.push_back(param);
        }
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_VALUE);
            param.set_intvalue(oldPrice/100);
            params.push_back(param);

            param.set_varianttype(VariantParamType_ENUMS_VALUE);
            param.set_intvalue(oldPrice%100);
            params.push_back(param);
        }
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_VALUE);
            param.set_intvalue(bookInfo->variantinfo().price()/100);
            params.push_back(param);

            param.set_varianttype(VariantParamType_ENUMS_VALUE);
            param.set_intvalue(bookInfo->variantinfo().price()%100);
            params.push_back(param);
        }
        sysLog->AddLog(libarayInfo->id(), "ENTER_BOOK_LOG_TITLE", {}, "ENTER_BOOK_LOG_CONTENT", params);
    
    }while(false);

    AddLibraryBookRes res;
    res.set_errcode(err);
    user->Send(Opcodes::OpcodeConst::OPCODE_AddLibraryBookRes, res, packet->GetPacketId());
}

void LibraryGlobal::_OnGetBookInfoReq(KERNEL_NS::LibPacket *&packet)
{
    auto userMgr = GetGlobalSys<IUserMgr>();
    auto user = userMgr->GetUserBySessionId(packet->GetSessionId());
    if(UNLIKELY(!user))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user not online packet:%s"), packet->ToString().c_str());
        return;
    }

    auto req = packet->GetCoder<GetBookInfoReq>();
    Int32 err = Status::Success;

    GetBookInfoRes res;
    do
    {
        if(req->isbncode().size() == 0)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("have no isbn code packet:%s"), packet->ToString().c_str());
            err = Status::ParamError;
            break;
        }

        auto libraryMgr = user->GetSys<ILibraryMgr>();
        auto libraryInfo = GetLibraryInfo(libraryMgr->GetMyLibraryId());
        if(!libraryInfo)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("not join libarary packet:%s"), packet->ToString().c_str());
            err = Status::NotJoinAnyLibrary;
            break;
        }

        auto bookInfo = _GetBookInfo(libraryInfo->id(), req->isbncode());
        if(!bookInfo)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("book not found isbn code:%s, packet:%s"), req->isbncode().c_str(), packet->ToString().c_str());
            err = Status::BookNotFound;
            break;
        }

        *res.mutable_bookinfo() = *bookInfo;

    } while (false);
    
    res.set_errcode(err);
    user->Send(Opcodes::OpcodeConst::OPCODE_GetBookInfoRes, res, packet->GetPacketId());
}

void LibraryGlobal::_OnGetBookByBookNameReq(KERNEL_NS::LibPacket *&packet)
{
    auto userMgr = GetGlobalSys<IUserMgr>();
    auto user = userMgr->GetUserBySessionId(packet->GetSessionId());
    if(UNLIKELY(!user))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user not online packet:%s"), packet->ToString().c_str());
        return;
    }

    auto libraryMgr = user->GetSys<ILibraryMgr>();
    auto libraryInfo = GetLibraryInfo(libraryMgr->GetMyLibraryId());

    auto req = packet->GetCoder<GetBookByBookNameReq>();
    GetBookByBookNameRes res;
    if(libraryInfo)
    {
        if(!req->bookname().empty())
        {
            const KERNEL_NS::LibString &reqName = req->bookname();

            for(auto &bookInfo : libraryInfo->booklist())
            {
                const KERNEL_NS::LibString &bookName = bookInfo.bookname();
                if(bookName.GetRaw().find(reqName.GetRaw()) != std::string::npos)
                {
                    *res.add_bookinfolist() = bookInfo;
                    continue;
                }

                for(auto &keywordItem : bookInfo.keywords())
                {
                    const KERNEL_NS::LibString &keyword = keywordItem;
                    if(keyword.GetRaw().find(reqName.GetRaw()) != std::string::npos)
                    {
                        *res.add_bookinfolist() = bookInfo;
                        break;
                    }
                }
            }
        }
    }
    
    user->Send(Opcodes::OpcodeConst::OPCODE_GetBookByBookNameRes, res, packet->GetPacketId());
}

void LibraryGlobal::_OnGetBookInfoListReq(KERNEL_NS::LibPacket *&packet)
{
    auto userMgr = GetGlobalSys<IUserMgr>();
    auto user = userMgr->GetUserBySessionId(packet->GetSessionId());
    if(UNLIKELY(!user))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user not online packet:%s"), packet->ToString().c_str());
        return;
    }

    auto libraryMgr = user->GetSys<ILibraryMgr>();

    auto req = packet->GetCoder<GetBookInfoListReq>();
    GetBookInfoListRes res;
    std::map<UInt64, const BookInfo *> bookInfoList;
    const UInt32 bookCount = static_cast<UInt32>(std::abs(req->bookcount()));

    do
    {
        if(libraryMgr->GetMyLibraryId() == 0)
            break;

        auto &bookInfos = _GetBookInfos(libraryMgr->GetMyLibraryId());
        if(bookInfos.empty())
            break;

        if(req->basebookid() == 0)
        {
            // 前n个
            if(req->bookcount() > 0)
                _GetBooksAfter(bookInfos, 0, bookCount, bookInfoList);

            break;
        }

        if(req->bookcount() > 0)
        {
            _GetBooksAfter(bookInfos, req->basebookid(), bookCount, bookInfoList);
        }
        else
        {
            _GetBooksBefore(bookInfos, req->basebookid(), bookCount, bookInfoList);
        }

    }while(false);

    _BuildBookInfos(bookInfoList, res.mutable_bookinfolist());

    user->Send(Opcodes::OpcodeConst::OPCODE_GetBookInfoListRes, res, packet->GetPacketId());
}

void LibraryGlobal::_OnGetBookOrderDetailInfoReq(KERNEL_NS::LibPacket *&packet)
{
    auto userMgr = GetGlobalSys<IUserMgr>();
    auto user = userMgr->GetUserBySessionId(packet->GetSessionId());
    if(UNLIKELY(!user))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user not online packet:%s"), packet->ToString().c_str());
        return;
    }

    // 先推送图书馆信息
    _SendLibraryInfoNty(user);

    Int32 err = _SendOrderDetailInfoNty(user);
    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("send order detail info fail err:%d, in any library user:%s"), err, user->ToString().c_str());
    }

    GetBookOrderDetailInfoRes res;
    res.set_errcode(err);
    user->Send(Opcodes::OpcodeConst::OPCODE_GetBookOrderDetailInfoRes, res, packet->GetPacketId());
}

void LibraryGlobal::_OnOutStoreOrderReq(KERNEL_NS::LibPacket *&packet)
{
    auto userMgr = GetGlobalSys<IUserMgr>();
    auto user = userMgr->GetUserBySessionId(packet->GetSessionId());
    if(UNLIKELY(!user))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user not online packet:%s"), packet->ToString().c_str());
        return;
    }

    auto req = packet->GetCoder<OutStoreOrderReq>();
    Int32 err = Status::Success;
    do
    {
        auto libraryId = user->GetSys<ILibraryMgr>()->GetMyLibraryId();
        if(libraryId == 0)
        {
            err = Status::NotJoinAnyLibrary;
            g_Log->Warn(LOGFMT_OBJ_TAG("not in any library user:%s"), user->ToString().c_str());
            break;
        }

        // 必须要有管理员权限
        if(!_IsManager(libraryId, user->GetUserId()))
        {
            err = Status::NotManager;
            g_Log->Warn(LOGFMT_OBJ_TAG("not library manager libraryId:%llu, user:%s"), libraryId, user->ToString().c_str());
            break;
        }

        auto orderInfo = GetOrderInfo(libraryId, req->orderid());
        if(!orderInfo)
        {
            err = Status::InvalidOrder;
            g_Log->Warn(LOGFMT_OBJ_TAG("not library manager libraryId:%llu, user:%s"), libraryId, user->ToString().c_str());
            break;
        }

        // 必须是待出库状态
        if(orderInfo->orderstate() != BorrowOrderState_ENUMS_WAITING_OUT_OF_WAREHOUSE)
        {
            err = Status::InvalidOrderState;
            g_Log->Warn(LOGFMT_OBJ_TAG("order not at waiting out of warehouse state:%d,%s order id:%llu manager libraryId:%llu, user:%s")
            ,orderInfo->orderstate(), BorrowOrderState_ENUMS_Name(orderInfo->orderstate()).c_str(), static_cast<UInt64>(orderInfo->orderid())
            , libraryId, user->ToString().c_str());
            break;
        }

        std::map<UInt64, Int64> bookIdRefCount;
        for(auto &outStoreParam : req->bookparams())
        {
            auto iter = bookIdRefCount.find(outStoreParam.bookid());
            if(iter == bookIdRefCount.end())
                iter = bookIdRefCount.insert(std::make_pair(outStoreParam.bookid(), 0)).first;
            iter->second += outStoreParam.count();
        }

        // 订单书的数量统计
        std::map<UInt64, Int64> bookOfOrder;
        for(auto &subOrder : orderInfo->borrowbooklist())
        {
            auto iter = bookOfOrder.find(subOrder.bookid());
            if(iter == bookOfOrder.end())
                iter = bookOfOrder.insert(std::make_pair(subOrder.bookid(), 0)).first;
            iter->second += static_cast<Int64>(subOrder.borrowcount());
        }

        for(auto iter = bookIdRefCount.begin(); iter != bookIdRefCount.end();)
        {
            auto iterOrder = bookOfOrder.find(iter->first);
            if(iterOrder == bookOfOrder.end())
            {
                err = Status::OutStoreNotMatchOrder;
                g_Log->Warn(LOGFMT_OBJ_TAG("not match order libraryId:%llu, user:%s"), libraryId, user->ToString().c_str());
                break;
            }

            if(iterOrder->second != iter->second)
            {
                err = Status::OutStoreNotMatchOrder;
                g_Log->Warn(LOGFMT_OBJ_TAG("not match order libraryId:%llu, user:%s"), libraryId, user->ToString().c_str());
                break;
            }

            bookOfOrder.erase(iterOrder);
            iter = bookIdRefCount.erase(iter);
        }

        if(err != Status::Success)
            break;

        if(!bookOfOrder.empty())
        {
            err = Status::OutStoreNotMatchOrder;
            g_Log->Warn(LOGFMT_OBJ_TAG("not match order libraryId:%llu, user:%s"), libraryId, user->ToString().c_str());
            break;
        }

        // 切换状态出库成功
        orderInfo->set_orderstate(BorrowOrderState_ENUMS_WAIT_USER_RECEIVE);

        auto outstoreWaitGotDaysConfig = GetGlobalSys<ConfigLoader>()->GetComp<CommonConfigMgr>()->GetConfigById(CommonConfigIdEnums::OUTSTORE_WAIT_GOT_DAYS);
        const auto &nowTime = KERNEL_NS::LibTime::Now();
        const auto &getOverTime = nowTime.AddDays(outstoreWaitGotDaysConfig->_value);
        orderInfo->set_getovertime(getOverTime.GetMilliTimestamp());

        // 开启定时器
        _StartCacelOrderTimer(libraryId, orderInfo->orderid(), (getOverTime - nowTime).GetTotalMilliSeconds());

        MaskNumberKeyModifyDirty(libraryId);

        _SendLibraryInfoNty(user->GetUserId(), libraryId);

        _SendOrderDetailInfoNty(user);

        auto notifyGlobal = GetGlobalSys<INotifyGlobal>();
        // 订单{0}已经出库，请即时领取
        std::vector<VariantParam> params;
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_STRING);
            param.set_strvalue(user->GetNickname());
            params.push_back(param);
        }
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
            param.set_unsignedvalue(orderInfo->orderid());
            params.push_back(param);
        }
        notifyGlobal->SendNotify(orderInfo->userid(), "OUTSTORE_TITLE", {}, "OUTSTORE_CONTENT"
        , params);

        // "操作人:【{0}】,id:{1}, 订单号:{2},借书用户id:{3},等待用户领书"
        auto sysLog = GetGlobalSys<ISystemLogGlobal>();
        params.clear();
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_STRING);
            param.set_strvalue(user->GetNickname());
            params.push_back(param);
        }
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
            param.set_unsignedvalue(user->GetUserId());
            params.push_back(param);
        }
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
            param.set_unsignedvalue(orderInfo->orderid());
            params.push_back(param);
        }
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
            param.set_unsignedvalue(orderInfo->userid());
            params.push_back(param);
        }
        sysLog->AddLog(libraryId, "OUTSTORE_LOG_TITLE", {}, "OUTSTORE_LOG_CONTENT", params);

    } while (false);
    
    OutStoreOrderRes res;
    res.set_errcode(err);
    user->Send(Opcodes::OpcodeConst::OPCODE_OutStoreOrderRes, res, packet->GetPacketId());
}

void LibraryGlobal::_OnManagerScanOrderForUserGettingBooksReq(KERNEL_NS::LibPacket *&packet)
{
    auto userMgr = GetGlobalSys<IUserMgr>();
    auto user = userMgr->GetUserBySessionId(packet->GetSessionId());
    if(UNLIKELY(!user))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user not online packet:%s"), packet->ToString().c_str());
        return;
    }

    auto req = packet->GetCoder<ManagerScanOrderForUserGettingBooksReq>();
    Int32 err = Status::Success;
    do
    {
        // 图书馆信息
        auto librarayMgr = user->GetSys<ILibraryMgr>();
        const auto libararyId = librarayMgr->GetMyLibraryId();

        // 1.必须是管理员
        auto libraryInfo = GetLibraryInfo(libararyId);
        if(!libraryInfo)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("not join any library user:%s req:%s"), user->ToString().c_str(), req->ToJsonString().c_str());
            err = Status::NotJoinAnyLibrary;
            break;
        }

        auto memberInfo = GetMemberInfo(libararyId, user->GetUserId());
        if(!memberInfo)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("not member libraryid:%llu, user:%s req:%s"), libararyId, user->ToString().c_str(), req->ToJsonString().c_str());
            err = Status::NotMember;
            break;
        }

        // 2.查找订单是否存在
        auto orderInfo = GetOrderInfo(libraryInfo->id(), req->orderid());
        if(!orderInfo)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("order not exists user:%s req:%s"), user->ToString().c_str(), req->ToJsonString().c_str());
            err = Status::InvalidOrder;
            break;
        }

        // 必须是等待用户领取的
        if(orderInfo->orderstate() != BorrowOrderState_ENUMS_WAIT_USER_RECEIVE)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("invalid order state user:%s req:%s"), user->ToString().c_str(), req->ToJsonString().c_str());
            err = Status::InvalidOrderState;
            break;
        }

        // 3.订单的用户必须在线
        auto orderUser = GetGlobalSys<IUserMgr>()->GetUser(orderInfo->userid());
        if(!orderUser)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("order user:%llu not online, user:%s req:%s"), static_cast<UInt64>(orderInfo->userid()), user->ToString().c_str(), req->ToJsonString().c_str());
            err = Status::NotOnline;
            break;
        }

        if(!orderUser->IsOnLine())
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("order user not online, user:%s req:%s"), user->ToString().c_str(), req->ToJsonString().c_str());
            err = Status::NotOnline;
            break;
        }

        // 4.向用户推送确认码(1分钟有效)
        auto guidMgr = GetGlobalSys<IGlobalUidMgr>();
        UserGetBooksOrderConfirmNty nty;
        const auto confirmId = guidMgr->NewGuid();

        auto iterConfirms = _orderIdRefConfirmCodes.find(orderInfo->orderid());
        if(iterConfirms == _orderIdRefConfirmCodes.end())
            iterConfirms = _orderIdRefConfirmCodes.insert(std::make_pair(orderInfo->orderid(), std::set<UInt64>())).first;
        iterConfirms->second.insert(confirmId);
        _confirmCodeRefOrderId.insert(std::make_pair(confirmId, orderInfo->orderid()));
        
        nty.set_confirmcode(confirmId);
        nty.set_orderid(orderInfo->orderid());
        orderUser->Send(Opcodes::OpcodeConst::OPCODE_UserGetBooksOrderConfirmNty, nty);

        auto newTimer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
        newTimer->GetMgr()->TakeOverLifeTime(newTimer, [](KERNEL_NS::LibTimer *t){
            KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
        });
        newTimer->SetTimeOutHandler([this, confirmId](KERNEL_NS::LibTimer *t) mutable -> void
        {
            _RemoveConfirm(confirmId);
            KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
        });

        auto confirmTimeConfig = GetGlobalSys<ConfigLoader>()->GetComp<CommonConfigMgr>()->GetConfigById(CommonConfigIdEnums::CONFIRM_CODE_TIME);
        newTimer->Schedule(KERNEL_NS::TimeSlice::FromSeconds(confirmTimeConfig->_int64Value * KERNEL_NS::TimeDefs::SECOND_PER_MINUTE));
        
        g_Log->Info(LOGFMT_OBJ_TAG("gen confirm code:%llu, order info:%s, library id:%llu, operate user:%s, order user:%s")
        , confirmId, orderInfo->ToJsonString().c_str(), static_cast<UInt64>(libraryInfo->id()), user->ToString().c_str(), orderUser->ToString().c_str());
    } while (false);

    ManagerScanOrderForUserGettingBooksRes res;
    res.set_errcode(err);
    user->Send(Opcodes::OpcodeConst::OPCODE_ManagerScanOrderForUserGettingBooksRes, res, packet->GetPacketId());
}

void LibraryGlobal::_OnUserGetBooksOrderConfirmReq(KERNEL_NS::LibPacket *&packet)
{
    auto userMgr = GetGlobalSys<IUserMgr>();
    auto user = userMgr->GetUserBySessionId(packet->GetSessionId());
    if(UNLIKELY(!user))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user not online packet:%s"), packet->ToString().c_str());
        return;
    }

    auto req = packet->GetCoder<UserGetBooksOrderConfirmReq>();
    Int32 err = Status::Success;
    do
    {
        // 必须是会员以上,有权限的
        auto libraryMgr = user->GetSys<ILibraryMgr>();
        const auto libraryId = libraryMgr->GetMyLibraryId();
        const auto libraryInfo = GetLibraryInfo(libraryId);
        if(!libraryInfo)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("not join any library user:%s req:%s"), user->ToString().c_str(), req->ToJsonString().c_str());
            err = Status::NotJoinAnyLibrary;
            break;
        }

        auto memberInfo = GetMemberInfo(libraryId, user->GetUserId());
        if(!memberInfo)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("not member libraryid:%llu, user:%s req:%s"), libraryId, user->ToString().c_str(), req->ToJsonString().c_str());
            err = Status::NotMember;
            break;
        }

        // 确认码是否正确
        auto iterOrderId = _confirmCodeRefOrderId.find(req->confirmcode());
        if(iterOrderId == _confirmCodeRefOrderId.end())
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("invalid confirm code user:%s req:%s"), user->ToString().c_str(), req->ToJsonString().c_str());
            err = Status::InvalidConfirmCode;
            break;
        }

        // 订单号是否正确
        auto orderInfo = GetOrderInfo(libraryInfo->id(), iterOrderId->second);
        if(!orderInfo)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("cant find order confirm code%llu, order id:%llu, user:%s req:%s"), static_cast<UInt64>(req->confirmcode()), iterOrderId->second, user->ToString().c_str(), req->ToJsonString().c_str());
            err = Status::InvalidConfirmCode;
            break;
        }

        // 订单是否出库待领取状态
        if(orderInfo->orderstate() != BorrowOrderState_ENUMS_WAIT_USER_RECEIVE)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("order is not wait reciecve confirm code%llu, order id:%llu, order state:%d,%s user:%s req:%s")
            , static_cast<UInt64>(req->confirmcode()), iterOrderId->second, orderInfo->orderstate()
            , BorrowOrderState_ENUMS_Name(orderInfo->orderstate()).c_str()
            , user->ToString().c_str(), req->ToJsonString().c_str());
            err = Status::InvalidOrderState;
            break;
        }

        _RemoveOrderConfirm(orderInfo->orderid());

        orderInfo->set_orderstate(BorrowOrderState_ENUMS_WAIT_USER_RETURN_BACK);
        const Int32 subOrderCount = orderInfo->borrowbooklist_size();
        const auto &nowTime = KERNEL_NS::LibTime::Now();
        for(Int32 idx = 0; idx < subOrderCount; ++idx)
        {
            auto subOrder = orderInfo->mutable_borrowbooklist(idx);
            const auto &planBakcTime = nowTime.AddDays(subOrder->borrowdays());
            subOrder->set_plangivebacktime(planBakcTime.GetMilliTimestamp());
        }
        MaskNumberKeyModifyDirty(libraryInfo->id());

        g_Log->Info(LOGFMT_OBJ_TAG("get book confirm user:%s, order info:%s"), user->ToString().c_str(), orderInfo->ToJsonString().c_str());

        _SendLibraryInfoNty(user);
        _SendOrderDetailInfoNty(user);

        // "用户【{0}】领书,用户id:{1}, 订单号:{2},订单状态:{3}({4})"
        auto sysLog = GetGlobalSys<ISystemLogGlobal>();
        std::vector<VariantParam> params;
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_STRING);
            param.set_strvalue(user->GetNickname());
            params.push_back(param);
        }
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
            param.set_unsignedvalue(user->GetUserId());
            params.push_back(param);
        }
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
            param.set_unsignedvalue(orderInfo->orderid());
            params.push_back(param);
        }
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_VALUE);
            param.set_intvalue(static_cast<Int64>(orderInfo->orderstate()));
            params.push_back(param);
        }
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_STRING);
            param.set_strvalue(BorrowOrderState_ENUMS_Name(orderInfo->orderstate()));
            params.push_back(param);
        }
        sysLog->AddLog(libraryId, "USER_GET_BOOK_TITLE", {}, "USER_GET_BOOK_CONTENT", params);
        
    } while (false);

    UserGetBooksOrderConfirmRes res;
    res.set_errcode(err);

    user->Send(Opcodes::OpcodeConst::OPCODE_UserGetBooksOrderConfirmRes, res, packet->GetPacketId());
}

void LibraryGlobal::_OnCancelOrderReq(KERNEL_NS::LibPacket *&packet)
{
    auto userMgr = GetGlobalSys<IUserMgr>();
    auto user = userMgr->GetUserBySessionId(packet->GetSessionId());
    if(UNLIKELY(!user))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user not online packet:%s"), packet->ToString().c_str());
        return;
    }

    auto req = packet->GetCoder<CancelOrderReq>();
    Int32 err = Status::Success;
    do
    {
        auto libraryMgr = user->GetSys<ILibraryMgr>();
        const auto libraryId = libraryMgr->GetMyLibraryId();

        auto libraryInfo = GetLibraryInfo(libraryId);
        if(!libraryInfo)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("not join any library user:%s req:%s"), user->ToString().c_str(), req->ToJsonString().c_str());
            err = Status::NotJoinAnyLibrary;
            break;
        }

        auto memberInfo = GetMemberInfo(libraryId, user->GetUserId());
        if(!memberInfo)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("not member libraryid:%llu, user:%s req:%s"), libraryId, user->ToString().c_str(), req->ToJsonString().c_str());
            err = Status::NotMember;
            break;
        }

        // 订单是否存在
        auto orderInfo = GetOrderInfo(libraryId, req->orderid());
        if(!orderInfo)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("order not exists user:%s req:%s"), user->ToString().c_str(), req->ToJsonString().c_str());
            err = Status::InvalidOrder;
            break;
        }

        // 原因内容是否合法
        auto contentConfig = GetGlobalSys<ConfigLoader>()->GetComp<CommonConfigMgr>()->GetConfigById(CommonConfigIdEnums::CONTENT_LIMIT);
        KERNEL_NS::LibString decodedContent;
        if(!req->reason().empty())
        {
            decodedContent = KERNEL_NS::UrlCoder::Decode(req->reason());
            if(!decodedContent.IsUtf8())
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("cancel reason not utf8, user:%s req:%s"), user->ToString().c_str(), req->ToJsonString().c_str());
                err = Status::InvalidContent;
                break;
            }

            const Int32 len = static_cast<Int32>(decodedContent.length_with_utf8());
            if(len > contentConfig->_value)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("content too long len:%d, limit:%d not exists user:%s req:%s")
                ,len, contentConfig->_value,  user->ToString().c_str(), req->ToJsonString().c_str());
                break;
            }
        }

        // 订单必须是没被取消的
        if(orderInfo->orderstate() == BorrowOrderState_ENUMS_CANCEL_ORDER)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("cancel repeat user:%s req:%s"), user->ToString().c_str(), req->ToJsonString().c_str());
            err = Status::Repeat;
            break;
        }

        // 订单必须是还没领取的
        if(orderInfo->orderstate() > BorrowOrderState_ENUMS_WAIT_USER_RECEIVE)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("invalid order state user:%s req:%s, orderInfo:%s")
            , user->ToString().c_str(), req->ToJsonString().c_str(), orderInfo->ToJsonString().c_str());
            err = Status::InvalidOrderState;
            break;
        }

        // 若是管理员, 则必须有原因
        Int32 reasonType = CancelOrderReasonType_ENUMS_UNKNOWN;
        if(_IsManager(libraryId, user->GetUserId()))
        {
            if(req->reason().empty())
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("manager must give reason when cancel order, user:%s req:%s, orderInfo:%s")
                , user->ToString().c_str(), req->ToJsonString().c_str(), orderInfo->ToJsonString().c_str());
                err = Status::NeedReason;
                break;
            }

            reasonType = CancelOrderReasonType_ENUMS_MANAGTER_CANCEL;
        }
        else
        {
            // 是否订单用户本人取消
            if(orderInfo->userid() != user->GetUserId())
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("not order owner cancel order, user:%s req:%s, orderInfo:%s")
                , user->ToString().c_str(), req->ToJsonString().c_str(), orderInfo->ToJsonString().c_str());
                err = Status::NotOrderOwner;
                break;
            }

            reasonType = CancelOrderReasonType_ENUMS_USER_SELF_CANCEL;
        }

        // 设置订单状态为取消
        orderInfo->set_orderstate(BorrowOrderState_ENUMS_CANCEL_ORDER);
        auto reasonInfo = orderInfo->mutable_cancelreason();
        reasonInfo->set_cancelreason(reasonType);
        reasonInfo->set_cancelinfo(req->reason());
        MaskNumberKeyModifyDirty(libraryId);

        _SendLibraryInfoNty(user);
        _SendOrderDetailInfoNty(user);

        g_Log->Info(LOGFMT_OBJ_TAG("cancel order library id:%llu, operate user:%s, memberInfo:%s, order info:%s, order reason:%s")
        ,  libraryId, user->ToString().c_str(), memberInfo->ToJsonString().c_str(), orderInfo->ToJsonString().c_str(), decodedContent.c_str());

        // 取消订单需要通知管理员(所有包括馆长)订单取消
        auto notifyGlobal = GetGlobalSys<INotifyGlobal>();
        // "订单号:{0}, 订单所属用户id:{1}, 昵称:{2}, 操作人id:{3}, 操作人昵称:{4}, 取消原因:{5}."
        std::vector<VariantParam> params;
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
            param.set_unsignedvalue(orderInfo->orderid());
            params.push_back(param);
        }
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
            param.set_unsignedvalue(orderInfo->userid());
            params.push_back(param);
        }
        {
            auto orderOwnerMember = GetMemberInfo(libraryId, orderInfo->userid());

            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_STRING);
            param.set_strvalue(orderOwnerMember->nickname());
            params.push_back(param);
        }
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
            param.set_unsignedvalue(user->GetUserId());
            params.push_back(param);
        }
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_STRING);
            param.set_strvalue(user->GetNickname());
            params.push_back(param);
        }
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_STRING);
            param.set_strvalue(req->reason());
            params.push_back(param);
        }

        bool isNotified = false;
        for(auto &manager:libraryInfo->managerinfolist())
        {
            if(manager.userid() == orderInfo->userid())
                isNotified = true;

            notifyGlobal->SendNotify(manager.userid(), "CANCEL_ORDER_TITLE", {}, "CANCEL_ORDER_CONTENT"
            , params);
        }

        // 取消订单需要同时通知用户订单取消了
        if(!isNotified)
        {
            notifyGlobal->SendNotify(orderInfo->userid(), "CANCEL_ORDER_TITLE", {}, "CANCEL_ORDER_CONTENT"
                , params);
        }

        // "用户主动取消订单, 用户id:{0},昵称:【{1}】, 订单号:{2}, 订单状态:{3}({4}), 备注:{5}"
        auto sysLog = GetGlobalSys<ISystemLogGlobal>();
        params.clear();
        if(reasonType == CancelOrderReasonType_ENUMS_MANAGTER_CANCEL)
        {
            // "管理员取消订单, 管理员用户id:{0},昵称:【{1}】, 订单号:{2}, 订单状态:{3}({4}) 借书用户id:{5}, 备注:{6}"
            {
                VariantParam param;
                param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
                param.set_unsignedvalue(user->GetUserId());
                params.push_back(param);
            }
            {
                VariantParam param;
                param.set_varianttype(VariantParamType_ENUMS_STRING);
                param.set_strvalue(user->GetNickname());
                params.push_back(param);
            }
            {
                VariantParam param;
                param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
                param.set_unsignedvalue(orderInfo->orderid());
                params.push_back(param);
            }
            {
                VariantParam param;
                param.set_varianttype(VariantParamType_ENUMS_VALUE);
                param.set_intvalue(static_cast<Int64>(orderInfo->orderstate()));
                params.push_back(param);
            }
            {
                VariantParam param;
                param.set_varianttype(VariantParamType_ENUMS_STRING);
                param.set_strvalue(BorrowOrderState_ENUMS_Name(orderInfo->orderstate()));
                params.push_back(param);
            }
            {
                VariantParam param;
                param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
                param.set_unsignedvalue(orderInfo->userid());
                params.push_back(param);
            }
            {
                VariantParam param;
                param.set_varianttype(VariantParamType_ENUMS_STRING);
                param.set_strvalue(orderInfo->cancelreason().cancelinfo());
                params.push_back(param);
            }
            sysLog->AddLog(libraryId, "CANCEL_ORDER_LOG_TITLE", {}, "CANCEL_ORDER_BY_MANAGER_CONTENT", params);
        }
        else
        {
            // "用户主动取消订单, 用户id:{0},昵称:【{1}】, 订单号:{2}, 订单状态:{3}({4}), 备注:{5}"
            {
                VariantParam param;
                param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
                param.set_unsignedvalue(user->GetUserId());
                params.push_back(param);
            }
            {
                VariantParam param;
                param.set_varianttype(VariantParamType_ENUMS_STRING);
                param.set_strvalue(user->GetNickname());
                params.push_back(param);
            }
            {
                VariantParam param;
                param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
                param.set_unsignedvalue(orderInfo->orderid());
                params.push_back(param);
            }
            {
                VariantParam param;
                param.set_varianttype(VariantParamType_ENUMS_VALUE);
                param.set_intvalue(static_cast<Int64>(orderInfo->orderstate()));
                params.push_back(param);
            }
            {
                VariantParam param;
                param.set_varianttype(VariantParamType_ENUMS_STRING);
                param.set_strvalue(BorrowOrderState_ENUMS_Name(orderInfo->orderstate()));
                params.push_back(param);
            }
            {
                VariantParam param;
                param.set_varianttype(VariantParamType_ENUMS_STRING);
                param.set_strvalue(orderInfo->cancelreason().cancelinfo());
                params.push_back(param);
            }
            sysLog->AddLog(libraryId, "CANCEL_ORDER_LOG_TITLE", {}, "CANCEL_ORDER_BY_SELF_CONTENT", params);
        }

    } while (false);
    
    CancelOrderRes res;
    res.set_errcode(err);
    user->Send(Opcodes::OpcodeConst::OPCODE_CancelOrderRes, res, packet->GetPacketId());
}

void LibraryGlobal::_OnReturnBackReq(KERNEL_NS::LibPacket *&packet)
{
    auto userMgr = GetGlobalSys<IUserMgr>();
    auto user = userMgr->GetUserBySessionId(packet->GetSessionId());
    if(UNLIKELY(!user))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user not online packet:%s"), packet->ToString().c_str());
        return;
    }

    auto req = packet->GetCoder<ReturnBackReq>();
    Int32 err = Status::Success;
    do
    {
        auto libraryMgr = user->GetSys<ILibraryMgr>();
        const auto libraryId = libraryMgr->GetMyLibraryId();
        auto libarayInfo = GetLibraryInfo(libraryId);
        if(!libarayInfo)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("not join any library user:%s req:%s"), user->ToString().c_str(), req->ToJsonString().c_str());
            err = Status::NotJoinAnyLibrary;
            break;
        }

        if(!_IsManager(libraryId, user->GetUserId()))
        {
            err = Status::NotManager;
            g_Log->Warn(LOGFMT_OBJ_TAG("not library manager libraryId:%llu, user:%s"), libraryId, user->ToString().c_str());
            break;
        }

        auto orderInfo = GetOrderInfo(libraryId, req->orderid());
        if(!orderInfo)
        {
            err = Status::ParamError;
            g_Log->Warn(LOGFMT_OBJ_TAG("order not exist libraryId:%llu, user:%s, req:%s")
            , libraryId, user->ToString().c_str(), req->ToJsonString().c_str());
            break;
        }

        // 必须是领书完待还
        if(orderInfo->orderstate() != BorrowOrderState_ENUMS_WAIT_USER_RETURN_BACK)
        {
            err = Status::InvalidOrderState;
            g_Log->Warn(LOGFMT_OBJ_TAG("order state invalid exist libraryId:%llu, user:%s, req:%s, orderInfo:%s")
            , libraryId, user->ToString().c_str(), req->ToJsonString().c_str(), orderInfo->ToJsonString().c_str());
            break;
        }

        // 如果还部分的订单
        std::set<UInt64> suborders;
        for(UInt64 subOrderId : req->suborderids())
        {
            bool isExists = false;
            for(auto &subOrder : orderInfo->borrowbooklist())
            {
                if(subOrder.suborderid() == subOrderId)
                {
                    isExists = true;
                    suborders.insert(subOrderId);
                    break;
                }
            }

            if(!isExists)
            {
                err = Status::ParamError;
                g_Log->Warn(LOGFMT_OBJ_TAG("sub order not exist sub order id:%llu libraryId:%llu, user:%s, req:%s, orderInfo:%s")
                , subOrderId, libraryId, user->ToString().c_str(), req->ToJsonString().c_str(), orderInfo->ToJsonString().c_str());
                break;
            }
        }

        if(err != Status::Success)
        {
            break;
        }

        const auto &nowTime = KERNEL_NS::LibTime::Now();
        // isbn, bookname, count,
        std::map<UInt64, std::tuple<KERNEL_NS::LibString, KERNEL_NS::LibString, UInt64>> returnBackBookInfoList;
        if(suborders.empty())
        {
            const Int32 count = orderInfo->borrowbooklist_size();
            for(Int32 idx = 0; idx < count; ++idx)
            {
                auto subOrderInfo = orderInfo->mutable_borrowbooklist(idx);
                subOrderInfo->set_realgivebacktime(nowTime.GetMilliTimestamp());
                subOrderInfo->set_returnbackcount(subOrderInfo->borrowcount());

                auto bookInfo = GetBookInfo(libraryId, subOrderInfo->bookid());
                // 库存数量增加
                if(LIKELY(bookInfo))
                {
                    auto variantInfo = bookInfo->mutable_variantinfo();
                    variantInfo->set_count(variantInfo->count() + subOrderInfo->borrowcount());
                }
                else
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("book not exists when return back book book id:%llu, isbn:%s, library id:%llu, order info:%s, user:%s")
                    , static_cast<UInt64>(subOrderInfo->bookid()), subOrderInfo->isbncode().c_str(), libraryId, orderInfo->ToJsonString().c_str(), user->ToString().c_str());
                }

                auto iter = returnBackBookInfoList.find(subOrderInfo->bookid());
                if(iter == returnBackBookInfoList.end())
                {
                    auto tup = std::make_tuple(subOrderInfo->isbncode(), bookInfo?bookInfo->bookname() : "", 0);
                    iter = returnBackBookInfoList.insert(std::make_pair(subOrderInfo->bookid(), tup)).first;
                }

                auto &bookCount = std::get<2>(iter->second);
                bookCount += subOrderInfo->borrowcount();
            }
        }
        else
        {
            const Int32 count = orderInfo->borrowbooklist_size();
            for(Int32 idx = 0; idx < count; ++idx)
            {
                auto subOrderInfo = orderInfo->mutable_borrowbooklist(idx);
                if(suborders.find(subOrderInfo->suborderid()) == suborders.end())
                    continue;

                subOrderInfo->set_realgivebacktime(nowTime.GetMilliTimestamp());
                subOrderInfo->set_returnbackcount(subOrderInfo->borrowcount());
                // 库存数量增加
                auto bookInfo = GetBookInfo(libraryId, subOrderInfo->bookid());
                if(LIKELY(bookInfo))
                {
                    auto variantInfo = bookInfo->mutable_variantinfo();
                    variantInfo->set_count(variantInfo->count() + subOrderInfo->borrowcount());
                }
                else
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("book not exists when return back book book id:%llu, isbn:%s, library id:%llu, order info:%s, user:%s")
                    , static_cast<UInt64>(subOrderInfo->bookid()), subOrderInfo->isbncode().c_str(), libraryId, orderInfo->ToJsonString().c_str(), user->ToString().c_str());
                }

                auto iter = returnBackBookInfoList.find(subOrderInfo->bookid());
                if(iter == returnBackBookInfoList.end())
                {
                    auto tup = std::make_tuple(subOrderInfo->isbncode(), bookInfo?bookInfo->bookname() : "", 0);
                    iter = returnBackBookInfoList.insert(std::make_pair(subOrderInfo->bookid(), tup)).first;
                }

                auto &bookCount = std::get<2>(iter->second);
                bookCount += subOrderInfo->borrowcount();
            }
        }

        // 所有书是否全部还完
        bool notReturnAll = false;
        for(auto subOrderInfo : orderInfo->borrowbooklist())
        {
            if(static_cast<UInt64>(subOrderInfo.borrowcount()) != subOrderInfo.returnbackcount())
            {
                notReturnAll = true;
                break;
            }
        }
        
        if(!notReturnAll)
        {
            orderInfo->set_orderstate(BorrowOrderState_ENUMS_RETURN_BAKCK);
        }

        MaskNumberKeyModifyDirty(libraryId);

        _SendLibraryInfoNty(user);
        _SendOrderDetailInfoNty(user);

        auto targetMemberInfo = GetMemberInfo(libraryId, orderInfo->userid());
        g_Log->Info(LOGFMT_OBJ_TAG("return back books library id:%llu, operate user:%s, target user member info:%s, order info:%s")
        ,  libraryId, user->ToString().c_str(), targetMemberInfo ? targetMemberInfo->ToJsonString().c_str() : KERNEL_NS::StringUtil::Num2Str(orderInfo->userid()).c_str()
        , orderInfo->ToJsonString().c_str());

        // 取消订单需要通知管理员(所有包括馆长)订单取消
        auto notifyGlobal = GetGlobalSys<INotifyGlobal>();
        // "还书操作, 操作人id:{0},操作人昵称:{1}, 订单号:{2}, 借书人id:{3}, 借书人昵称:{4}, 归还的书:\n{5},\n 是否已还全部:{6},剩余未归还图书是否有逾期:{7}."
        std::vector<VariantParam> params;
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
            param.set_unsignedvalue(user->GetUserId());
            params.push_back(param);
        }
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_STRING);
            param.set_strvalue(user->GetNickname());
            params.push_back(param);
        }
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
            param.set_unsignedvalue(orderInfo->orderid());
            params.push_back(param);
        }
        {// 借书人id
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
            param.set_unsignedvalue(orderInfo->userid());
            params.push_back(param);
        }
        {
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_STRING);
            param.set_strvalue(targetMemberInfo ? targetMemberInfo->nickname() : "");
            params.push_back(param);
        }
        {// 归还的书的列表
            KERNEL_NS::LibString returnBackInfo;
            for(auto iter : returnBackBookInfoList)
            {
                auto &tup = iter.second;
                auto &isbnCode = std::get<0>(tup);
                auto &bookName = std::get<1>(tup);
                auto &bookCount = std::get<2>(tup);

                returnBackInfo.AppendData("BookName:");
                returnBackInfo.AppendData(bookName);
                returnBackInfo.AppendData(", ISBN:");
                returnBackInfo.AppendData(isbnCode);
                returnBackInfo.AppendFormat(" x %llu\n", bookCount);
            }
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_STRING);
            param.set_strvalue(returnBackInfo.GetRaw());
            params.push_back(param);
        }
        {// 是否归还全部
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_VALUE);
            param.set_intvalue(notReturnAll ? 0 : 1);
            params.push_back(param);
        }
        {// 剩余的是否有逾期
            VariantParam param;
            param.set_varianttype(VariantParamType_ENUMS_VALUE);
            auto hasOverDeadlineBook = _HasOberDeadlineBook(orderInfo, nowTime);
            param.set_intvalue(hasOverDeadlineBook ? 1 : 0);
            params.push_back(param);
        }
        bool isNotified = false;
        for(auto &manager: libarayInfo->managerinfolist())
        {
            if(manager.userid() == orderInfo->userid())
                isNotified = true;

            notifyGlobal->SendNotify(manager.userid(), "RETURN_BACK_BOOK_TITLE", {}, "RETURN_BACK_BOOK_CONTENT"
            , params);
        }

        if(!isNotified)
        {
            notifyGlobal->SendNotify(orderInfo->userid(), "RETURN_BACK_BOOK_TITLE", {}, "RETURN_BACK_BOOK_CONTENT"
                , params);
        }

        auto systemLog = GetGlobalSys<ISystemLogGlobal>();
        systemLog->AddLog(libraryId, "RETURN_BACK_BOOK_TITLE", {}, "RETURN_BACK_BOOK_CONTENT", params);

    }while(false);

    ReturnBackRes res;
    res.set_errcode(err);
    user->Send(Opcodes::OpcodeConst::OPCODE_ReturnBackRes, res, packet->GetPacketId());
}

void LibraryGlobal::_BuildOrderDetailInfo(const LibraryInfo *libraryInfo, UInt64 memberUserId, ::google::protobuf::RepeatedPtrField<::CRYSTAL_NET::service::BorrowOrderDetailInfo> *detailInfoList) const
{
    if(memberUserId)
    {
        auto memberInfo = GetMemberInfo(libraryInfo->id(), memberUserId);
        if(!memberInfo)
            return;

        _BuildOrderDetailInfo(libraryInfo->id(), memberInfo, detailInfoList);

        return;
    }

    for(auto &memberInfo : libraryInfo->memberlist())
        _BuildOrderDetailInfo(libraryInfo->id(), &memberInfo, detailInfoList);
}

void LibraryGlobal::_BuildOrderDetailInfo(UInt64 libraryId, const MemberInfo *memberInfo, ::google::protobuf::RepeatedPtrField<::CRYSTAL_NET::service::BorrowOrderDetailInfo> *detailInfoList) const
{
    for(auto &orderInfo : memberInfo->borrowlist())
    {
        auto newDetailInfo = detailInfoList->Add();
        newDetailInfo->set_orderid(orderInfo.orderid());
        newDetailInfo->set_createordertime(orderInfo.createordertime());
        newDetailInfo->set_orderstate(orderInfo.orderstate());
        *newDetailInfo->mutable_cancelreason() = orderInfo.cancelreason();
        newDetailInfo->set_getovertime(orderInfo.getovertime());
        newDetailInfo->set_remark(orderInfo.remark());
        newDetailInfo->set_userid(memberInfo->userid());
        newDetailInfo->set_nickname(memberInfo->nickname());

        for(auto &borrowBook : orderInfo.borrowbooklist())
        {
            auto bookInfo = GetBookInfo(libraryId, borrowBook.bookid());
            if(!bookInfo)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("book info not found library id:%llu, book id:%llu"), libraryId, static_cast<UInt64>(borrowBook.bookid()));
                continue;
            }

            auto newBorrowBook = newDetailInfo->add_borrowbooklist();
            *newBorrowBook->mutable_bookinfo() = borrowBook;
            newBorrowBook->set_bookname(bookInfo->bookname());
        }
    }

}

Int32 LibraryGlobal::_ContinueModifyMember(LibraryInfo *libraryInfo, UInt64 reqUserId, IUser *targetUser, const ModifyMemberInfoReq &req)
{
    auto memberInfo = GetMemberInfo(libraryInfo->id(), reqUserId);
    auto targetMember = GetMemberInfo(libraryInfo->id(), targetUser->GetUserId());
    auto userMgr = GetGlobalSys<IUserMgr>();

    const auto oldPhone = targetUser->GetUserBaseInfo()->bindphone();
    const auto oldRole = targetMember->role();

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

        bool hasBindedPhone = userMgr->IsBindedPhone(targetMember->userid());
        if(targetMember->role() == RoleType_ENUMS_NoAuth)
        {// 必须检查手机
            if(!hasBindedPhone)
            {
                if(!req.has_newmemberphone())
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("need bind phone from no auth to new role:%d, phone number:%llu library:%s, member role:%d,%s, target member role:%d,%s req user id:%llu")
                    ,req.newrole(), static_cast<UInt64>(req.newmemberphone()), LibraryToString(libraryInfo).c_str(), memberInfo->role(), RoleType::ENUMS_Name(memberInfo->role()).c_str()
                    , targetMember->role(), RoleType::ENUMS_Name(targetMember->role()).c_str(), reqUserId);
                    return Status::InvalidPhoneNubmer;
                }

                if(!_IsValidPhone(req.newmemberphone()))
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("user bind a invalid phone from no auth to new role:%d, phone number:%llu library:%s, member role:%d,%s, target member role:%d,%s req user id:%llu")
                    ,req.newrole(), static_cast<UInt64>(req.newmemberphone()), LibraryToString(libraryInfo).c_str(), memberInfo->role(), RoleType::ENUMS_Name(memberInfo->role()).c_str()
                    , targetMember->role(), RoleType::ENUMS_Name(targetMember->role()).c_str(), reqUserId);
                    return Status::InvalidPhoneNubmer;
                }
            }
        }

        targetMember->set_role(req.newrole());

        if(oldRole == RoleType_ENUMS_NoAuth)
        {// TODO:绑定手机
            if(!hasBindedPhone)
               targetUser->BindPhone(req.newmemberphone());
        }

        // 管理员变更
        if(_IsManager(oldRole))
        {
            _RemoveManager(libraryInfo, targetUser->GetUserId());
        }

        if(_IsManager(req.newrole()))
        {
            _AddManger(libraryInfo, targetUser->GetUserId());
        }

        MaskNumberKeyModifyDirty(libraryInfo->id());
    }

    if(req.newmemberphone() > 0)
    {
        if(!_IsValidPhone(req.newmemberphone()))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("user bind a invalid phone from no auth to new role:%d, phone number:%llu library:%s, member role:%d,%s, target member role:%d,%s req user id:%llu")
            ,req.newrole(), static_cast<UInt64>(req.newmemberphone()), LibraryToString(libraryInfo).c_str(), memberInfo->role(), RoleType::ENUMS_Name(memberInfo->role()).c_str()
            , targetMember->role(), RoleType::ENUMS_Name(targetMember->role()).c_str(), reqUserId);
            return Status::InvalidPhoneNubmer;
        }

        targetUser->BindPhone(req.newmemberphone());
        MaskNumberKeyModifyDirty(libraryInfo->id());
    }

    _SendLibraryInfoNty(reqUserId, libraryInfo);

    const auto newRole = targetMember->role();
    const auto newPhone = targetUser->GetUserBaseInfo()->bindphone();

    // 操作人:【{0}】,id:{1} 角色:{2}({3}), 修改用户信息, 被修改用户id:{4},昵称:【{5}】 角色变更:{6}({7})=>{8}({9}),手机变更:{10}=>{11}
    auto sysLog = GetGlobalSys<ISystemLogGlobal>();
    std::vector<VariantParam> params;
    {
        VariantParam param;
        param.set_varianttype(VariantParamType_ENUMS_STRING);
        param.set_strvalue(memberInfo->nickname());
        params.push_back(param);
    }
    {
        VariantParam param;
        param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
        param.set_unsignedvalue(memberInfo->userid());
        params.push_back(param);
    }
    {
        VariantParam param;
        param.set_varianttype(VariantParamType_ENUMS_VALUE);
        param.set_intvalue(static_cast<Int64>(memberInfo->role()));
        params.push_back(param);
    }
    {
        VariantParam param;
        param.set_varianttype(VariantParamType_ENUMS_STRING);
        param.set_strvalue(RoleType_ENUMS_Name(memberInfo->role()));
        params.push_back(param);
    }
    {
        VariantParam param;
        param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
        param.set_unsignedvalue(targetMember->userid());
        params.push_back(param);
    }
    {
        VariantParam param;
        param.set_varianttype(VariantParamType_ENUMS_STRING);
        param.set_strvalue(targetMember->nickname());
        params.push_back(param);
    }

    {
        VariantParam param;
        param.set_varianttype(VariantParamType_ENUMS_VALUE);
        param.set_intvalue(static_cast<Int64>(oldRole));
        params.push_back(param);
    }
    {
        VariantParam param;
        param.set_varianttype(VariantParamType_ENUMS_STRING);
        param.set_strvalue(RoleType_ENUMS_Name(oldRole));
        params.push_back(param);
    }
    {
        VariantParam param;
        param.set_varianttype(VariantParamType_ENUMS_VALUE);
        param.set_intvalue(static_cast<Int64>(newRole));
        params.push_back(param);
    }
    {
        VariantParam param;
        param.set_varianttype(VariantParamType_ENUMS_STRING);
        param.set_strvalue(RoleType_ENUMS_Name(newRole));
        params.push_back(param);
    }
    {
        VariantParam param;
        param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
        param.set_unsignedvalue(oldPhone);
        params.push_back(param);
    }
    {
        VariantParam param;
        param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
        param.set_unsignedvalue(newPhone);
        params.push_back(param);
    }
    sysLog->AddLog(libraryInfo->id(), "MODIFY_MEMBER_LOG_TITLE", {}, "MODIFY_MEMBER_LOG_CONTENT", params);

    g_Log->Info(LOGFMT_OBJ_TAG("target member info is modified by user id:%llu, new target member:%s"), reqUserId, _MemberToString(targetMember).c_str());

    return Status::Success;
}

KERNEL_NS::LibString LibraryGlobal::_MemberToString(const MemberInfo *memberInfo) const
{
    KERNEL_NS::LibString info;
    info.AppendFormat("user id:%llu, nickname:%s, role:%d,%s, borrow list:%d, locktime ms:%lld, cur time:%lld"
    , static_cast<UInt64>(memberInfo->userid())
    , memberInfo->nickname().c_str()
    , memberInfo->role()
    , RoleType::ENUMS_Name(memberInfo->role()).c_str()
    , memberInfo->borrowlist_size()
    , static_cast<Int64>(memberInfo->locktimestampms())
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

    auto systemLog = GetGlobalSys<ISystemLogGlobal>();
    // "【{0}】创建图书馆, 用户id:{1}"
    std::vector<VariantParam> params;
    {
        VariantParam param;
        param.set_varianttype(VariantParamType_ENUMS_STRING);
        param.set_strvalue(user->GetNickname());
        params.push_back(param);
    }
    {
        VariantParam param;
        param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
        param.set_unsignedvalue(user->GetUserId());
        params.push_back(param);
    }
    systemLog->AddLog(newLibrary->id(), "CREATE_LIBRARY_TITLE", {}, "CREATE_LIBRARY_CONTENT", params);

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

        // 订单取消, 和还完
        if((orderInfo.orderstate() == BorrowOrderState_ENUMS_CANCEL_ORDER) || 
            (orderInfo.orderstate() == BorrowOrderState_ENUMS_RETURN_BAKCK))
            continue;

        const Int32 bookListSize = orderInfo.borrowbooklist_size();
        for(Int32 bookIdx = 0; bookIdx < bookListSize; ++bookIdx)
        {
            auto &borrowBookInfo = orderInfo.borrowbooklist(bookIdx);
            if(borrowBookInfo.returnbackcount() < static_cast<UInt64>(borrowBookInfo.borrowcount()))
                return false;
        }
    }

    return true;
}

bool LibraryGlobal::_DoesRoleHaveAuthToBorrow(Int32 role) const
{
    return role != RoleType_ENUMS_NoAuth;
}

bool LibraryGlobal::_HasOverDeadlineOrder(const MemberInfo *memberInfo, const KERNEL_NS::LibTime &nowTime) const
{
    const Int32 len = memberInfo->borrowlist_size();
    for(Int32 idx = 0; idx < len; ++idx)
    {
        auto &orderInfo = memberInfo->borrowlist(idx);

        // 已领取之前
        if(orderInfo.orderstate() < BorrowOrderState_ENUMS_WAIT_USER_RETURN_BACK)
            continue;

        // 取消和已归还
        if((orderInfo.orderstate() == BorrowOrderState_ENUMS_CANCEL_ORDER) || 
           (orderInfo.orderstate() == BorrowOrderState_ENUMS_RETURN_BAKCK))
            continue;

        if(_HasOberDeadlineBook(&orderInfo, nowTime))
            return true;
    }

    return false;
}

bool LibraryGlobal::_HasOberDeadlineBook(const BorrowOrderInfo *orderInfo, const KERNEL_NS::LibTime &nowTime) const
{
    const auto bookListSize = orderInfo->borrowbooklist_size();
    for(Int32 bookIdx = 0; bookIdx < bookListSize; ++bookIdx)
    {
        auto &borrowBookInfo = orderInfo->borrowbooklist(bookIdx);
        if(borrowBookInfo.returnbackcount() < static_cast<UInt64>(borrowBookInfo.borrowcount()))
        {
            if(nowTime.GetMilliTimestamp() >= borrowBookInfo.plangivebacktime())
                return true;
        }
    }

    return false;
}

bool LibraryGlobal::_RemoveMember(LibraryInfo *libraryInfo, UInt64 userId)
{
    if(libraryInfo->librarianuserid() == userId)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("librarian cant be removed userId:%llu, library:%s"), userId, LibraryToString(libraryInfo).c_str());
        return false;
    }

    KERNEL_NS::LibString nickName;
    auto iterMembers = _libraryIdRefUserRefMember.find(libraryInfo->id());
    if(iterMembers != _libraryIdRefUserRefMember.end())
        iterMembers->second.erase(userId);

    {
        const Int32 len = libraryInfo->memberlist_size();
        for(Int32 idx = 0; idx < len; ++idx)
        {
            if(libraryInfo->memberlist(idx).userid() == userId)
            {
                nickName = libraryInfo->memberlist(idx).nickname();
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

    // 退出图书馆日志【{0}】退出了图书馆, 用户id:{1}
    auto systemLog = GetGlobalSys<ISystemLogGlobal>();
    std::vector<VariantParam> params;
    {
        VariantParam param;
        param.set_varianttype(VariantParamType_ENUMS_STRING);
        param.set_strvalue(nickName.GetRaw());
        params.push_back(param);
    }
    {
        VariantParam param;
        param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
        param.set_unsignedvalue(userId);
        params.push_back(param);
    }
    systemLog->AddLog(libraryInfo->id(), "QUIT_LIBRARY_LOG_TITLE", {}, "QUIT_LIBRARY_LOG_CONTENT", params);
    
    return true;
}

bool LibraryGlobal::_RemoveMember(LibraryInfo *libraryInfo, IUser *user)
{
    return _RemoveMember(libraryInfo, user->GetUserId());
}

void LibraryGlobal::_RemoveManager(LibraryInfo *libraryInfo, UInt64 userId)
{
    // 管理员列表
    const Int32 count = libraryInfo->managerinfolist_size();
    for(Int32 idx = count - 1; idx >= 0; --idx)
    {
        if(libraryInfo->managerinfolist(idx).userid() == userId)
        {
            libraryInfo->mutable_managerinfolist()->DeleteSubrange(idx, 1);
            MaskNumberKeyModifyDirty(libraryInfo->id());
        }
    }
}

void LibraryGlobal::_AddManger(LibraryInfo *libraryInfo, UInt64 userId)
{
    _RemoveManager(libraryInfo, userId);
    
    libraryInfo->add_managerinfolist()->set_userid(userId);
    MaskNumberKeyModifyDirty(libraryInfo->id());
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

bool LibraryGlobal::_IsManager(UInt64 libraryId, UInt64 userId) const
{
    auto memberInfo = GetMemberInfo(libraryId, userId);
    if(!memberInfo)
        return false;

    return _IsManager(memberInfo->role());
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

    g_Log->Info(LOGFMT_OBJ_TAG("library:%s, member user id:%llu transfer %d => %d"), LibraryToString(libraryInfo).c_str(), static_cast<UInt64>(memberInfo->userid()), role, targetRole);
    g_Log->Info(LOGFMT_OBJ_TAG("library:%s, member user id:%llu transfer %d => %d"), LibraryToString(libraryInfo).c_str(), static_cast<UInt64>(targetMember->userid()), targetRole, role);

    // TODO:系统日志 原图书馆馆长:【{0}】,用户id:{1}, 将图书馆转让给用户:【{2}】,用户id:{3}
    auto sysLog = GetGlobalSys<ISystemLogGlobal>();
    std::vector<VariantParam> params;
    {
        VariantParam param;
        param.set_varianttype(VariantParamType_ENUMS_STRING);
        param.set_strvalue(memberInfo->nickname());
        params.push_back(param);
    }
    {
        VariantParam param;
        param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
        param.set_unsignedvalue(memberInfo->userid());
        params.push_back(param);
    }
    {
        VariantParam param;
        param.set_varianttype(VariantParamType_ENUMS_STRING);
        param.set_strvalue(targetMember->nickname());
        params.push_back(param);
    }
    {
        VariantParam param;
        param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
        param.set_unsignedvalue(targetMember->userid());
        params.push_back(param);
    }
    sysLog->AddLog(libraryInfo->id(), "TRANSFER_LIBRARY_LOG_TITLE", {}, "TRANSFER_LIBRARY_LOG_CONTENT", params);

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
    LibraryInfoNty nty;
    auto newLibraryInfo = nty.mutable_libraryinfo();
    *newLibraryInfo = *libraryInfo;
    newLibraryInfo->clear_memberlist();

    // 图书列表单独请求
    newLibraryInfo->clear_booklist();

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
                for(auto &item : libraryInfo->memberlist())
                {
                    auto copyMemberInfo = item;
                    if(copyMemberInfo.userid() == memberInfo->userid())
                    {
                        *newLibraryInfo->mutable_memberlist()->Add() = copyMemberInfo;
                    }
                    else
                    {
                        copyMemberInfo.clear_borrowlist();
                        copyMemberInfo.set_locktimestampms(0);
                        copyMemberInfo.set_bindphone(0);
                        *newLibraryInfo->mutable_memberlist()->Add() = copyMemberInfo;
                    }
                }
            }break;
            default:
            {
                for(auto &item : libraryInfo->memberlist())
                {
                    auto copyMemberInfo = item;
                    if(copyMemberInfo.userid() == memberInfo->userid())
                    {
                        *newLibraryInfo->mutable_memberlist()->Add() = copyMemberInfo;
                    }
                    else
                    {
                        copyMemberInfo.clear_borrowlist();
                        copyMemberInfo.set_locktimestampms(0);
                        copyMemberInfo.set_bindphone(0);
                        *newLibraryInfo->mutable_memberlist()->Add() = copyMemberInfo;
                    }
                }
            }break;
        }
    }

    user->Send(Opcodes::OpcodeConst::OPCODE_LibraryInfoNty, nty);
}

void LibraryGlobal::_SendLibraryInfoNty(const IUser *user) const
{
    auto libraryMgr = user->GetSys<ILibraryMgr>();
    auto libraryInfo = GetLibraryInfo(libraryMgr->GetMyLibraryId());
    if(!libraryInfo)
        return;

    _SendLibraryInfoNty(user, libraryInfo);
}

void LibraryGlobal::_SendLibraryInfoNty(UInt64 userId, const LibraryInfo *libraryInfo) const
{
    auto user = GetGlobalSys<IUserMgr>()->GetUser(userId);
    if(!user)
        return;

    _SendLibraryInfoNty(user, libraryInfo);
}

void LibraryGlobal::_SendLibraryInfoNty(UInt64 userId, UInt64 libraryId) const
{
    auto user = GetGlobalSys<IUserMgr>()->GetUser(userId);
    if(!user)
        return;

    auto libraryInfo = GetLibraryInfo(libraryId);
    if(!libraryInfo)
        return;

    _SendLibraryInfoNty(user, libraryInfo);
}

Int32 LibraryGlobal::_SendOrderDetailInfoNty(const IUser *user) const
{
    auto libraryMgr = user->GetSys<ILibraryMgr>();
    const auto libraryId = libraryMgr->GetMyLibraryId();
    auto libraryInfo = GetLibraryInfo(libraryId);
    if(!libraryInfo)
    {
        return Status::NotJoinAnyLibrary;
    }

    // 管理员全部都拿, 非管理员只能拿自己
    bool isGetSelf = false;
    const auto userId = user->GetUserId();
    if(!_IsManager(libraryId, userId))
    {
        isGetSelf = true;
    }

    GetBookOrderDetailInfoNty nty;
    auto detailInfo = nty.mutable_detailinfo();
    auto memberUserId = isGetSelf ? userId : 0;
    _BuildOrderDetailInfo(libraryInfo, memberUserId, detailInfo);
    user->Send(Opcodes::OpcodeConst::OPCODE_GetBookOrderDetailInfoNty, nty);

    return Status::Success;
}

void LibraryGlobal::_Clear()
{
    KERNEL_NS::ContainerUtil::DelContainer2(_idRefLibraryInfo);
    _libraryIdRefUserRefMember.clear();
    _libraryIdRefIsbnRefBookInfo.clear();
    _libraryIdRefIdRefBookInfo.clear();
}

void LibraryGlobal::_MakeBookDict(UInt64 libraryId, BookInfo *bookInfo)
{
    {
        auto iter = _libraryIdRefIsbnRefBookInfo.find(libraryId);
        if(iter == _libraryIdRefIsbnRefBookInfo.end())
            iter = _libraryIdRefIsbnRefBookInfo.insert(std::make_pair(libraryId, std::map<KERNEL_NS::LibString, BookInfo *>())).first;
        iter->second.insert(std::make_pair(bookInfo->isbncode(), bookInfo));
    }

    {
        auto iter = _libraryIdRefIdRefBookInfo.find(libraryId);
        if(iter == _libraryIdRefIdRefBookInfo.end())
            iter = _libraryIdRefIdRefBookInfo.insert(std::make_pair(libraryId, std::map<UInt64, BookInfo *>())).first;
        iter->second.insert(std::make_pair(bookInfo->id(), bookInfo));
    }
}

BookInfo *LibraryGlobal::_GetBookInfo(UInt64 libraryId, const KERNEL_NS::LibString &isbnCode)
{
    auto iter = _libraryIdRefIsbnRefBookInfo.find(libraryId);
    if(iter == _libraryIdRefIsbnRefBookInfo.end())
        return NULL;
    auto iterBook = iter->second.find(isbnCode);
    return iterBook == iter->second.end() ? NULL : iterBook->second;
}

const BookInfo *LibraryGlobal::_GetBookInfo(UInt64 libraryId, const KERNEL_NS::LibString &isbnCode) const
{
    auto iter = _libraryIdRefIsbnRefBookInfo.find(libraryId);
    if(iter == _libraryIdRefIsbnRefBookInfo.end())
        return NULL;
    auto iterBook = iter->second.find(isbnCode);
    return iterBook == iter->second.end() ? NULL : iterBook->second;
}

const std::map<UInt64, BookInfo *> &LibraryGlobal::_GetBookInfos(UInt64 libraryId) const
{
    static std::map<UInt64, BookInfo *> s_empty;
    auto iter = _libraryIdRefIdRefBookInfo.find(libraryId);
    return iter == _libraryIdRefIdRefBookInfo.end() ? s_empty : iter->second;
}

void LibraryGlobal::_GetBooksAfter(const std::map<UInt64, BookInfo *> &totalBooks, UInt64 bookId, UInt32 bookCount, std::map<UInt64, const BookInfo *> &bookIdRefBook) const
{
    if(bookCount == 0)
        return;

    if(totalBooks.empty())
        return;

    auto iterBegin = totalBooks.begin();
    if(bookId > 0)
    {
        iterBegin = totalBooks.find(bookId);
        if(iterBegin != totalBooks.end())
            ++iterBegin;
    }

    if(iterBegin == totalBooks.end())
        return;

    UInt32 loopCount = 0;
    for(auto iter = iterBegin; iter != totalBooks.end(); ++iter)
    {
        ++loopCount;
        bookIdRefBook.insert(std::make_pair(iter->first, iter->second));

        if(loopCount >= bookCount)
            break;
    }
}

void LibraryGlobal::_GetBooksBefore(const std::map<UInt64, BookInfo *> &totalBooks, UInt64 bookId, UInt32 bookCount, std::map<UInt64, const BookInfo *> &bookIdRefBook) const
{
    if(bookCount == 0)
        return;

    if(totalBooks.empty())
        return;

    auto iterBase = totalBooks.find(bookId);
    if(iterBase == totalBooks.end())
        return;

    if(iterBase != totalBooks.begin())
    {
        UInt32 loopCount = 0;
        for(auto iter = --iterBase;; --iter)
        {
            ++loopCount;
            bookIdRefBook.insert(std::make_pair(iter->first, iter->second));

            if(loopCount >= bookCount)
                break;

            if(iter == totalBooks.begin())
                break;
        }
    }
}

void LibraryGlobal::_BuildBookInfos(const std::map<UInt64, const BookInfo *> &dict,  ::google::protobuf::RepeatedPtrField< ::CRYSTAL_NET::service::BookInfo > *bookInfoList) const
{
    for(auto iter : dict)
        *bookInfoList->Add() = *iter.second;
}

void LibraryGlobal::_MakeOrderDict(UInt64 libraryId, BorrowOrderInfo *orderInfo)
{
    auto iter = _libraryIdRefBorrowOrder.find(libraryId);
    if(iter == _libraryIdRefBorrowOrder.end())
        iter = _libraryIdRefBorrowOrder.insert(std::make_pair(libraryId, std::map<UInt64, BorrowOrderInfo *>())).first;

    iter->second.insert(std::make_pair(orderInfo->orderid(), orderInfo));
}

void LibraryGlobal::_CancelOrder(UInt64 libraryId, UInt64 orderId, Int32 cancelReason, const KERNEL_NS::LibString &detailReason)
{
    auto orderInfo = GetOrderInfo(libraryId, orderId);
    if(!orderInfo)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("order not found libraryId:%llu, orderId:%llu, cancel reason:%d, detail reason:%s")
        , libraryId, orderId, cancelReason, detailReason.c_str());
        return;
    }

    if(orderInfo->orderstate() > BorrowOrderState_ENUMS_WAIT_USER_RECEIVE)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("order state change before timeout, orderInfo:%s, libraryId:%llu, cancelReason:%d,%s")
        , orderInfo->ToJsonString().c_str(), libraryId, cancelReason, BorrowOrderState_ENUMS_Name(cancelReason).c_str());
        return;
    }

    // "订单等待用户领取超时取消, 订单号:{0}, 待领取用户id:{1}"
    auto sysLog = GetGlobalSys<ISystemLogGlobal>();
    std::vector<VariantParam> params;
    {
        VariantParam param;
        param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
        param.set_unsignedvalue(orderInfo->orderid());
        params.push_back(param);
    }
    {
        VariantParam param;
        param.set_varianttype(VariantParamType_ENUMS_UNSIGNED_VALUE);
        param.set_unsignedvalue(orderInfo->userid());
        params.push_back(param);
    }
    sysLog->AddLog(libraryId, "CANCEL_ORDER_LOG_TITLE", {}, "CANCEL_ORDER_WAIT_USER_RECEIVE_TIMEOUT_CONTENT", params);

    orderInfo->set_orderstate(BorrowOrderState_ENUMS_CANCEL_ORDER);
    orderInfo->mutable_cancelreason()->set_cancelreason(cancelReason);
    orderInfo->mutable_cancelreason()->set_cancelinfo(detailReason.GetRaw());
    MaskNumberKeyModifyDirty(libraryId);
}

void LibraryGlobal::_StartCacelOrderTimer(UInt64 libraryId, UInt64 orderId, Int64 delayMilliseconds)
{
    auto timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
    timer->GetMgr()->TakeOverLifeTime(timer, [](KERNEL_NS::LibTimer *t){
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(KERNEL_NS::KernelCastTo<KERNEL_NS::LibTimer>(t));
    });

    timer->SetTimeOutHandler([this, libraryId, orderId](KERNEL_NS::LibTimer *t) mutable
    {
        _CancelOrder(libraryId, orderId, CancelOrderReasonType_ENUMS_WAIT_USER_GET_TIME_OUT, "Waiting for user gotting time out." );
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
    });
    timer->Schedule(delayMilliseconds);
}

void LibraryGlobal::_RemoveConfirm(UInt64 confirmId)
{
    auto iter = _confirmCodeRefOrderId.find(confirmId);
    if(iter == _confirmCodeRefOrderId.end())
        return;

    const auto orderId = iter->second;
    _confirmCodeRefOrderId.erase(iter);

    auto iterConfirms = _orderIdRefConfirmCodes.find(orderId);
    if(iterConfirms == _orderIdRefConfirmCodes.end())
        return;

    iterConfirms->second.erase(confirmId);
    if(iterConfirms->second.empty())
        _orderIdRefConfirmCodes.erase(iterConfirms);
}

void LibraryGlobal::_RemoveOrderConfirm(UInt64 orderId)
{
    auto iterConfirms = _orderIdRefConfirmCodes.find(orderId);
    if(iterConfirms == _orderIdRefConfirmCodes.end())
        return;

    for(auto confirmId : iterConfirms->second)
        _confirmCodeRefOrderId.erase(confirmId);

    _orderIdRefConfirmCodes.erase(iterConfirms);
}

SERVICE_END

