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
 * Date: 2022-12-08 23:39:00
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <kernel/kernel.h>

// app配置
static const KERNEL_NS::LibString s_appIniContent = 
"[KERNEL_OPTION]\n"
"\n"
"    BlackWhiteListMode=14\n"
"\n"
"    MaxSessionQuantity=100000\n"
"\n"
"    LinkInOutPollerAmount = 0\n"
"\n"
"    DataTransferPollerAmount = 0\n"
"\n"
"    MaxRecvBytesPerFrame = 16384\n"
"\n"
"    MaxSendBytesPerFrame = 16384\n"
"\n"
"    MaxAcceptCountPerFrame = 1024\n"
"\n"
"    MaxPieceTimeInMicroSecPerFrame = 8000\n"
"\n"
"    MaxPollerScanMilliseconds = 1\n"
"\n"
"    MaxPollerMsgPriorityLevel = 3\n"
"\n"
"    PollerMonitorEventPriorityLevel = -1\n"
"\n"
"    PriorityLevelDefine = 0,INNER|1,DB|2,OUTER1|3,OUTER2\n"
"\n"
"    SessionBufferCapicity = 8192\n"
"\n"
"\n"
"[ServiceCommon]\n"
"\n"
"    ActiveServiceItem = ProtoGen\n"
"\n"
"[ApplicationConfig]\n"
"\n"
"    AliasName = proto_gen\n"
"\n"
"    ProjectMainServiceName = ProtoGen\n"
"\n"
"    MachineId = 0\n"
"\n"
"    RegisterTime = 0\n"
"\n"
"    RegisterPath = \n"
"\n"
"    RegisterProcessId = 0\n"
"\n"
"    MachineApplyId = \n"
"\n"
"[ProtoGen]\n"
"    MaxPieceTimeInMicroseconds = 8000\n"
"\n"
"    PollerMaxSleepMilliseconds = 20\n"
"\n"
"    FrameUpdateTimeMs = 50\n"
"\n"
"    PORT_SESSION_TYPE = 3900,OUTER|3901,INNER\n"
;

// 日志配置表
static const KERNEL_NS::LibString s_logIniContent = 
"[Common]\n"
"FileName=0,,1|1,Crash,2|2,Net,3|3,Custom,1|4,Sys,2|5,MemMonitor,2|6,Trace,2\n"
"ExtName=.log\n"
"MaxLogCacheMB=16\n"
"LogTimerIntervalMs=1000\n"
"MaxFileSizeMB=256\n"
"IsEnableLog=1\n"
"LogPath=1,\n"
"\n"
"[LogLevel]\n"
"Debug           = 1,0,0,White|Black,1,0,0\n"
"Info            = 1,0,0,White|Black,2,0,0\n"
"Warn            = 1,0,1,LightYellow|Black,3,0,0\n"
"Error           = 1,0,1,Red|Black,4,0,1\n"
"Crash           = 1,1,0,Red|Black,5,0,0\n"
"Net             = 1,2,0,White|Black,6,0,0\n"
"NetDebug        = 1,2,0,White|Black,7,0,0\n"
"NetInfo         = 1,2,0,White|Black,8,0,0\n"
"NetWarn         = 1,2,0,LightYellow|Black,9,0,0\n"
"NetError        = 1,2,0,Red|Black,10,0,1\n"
"NetTrace        = 1,2,0,Green|Black,11,1,0\n"
"Custom          = 1,3,1,White|Black,12,0,0\n"
"Sys    	     = 1,4,0,White|Black,13,0,0\n"
"MemMonitor    	= 1,5,0,White|Black,14,0,0\n"
"Trace           = 1,6,0,Green|Black,15,1,0\n"
;

// 控制台配置表
static const KERNEL_NS::LibString s_consoleIniContent = 
"[Win32FrontConsoleColor]\n"
"Black        = 0\n"
"Red          = 4\n"
"Green        = 2\n"
"Blue         = 1\n"
"Yellow       = 6\n"
"Purple       = 5\n"
"Cyan         = 3\n"
"White        = 7\n"
"Gray         = 8\n"
"LightYellow  = 14\n"
"Highlight    = 8\n"
"FrontDefault = 7\n"
"\n"
"[Win32BackConsoleColor]\n"
"Black       = 0  \n"
"Red         = 64 \n"
"Green       = 32 \n"
"Blue        = 16 \n"
"Yellow      = 96 \n"
"Purple      = 80 \n"
"Cyan        = 48 \n"
"White       = 112\n"
"Highlight   = 128\n"
"BackDefault = 0  \n"
;

