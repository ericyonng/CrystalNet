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
 * Date: 2023-09-17 19:55:11
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <Comps/PassTime/interface/IPassTimeGlobal.h>
#include <protocols/protocols.h>
#include <kernel/comp/LibStream.h>
#include <kernel/comp/LibTime.h>
#include <kernel/comp/Timer/Timer.h>

SERVICE_BEGIN

class PassTimeGlobal : public IPassTimeGlobal
{
    POOL_CREATE_OBJ_DEFAULT_P1(IPassTimeGlobal, PassTimeGlobal);

public:
    PassTimeGlobal();
    ~PassTimeGlobal();
    void Release() override;
    void OnRegisterComps() override;

    Int32 OnLoaded(UInt64 key, const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) override;
    Int32 OnSave(UInt64 key, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) const override;
    
    virtual void CheckPassTime() override;

    OBJ_GET_OBJ_TYPEID_DECLARE();

private:
    void _OnZeroTimeOut(KERNEL_NS::LibTimer *t);
    void _DoCheckPassTime(const KERNEL_NS::LibTime &nowTime);

    void _Clear();

private:
    UInt64 _key;
    PassTimeData *_passTimeData;
    KERNEL_NS::LibTimer *_timer;
};

SERVICE_END