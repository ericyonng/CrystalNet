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
 * Date: 2023-10-14 20:51:00
 * Author: Eric Yonng
 * Description: 
*/
#include <pch.h>
#include <Comps/UserSys/BookBag/impl/BookBagMgr.h>
#include <Comps/UserSys/BookBag/impl/BookBagMgrFactory.h>
#include <protocols/protocols.h>
#include <Comps/Library/library.h>
#include <Comps/UserSys/Library/Library.h>
#include <Comps/config/config.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(IBookBagMgr);
POOL_CREATE_OBJ_DEFAULT_IMPL(BookBagMgr);

BookBagMgr::BookBagMgr()
:_bookBagInfo(new BookBagInfo)
,_quitLibraryStub(INVALID_LISTENER_STUB)
,_joinLibraryStub(INVALID_LISTENER_STUB)
{

}

BookBagMgr::~BookBagMgr()
{
    _Clear();
}

void BookBagMgr::Release()
{
    BookBagMgr::DeleteByAdapter_BookBagMgr(BookBagMgrFactory::_buildType.V, this);
}

Int32 BookBagMgr::OnLoaded(const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db)
{
    if(UNLIKELY(!_bookBagInfo->FromJsonString(db.GetReadBegin(), static_cast<size_t>(db.GetReadableSize()))))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("parse book bag info fail user:%s"), GetUser()->ToString().c_str());
        return Status::ParseFail;
    }

    return Status::Success;
}

Int32 BookBagMgr::OnSave(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) const
{
    KERNEL_NS::LibString data;
    if(UNLIKELY(!_bookBagInfo->ToJsonString(&(data.GetRaw()))))
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

void BookBagMgr::OnLogin()
{

}

void BookBagMgr::OnLoginFinish()
{
    SendBookBagInfoNty();
}

void BookBagMgr::OnLogout()
{

}

void BookBagMgr::SendBookBagInfoNty() const
{
    BookBagInfoNty nty;
    *nty.mutable_bookbaginfo() = *_bookBagInfo;
    auto libraryGlobal = GetUserMgr()->GetGlobalSys<ILibraryGlobal>();

    auto libraryMgr = GetUser()->GetSys<ILibraryMgr>();
    if(libraryMgr->GetMyLibraryId())
    {
        auto bookInfoList = nty.mutable_bookinfolist();
        for(auto &item : _bookBagInfo->bookinfoitemlist())
        {
            auto bookInfo = libraryGlobal->GetBookInfo(libraryMgr->GetMyLibraryId(), item.bookid());
            if(!bookInfo)
                continue;

            auto newInfo = bookInfoList->Add();
            *newInfo = *bookInfo;
        }
    }

    Send(Opcodes::OpcodeConst::OPCODE_BookBagInfoNty, nty);
}

Int32 BookBagMgr::SetBookBagInfo(const BookInfoItem &item)
{
    Int32 err = Status::Success;
    auto libraryMgr = GetUser()->GetSys<ILibraryMgr>();
    do
    {
        if(libraryMgr->GetMyLibraryId() == 0)
        {
            err = Status::NotJoinAnyLibrary;
            g_Log->Warn(LOGFMT_OBJ_TAG("not join any library user:%s"), GetUser()->ToString().c_str());
            break;
        }

        // 图书是否存在
        auto libraryGlobal = GetUserMgr()->GetGlobalSys<ILibraryGlobal>();
        auto bookInfo = libraryGlobal->GetBookInfo(libraryMgr->GetMyLibraryId(), item.bookid());
        if(!bookInfo)
        {
            err = Status::BookNotFound;
            g_Log->Warn(LOGFMT_OBJ_TAG("book not exists book id:%llu, library id:%llu user:%s")
            , item.bookid(), libraryMgr->GetMyLibraryId(), GetUser()->ToString().c_str());
            break;
        }

        if(item.bookcount() < 0)
        {
            err = Status::ParamError;
            g_Log->Warn(LOGFMT_OBJ_TAG("book count error book id:%llu, library id:%llu user:%s")
            , item.bookid(), libraryMgr->GetMyLibraryId(), GetUser()->ToString().c_str());
            break;
        }

        // 是否超过库存
        if(bookInfo->variantinfo().count() < item.bookcount())
        {
            err = Status::BookCountOverCapacity;
            g_Log->Warn(LOGFMT_OBJ_TAG("book over count%lld, item bookcount:%lld book id:%llu, library id:%llu user:%s")
            ,bookInfo->variantinfo().count(), item.bookcount(), item.bookid(), libraryMgr->GetMyLibraryId(), GetUser()->ToString().c_str());
            break;
        }

        
        bool isExists = false;
        const Int32 count = _bookBagInfo->bookinfoitemlist_size();
        for(Int32 idx = 0; idx < count; ++idx)
        {
            auto bookInfoItem = _bookBagInfo->mutable_bookinfoitemlist(idx);
            if(bookInfoItem->bookid() == item.bookid())
            {
                if(item.bookcount() == 0)
                {
                    _bookBagInfo->mutable_bookinfoitemlist()->DeleteSubrange(idx, 1);
                }
                else
                {
                    bookInfoItem->set_bookcount(item.bookcount());
                }

                MaskDirty();
                isExists = true;
                break;
            }
        }

        if(!isExists)
        {
            if(item.bookcount() != 0)
            {
                auto newItem = _bookBagInfo->add_bookinfoitemlist();
                *newItem = item;
                MaskDirty();
            }
        }

        SendBookBagInfoNty();
    } while (false);

    return err;
}

Int32 BookBagMgr::Submit(const ::google::protobuf::RepeatedPtrField< ::CRYSTAL_NET::service::BorrowBookItem >&borrowBookItemList)
{
    // 1.书袋中有没有书
    if(_bookBagInfo->bookinfoitemlist_size() == 0)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("have no books user:%s"), GetUser()->ToString().c_str());
        return Status::HaveNoBook;
    }

    // 2.参数有没有与书袋中书匹配
    if(_bookBagInfo->bookinfoitemlist_size() != borrowBookItemList.size())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("param not match, bag item size:%d, param size:%d user:%s"), _bookBagInfo->bookinfoitemlist_size(), borrowBookItemList.size(), GetUser()->ToString().c_str());
        return Status::ParamError;
    }

    std::set<UInt64> bookIds;
    for(auto &item : _bookBagInfo->bookinfoitemlist())
    {
        if(item.bookcount() == 0)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("bookcount is zero book id:%llu, user:%s"), item.bookid(), borrowBookItemList.size(), GetUser()->ToString().c_str());
            return Status::ParamError;
        }

        bookIds.insert(item.bookid());
    }

    auto maxBorrowDaysConfig = GetService()->GetComp<ConfigLoader>()->GetComp<CommonConfigMgr>()->GetConfigById(CommonConfigIdEnums::MAX_BORROW_DAYS);
    std::set<UInt64> borrowParams;
    for(auto &item : borrowBookItemList)
    {
        if(item.borrowdays() == 0)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("borrowdays is zero book id:%llu, user:%s"), item.bookid(), borrowBookItemList.size(), GetUser()->ToString().c_str());
            return Status::ParamError;
        }

        borrowParams.insert(item.bookid());
    }

    // 3.借阅天数是0的返回错误
    // 4.借阅数量是0的报错误
    // 5.借阅天数不可超过配置的天数(30天等)

    return Status::Success;
}

Int32 BookBagMgr::_OnUserSysInit()
{
    _RegisterEvents();
    return Status::Success;
}

Int32 BookBagMgr::_OnHostStart()
{
    return Status::Success;
}

void BookBagMgr::_OnSysClose() 
{
    _Clear();
}

void BookBagMgr::_Clear()
{
    CRYSTAL_RELEASE_SAFE(_bookBagInfo);
}

void BookBagMgr::_RegisterEvents()
{
    if(_quitLibraryStub == INVALID_LISTENER_STUB)
    {
        _quitLibraryStub = AddListener(EventEnums::REMOVE_LIBRARY_MEMBER, this, &BookBagMgr::_OnQuitLibrary);
    }

    if(_joinLibraryStub == INVALID_LISTENER_STUB)
    {
        _joinLibraryStub = AddListener(EventEnums::JOIN_LIBRARY_MEMBER, this, &BookBagMgr::_OnJoinLibrary);
    }
}

void BookBagMgr::_UnRegisterEvents()
{
    if(_quitLibraryStub != INVALID_LISTENER_STUB)
    {
        RemoveListenerX(_quitLibraryStub);
    }

    if(_joinLibraryStub != INVALID_LISTENER_STUB)
    {
        RemoveListenerX(_joinLibraryStub);
    }
}

void BookBagMgr::_OnQuitLibrary(KERNEL_NS::LibEvent *ev)
{
    _bookBagInfo->Clear();
    MaskDirty();

    SendBookBagInfoNty();
}

void BookBagMgr::_OnJoinLibrary(KERNEL_NS::LibEvent *ev)
{
    _bookBagInfo->Clear();
    MaskDirty();

    SendBookBagInfoNty();
}

SERVICE_END
