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

#pragma once

#include <Comps/BookBag/interface/IBookBagGlobal.h>
#include <kernel/comp/LibStream.h>

KERNEL_BEGIN

class LibPacket;

KERNEL_END

SERVICE_BEGIN

class IUser;

class BookBagGlobal : public IBookBagGlobal
{
    POOL_CREATE_OBJ_DEFAULT_P1(IBookBagGlobal, BookBagGlobal);

public:
    BookBagGlobal();
    ~BookBagGlobal();
    void Release() override;
    void OnRegisterComps() override;

    Int32 OnLoaded(UInt64 key, const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) override;
    Int32 OnSave(UInt64 key, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) const override;
    
protected:
    virtual Int32 _OnGlobalSysInit() override;

    virtual Int32 _OnGlobalSysCompsCreated() override;

    virtual void _OnGlobalSysClose() override;

    void _Clear();

    // 协议
    void _OnBookBagInfoReq(KERNEL_NS::LibPacket *&packet);
    void _OnSetBookBagInfoReq(KERNEL_NS::LibPacket *&packet);
    void _OnSubmitBookBagBorrowInfoReq(KERNEL_NS::LibPacket *&packet);
    
};

SERVICE_END