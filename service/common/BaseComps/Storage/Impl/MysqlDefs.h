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
 * Date: 2023-07-23 17:49:00
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <kernel/kernel.h>
#include <service/common/macro.h>
#include <service/common/status.h>

SERVICE_BEGIN

// 系统所支持的mysql数据类型, 存储类型定义
class StorageTypeDefine
{
public:
    // 数值
    static constexpr const Byte8 * BYTE8_INT_TYPE = "TINYINT";
    static constexpr const Byte8 * SHORT_INT_TYPE = "SMALLINT";
    static constexpr const Byte8 * INT_TYPE = "INT";
    static constexpr const Byte8 * INT64_TYPE = "BIGINT";
    static constexpr const Byte8 * FLOAT_TYPE = "FLOAT";
    static constexpr const Byte8 * DOUBLE_TYPE = "DOUBLE";
    static constexpr const Byte8 * DATE_TIME_TYPE = "DATETIME";

    // 字符串
    static constexpr const Byte8 * VARCHAR = "VARCHAR";

    // 二进制
    static constexpr const Byte8 * VARBINARY = "VARBINARY";
    static constexpr const Byte8 * BLOB = "BLOB";
    static constexpr const Byte8 * MEDIUM_BLOB = "MEDIUMBLOB";
    static constexpr const Byte8 * LONG_BLOB = "LONGBLOB";
};

class StorageCommonDefine
{
public:
    static constexpr const Byte8 * STRING_KEY = "StringKey";
    static constexpr const Byte8 * NUMBER_KEY = "NumberKey";
    static constexpr const Byte8 * SYSTEM_DATA = "SystemData";
};

SERVICE_END
