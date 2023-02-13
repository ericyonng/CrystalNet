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
 * Date: 2023-01-23 18:06:33
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_XLSX_XLSX_WORKBOOK_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_XLSX_XLSX_WORKBOOK_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/memory.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Archive/archive.h>
#include <kernel/comp/Log/log.h>
#include <kernel/comp/xml/xml.h>
#include <kernel/comp/Utils/ContainerUtil.h>
#include <kernel/comp/Utils/StringUtil.h>
#include <kernel/comp/Utils/DirectoryUtil.h>
#include <kernel/comp/SmartPtr.h>

KERNEL_BEGIN

class LibString;
class ArchiveFile;
class XMLDocument;
class XlsxSheet;

class XlsxWorkbook
{
    POOL_CREATE_OBJ_DEFAULT(XlsxWorkbook);

public:
    XlsxWorkbook(bool useThreadlocal = true);
    virtual ~XlsxWorkbook();

    bool Parse(const LibString &xlsxPath);
    void Clear();

    bool GetSheetIndexByName(const LibString &sheetName, UInt64 &sheetIdx) const;
    WorksheetType *GetSheet(UInt64 sheetIdx);
    const WorksheetType *GetSheet(UInt64 sheetIdx) const;
    UInt64 GetSheetAmount() const;
    const LibString &GetWorkbookName() const;
    bool GetSharedString(UInt64 sharedStringIndex, const LibString *&sharedString) const;

private:
    bool _ParseAllSharedStrings(const ArchiveFile &achive, std::vector<LibString> &allSharedString);
    // tuple:<sheet_name, sheet_id, relation_id>
    bool _ParseAllSheetRelation(const ArchiveFile &achive, std::vector<std::tuple<LibString, UInt64, LibString>> &allSheetReleations);
    bool _ParseCells(UInt64 sheetIdx, std::vector<std::tuple<UInt64, UInt64, UInt64>> *&allCells);

    bool _ParseXml(const ArchiveFile &achive, const LibString &docPath, XMLDocument *&docXml);

    std::pair<UInt64, UInt64> _ParseRowColumnFrom(const LibString &tupleString) const;
    UInt64 _ColumnIndexFrom(const LibString &columnString) const;

    void _StripBlank(LibString &currentValue);

    XMLDocument *_GetDocument(const LibString &docPath);
    const XMLDocument *_GetDocument(const LibString &docPath) const;
    XMLDocument *_GetDocumentBy(UInt64 sheetIdx);

    UInt64 _GetIndexFromString(const LibString &shareString);

private:
    bool _useThreadlocal;
    ArchiveFile *_achive;
    LibString _workBookName;
    std::unordered_map<LibString, XMLDocument *> _docPathRefXmlDoc;

    // 单元格 tuple:row_index, column_index, share_string_index
    std::unordered_map<UInt64, std::vector<std::tuple<UInt64, UInt64, UInt64>>> _sheetIdxRefAllCells;

    std::unordered_map<LibString, XlsxSheet *> _sheetIdRefSheet;   // xlsx中的sheetId

    std::unordered_map<LibString, XlsxSheet *> _sheetNameRefSheet;
};




template<typename WorksheetType>
ALWAYS_INLINE bool XlsxWorkbook<WorksheetType>::GetSheetIndexByName(const LibString &sheetName, UInt64 &sheetIdx) const
{
    auto iter = _sheetNameRefSheetRelationVectorIndex.find(sheetName);
    if(iter == _sheetNameRefSheetRelationVectorIndex.end())
        return false;

    sheetIdx = iter->second;
    return true;
}

template<typename WorksheetType>
ALWAYS_INLINE WorksheetType *XlsxWorkbook<WorksheetType>::GetSheet(UInt64 sheetIdx)
{
    if(UNLIKELY(_sheets.size() <= sheetIdx))
        return NULL;

    return _sheets[sheetIdx];
}

template<typename WorksheetType>
ALWAYS_INLINE const WorksheetType *XlsxWorkbook<WorksheetType>::GetSheet(UInt64 sheetIdx) const
{
    if(UNLIKELY(_sheets.size() <= sheetIdx))
        return NULL;

    return _sheets[sheetIdx];
}

template<typename WorksheetType>
ALWAYS_INLINE UInt64 XlsxWorkbook<WorksheetType>::GetSheetAmount() const
{
    return static_cast<UInt64>(_sheetNameRefSheet.size());
}

template<typename WorksheetType>
ALWAYS_INLINE const LibString &XlsxWorkbook<WorksheetType>::GetWorkbookName() const
{
    return _workBookName;
}

template<typename WorksheetType>
ALWAYS_INLINE bool XlsxWorkbook<WorksheetType>::GetSharedString(UInt64 sharedStringIndex, const LibString *&sharedString) const
{
    if(UNLIKELY(static_cast<UInt64>(_allXmlSharedStrings.size()) <= sharedStringIndex))
    {
        return false;
    }

    sharedString = &(_allXmlSharedStrings[sharedStringIndex]);
    return true;
}


template<typename WorksheetType>
ALWAYS_INLINE std::pair<UInt64, UInt64> XlsxWorkbook<WorksheetType>::_ParseRowColumnFrom(const LibString &tupleString) const
{
    //sample input A1
    uint32_t columnCharSize = 0;
    while (::isalpha(tupleString[columnCharSize]))
        ++columnCharSize;

    UInt64 columnValue = _ColumnIndexFrom(tupleString.GetRaw().substr(0, columnCharSize));
    UInt64 rowValue = StringUtil::StringToUInt64(tupleString.GetRaw().substr(columnCharSize).c_str());
    return std::make_pair(rowValue, columnValue);
}

template<typename WorksheetType>
ALWAYS_INLINE UInt64 XlsxWorkbook<WorksheetType>::_ColumnIndexFrom(const LibString &columnString) const
{
    if (columnString.length() > 3 || columnString.empty())
    {
        return 0;
    }

    UInt64 columnIndex = 0;
    Int32 place = 1;

    for (Int32 i = static_cast<int>(columnString.length()) - 1; i >= 0; --i)
    {
        if (!std::isalpha(columnString[static_cast<std::size_t>(i)]))
        {
            return 0;
        }

        auto charIndex = std::toupper(columnString[static_cast<std::size_t>(i)]) - 'A';

        columnIndex += static_cast<UInt64>((charIndex + 1) * place);
        place *= 26;
    }

    return columnIndex;
}

template<typename WorksheetType>
ALWAYS_INLINE void XlsxWorkbook<WorksheetType>::_StripBlank(LibString &currentValue)
{
    currentValue.strip(" \t\n");
}

template<typename WorksheetType>
ALWAYS_INLINE XMLDocument *XlsxWorkbook<WorksheetType>::_GetDocument(const LibString &docPath)
{
    auto iter = _docPathRefXmlDoc.find(docPath);
    return iter == _docPathRefXmlDoc.end() ? NULL : iter->second;
}

template<typename WorksheetType>
ALWAYS_INLINE const XMLDocument *XlsxWorkbook<WorksheetType>::_GetDocument(const LibString &docPath) const
{
    auto iter = _docPathRefXmlDoc.find(docPath);
    return iter == _docPathRefXmlDoc.end() ? NULL : iter->second;
}

template<typename WorksheetType>
ALWAYS_INLINE XMLDocument *XlsxWorkbook<WorksheetType>::_GetDocumentBy(UInt64 sheetIdx)
{
    LibString sheetPath = "xl/worksheets/sheet" + StringUtil::Num2Str(sheetIdx) + ".xml";
    XMLDocument *doc = NULL;
    if(!_ParseXml(*_achive, sheetPath, doc))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("parse xml fail when get document by sheet idx :%llu, achive:%s.")
        , sheetIdx, _achive->GetFilePath().c_str());
        return NULL;
    }

    return doc;
}

template<typename WorksheetType>
ALWAYS_INLINE UInt64 XlsxWorkbook<WorksheetType>::_GetIndexFromString(const LibString &shareString)
{
    auto iter = _sharedStringIndexs.find(shareString);
    if (iter != _sharedStringIndexs.end())
        return iter->second;

    _allXmlSharedStrings.push_back(shareString);
    auto idx = static_cast<UInt64>(_allXmlSharedStrings.size() - 1);
    _sharedStringIndexs[shareString] = idx;
    return idx;
}


KERNEL_END

#endif

