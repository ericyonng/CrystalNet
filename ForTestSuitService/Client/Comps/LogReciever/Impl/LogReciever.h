// MIT License
// 
// Copyright (c) 2020 ericyonng<120453674@qq.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// Date: 2026-01-18 20:01:00
// Author: Eric Yonng
// Description:


#pragma once

#include <service/Client/Comps/LogReciever/Interface/ILogReciever.h>
#include <protocols/protocols.h>

KERNEL_BEGIN
class LibPacket;
class LibEventLoopThread;
class LibThread;
KERNEL_END

SERVICE_BEGIN

class LogReciever : public ILogReciever
{
    POOL_CREATE_OBJ_DEFAULT_P1(ILogReciever, LogReciever);
    
public:
    LogReciever();
    ~LogReciever() override;
    void Release() override;

private:
    Int32 _OnGlobalSysInit() override;
    void _OnGlobalSysClose() override;
    virtual void _OnHostBeforeCompsWillClose() override;
    virtual void OnStartup() override;

    void _OnBroadcastSendDataNty(KERNEL_NS::LibPacket *&packet);

    void _OnFileThread(KERNEL_NS::LibThread *thread);

    void _Clear();

    KERNEL_NS::LibThread *_thread;
    KERNEL_NS::ConditionLocker _lock;
    KERNEL_NS::SpinLock _spinLock;
    KERNEL_NS::LibList<BroadcastSendDataNty *> *_dataList;
    KERNEL_NS::LibList<BroadcastSendDataNty *> *_swapList;

    // bitmap
    std::map<Int64, Int64> _packetIdBitmap;
};

SERVICE_END