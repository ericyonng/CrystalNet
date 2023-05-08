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
 * Date: 2023-02-21 11:03:07
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <service/ConfigExporter/ServiceCompHeader.h>

SERVICE_BEGIN

class XlsxConfigTableInfo;

// 字段信息
class XlsxConfigFieldInfo
{
    POOL_CREATE_OBJ_DEFAULT(XlsxConfigFieldInfo);

public:
    XlsxConfigFieldInfo(const XlsxConfigTableInfo *owner);
    XlsxConfigFieldInfo(const XlsxConfigFieldInfo &other);

    ~XlsxConfigFieldInfo(){}

    XlsxConfigFieldInfo &operator=(const XlsxConfigFieldInfo &other);

    const XlsxConfigTableInfo *_owner;

    KERNEL_NS::LibString _ownType;      // C, S等类别
    KERNEL_NS::LibString _fieldName;    // 字段名
    KERNEL_NS::LibString _dataType;     // 配置中定义的数据类型
    KERNEL_NS::LibString _check;        // 校验
    KERNEL_NS::LibString _flags;        // 字段功能标志
    KERNEL_NS::LibString _defaultValue; // 默认值
    KERNEL_NS::LibString _desc;         // 描述信息
    UInt64 _columnId;                   // 列id
};

// 表信息
class XlsxConfigTableInfo
{
    POOL_CREATE_OBJ_DEFAULT(XlsxConfigTableInfo);

public:
    XlsxConfigTableInfo(){}
    XlsxConfigTableInfo(const XlsxConfigTableInfo &other);

    ~XlsxConfigTableInfo();

    XlsxConfigTableInfo &operator=(const XlsxConfigTableInfo &other);

    bool CheckHeaderSame(const XlsxConfigTableInfo *other, KERNEL_NS::LibString &errInfo) const;

    KERNEL_NS::LibString _wholeSheetName;       // 页签名 xxx | tableClass
    KERNEL_NS::LibString _tableClassName;       // 表类型名 tableClass
    KERNEL_NS::LibString _xlsxPath;             // xls路径, 如果多个xlsx则会用;并换行拼接
    std::vector<XlsxConfigFieldInfo *> _fieldInfos; // 字段信息
    std::unordered_set<KERNEL_NS::LibString> _fieldNames; // 字段名 用于校验字段名

    std::map<UInt64, KERNEL_NS::LibString> _rowIdRefFunctionBarColumn;   // 功能列（表的第一列）
    std::vector<std::map<UInt64, KERNEL_NS::LibString>> _values; // 数据 value[行id][列id]
};

// meta文件内容(用于判断xlsx是否发生了变更)
class XlsxConfigMetaInfo
{
public:
    POOL_CREATE_OBJ_DEFAULT(XlsxConfigMetaInfo);

    XlsxConfigMetaInfo(){}
    ~XlsxConfigMetaInfo(){}

    KERNEL_NS::LibString _metaRootPath;     // meta所在目录
    KERNEL_NS::LibString _relationPath;     // meta文件相对于 meta路径的相对路径
    KERNEL_NS::LibString _metaFileName;     // meta文件名

    KERNEL_NS::LibString _xlsxFileName;     // xlsx文件名
    KERNEL_NS::LibString _lastMd5;           // 最后一次的md5值
};

class ConfigTableDefine
{
public:
    static constexpr UInt64 HEADER_ROW_NUMBER = 7;      // 表头7行
    static constexpr UInt64 OWN_TYPE = 1;          // 所属版本
    static constexpr UInt64 FIELD_NAME = 2;        // 字段名
    static constexpr UInt64 DATA_TYPE = 3;         // 数据类型
    static constexpr UInt64 CHECK = 4;             // 校验
    static constexpr UInt64 FLAGS = 5;             // 字段功能标志
    static constexpr UInt64 DEFAULT_VALUE = 6;     // 字段功能标志
    static constexpr UInt64 DESC = 7;              // 字段功能标志

    static const KERNEL_NS::LibString SINGLE_DISABLE_FLAG;        // 注释单行 #
    static const KERNEL_NS::LibString MULTI_DISABLE_FLAG;        // 注释多行 ###
    static constexpr Byte8 OWN_TYPE_SEP = '|';        // own type的分隔符
};

SERVICE_END