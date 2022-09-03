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
 * Author: Eric Yonng
 * Date: 2021-03-22 17:33:46
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/NetEngine/LibIocp.h>
#include <kernel/comp/Log/log.h>
#include <kernel/comp/Utils/SocketUtil.h>
#include <kernel/comp/NetEngine/Defs/IoData.h>
#include <kernel/comp/NetEngine/Defs/IoEvent.h>
#include <kernel/comp/Utils/SocketUtil.h>

#if CRYSTAL_TARGET_PLATFORM_WINDOWS

KERNEL_BEGIN

LibIocp::LibIocp()
:_completionPort(NULL)
{

}

LibIocp::~LibIocp()
{
    Destroy();
}

// init/reg
Int32 LibIocp::Create()
{
    if(_completionPort)
        return Status::Success;

    // 创建完成端口IOCP NumberOfConcurrentThreads=0表示默认cpu核数
    _completionPort = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if(!_completionPort)
    {
        auto err = GetLastError();
        g_Log->NetError(LOGFMT_OBJ_TAG("Create failed windows error<%d> status[%d]")
                          , err, Status::IOCP_CreateCompletionPortFail);
        return Status::IOCP_CreateCompletionPortFail;
    }

    return Status::Success;
}

void LibIocp::Destroy()
{
    // 关闭完成端口
    if(LIKELY(_completionPort))
        CloseHandle(_completionPort);
    _completionPort = NULL;
}

Int32 LibIocp::Reg(SOCKET sockfd)
{
    // 关联IOCP 与 sockfd
    // completionKey传入的一个数值，完成时会原样传回来; NumberOfConcurrentThreads这个参数在关联完成端口时被忽略
    // completekey可以是自定义的结构体指针或者其他数据的指针，便于获取完成状态时候识别 当处于关联时numofthread会被忽略
    HANDLE ret = CreateIoCompletionPort(reinterpret_cast<HANDLE>(sockfd), _completionPort, ULONG_PTR(sockfd), 0);
    if(!ret)
    {
        auto err = GetLastError();
        g_Log->NetError(LOGFMT_OBJ_TAG("Reg sockfd[%llu] to completionport failed windows error<%d> status[%d]")
                          , sockfd, err, Status::IOCP_RegSocketToCompletionPortFail);
        return Status::IOCP_RegSocketToCompletionPortFail;
    }

    return Status::Success;
}

Int32 LibIocp::Reg(SOCKET sockfd, void *ptr)
{
    // 关联IOCP 与 sockfd
    // completionKey传入的一个数值，完成时会原样传回来; NumberOfConcurrentThreads这个参数在关联完成端口时被忽略
    // completekey可以是自定义的结构体指针或者其他数据的指针，便于获取完成状态时候识别 当处于关联时numofthread会被忽略
    HANDLE ret = CreateIoCompletionPort(reinterpret_cast<HANDLE>(sockfd), _completionPort, ULONG_PTR(ptr), 0);
    if(!ret)
    {
        auto err = GetLastError();
        g_Log->NetError(LOGFMT_OBJ_TAG("Reg sockfd[%llu] ptr[%p] to completionport failed windows error<%d> status[%d]")
                          , sockfd, ptr, err, Status::IOCP_RegSocketToCompletionPortFail);
        return Status::IOCP_RegSocketToCompletionPortFail;
    }

    return Status::Success;
}

Int32 LibIocp::Reg(SOCKET sockfd, UInt64 sessionId)
{
    // 关联IOCP 与 sockfd
// completionKey传入的一个数值，完成时会原样传回来; NumberOfConcurrentThreads这个参数在关联完成端口时被忽略
// completekey可以是自定义的结构体指针或者其他数据的指针，便于获取完成状态时候识别 当处于关联时numofthread会被忽略
    HANDLE ret = CreateIoCompletionPort(reinterpret_cast<HANDLE>(sockfd), _completionPort, ULONG_PTR(sessionId), 0);
    if(!ret)
    {
        auto err = GetLastError();
        g_Log->NetError(LOGFMT_OBJ_TAG("Reg sockfd[%llu] clientId[%llu] to completionport failed windows error<%d> status[%d]")
                          , sockfd, sessionId, err, Status::IOCP_RegSocketToCompletionPortFail);
        return Status::IOCP_RegSocketToCompletionPortFail;
    }

    return Status::Success;
}



//
Int32 LibIocp::PostQuit(UInt64 wakeupSessionId)
{
    if(false == ::PostQueuedCompletionStatus(_completionPort, 0, ULONG_PTR(wakeupSessionId), NULL))
    {
        auto err = GetLastError();
        g_Log->NetError(LOGFMT_OBJ_TAG("PostQuit win error<%d> status[%d]")
                          , err, Status::IOCP_PostQuitFail);
        return Status::IOCP_PostQuitFail;
    }

    return Status::Success;
}

Int32 LibIocp::WaitForCompletion(IoEvent &ioEvent, Int32 &errorCode, ULong millisec)    // clientId为完成键
{
    // 获取完成端口状态
    // 关键在于 completekey(关联iocp端口时候传入的自定义完成键)会原样返回
    // 以及重叠结构ioDataPtr 用于获取数据重叠结构会原样返回
    ioEvent._bytesTrans = 0;
    ioEvent._ioData = NULL;
    ioEvent._sessionId = 0;
    errorCode = Status::Success;
    if(FALSE == GetQueuedCompletionStatus(_completionPort
                                          , LPDWORD(&ioEvent._bytesTrans)
                                          , reinterpret_cast<PULONG_PTR>(&ioEvent._sessionId)
                                          , reinterpret_cast<LPOVERLAPPED *>(&ioEvent._ioData)
                                          , millisec))
    {
        const Int32 error = GetLastError();
        if (WAIT_TIMEOUT == error)
        {
            errorCode = Status::IOCP_WaitTimeOut;
            g_Log->NetDebug(LOGFMT_OBJ_TAG(LOGFMT_OBJ_TAG("wait timeout from completion io event:%s")), ioEvent.ToString().c_str());
            return Status::Ignore;
        }

        DWORD flags = 0;
        DWORD bytesTransfer = 0;
        auto ioData = ioEvent._ioData;
        if (ioData)
        {
#if _DEBUG
            BOOL grRet =
#endif
                ::WSAGetOverlappedResult(ioData->_sock,
                (LPOVERLAPPED)(ioEvent._ioData),
                    &bytesTransfer,
                    TRUE,
                    &flags);
#if _DEBUG
            ASSERT(grRet == FALSE && "library internal error, in GetQueuedCompletionStatus()!");
#endif
        }
        
        if(ERROR_NETNAME_DELETED == error)
        {
//             g_Log->net<LibIocp>("WaitForMessage session closed sessionId[%llu] bytesTrans<%lld> error<%d> status[%d]"
//                        , ioEvent._data._sessionId,ioEvent._bytesTrans, error, StatusDefs::IOCP_IODisconnect);
//             g_Log->any<LibIocp>("WaitForMessage client closed sockfd=%llu\n error<%d> status[%d]"
//                                 , ioEvent._ioData->_sock, error, StatusDefs::IOCP_IODisconnect);
            // 此时ioevent的数据被正确的填充，只是ioEvent._bytesTrans<=0这个事件可以在recv事件做处理
            // closesocket(ioEvent._ioData->_sock);
            // return StatusDefs::IOCP_IODisconnect;
            g_Log->NetWarn(LOGFMT_OBJ_TAG("The specified network name is no longer available. io event:%s "), ioEvent.ToString().c_str());
            return Status::IOCP_NetError;
        }

        if(ERROR_CONNECTION_ABORTED == error)
        {// io socket关闭,未完成的io操作或者完成的io操作返回
            g_Log->NetWarn(LOGFMT_OBJ_TAG("The network connection was aborted by the local system. session id[%llu] bytesTrans<%llu>. io aborted. WaitForMessage invalid client socket error<%d> status<%d>")
                              , ioEvent._sessionId, ioEvent._bytesTrans, error, Status::IOCP_IO_Aborted);
            errorCode = Status::IOCP_IO_Aborted;
            return Status::IOCP_IO_Aborted;
        }

        if(ERROR_SEM_TIMEOUT == error)
        {// TODO:这个错误码要不要处理 压力过大可以重新投递相应的数据
            g_Log->NetWarn(LOGFMT_OBJ_TAG("pressure is too large for this machine please improve machine performance or expand net card bindwidth error:%d, io event:%s")
                            ,error, ioEvent.ToString().c_str());

//             g_Log->w<LibIocp>(_LOGFMT_("pressure is too large for this machine."
//                                        " please improve machine performance or "
//                                        "expand net card bandwidth error<%d> status<%d>"
//                                        "sessionId<%llu> bytesTrans<%lld>")
//                               , error
//                               , StatusDefs::Unknown
//                               , ioEvent._data._sessionId
//                               , ioEvent._bytesTrans);
            return Status::IOCP_NetError;
        }

        if(ERROR_OPERATION_ABORTED == error)
        {// 由于其他原因或者调用CancelIo，CancelIoEx等导致io被取消 此时bytestransfer为0 可以断开session连接
            g_Log->NetWarn(LOGFMT_OBJ_TAG("The I/O operation has been aborted because of either a thread exit or an application request. session id[%llu] bytesTrans<%llu>. io aborted. WaitForMessage invalid client socket error<%d> status<%d>")
                              , ioEvent._sessionId, ioEvent._bytesTrans, error, Status::IOCP_IO_ThreadExist);
            errorCode = Status::IOCP_IO_ThreadExist;
            return Status::IOCP_IO_ThreadExist;
        }

//         if (ERROR_ABANDONED_WAIT_0 == error)
//         {// session断开 与ERROR_NETNAME_DELETED处理相同
//             errorCode = StatusDefs::Session_Abort;
//             return Status::Ignore;
//         }

        g_Log->NetError(LOGFMT_OBJ_TAG("WaitForMessage other error error<%d> io event:%s")
                          , error
                          , ioEvent.ToString().c_str());
                          
        errorCode = Status::IOCP_WaitOtherError;
        return Status::IOCP_WaitOtherError;
    }

    return Status::Success;
}



KERNEL_END

#endif

