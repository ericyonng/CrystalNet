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

#include <pch.h>
#include <service/ConfigExporter/Comps/Exporter/Impl/XlsxConfigInfo.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(XlsxConfigFieldInfo);

POOL_CREATE_OBJ_DEFAULT_IMPL(XlsxConfigTableInfo);

POOL_CREATE_OBJ_DEFAULT_IMPL(XlsxConfigMetaInfo);

XlsxConfigFieldInfo::XlsxConfigFieldInfo(const XlsxConfigTableInfo *owner)
:_owner(owner)
{

}

XlsxConfigFieldInfo::XlsxConfigFieldInfo(const XlsxConfigFieldInfo &other)
{
    _owner = other._owner;
    _ownType = other._ownType;
    _fieldName = other._fieldName;
    _dataType = other._dataType;
    _check = other._check;
    _flags = other._flags;
    _defaultValue = other._defaultValue;
    _desc = other._desc;
}

XlsxConfigFieldInfo &XlsxConfigFieldInfo::operator=(const XlsxConfigFieldInfo &other)
{
    _owner = other._owner;
    _ownType = other._ownType;
    _fieldName = other._fieldName;
    _dataType = other._dataType;
    _check = other._check;
    _flags = other._flags;
    _defaultValue = other._defaultValue;
    _desc = other._desc;
}


XlsxConfigTableInfo::XlsxConfigTableInfo(const XlsxConfigTableInfo &other)
{
    _ownType = other._ownType;
    _wholeSheetName = other._wholeSheetName;
    _tableClassName = other._tableClassName;
    _fieldNames = other._fieldNames;
    _rowIdRefFunctionBarColumn = other._rowIdRefFunctionBarColumn;
    _values = other._values;    // 需要根据不同的ownType移除不同的内容

    // _fieldInfos
    for(auto &fieldInfo : other._fieldInfos)
    {
        auto newInfo = XlsxConfigFieldInfo::New_XlsxConfigFieldInfo(this);
        *newInfo = *fieldInfo;

        _fieldInfos.push_back(newInfo);
    }
}

XlsxConfigTableInfo::~XlsxConfigTableInfo()
{
    KERNEL_NS::ContainerUtil::DelContainer(_fieldInfos, [](XlsxConfigFieldInfo *ptr){
        XlsxConfigFieldInfo::Delete_XlsxConfigFieldInfo(ptr);
    });
}

XlsxConfigTableInfo &XlsxConfigTableInfo::operator=(const XlsxConfigTableInfo &other)
{
    KERNEL_NS::ContainerUtil::DelContainer(_fieldInfos, [](XlsxConfigFieldInfo *ptr){
        XlsxConfigFieldInfo::Delete_XlsxConfigFieldInfo(ptr);
    });

    _ownType = other._ownType;
    _wholeSheetName = other._wholeSheetName;
    _tableClassName = other._tableClassName;
    _fieldNames = other._fieldNames;
    _rowIdRefFunctionBarColumn = other._rowIdRefFunctionBarColumn;
    _values = other._values;    // 需要根据不同的ownType移除不同的内容

    // _fieldInfos
    for(auto &fieldInfo : other._fieldInfos)
    {
        auto newInfo = XlsxConfigFieldInfo::New_XlsxConfigFieldInfo(this);
        *newInfo = *fieldInfo;

        _fieldInfos.push_back(newInfo);
    }
}


const KERNEL_NS::LibString ConfigTableDefine::SINGLE_DISABLE_FLAG = "#";
const KERNEL_NS::LibString ConfigTableDefine::MULTI_DISABLE_FLAG = "###";


SERVICE_END
