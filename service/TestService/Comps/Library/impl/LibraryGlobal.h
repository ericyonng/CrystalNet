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

#pragma once

#include <Comps/Library/interface/ILibraryGlobal.h>
#include <protocols/protocols.h>
#include <kernel/comp/LibStream.h>
#include <kernel/comp/NetEngine/LibPacket.h>
#include <kernel/comp/LibTime.h>
#include <map>
#include <set>

SERVICE_BEGIN

class IUser;

class LibraryGlobal : public ILibraryGlobal
{
    POOL_CREATE_OBJ_DEFAULT_P1(ILibraryGlobal, LibraryGlobal);

public:
    LibraryGlobal();
    ~LibraryGlobal();
    void Release() override;
    void OnRegisterComps() override;

    Int32 OnLoaded(UInt64 key, const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) override;
    Int32 OnSave(UInt64 key, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) const override;
    
    const LibraryInfo *GetLibraryInfo(UInt64 libraryId) const override;
    LibraryInfo *GetLibraryInfo(UInt64 libraryId);
    const MemberInfo *GetMemberInfo(UInt64 libraryId, UInt64 userId) const override;
    MemberInfo *GetMemberInfo(UInt64 libraryId, UInt64 userId);
    virtual KERNEL_NS::LibString LibraryToString(const LibraryInfo *libraryInfo) const override;
    virtual KERNEL_NS::LibString LibraryToString(UInt64 libraryId) const override;

    virtual const BookInfo *GetBookInfo(UInt64 libraryId, UInt64 bookId) const override;
    BookInfo *GetBookInfo(UInt64 libraryId, UInt64 bookId);

    BorrowOrderInfo *GetOrderInfo(UInt64 libraryId, UInt64 orderId);
    const BorrowOrderInfo *GetOrderInfo(UInt64 libraryId, UInt64 orderId) const;

    virtual Int32 CreateBorrowOrder(UInt64 libraryId, const IUser *user, const BookBagInfo &bookBagInfo, const KERNEL_NS::LibString &remark) override;

    virtual bool IsManager(UInt64 libraryId, UInt64 userId) const override;

    OBJ_GET_OBJ_TYPEID_DECLARE();

protected:
    virtual Int32 _OnGlobalSysInit() override;

    virtual Int32 _OnGlobalSysCompsCreated() override;

    virtual void _OnGlobalSysClose() override;

    // 协议处理
    void _OnGetLibraryInfoReq(KERNEL_NS::LibPacket *&packet);
    void _OnGetLibraryListReq(KERNEL_NS::LibPacket *&packet);
    void _OnCreateLibraryReq(KERNEL_NS::LibPacket *&packet);
    void _OnJoinLibraryReq(KERNEL_NS::LibPacket *&packet);
    void _OnQuitLibraryReq(KERNEL_NS::LibPacket *&packet);
    void _OnTransferLibraianReq(KERNEL_NS::LibPacket *&packet);
    void _OnModifyMemberInfoReq(KERNEL_NS::LibPacket *&packet);
    void _OnGetLibraryMemberSimpleInfoReq(KERNEL_NS::LibPacket *&packet);
    void _OnAddLibraryBookReq(KERNEL_NS::LibPacket *&packet);
    void _OnGetBookInfoReq(KERNEL_NS::LibPacket *&packet);
    // 书名或者关键字
    void _OnGetBookByBookNameReq(KERNEL_NS::LibPacket *&packet);
    void _OnGetBookInfoListReq(KERNEL_NS::LibPacket *&packet);
    void _OnGetBookOrderDetailInfoReq(KERNEL_NS::LibPacket *&packet);
    void _OnOutStoreOrderReq(KERNEL_NS::LibPacket *&packet);
    void _OnManagerScanOrderForUserGettingBooksReq(KERNEL_NS::LibPacket *&packet);
    void _OnUserGetBooksOrderConfirmReq(KERNEL_NS::LibPacket *&packet);
    void _OnCancelOrderReq(KERNEL_NS::LibPacket *&packet);
    void _OnReturnBackReq(KERNEL_NS::LibPacket *&packet);

    void _BuildOrderDetailInfo(const LibraryInfo *libraryInfo, UInt64 memberUserId, ::google::protobuf::RepeatedPtrField<::CRYSTAL_NET::service::BorrowOrderDetailInfo> *detailInfoList) const;
    void _BuildOrderDetailInfo(UInt64 libraryId, const MemberInfo *memberInfo, ::google::protobuf::RepeatedPtrField<::CRYSTAL_NET::service::BorrowOrderDetailInfo> *detailInfoList) const;
    Int32 _ContinueModifyMember(LibraryInfo *libraryInfo, UInt64 reqUserId, IUser *targetUser, const ModifyMemberInfoReq &req);

    void _BuildPreviewInfo(LibraryPreviewInfo *previewInfo, const LibraryInfo *libraryInfo) const;

    void _SendLibraryInfoNty(const IUser *user, const LibraryInfo *libraryInfo) const;
    void _SendLibraryInfoNty(const IUser *user) const;
    void _SendLibraryInfoNty(UInt64 userId, const LibraryInfo *libraryInfo) const;
    void _SendLibraryInfoNty(UInt64 userId, UInt64 libraryId) const;
    Int32 _SendOrderDetailInfoNty(const IUser *user) const;

    KERNEL_NS::LibString _MemberToString(const MemberInfo *memberInfo) const;

    LibraryInfo *_CreateLibrary(IUser *user, const KERNEL_NS::LibString &libraryName, const KERNEL_NS::LibString &address, const KERNEL_NS::LibString &openTime, const KERNEL_NS::LibString &telphoneNumber, UInt64 bindPhone);
    void _JoinMember(LibraryInfo *libraryInfo, IUser *user, Int32 roleType);

    bool _IsReturnBackAllBook(const MemberInfo *memberInfo) const;

    bool _DoesRoleHaveAuthToBorrow(Int32 role) const;

    // 是否有逾期
    bool _HasOverDeadlineOrder(const MemberInfo *memberInfo, const KERNEL_NS::LibTime &nowTime) const;
    bool _HasOberDeadlineBook(const BorrowOrderInfo *orderInfo, const KERNEL_NS::LibTime &nowTime) const;

    bool _RemoveMember(LibraryInfo *libraryInfo, UInt64 userId);
    bool _RemoveMember(LibraryInfo *libraryInfo, IUser *user);
    void _RemoveManager(LibraryInfo *libraryInfo, UInt64 userId);
    void _AddManger(LibraryInfo *libraryInfo, UInt64 userId);

    bool _IsManager(Int32 roleType) const;
    bool _IsManager(UInt64 libraryId, UInt64 userId) const;

    bool _CanHandle(UInt64 libraryId, UInt64 userId) const;
    void _LockMember(UInt64 libraryId, UInt64 userId, Int64 timeoutMs = 5000);
    void _UnlockMember(UInt64 libraryId, UInt64 userId);

    void _TransferMember(LibraryInfo *libraryInfo, MemberInfo *memberInfo, MemberInfo *targetMember);

    // 转正常成员 RoleType > NoAuth
    Int32 _CheckTurnNormalMember(const LibraryInfo *libraryInfo, const IUser *user) const;

    bool _IsValidPhone(UInt64 phoneNumber) const;

    void _Clear();

    // 图书字典
    void _MakeBookDict(UInt64 libraryId, BookInfo *bookInfo);
    BookInfo *_GetBookInfo(UInt64 libraryId, const KERNEL_NS::LibString &isbnCode);
    const BookInfo *_GetBookInfo(UInt64 libraryId, const KERNEL_NS::LibString &isbnCode) const;
    const std::map<UInt64, BookInfo *> &_GetBookInfos(UInt64 libraryId) const;
    // 后n本图书
    void _GetBooksAfter(const std::map<UInt64, BookInfo *> &totalBooks, UInt64 bookId, UInt32 bookCount, std::map<UInt64, const BookInfo *> &bookIdRefBook) const;
    // 前n本图书
    void _GetBooksBefore(const std::map<UInt64, BookInfo *> &totalBooks, UInt64 bookId, UInt32 bookCount, std::map<UInt64, const BookInfo *> &bookIdRefBook) const;
    void _BuildBookInfos(const std::map<UInt64, const BookInfo *> &dict,  ::google::protobuf::RepeatedPtrField< ::CRYSTAL_NET::service::BookInfo > *bookInfoList) const;
    
    // 订单
    void _MakeOrderDict(UInt64 libraryId, BorrowOrderInfo *orderInfo);

    // 取消订单
    void _CancelOrder(UInt64 libraryId, UInt64 orderId, Int32 cancelReason, const KERNEL_NS::LibString &detailReason);
    void _StartCacelOrderTimer(UInt64 libraryId, UInt64 orderId, Int64 delayMilliseconds);

    void _RemoveConfirm(UInt64 confirmId);
    void _RemoveOrderConfirm(UInt64 orderId);

private:
    std::map<UInt64, LibraryInfo *> _idRefLibraryInfo;

    std::map<UInt64, std::map<UInt64, MemberInfo *>> _libraryIdRefUserRefMember;

    // 图书
    std::map<UInt64, std::map<KERNEL_NS::LibString, BookInfo *>> _libraryIdRefIsbnRefBookInfo;
    std::map<UInt64, std::map<UInt64, BookInfo *>> _libraryIdRefIdRefBookInfo;

    // 订单
    std::map<UInt64, std::map<UInt64, BorrowOrderInfo *>> _libraryIdRefBorrowOrder;

    // 领取图书确认码 确认码 => 订单id
    std::map<UInt64, UInt64> _confirmCodeRefOrderId;
    std::map<UInt64, std::set<UInt64>> _orderIdRefConfirmCodes;
};

ALWAYS_INLINE BorrowOrderInfo *LibraryGlobal::GetOrderInfo(UInt64 libraryId, UInt64 orderId)
{
    auto iter = _libraryIdRefBorrowOrder.find(libraryId);
    if(iter == _libraryIdRefBorrowOrder.end())
        return NULL;

    auto iterOrder = iter->second.find(orderId);
    return iterOrder == iter->second.end() ? NULL : iterOrder->second;
}

ALWAYS_INLINE const BorrowOrderInfo *LibraryGlobal::GetOrderInfo(UInt64 libraryId, UInt64 orderId) const
{
    auto iter = _libraryIdRefBorrowOrder.find(libraryId);
    if(iter == _libraryIdRefBorrowOrder.end())
        return NULL;

    auto iterOrder = iter->second.find(orderId);
    return iterOrder == iter->second.end() ? NULL : iterOrder->second;
}

SERVICE_END