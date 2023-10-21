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
 * Date: 2023-10-21 19:22:11
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <Comps/Offline/interface/IOfflineGlobal.h>

SERVICE_BEGIN

class OfflineGlobal : public IOfflineGlobal
{
    POOL_CREATE_OBJ_DEFAULT_P1(IOfflineGlobal, OfflineGlobal);

public:
    OfflineGlobal();
    ~OfflineGlobal();
    void Release() override;
    void OnRegisterComps() override;

    Int32 OnLoaded(UInt64 key, const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) override;
    Int32 OnSave(UInt64 key, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) const override;
    
    virtual bool AddOfflineData(Int32 offlineType, UInt64 userId, const KERNEL_NS::LibString &offlineData) override;

private:
    virtual Int32 _OnGlobalSysInit() override;
    virtual void _OnGlobalSysClose() override;

    void _Clear();

    void _RegisterEvents();
    void _UnRegisterEvents();

    void _OnAfterUserLoaded(KERNEL_NS::LibEvent *ev);

    void _MakeDict(UInt64 id, OfflineData *offlineData);

private:
    KERNEL_NS::ListenerStub _afteUserLoadedStub;

    std::map<UInt64, OfflineData *> _idRefOfflineData;
    std::map<UInt64, std::map<UInt64, OfflineData *>> _userIdRefIdRefOfflineData;
};

SERVICE_END