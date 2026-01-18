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
// Date: 2026-01-17 23:01:04
// Author: Eric Yonng
// Description:


#pragma once

#include <Comps/MiddleNode/Interface/IMiddleNodeMgr.h>
#include <protocols/protocols.h>

KERNEL_BEGIN

class LibPacket;

KERNEL_END

SERVICE_BEGIN

enum AccountType : Int32
{
    Normal = 0,

    BroadcastRole = 1,
};

class ReqInfo
{
    POOL_CREATE_OBJ_DEFAULT(ReqInfo);

public:
    ReqInfo()
        :_req(NULL)
    ,_msTime(0)
    {
        
    }

    ~ReqInfo()
    {
        _req->Release();
        _req = NULL;
    }
    
    void Release();
    SendDataRequest *_req;
    Int64 _msTime;
};

class ReqInfoCompare
{
public:
    bool operator()(const ReqInfo * left, const ReqInfo * right) const;
};

class MiddleNodeMgr : public IMiddleNodeMgr
{
    POOL_CREATE_OBJ_DEFAULT_P1(IMiddleNodeMgr, MiddleNodeMgr);
    
public:
    MiddleNodeMgr();
    ~MiddleNodeMgr() override;
    void Release() override;

private:
    Int32 _OnGlobalSysInit() override;
    void _OnGlobalSysClose() override;

    // 消息
    void _OnSendDataRequest(KERNEL_NS::LibPacket *&packet);
    void _OnBroadcastSendDataConfirmResponse(KERNEL_NS::LibPacket *&packet);
    void _OnResendTimer(KERNEL_NS::LibTimer *timer);

    void _Clear();
    
    std::set<SERVICE_NS::ReqInfo *, ReqInfoCompare> _dataSortedByTime;

    // 待确认的包, 确认后list的数据会被清空, 并发送下一批数据, 
    BroadcastSendDataNty _requestWaitConfirm;
    Int64 _waitConfirmId;

    // 超时重传
    KERNEL_NS::LibTimer *_reSendTimer;
    KERNEL_NS::TimeSlice _reSendInterval;

    // bitmap
    std::map<Int64, Int64> _packetIdBitmap;
};

SERVICE_END