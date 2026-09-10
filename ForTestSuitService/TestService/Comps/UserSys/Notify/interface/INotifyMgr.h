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
 * Date: 2023-10-22 18:18:33
 * Author: Eric Yonng
 * Description: 
*/


#pragma once

#include <ServiceCompHeader.h>
#include <Comps/User/interface/IUserSys.h>

SERVICE_BEGIN

class UserNotifyDataItem;

class INotifyMgr : public IUserSys
{
    POOL_CREATE_OBJ_DEFAULT_P1(IUserSys, INotifyMgr);

public:
    INotifyMgr(UInt64 objTypeId) : IUserSys(objTypeId) {}
    virtual void AddNotify(const UserNotifyDataItem &item) = 0;
    
    virtual Int32 ReadNotify(UInt64 notifyId) = 0;

    virtual void OnekeyClearNotify(Int32 clearType) = 0;
};

SERVICE_END