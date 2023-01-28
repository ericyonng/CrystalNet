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

template<class WorkbookType>
class Worksheet
{
    POOL_CREATE_TEMPLATE_OBJ_DEFAULT(Worksheet, WorkbookType);

public:
    Worksheet(WorkbookType *workbook, const LibString &sheetName, UInt64 sheetId, const std::vector<std::tuple<UInt64, UInt64, UInt64>> &allCells)
    :_workbook(workbook)
    ,_sheetName(sheetName)
    ,_sheetId(sheetId)
    ,_allCells(allCells)
    {

    }

    LibString LoadCells();

    const LibString &GetSheetName() const;
    UInt64 GetMaxRow() const;
    UInt64 GetMaxColumn() const;
    const WorkbookType *GetWorkbook() const;

    const std::vector<std::vector<UInt64>> &GetAllSharedStringIdxs() const;
    const std::vector<UInt64> &GetRowSharedStringIdxs(UInt64 rowId) const;
    bool GetCellShareStringIndex(UInt64 rowId, UInt64 columnId, UInt64 &shareStringIdx) const;

protected:
    virtual LibString _AfterLoaded()
    {
        return LibString();
    }
    
protected:
    WorkbookType *_workbook;
    LibString _sheetName;
    UInt64 _sheetId;
    const std::vector<std::tuple<UInt64, UInt64, UInt64>> &_allCells;
    UInt64 _maxRow;
    UInt64 _maxColumn;

    // 行 列 映射 数据的索引
    std::vector<std::vector<UInt64>> _rowColumnRefShareStringIdx;
};

template<class WorkbookType>
POOL_CREATE_TEMPLATE_OBJ_DEFAULT_IMPL(Worksheet, WorkbookType);

template<class WorkbookType>
POOL_CREATE_TEMPLATE_OBJ_DEFAULT_TL_IMPL(Worksheet, WorkbookType);


template<class WorkbookType>
ALWAYS_INLINE LibString Worksheet<WorkbookType>::LoadCells()
{
    _rowColumnRefShareStringIdx.clear();
    _maxRow = 0;
    _maxColumn = 0;
    for(const auto &cellItem : _allCells)
    {
        UInt64 rowId = std::get<0>(cellItem);
        _maxRow = std::max<UInt64>(rowId, _maxRow);
        UInt64 columnId = std::get<1>(cellItem);
        _maxColumn = std::max<UInt64>(_maxColumn, columnId);
    }

    _rowColumnRefShareStringIdx.reserve(_maxRow + 1);
    _rowColumnRefShareStringIdx.emplace_back();
    for (UInt64 idx = 0; idx < _maxRow; ++idx)
        _rowColumnRefShareStringIdx.emplace_back(_maxColumn + 1);

    for (const auto &cellItem : _allCells)
    {
        UInt64 rowId = std::get<0>(cellItem);
        UInt64 columnId = std::get<1>(cellItem);
        UInt64 ssIdx = std::get<2>(cellItem);
        _rowColumnRefShareStringIdx[rowId][columnId] = ssIdx;
    }

    return _AfterLoaded();
}

template<class WorkbookType>
ALWAYS_INLINE const LibString &Worksheet<WorkbookType>::GetSheetName() const
{
    return _sheetName;
}

template<class WorkbookType>
ALWAYS_INLINE UInt64 Worksheet<WorkbookType>::GetMaxRow() const
{
    return _maxRow;
}

template<class WorkbookType>
ALWAYS_INLINE UInt64 Worksheet<WorkbookType>::GetMaxColumn() const
{
    return _maxColumn;
}

template<class WorkbookType>
ALWAYS_INLINE const WorkbookType *Worksheet<WorkbookType>::GetWorkbook() const
{
    return _workbook;
}

template<class WorkbookType>
ALWAYS_INLINE const std::vector<std::vector<UInt64>> &Worksheet<WorkbookType>::GetAllSharedStringIdxs() const
{
    return _rowColumnRefShareStringIdx;
}

template<class WorkbookType>
ALWAYS_INLINE const std::vector<UInt64> &Worksheet<WorkbookType>::GetRowSharedStringIdxs(UInt64 rowId) const
{
    static const std::vector<UInt64> s_empty;
    if(UNLIKELY(rowId >= static_cast<UInt64>(_rowColumnRefShareStringIdx.size())))
    {
        return s_empty;
    }

    return _rowColumnRefShareStringIdx[rowId];
}

template<class WorkbookType>
ALWAYS_INLINE bool Worksheet<WorkbookType>::GetCellShareStringIndex(UInt64 rowId, UInt64 columnId, UInt64 &shareStringIdx) const
{
    const auto &columnShareIdxs = GetRowSharedStringIdxs(rowId);
    if(UNLIKELY(columnShareIdxs.empty()))
        return false;

    if(UNLIKELY(columnId >= static_cast<UInt64>(columnShareIdxs.size())))
        return false;

    shareStringIdx = columnShareIdxs[columnId];
    return true;
}

KERNEL_END

#endif
