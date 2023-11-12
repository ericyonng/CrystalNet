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
 * Date: 2023-10-22 17:38:52
 * Author: Eric Yonng
 * Description: 
*/


#pragma once

#include <Comps/Notify/interface/INotifyGlobal.h>

SERVICE_BEGIN

class NotifyGlobal : public INotifyGlobal
{
    POOL_CREATE_OBJ_DEFAULT_P1(INotifyGlobal, NotifyGlobal);

public:
    NotifyGlobal();
    ~NotifyGlobal();
    void Release() override;

    virtual void SendNotify(UInt64 userId, const KERNEL_NS::LibString &titleId
    , const std::vector<VariantParam> &titleParams
    , const KERNEL_NS::LibString &contentId
    , const std::vector<VariantParam> &contentParams) override;
    
private:
    virtual Int32 _OnGlobalSysInit() override;

    void _OnReadNotifyReq(KERNEL_NS::LibPacket *&packet);
    void _OnOnekeyClearNotifyReq(KERNEL_NS::LibPacket *&packet);

private:
};

SERVICE_END