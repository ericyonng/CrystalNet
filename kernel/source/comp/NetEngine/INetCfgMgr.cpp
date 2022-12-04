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
 * Date: 2021-04-18 21:30:46
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/NetEngine/INetCfgMgr.h>
#include <kernel/comp/File/LibIniFile.h>
#include <kernel/comp/Log/log.h>
#include <kernel/comp/NetEngine/Defs/NetCfgDefs.h>
#include <kernel/comp/Utils/StringUtil.h>
#include <kernel/comp/BlackWhiteList/BlackWhiteDef.h>

KERNEL_BEGIN

INetCfgMgr::INetCfgMgr()
    :_ini(NULL)
    ,_connectTimoutMsPerTimes(0)
    ,_reconnectTimes(0)
    ,_connectBlackWhiteFlagMode(0)
{

}

INetCfgMgr::~INetCfgMgr()
{
    Clear();
}

Int32 INetCfgMgr::Init(const Byte8 *iniFile)
{
    if(_ini)
    {
        g_Log->NetWarn(LOGFMT_OBJ_TAG("net cfg mgr is already init, init new ini file[%s] fail."), iniFile);
        return Status::Success;
    }

    _ini = LibIniFile::New_LibIniFile();
    if(!_ini->Init(iniFile))
    {
        g_Log->NetError(LOGFMT_OBJ_TAG("net cfg mgr init fail ini file[%s]"), iniFile);
        return Status::InitFail;
    }

    // // 读取连接器配置
    // 连接超时间隔
    if(!_ini->CheckReadInt(CONNECTOR_SEGMENT, CONNECTOR_TIMEOUT_MS_PER_TIMES_FIELD_NAME, _connectTimoutMsPerTimes))
    {
        _connectTimoutMsPerTimes = CONNECTOR_TIMEOUT_MS_PER_TIMES_FIELD_VALUE;
        const auto &value = StringUtil::Num2Str(_connectTimoutMsPerTimes);
        if(!_ini->WriteStr(CONNECTOR_SEGMENT, CONNECTOR_TIMEOUT_MS_PER_TIMES_FIELD_NAME, value.c_str()))
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("write segment[%s], key[%s], value[%s] fail.")
            ,CONNECTOR_SEGMENT,  CONNECTOR_TIMEOUT_MS_PER_TIMES_FIELD_NAME, value.c_str());
            return Status::InitFail;
        }
    }

    // 重连次数
    if(!_ini->CheckReadInt(CONNECTOR_SEGMENT, CONNECTOR_RETRY_TIMES_FIELD_NAME, _reconnectTimes))
    {
        _reconnectTimes = CONNECTOR_RETRY_TIMES_FIELD_VALUE;
        const auto &value = StringUtil::Num2Str(_reconnectTimes);
        if(!_ini->WriteStr(CONNECTOR_SEGMENT, CONNECTOR_RETRY_TIMES_FIELD_NAME, value.c_str()))
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("write segment[%s], key[%s], value[%s] fail.")
            ,CONNECTOR_SEGMENT,  CONNECTOR_RETRY_TIMES_FIELD_NAME, value.c_str());
            return Status::InitFail;
        }
    }

    // 连接器黑白名单模式
    UInt64 result = 0;
    if(!_ini->CheckReadUInt(CONNECTOR_SEGMENT, CONNECTOR_BLACK_WHITE_MODE_FIELD_NAME, result))
    {
        result = CONNECTOR_BLACK_WHITE_MODE_FIELD_VALUE;
        const auto &value = StringUtil::Num2Str(result);
        if(!_ini->WriteStr(CONNECTOR_SEGMENT, CONNECTOR_BLACK_WHITE_MODE_FIELD_NAME, value.c_str()))
        {
            g_Log->NetError(LOGFMT_OBJ_TAG("write segment[%s], key[%s], value[%s] fail.")
            ,CONNECTOR_SEGMENT,  CONNECTOR_BLACK_WHITE_MODE_FIELD_NAME, value.c_str());
            return Status::InitFail;
        }
    }

    _connectBlackWhiteFlagMode = static_cast<UInt32>(result);
    

    return Status::Success;
}

void INetCfgMgr::Clear()
{
    if(!_ini)
        return;

    LibIniFile::Delete_LibIniFile(_ini);
    _ini = NULL;
}


KERNEL_END