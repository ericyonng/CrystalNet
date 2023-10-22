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
 * Date: 2023-09-14 16:07:11
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <ServiceCompHeader.h>

SERVICE_BEGIN

class LibraryInfo;
class BookBagInfo;
class IUser;

class ILibraryGlobal : public IGlobalSys
{
    POOL_CREATE_OBJ_DEFAULT_P1(IGlobalSys, ILibraryGlobal);

public:
    // 获取图书馆信息
    virtual const LibraryInfo *GetLibraryInfo(UInt64 libraryId) const = 0;

    // 获取成员信息
    virtual const MemberInfo *GetMemberInfo(UInt64 libraryId, UInt64 userId) const = 0;

    // 图书馆信息
    virtual KERNEL_NS::LibString LibraryToString(const LibraryInfo *libraryInfo) const = 0;
    virtual KERNEL_NS::LibString LibraryToString(UInt64 libraryId) const = 0;

    // 获取图书信息
    virtual const BookInfo *GetBookInfo(UInt64 libraryId, UInt64 bookId) const = 0;

    // 创建订单
    virtual Int32 CreateBorrowOrder(UInt64 libraryId, const IUser *user, const BookBagInfo &bookBagInfo) = 0;
};

SERVICE_END
