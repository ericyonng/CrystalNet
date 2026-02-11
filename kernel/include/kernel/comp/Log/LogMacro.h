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
 * Date: 2024-12-05 00:24:51
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LOG_LOG_MACRO_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LOG_LOG_MACRO_H__

#pragma once

// 函数与行号便利宏
#undef _FUNC_LINE_ARGS_
#define _FUNC_LINE_ARGS_ __FILE__, __FUNCTION__, __LINE__

// // 获取tag

// 类实例获取tag
#undef LOG_OBJ_TAG
#define LOG_OBJ_TAG() KERNEL_NS::RttiUtil::GetByType<decltype(*this)>().c_str()
// 非类实例获取tag
#undef LOG_NON_OBJ_TAG
#define LOG_NON_OBJ_TAG(classType) KERNEL_NS::RttiUtil::GetByType<classType>().c_str()


// 带修饰的log宏
#undef LOGFMT_DETAIL
#define LOGFMT_DETAIL(tag, x) tag, _FUNC_LINE_ARGS_, x

// NO FMT
#undef LOGFMT_DETAIL_NO_FMT
#define LOGFMT_DETAIL_NO_FMT(tag) tag, _FUNC_LINE_ARGS_


// // 带tag宏,除了Net接口特殊外都可用

// 类实例tag
#undef LOGFMT_OBJ_TAG
#define LOGFMT_OBJ_TAG(x) LOGFMT_DETAIL(LOG_OBJ_TAG(), x)

// no fmt
#undef LOGFMT_OBJ_TAG_NO_FMT
#define LOGFMT_OBJ_TAG_NO_FMT() LOGFMT_DETAIL_NO_FMT(LOG_OBJ_TAG())

// 非类实例tag
#undef LOGFMT_NON_OBJ_TAG
#define LOGFMT_NON_OBJ_TAG(classType, x) LOGFMT_DETAIL(LOG_NON_OBJ_TAG(classType), x)

// no fmt
#undef LOGFMT_NON_OBJ_TAG_NO_FMT
#define LOGFMT_NON_OBJ_TAG_NO_FMT(classType) LOGFMT_DETAIL_NO_FMT(LOG_NON_OBJ_TAG(classType))

// LOG便利宏

// Debug
#undef CLOG_DEBUG_GLOBAL
#define CLOG_DEBUG_GLOBAL(classType, fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::Debug))  \
    g_Log->Debug(LOGFMT_NON_OBJ_TAG(classType, fmt), ##__VA_ARGS__)

// 通过this获取tag
#undef CLOG_DEBUG
#define CLOG_DEBUG(fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::Debug))  \
    g_Log->Debug(LOGFMT_OBJ_TAG(fmt), ##__VA_ARGS__)

// args,如果是类类型需要ToString(),详情见:StringOutAdapter::output
#undef CLOG_DEBUG_ARGS
#define CLOG_DEBUG_ARGS(...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::Debug))  \
    g_Log->Debug2(LOGFMT_OBJ_TAG_NO_FMT(), ##__VA_ARGS__)

// Info
#undef CLOG_INFO_GLOBAL
#define CLOG_INFO_GLOBAL(classType, fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::Info))  \
    g_Log->Info(LOGFMT_NON_OBJ_TAG(classType, fmt), ##__VA_ARGS__)

// 通过this获取tag
#undef CLOG_INFO
#define CLOG_INFO(fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::Info))  \
    g_Log->Info(LOGFMT_OBJ_TAG(fmt), ##__VA_ARGS__)

// args,如果是类类型需要ToString(),详情见:StringOutAdapter::output
#undef CLOG_INFO_ARGS
#define CLOG_INFO_ARGS(...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::Info))  \
    g_Log->Info2(LOGFMT_OBJ_TAG_NO_FMT(), ##__VA_ARGS__)


// 通过this获取tag
#undef CLOG_WARN_GLOBAL
#define CLOG_WARN_GLOBAL(classType, fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::Warn))  \
    g_Log->Warn(LOGFMT_NON_OBJ_TAG(classType, fmt), ##__VA_ARGS__)

// 通过this获取tag
#undef CLOG_WARN
#define CLOG_WARN(fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::Warn))  \
    g_Log->Warn(LOGFMT_OBJ_TAG(fmt), ##__VA_ARGS__)


// args,如果是类类型需要ToString(),详情见:StringOutAdapter::output
#undef CLOG_WARN_ARGS
#define CLOG_WARN_ARGS(...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::Warn))  \
    g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), ##__VA_ARGS__)

#undef CLOG_ERROR_GLOBAL
#define CLOG_ERROR_GLOBAL(classType, fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::Error))  \
    g_Log->Error(LOGFMT_NON_OBJ_TAG(classType, fmt), ##__VA_ARGS__)

// 通过this获取tag
#undef CLOG_ERROR
#define CLOG_ERROR(fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::Error))  \
    g_Log->Error(LOGFMT_OBJ_TAG(fmt), ##__VA_ARGS__)

// 通过this获取tag
#undef CLOG_ERROR_ARGS
#define CLOG_ERROR_ARGS(...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::Error))  \
    g_Log->Error2(LOGFMT_OBJ_TAG_NO_FMT(), ##__VA_ARGS__)

#undef CLOG_CRASH
#define CLOG_CRASH(fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::Crash))  \
    g_Log->Crash(fmt, ##__VA_ARGS__)

#undef CLOG_NET_GLOBAL
#define CLOG_NET_GLOBAL(classType, fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::Net))  \
    g_Log->Net(LOG_NON_OBJ_TAG(classType), fmt, ##__VA_ARGS__)

// 通过this获取tag
#undef CLOG_NET
#define CLOG_NET(fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::Net))  \
    g_Log->Net(LOG_OBJ_TAG(), fmt, ##__VA_ARGS__)

#undef CLOG_NET_DEBUG_GLOBAL
#define CLOG_NET_DEBUG_GLOBAL(classType, fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::NetDebug))  \
    g_Log->NetDebug(LOGFMT_NON_OBJ_TAG(classType, fmt), ##__VA_ARGS__)

// 通过this获取tag
#undef CLOG_NET_DEBUG
#define CLOG_NET_DEBUG(fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::NetDebug))  \
    g_Log->NetDebug(LOGFMT_OBJ_TAG(fmt), ##__VA_ARGS__)

#undef CLOG_NET_INFO_GLOBAL
#define CLOG_NET_INFO_GLOBAL(classType, fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::NetInfo))  \
    g_Log->NetInfo(LOGFMT_NON_OBJ_TAG(classType, fmt), ##__VA_ARGS__)

// 通过this获取tag
#undef CLOG_NET_INFO
#define CLOG_NET_INFO(fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::NetInfo))  \
    g_Log->NetInfo(LOGFMT_OBJ_TAG(fmt), ##__VA_ARGS__)

#undef CLOG_NET_WARN_GLOBAL
#define CLOG_NET_WARN_GLOBAL(classType, fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::NetWarn))  \
    g_Log->NetWarn(LOGFMT_NON_OBJ_TAG(classType, fmt), ##__VA_ARGS__)

// 通过this获取tag
#undef CLOG_NET_WARN
#define CLOG_NET_WARN(fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::NetWarn))  \
    g_Log->NetWarn(LOGFMT_OBJ_TAG(fmt), ##__VA_ARGS__)

#undef CLOG_NET_ERROR_GLOBAL
#define CLOG_NET_ERROR_GLOBAL(classType, fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::NetError))  \
    g_Log->NetError(LOGFMT_NON_OBJ_TAG(classType, fmt), ##__VA_ARGS__)

// 通过this获取tag
#undef CLOG_NET_ERROR
#define CLOG_NET_ERROR(fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::NetError))  \
    g_Log->NetError(LOGFMT_OBJ_TAG(fmt), ##__VA_ARGS__)

#undef CLOG_NET_TRACE_GLOBAL
#define CLOG_NET_TRACE_GLOBAL(classType, fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::NetTrace))  \
    g_Log->NetTrace(LOGFMT_NON_OBJ_TAG(classType, fmt), ##__VA_ARGS__)

// 通过this获取tag
#undef CLOG_NET_TRACE
#define CLOG_NET_TRACE(fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::NetTrace))  \
    g_Log->NetTrace(LOGFMT_OBJ_TAG(fmt), ##__VA_ARGS__)

#undef CLOG_CUSTOM
#define CLOG_CUSTOM(fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::Custom))  \
    g_Log->Custom(fmt, ##__VA_ARGS__)

#undef CLOG_SYS_GLOBAL
#define CLOG_SYS_GLOBAL(classType, fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::Sys))  \
    g_Log->Sys(LOGFMT_NON_OBJ_TAG(classType, fmt), ##__VA_ARGS__)

// 通过this获取tag
#undef CLOG_SYS
#define CLOG_SYS(fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::Sys))  \
    g_Log->Sys(LOGFMT_OBJ_TAG(fmt), ##__VA_ARGS__)

#undef CLOG_MEM_MONITOR
#define CLOG_MEM_MONITOR(fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::MemMonitor))  \
    g_Log->MemMonitor(fmt, ##__VA_ARGS__)

#undef CLOG_TRACE_GLOBAL
#define CLOG_TRACE_GLOBAL(classType, fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::Trace))  \
    g_Log->Trace(LOGFMT_NON_OBJ_TAG(classType, fmt), ##__VA_ARGS__)

// 通过this获取tag
#undef CLOG_TRACE
#define CLOG_TRACE(fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::Trace))  \
    g_Log->Trace(LOGFMT_OBJ_TAG(fmt), ##__VA_ARGS__)

#undef CLOG_MONITOR
#define CLOG_MONITOR(fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::Monitor))  \
    g_Log->Monitor(fmt, ##__VA_ARGS__)


#undef CLOG_FAIL_SQL_GLOBAL
#define CLOG_FAIL_SQL_GLOBAL(classType, fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::FailSql))  \
    g_Log->FailSql(LOGFMT_NON_OBJ_TAG(classType, fmt), ##__VA_ARGS__)

// 通过this获取tag
#undef CLOG_FAIL_SQL
#define CLOG_FAIL_SQL(fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::FailSql))  \
    g_Log->FailSql(LOGFMT_OBJ_TAG(fmt), ##__VA_ARGS__)

#undef CLOG_DUMP_SQL_GLOBAL
#define CLOG_DUMP_SQL_GLOBAL(classType, fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::DumpSql))  \
    g_Log->DumpSql(LOGFMT_NON_OBJ_TAG(classType, fmt), ##__VA_ARGS__)

// 通过this获取tag
#undef CLOG_DUMP_SQL
#define CLOG_DUMP_SQL(fmt, ...)   \
if(g_Log->IsEnable(KERNEL_NS::LogLevel::DumpSql))  \
    g_Log->DumpSql(LOGFMT_OBJ_TAG(fmt), ##__VA_ARGS__)

#endif
