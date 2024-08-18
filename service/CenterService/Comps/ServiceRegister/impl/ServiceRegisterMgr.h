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
 * Date: 2023-08-08 01:50:38
 * Author: Eric Yonng
 * Description: TODO:willstartup/startup需要由某个时刻驱动
*/

#pragma once

#include <Comps/ServiceRegister/interface/IServiceRegisterMgr.h>

SERVICE_BEGIN

class ServiceRegisterMgr : public IServiceRegisterMgr
{
    POOL_CREATE_OBJ_DEFAULT_P1(IServiceRegisterMgr, ServiceRegisterMgr);

public:
    ServiceRegisterMgr();
    ~ServiceRegisterMgr();
    void Release() override;

    virtual void OnWillStartup() override;
    virtual void OnStartup() override;

    virtual void OnRegisterComps() override;

    OBJ_GET_OBJ_TYPEID_DECLARE();

protected:
   Int32 _OnGlobalSysInit() override;
   Int32 _OnHostStart() override;
   void _OnGlobalSysClose() override;

private:
    void _Clear();


};

SERVICE_END
