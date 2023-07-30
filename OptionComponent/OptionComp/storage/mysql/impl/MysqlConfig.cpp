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
 * Date: 2023-07-14 13:17:00
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <OptionComp/storage/mysql/impl/MysqlConfig.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(MysqlConfig);

MysqlConfig::MysqlConfig()
:_port(3306)
,_charset("utf8mb4")
,_dbCharset("utf8mb4")
,_dbCollate("utf8mb4_bin")
,_autoReconnect(1)
,_maxPacketSize(2 * 1024llu * 1024llu * 1024llu) // 2GB
,_enableMultiStatements(true)
,_retryWhenError(10)
,_dbThreadNum(1)
{

}

LibString MysqlConfig::ToString() const
{
    LibString info;
    info.AppendFormat("host:%s user:%s pwd:%s db:%s port:%hu bind ip:%s \ncharset:%s db charset:%s db collate:%s autoreconnect:%d \nmaxpacketsize:%llu _enableMultiStatements:%d, _retryWhenError:%d, _dbThreadNum:%d"
        , _host.c_str(), _user.c_str(), _pwd.c_str(), _dbName.c_str(), _port, _bindIp.c_str(), _charset.c_str(), _dbCharset.c_str(), _dbCollate.c_str(),
         _autoReconnect, _maxPacketSize, _enableMultiStatements, _retryWhenError, _dbThreadNum);

    return info;
}

KERNEL_END
