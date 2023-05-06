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
#include <kernel/comp/xlsx/xlsx_cell.h>
#include <kernel/comp/Utils/ContainerUtil.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(XlsxSheet);

XlsxSheet::XlsxSheet(XlsxWorkbook *workbook, const LibString &sheetName, UInt64 sheetId)
:_workbook(workbook)
,_sheetName(sheetName)
,_sheetId(sheetId)
,_maxRow(0)
,_maxColumn(0)
,_line(0)
{

}

void XlsxSheet::AddCell(UInt64 rowId, UInt64 columnId, const LibString &content)
{
    auto cell = XlsxCell::New_XlsxCell(this);
    cell->_row = rowId;
    cell->_column = columnId;
    cell->_content = content;
    cell->_owner = this;

    if(_maxRow < cell->_row)
    {
        _maxRow = cell->_row;
        ++_line;
    }
    
    if(_maxColumn < cell->_column)
        _maxColumn = cell->_column;

    {
        auto iter = _rowRefColumnRefCells.find(cell->_row);
        if(iter == _rowRefColumnRefCells.end())
            iter = _rowRefColumnRefCells.insert(std::make_pair(cell->_row, std::map<UInt64, XlsxCell *>())).first;
        auto &columnRefCells = iter->second;

        columnRefCells.insert(std::make_pair(cell->_column, cell));
    }

    {
        auto iter = _columnRefRowRefCells.find(cell->_column);
        if(iter == _columnRefRowRefCells.end())
            iter = _columnRefRowRefCells.insert(std::make_pair(cell->_column, std::map<UInt64, XlsxCell *>())).first;

        auto &rowRefCells = iter->second;
        rowRefCells.insert(std::make_pair(cell->_row, cell));
    }
}

void XlsxSheet::Clear()
{
   ContainerUtil::DelContainer(_rowRefColumnRefCells, [](std::map<UInt64, XlsxCell *> &dict){
        ContainerUtil::DelContainer(dict, [](XlsxCell *ptr){
            XlsxCell::Delete_XlsxCell(ptr);
        });
   });

   _columnRefRowRefCells.clear();
   _maxRow = 0;
   _maxColumn = 0;
}

LibString XlsxSheet::ToRowString(UInt64 rowId) const
{
    LibString rowInfo;
    auto &rowCells = GetRowCells(rowId);
    for(auto &iter : rowCells)
    {
        auto rowCell = iter.second;
        rowInfo += rowCell->ToString();
        rowInfo += "|";
    }

    return rowInfo;
}

KERNEL_END