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
 * Date: 2023-01-27 22:13:43
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/xlsx/xlsx_worksheet.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(XlsxSheet);

XlsxSheet::XlsxSheet(void *workbook, const LibString &sheetName, UInt64 sheetId, const std::vector<std::tuple<UInt64, UInt64, UInt64>> &allCells)
:_workbook(workbook)
,_sheetName(sheetName)
,_sheetId(sheetId)
,_allCells(allCells)
{

}

LibString XlsxSheet::LoadCells()
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

KERNEL_END