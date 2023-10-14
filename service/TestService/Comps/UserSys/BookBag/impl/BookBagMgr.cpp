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

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(IBookBagMgr);
POOL_CREATE_OBJ_DEFAULT_IMPL(BookBagMgr);

BookBagMgr::BookBagMgr()
:_bookBagInfo(new BookBagInfo)
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
    Send(Opcodes::OpcodeConst::OPCODE_BookBagInfoNty, nty);
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

}

void BookBagMgr::_UnRegisterEvents()
{

}

SERVICE_END
