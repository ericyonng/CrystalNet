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

#pragma once


#include <Comps/UserSys/Library/interface/ILibraryMgr.h>
#include <protocols/protocols.h>
#include <kernel/kernel.h>

SERVICE_BEGIN

class UserLibraryInfo;

class LibraryMgr : public ILibraryMgr
{
    POOL_CREATE_OBJ_DEFAULT_P1(ILibraryMgr, LibraryMgr);

public:
    LibraryMgr();
    ~LibraryMgr();
    void Release() override;

   virtual Int32 OnLoaded(const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) override;
   virtual Int32 OnSave(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) const override;

    virtual void OnRegisterComps() override;
    virtual void OnLogin() override;
    virtual void OnLoginFinish() override;
    virtual void OnLogout() override;

    virtual UInt64 GetMyLibraryId() const override;

    OBJ_GET_OBJ_TYPEID_DECLARE();

protected:
    virtual Int32 _OnUserSysInit() override;
    virtual Int32 _OnHostStart() override;
    virtual void _OnSysClose() override;
    void _Clear();

    void _RegisterEvents();
    void _UnRegisterEvents();

    void _SendUserLibraryInfo() const;

    void _OnQuitLibrary();
    void _OnJoinLibrary(UInt64 libraryId);

    // 事件
    void _OnRemoveLibraryMember(KERNEL_NS::LibEvent *ev);
    void _OnJoinLibraryMember(KERNEL_NS::LibEvent *ev);

private:
    UserLibraryInfo *_libraryInfo;

    KERNEL_NS::ListenerStub _removeLibraryMemberStub;
    KERNEL_NS::ListenerStub _joinLibraryMemberStub;
};

SERVICE_END