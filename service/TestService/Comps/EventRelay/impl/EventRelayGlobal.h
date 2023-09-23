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
 * Date: 2023-09-19 21:33:00
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <Comps/EventRelay/interface/IEventRelayGlobal.h>

SERVICE_BEGIN

// 从service转发到其他事件管理器
class EventRelayGlobal : public IEventRelayGlobal
{
    POOL_CREATE_OBJ_DEFAULT_P1(IEventRelayGlobal, EventRelayGlobal);

public:
    EventRelayGlobal();
    ~EventRelayGlobal();
    void Release() override;
    void OnRegisterComps() override;


private:
    virtual Int32 _OnGlobalSysInit() override;

    virtual Int32 _OnGlobalSysCompsCreated() override;

    virtual void _OnGlobalSysClose() override;

    void _Clear();

    void _RegisterEvents();
    void _UnRegisterEvents();

    // 事件转发
    void _OnRemoveLibraryMember(KERNEL_NS::LibEvent *ev);
    void _OnJoinLibraryMember(KERNEL_NS::LibEvent *ev);
    void _OnUserObjCreated(KERNEL_NS::LibEvent *ev);
    void _OnUserObjWillRemove(KERNEL_NS::LibEvent *ev);

    void _EventFromUserToGlobal(KERNEL_NS::LibEvent *ev);
    
private:
    KERNEL_NS::ListenerStub _removeLibraryMemberStub;
    KERNEL_NS::ListenerStub _joinLibraryMemberStub;
    KERNEL_NS::ListenerStub _userObjCreatedStub;
    KERNEL_NS::ListenerStub _userObjWillRemoveStub;

    std::map<UInt64, KERNEL_NS::ListenerStub> _fromUserToGlobalStub;
};

SERVICE_END
