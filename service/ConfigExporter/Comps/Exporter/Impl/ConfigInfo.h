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

class ConfigTableInfo;

// 字段信息
class ConfigFieldInfo
{
    POOL_CREATE_OBJ_DEFAULT(ConfigFieldInfo);

public:
    ConfigFieldInfo(const ConfigTableInfo *owner);
    ~ConfigFieldInfo(){}

    const ConfigTableInfo *_owner;

    KERNEL_NS::LibString _ownType;      // C, S等类别
    KERNEL_NS::LibString _fieldName;    // 字段名
    KERNEL_NS::LibString _dataType;     // 数据类型
    KERNEL_NS::LibString _check;        // 校验
    KERNEL_NS::LibString _flags;        // 字段功能标志
    KERNEL_NS::LibString _defaultValue; // 默认值
    KERNEL_NS::LibString _desc;         // 描述信息
};

// 表信息
class ConfigTableInfo
{
    POOL_CREATE_OBJ_DEFAULT(ConfigTableInfo);

public:
    ConfigTableInfo(){}

    ~ConfigTableInfo();
    KERNEL_NS::LibString _ownType;              // C, S等类别
    KERNEL_NS::LibString _wholeSheetName;       // 页签名 xxx | tableClass
    KERNEL_NS::LibString _tableClassName;       // 表类型名 tableClass
    std::vector<ConfigFieldInfo *> _fieldInfos; // 字段信息
    std::set<KERNEL_NS::LibString> _fieldNames; // 字段名 用于校验字段名
};

SERVICE_END