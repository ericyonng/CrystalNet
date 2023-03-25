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
 * Date: 2023-01-27 22:11:07
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_XLSX_XLSX_WORSHEET_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_XLSX_XLSX_WORSHEET_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/memory.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

class XlsxCell;
class XlsxWorkbook;

class KERNEL_EXPORT XlsxSheet
{
    POOL_CREATE_OBJ_DEFAULT(XlsxSheet);

public:
    XlsxSheet(XlsxWorkbook *workbook, const LibString &sheetName, UInt64 sheetId);

    const LibString &GetSheetName() const;
    UInt64 GetMaxRow() const;
    UInt64 GetMaxColumn() const;
    const XlsxWorkbook *GetWorkbook() const;

    void AddCell(UInt64 rowId, UInt64 columnId, const LibString &content);
    void Clear();

    const std::unordered_map<UInt64, XlsxCell *> &GetRowCells(UInt64 rowId) const;
    const std::unordered_map<UInt64, XlsxCell *> &GetColumnCells(UInt64 columnId) const;
    const XlsxCell *GetCell(UInt64 rowId, UInt64 columnId) const;

protected:
    XlsxWorkbook *_workbook;
    const LibString _sheetName;
    const UInt64 _sheetId;

    UInt64 _maxRow;
    UInt64 _maxColumn;

    // 单元格数据
    std::unordered_map<UInt64, std::unordered_map<UInt64, XlsxCell *>> _rowRefColumnRefCells;
    std::unordered_map<UInt64, std::unordered_map<UInt64, XlsxCell *>> _columnRefRowRefCells;
};

ALWAYS_INLINE const LibString &XlsxSheet::GetSheetName() const
{
    return _sheetName;
}

ALWAYS_INLINE UInt64 XlsxSheet::GetMaxRow() const
{
    return _maxRow;
}

ALWAYS_INLINE UInt64 XlsxSheet::GetMaxColumn() const
{
    return _maxColumn;
}

ALWAYS_INLINE const XlsxWorkbook *XlsxSheet::GetWorkbook() const
{
    return _workbook;
}

ALWAYS_INLINE const std::unordered_map<UInt64, XlsxCell *> &XlsxSheet::GetRowCells(UInt64 rowId) const
{
    static const std::unordered_map<UInt64, XlsxCell *> s_empty;
    auto iter = _rowRefColumnRefCells.find(rowId);
    if(iter == _rowRefColumnRefCells.end())
    {
        return s_empty;
    }

    return iter->second;
}

ALWAYS_INLINE const std::unordered_map<UInt64, XlsxCell *> &XlsxSheet::GetColumnCells(UInt64 columnId) const
{
    static const std::unordered_map<UInt64, XlsxCell *> s_empty;
    auto iter = _columnRefRowRefCells.find(columnId);
    if(iter == _columnRefRowRefCells.end())
    {
        return s_empty;
    }

    return iter->second;
}

ALWAYS_INLINE const XlsxCell *XlsxSheet::GetCell(UInt64 rowId, UInt64 columnId) const
{
    auto &rowDatas = GetRowCells(rowId);
    auto iter = rowDatas.find(columnId);
    return iter == rowDatas.end() ? NULL : iter->second;
}

KERNEL_END

#endif
