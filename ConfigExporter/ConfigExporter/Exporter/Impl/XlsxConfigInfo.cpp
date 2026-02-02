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
#include <ConfigExporter/Exporter/Impl/XlsxConfigInfo.h>

XlsxConfigFieldInfo::XlsxConfigFieldInfo(const XlsxConfigTableInfo *owner)
:_owner(owner)
,_columnId(KERNEL_NS::XlsxCell::COLUMN_BEGIN)
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
    _columnId = other._columnId;
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
    _columnId = other._columnId;
    return *this;
}


XlsxConfigTableInfo::XlsxConfigTableInfo(const XlsxConfigTableInfo &other)
{
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
        if(ptr)
            XlsxConfigFieldInfo::Delete_XlsxConfigFieldInfo(ptr);
    });
}

XlsxConfigTableInfo &XlsxConfigTableInfo::operator=(const XlsxConfigTableInfo &other)
{
    KERNEL_NS::ContainerUtil::DelContainer(_fieldInfos, [](XlsxConfigFieldInfo *ptr){
        if(ptr)
            XlsxConfigFieldInfo::Delete_XlsxConfigFieldInfo(ptr);
    });

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

    return *this;
}

bool XlsxConfigTableInfo::CheckHeaderSame(const XlsxConfigTableInfo *other, KERNEL_NS::LibString &errInfo) const
{
    // 表必须是同一个配置
    if(_tableClassName != other->_tableClassName)
    {
        errInfo.AppendFormat(", table class is not same current table name:%s, compare to other table name:%s, current xlsx path:%s, sheet namne:%s other xlsx path:%s, sheet name:%s"
                    , _tableClassName.c_str(), other->_tableClassName.c_str(), _xlsxPath.c_str(), _wholeSheetName.c_str(), other->_xlsxPath.c_str(), other->_wholeSheetName.c_str());
        return false;
    }

    if(_fieldInfos.size() != other->_fieldInfos.size())
    {
        errInfo.AppendFormat("table name:%s fields num is not match, current fields num:%llu, compare to other table fields num:%llu, current xlsx path:%s, sheet name:%s, other xlsx path:%s sheet name:%s"
            , _tableClassName.c_str(), static_cast<UInt64>(_fieldInfos.size()), static_cast<UInt64>(other->_fieldInfos.size()), _xlsxPath.c_str(), _wholeSheetName.c_str(), other->_xlsxPath.c_str(), other->_wholeSheetName.c_str());
        return false;
    }

    const Int32 fieldCount = static_cast<Int32>(_fieldInfos.size());
    for(Int32 idx = 0; idx < fieldCount; ++idx)
    {
        auto fieldInfo = _fieldInfos[idx];
        auto otherField = other->_fieldInfos[idx];

        if(!fieldInfo)
        {// 只对别非空的字段
            continue;
        }

        if(!otherField)
        {
            errInfo.AppendFormat("table name:%s fieldInfo not match current field info:%p, other field info:%p, idx:%d, current xlsx path:%s sheet name:%s, other xlsx path:%s sheet name:%s"
            , _tableClassName.c_str(), fieldInfo, other->_fieldInfos[idx], idx, _xlsxPath.c_str(), _wholeSheetName.c_str(), other->_xlsxPath.c_str(), other->_wholeSheetName.c_str());
            return false;
        }

        if(fieldInfo->_fieldName != otherField->_fieldName)
        {
            errInfo.AppendFormat("table class type:%s, field name not the same, idx:%d current field name:%s,  other field name:%s, current xlsx path:%s sheet name:%s, other xlsx path:%s sheet name:%s"
                                , _tableClassName.c_str(), idx, fieldInfo->_fieldName.c_str(), otherField->_fieldName.c_str(), _xlsxPath.c_str(), _wholeSheetName.c_str(), other->_xlsxPath.c_str(), other->_wholeSheetName.c_str());
            return false;
        }

        if(fieldInfo->_ownType != otherField->_ownType)
        {
            errInfo.AppendFormat("table class type:%s, own type not the same, current own type:%s, other own type:%s, idx:%d, current field name:%s,  other field name:%s, current xlsx path:%s sheet name:%s, other xlsx path:%s sheet name:%s"
                                , _tableClassName.c_str(), fieldInfo->_ownType.c_str(), otherField->_ownType.c_str(), idx, fieldInfo->_fieldName.c_str(), otherField->_fieldName.c_str(), _xlsxPath.c_str(), _wholeSheetName.c_str(), other->_xlsxPath.c_str(), other->_wholeSheetName.c_str());
            return false;
        }

        if(fieldInfo->_dataType != otherField->_dataType)
        {
            errInfo.AppendFormat("table class type:%s, data type not the same, current data type:%s, other data type:%s, idx:%d, current field name:%s,  other field name:%s, current xlsx path:%s sheet name:%s, other xlsx path:%s sheet name:%s"
                                , _tableClassName.c_str(), fieldInfo->_dataType.c_str(), otherField->_dataType.c_str(), idx, fieldInfo->_fieldName.c_str(), otherField->_fieldName.c_str(), _xlsxPath.c_str(), _wholeSheetName.c_str(), other->_xlsxPath.c_str(), other->_wholeSheetName.c_str());
            return false;
        }

        if(fieldInfo->_check != otherField->_check)
        {
            errInfo.AppendFormat("table class type:%s, check not the same, current check:%s, other check:%s, idx:%d, current field name:%s,  other field name:%s, current xlsx path:%s sheet name:%s, other xlsx path:%s sheet name:%s"
                                , _tableClassName.c_str(), fieldInfo->_check.c_str(), otherField->_check.c_str(), idx, fieldInfo->_fieldName.c_str(), otherField->_fieldName.c_str(), _xlsxPath.c_str(), _wholeSheetName.c_str(), other->_xlsxPath.c_str(), other->_wholeSheetName.c_str());
            return false;
        }

        if(fieldInfo->_flags != otherField->_flags)
        {
            errInfo.AppendFormat("table class type:%s, flags not the same, current flags:%s, other flags:%s, idx:%d, current field name:%s,  other field name:%s, current xlsx path:%s sheet name:%s, other xlsx path:%s sheet name:%s"
                                , _tableClassName.c_str(), fieldInfo->_flags.c_str(), otherField->_flags.c_str(), idx, fieldInfo->_fieldName.c_str(), otherField->_fieldName.c_str(), _xlsxPath.c_str(), _wholeSheetName.c_str(), other->_xlsxPath.c_str(), other->_wholeSheetName.c_str());
            return false;
        }
    }

    return true;    
}

const KERNEL_NS::LibString ConfigTableDefine::SINGLE_DISABLE_FLAG = "#";
const KERNEL_NS::LibString ConfigTableDefine::MULTI_DISABLE_FLAG = "###";
