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
 * Date: 2023-10-15 16:30:11
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <Comps/BookBag/impl/BookBagGlobal.h>
#include <Comps/BookBag/impl/BookBagGlobalFactory.h>
#include <Comps/BookBag/impl/BookBagGlobalStorageFactory.h>
#include <protocols/protocols.h>
#include <Comps/User/User.h>
#include <Comps/UserSys/UserSys.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(IBookBagGlobal);
POOL_CREATE_OBJ_DEFAULT_IMPL(BookBagGlobal);

BookBagGlobal::BookBagGlobal()
{

}

BookBagGlobal::~BookBagGlobal()
{
    _Clear();
}

void BookBagGlobal::Release()
{
    BookBagGlobal::DeleteByAdapter_BookBagGlobal(BookBagGlobalFactory::_buildType.V, this);
}

void BookBagGlobal::OnRegisterComps()
{
    RegisterComp<BookBagGlobalStorageFactory>();
}

Int32 BookBagGlobal::OnLoaded(UInt64 key, const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db)
{
    return Status::Success;
}

Int32 BookBagGlobal::OnSave(UInt64 key, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) const
{
    return Status::Success;
}
    
Int32 BookBagGlobal::_OnGlobalSysInit()
{
    auto service = GetService();
    service->Subscribe(Opcodes::OpcodeConst::OPCODE_BookBagInfoReq, this, &BookBagGlobal::_OnBookBagInfoReq);
    service->Subscribe(Opcodes::OpcodeConst::OPCODE_SetBookBagInfoReq, this, &BookBagGlobal::_OnSetBookBagInfoReq);

    return Status::Success;
}

Int32 BookBagGlobal::_OnGlobalSysCompsCreated()
{
    return Status::Success;
}

void BookBagGlobal::_OnGlobalSysClose()
{
    _Clear();
}

void BookBagGlobal::_Clear()
{

}

void BookBagGlobal::_OnBookBagInfoReq(KERNEL_NS::LibPacket *&packet)
{
    auto user = GetGlobalSys<IUserMgr>()->GetUserBySessionId(packet->GetSessionId());
    if(UNLIKELY(!user))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user not online packet:%s"), packet->ToString().c_str());
        return;
    }

    auto bookBagMgr = user->GetSys<IBookBagMgr>();
    bookBagMgr->SendBookBagInfoNty();

    BookBagInfoRes res;
    res.set_errcode(Status::Success);
    user->Send(Opcodes::OpcodeConst::OPCODE_BookBagInfoRes, res, packet->GetPacketId());
}

void BookBagGlobal::_OnSetBookBagInfoReq(KERNEL_NS::LibPacket *&packet)
{
    auto user = GetGlobalSys<IUserMgr>()->GetUserBySessionId(packet->GetSessionId());
    if(UNLIKELY(!user))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("user not online packet:%s"), packet->ToString().c_str());
        return;
    }

    auto req = packet->GetCoder<SetBookBagInfoReq>();
    
    auto bookBagMgr = user->GetSys<IBookBagMgr>();
    auto err = bookBagMgr->SetBookBagInfo(req->bookinfoitem());

    SetBookBagInfoRes res;
    res.set_errcode(err);
    user->Send(Opcodes::OpcodeConst::OPCODE_SetBookBagInfoRes, res, packet->GetPacketId());
}

SERVICE_END

