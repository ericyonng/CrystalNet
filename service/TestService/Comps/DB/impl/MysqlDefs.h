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
 * Date: 2023-07-16 14:37:39
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <kernel/common/common.h>
#include <service/common/common.h>
#include <vector>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN
class CreateTableSqlBuilder;
KERNEL_END

SERVICE_BEGIN

class StorageFieldDefine;
class StorageInfo;

class StorageSizeType
{
public:
    // 二进制数据大小
    enum BinaryCapacityType : UInt64
    {
        // 16字节
        TINY_SIZE = 16,
        // 32字节
        SMALL_SIZE = 32,
        // 64字节
        NORMAL_SIZE = 64,
        // 128字节
        LITTLE_BIG_SIZE = 128,
        // 256字节
        BIG_SIZE = 256,
        // 512字节
        VERY_BIG_SIZE = 512,
        // 1024字节
        VERY_VERY_BIG_SIZE = 1024,
        // 2048字节
        SUPER_BIG_SIZE = 2048,
        // 4K字节（4095）
        VERY_SUPER_BIG_SIZE = 4095,
        // 8K字节(8191)
        LARGE_SIZE = 8191,
        // 16K字节(16383)
        VERY_LARGE_SIZE = 16383,
        // 32K字节(32767)
        HUGE_SIZE = 32767,
        // 64K字节(65535)
        VERY_HUGE_SIZE = 65535,
        // 16MB
        SUPER_HUGE_SIZE = 16777216,
        // 4GB字节
        SERIOUS_SIZE = 4294967296,
    };

    static std::vector<UInt64> GenMultisThan16()
    {
        std::vector<UInt64> datas;
        const Int32 maxMulti = static_cast<Int32>(VERY_SUPER_BIG_SIZE + 1) / 16;
        datas.resize(maxMulti + 1);
        for(Int32 idx = 0; idx <= maxMulti; ++ idx)
        {
            auto sz = static_cast<UInt64>(idx) * TINY_SIZE;

            if(sz <= TINY_SIZE)
                sz = TINY_SIZE;
            else if(sz <= SMALL_SIZE)
                sz = SMALL_SIZE;
            else if(sz <= NORMAL_SIZE)
                sz = NORMAL_SIZE;
            else if(sz <= LITTLE_BIG_SIZE)
                sz = LITTLE_BIG_SIZE;
            else if(sz <= BIG_SIZE)
                sz = BIG_SIZE;
            else if(sz <= VERY_BIG_SIZE)
                sz = VERY_BIG_SIZE;
            else if(sz <= VERY_VERY_BIG_SIZE)
                sz = VERY_VERY_BIG_SIZE;
            else if(sz <= SUPER_BIG_SIZE)
                sz = SUPER_BIG_SIZE;
            else if(sz <= VERY_SUPER_BIG_SIZE)
                sz = VERY_SUPER_BIG_SIZE;
            else if(sz <= LARGE_SIZE)
                sz = LARGE_SIZE;
            else if(sz <= VERY_LARGE_SIZE)
                sz = VERY_LARGE_SIZE;
            else if(sz <= HUGE_SIZE)
                sz = HUGE_SIZE;
            else if(sz <= VERY_HUGE_SIZE)
                sz = VERY_HUGE_SIZE;
            else if(sz <= SERIOUS_SIZE)
                sz = SERIOUS_SIZE;

            datas[idx] = sz;
        }

        return datas;
    }

    static UInt64 GetCapacityType(UInt64 sz)
    {
        auto multi = sz / TINY_SIZE;
        const auto left = sz % TINY_SIZE;

        multi = multi + (left > 0 ? 1 : 0);
        if(UNLIKELY(multi >= multis_than_16.size()))
            return sz;

        return multis_than_16[multi];
    }

    static std::vector<UInt64> multis_than_16;
};

class MysqlFieldTypeHelper
{
public:
    static Int32 FieldTypeToMysqlType(const KERNEL_NS::LibString &fieldType);

    static bool GetFieldTypeBySize(UInt64 sz, KERNEL_NS::LibString &fieldType);

    // string primary key最大只支持1024
    static bool GetStringPrimaryDataTypeBySize(UInt64 sz, KERNEL_NS::LibString &dataType);

    // string字段大小适配, VARCHAR 最大指定到2048, 大于2048 会适配TEXT,MEDIUMTEXT,LONGTEXT
    static bool GetStringDataTypeBySize(UInt64 sz, bool onlyText, bool onlyVarchar, KERNEL_NS::LibString &dataType);

    // 构建字段描述
    static bool MakeFieldDescribe(const IStorageInfo *fieldDefine, KERNEL_NS::LibString &fieldDesc, bool isChangeColumnDescribe = false);
    
    // 数据类型
    static bool MakeFieldDataType(const IStorageInfo *fieldDefine, KERNEL_NS::LibString &dataType);

    // 获取纯粹的mysql数据类型
    static bool TurnToMysqlDataType(const IStorageInfo *fieldDefine, KERNEL_NS::LibString &dataType);

    // 是否是number类型
    static bool IsMysqlDataTypeNumber(const KERNEL_NS::LibString &dataType);
    // 是否是string类型
    static bool IsMysqlDataTypeString(const KERNEL_NS::LibString &dataType);
    // 是否是

    // 构建建表sql
    static KERNEL_NS::CreateTableSqlBuilder *NewCreateTableSqlBuilder(const KERNEL_NS::LibString &dbName, const IStorageInfo *storageInfo);

    // 判断mysqldatatype是否支持
    static bool CheckCanSupportMysqlDataType(const KERNEL_NS::LibString &dataType);
};

class MysqlDirtyType
{
public:
    enum ENUMS
    {
        BEGIN = 1,
        ADD_TYPE = BEGIN,           // 增
        MODIFY_TYPE = 2,        // 改
        DEL_TYPE = 3,           // 删
        REPLACE_TYPE = 4,       // 覆盖
        MAX_TYPE,               // 数量
    };
};

SERVICE_END