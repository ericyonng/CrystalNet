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
// Date: 2026-01-18 20:01:12
// Author: Eric Yonng
// Description:


#include <pch.h>
#include <Comps/LogReciever/Impl/LogReciever.h>
#include <Comps/LogReciever/Impl/LogRecieverFactory.h>
#include <Common/TestAccount.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(ILogReciever);
POOL_CREATE_OBJ_DEFAULT_IMPL(LogReciever);

LogReciever::LogReciever()
    :ILogReciever(KERNEL_NS::RttiUtil::GetTypeId<LogReciever>())
,_thread(NULL)
,_dataList(KERNEL_NS::LibList<BroadcastSendDataNty *>::New_LibList())
,_swapList(KERNEL_NS::LibList<BroadcastSendDataNty *>::New_LibList())
{
    
}

LogReciever::~LogReciever()
{
    _Clear();
}

void LogReciever::Release()
{
    LogReciever::DeleteByAdapter_LogReciever(LogRecieverFactory::_buildType.V, this);
}

Int32 LogReciever::_OnGlobalSysInit()
{
    AccountData info;
    info.ip = "127.0.0.1";
    info.port = 3900;
    info.Account = "test_role_broadcast";
    info.Pwd = "1998569%&Jd20.";
    TestAccount::AccountInfo = info;

    GetService()->Subscribe(Opcodes::OpcodeConst::OPCODE_BroadcastSendDataNty, this, &LogReciever::_OnBroadcastSendDataNty);

    _thread = new KERNEL_NS::LibThread;
    
    return Status::Success;
}

void LogReciever::_OnGlobalSysClose()
{
    _Clear();
}

void LogReciever::_OnHostBeforeCompsWillClose()
{
    _thread->HalfClose();
    while(_lock.HasWaiter())
    {
        _lock.Sinal();
        KERNEL_NS::SystemUtil::ThreadSleep(1000);
        g_Log->Info(LOGFMT_OBJ_TAG("wait thread quit..."));
    }
    _thread->FinishClose();
}

void LogReciever::OnStartup()
{
    _thread->AddTask(this, &LogReciever::_OnFileThread);
    _thread->Start();
}


void LogReciever::_OnBroadcastSendDataNty(KERNEL_NS::LibPacket *&packet)
{
    auto nty = packet->GetCoder<BroadcastSendDataNty>();
    auto packetId = nty->packetid();
    _spinLock.Lock();
    _dataList->PushBack(nty);
    packet->PopCoder();
    _spinLock.Unlock();
    _lock.Sinal();

    // res
    BroadcastSendDataConfirmResponse res;
    res.set_packetid(packetId);
    Send(packet->GetSessionId(), Opcodes::OpcodeConst::OPCODE_BroadcastSendDataConfirmResponse, res, packet->GetPacketId());
}

void LogReciever::_OnFileThread(KERNEL_NS::LibThread *thread)
{
    g_Log->Info(LOGFMT_OBJ_TAG("_OnFileThread start"));
    KERNEL_NS::SmartPtr<FILE, KERNEL_NS::AutoDelMethods::CustomDelete> fp(KERNEL_NS::FileUtil::OpenFile("LogReciever.log", true));
    fp.SetClosureDelegate([](void *p)
    {
        auto ptr = KERNEL_NS::KernelCastTo<FILE>(p);
        KERNEL_NS::FileUtil::CloseFile(*ptr);
    });

    while (!thread->IsDestroy())
    {
        _spinLock.Lock();
        if(!_dataList->IsEmpty())
        {
            auto swapList = _swapList;
            _swapList = _dataList;
            _dataList = swapList;
            _spinLock.Unlock();

            for(auto iter = _swapList->Begin(); iter;)
            {
                auto data = iter->_data;
                iter = _swapList->Erase(iter);

                auto reqList = data->mutable_reqlist();

                for(auto iterPb = reqList->begin(); iterPb != reqList->end();++iterPb)
                {
                    auto &req = *iterPb;
                    auto dataList= req.mutable_datalist();
                    for(auto iterReq = dataList->begin(); iterReq != dataList->end();++iterReq)
                    {
                        auto &sourceData = *iterReq;
                        KERNEL_NS::LibString str;
                        auto sd = sourceData.mutable_data();
                        str.Swap(*sd);
                        KERNEL_NS::FileUtil::WriteFile(*fp, str);
                    }
                }

                data->Release();
            }

            KERNEL_NS::FileUtil::FlushFile(*fp);
        }
        else
        {
            _spinLock.Unlock();

            // 没数据则等待
            _lock.Lock();
            _lock.Wait();
            _lock.Unlock();
        }
    }

    g_Log->Info(LOGFMT_OBJ_TAG("_OnFileThread quit"));
}

void LogReciever::_Clear()
{
    CRYSTAL_RELEASE_SAFE(_thread);

    if(_dataList)
    {
        for(auto iter = _dataList->Begin(); iter;)
        {
            auto data = iter->_data;
            data->Release();
            iter = _dataList->Erase(iter);
        }
        KERNEL_NS::LibList<BroadcastSendDataNty *>::Delete_LibList(_dataList);
    }
    _dataList = NULL;

    if(_swapList)
    {
        for(auto iter = _swapList->Begin(); iter;)
        {
            auto data = iter->_data;
            data->Release();
            iter = _swapList->Erase(iter);
        }
        KERNEL_NS::LibList<BroadcastSendDataNty *>::Delete_LibList(_swapList);
    }
    _swapList = NULL;
}


SERVICE_END
