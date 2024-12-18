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
 * Workbook解析原理
 * 1. 关键文件:sharedStrings.xml(相同数据会被单独放到该文件)
 * 2. 关键文件sheetxx.xml 页数据(当cell的attr == s时表示数据在sharedStrings.xml 它的value是sharedStrings的顺序id 从0开始)
 * 3. 数据要么在sharedStrings中要么在sheetxx.xml中
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_XLSX_XLSX_WORKBOOK_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_XLSX_XLSX_WORKBOOK_H__

#pragma once

#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/LibString.h>
#include <cstdint>
#include <map>

KERNEL_BEGIN

class LibString;
class ArchiveFile;
class XMLDocument;
class XlsxSheet;

class KERNEL_EXPORT XlsxWorkbook
{
    POOL_CREATE_OBJ_DEFAULT(XlsxWorkbook);

public:
    XlsxWorkbook(bool useThreadlocal = true);
    virtual ~XlsxWorkbook();

    bool Parse(const LibString &xlsxPath);
    void Clear();

    UInt64 GetSheetAmount() const;
    const LibString &GetWorkbookFileName() const;
    const LibString &GetWorkbookPath() const;

    const std::map<LibString, XlsxSheet *> &GetAllSheets() const;
    const XlsxSheet *GetSheet(const LibString &sheetName) const;

private:
    bool _ParseAllSharedStrings(const ArchiveFile &achive, std::vector<LibString> &allSharedString);
    // tuple:<sheet_name, sheet_id, relation_id>
    bool _ParseAllSheetRelation(const ArchiveFile &achive, std::vector<std::tuple<LibString, UInt64, LibString>> &allSheetReleations);
    bool _ParseCells(ArchiveFile &achive, UInt64 sheetIdx, const std::vector<LibString> &allSharedStrings, std::vector<std::tuple<UInt64, UInt64, LibString>> &newCells);
    bool _ParseXml(const ArchiveFile &achive, const LibString &docPath, XMLDocument *&docXml);

    std::pair<UInt64, UInt64> _ParseRowColumnFrom(const LibString &tupleString) const;
    UInt64 _ColumnIndexFrom(const LibString &columnString) const;

    // void _StripBlank(LibString &currentValue);

    XMLDocument *_GetDocument(const LibString &docPath);
    const XMLDocument *_GetDocument(const LibString &docPath) const;
    XMLDocument *_GetDocumentBy(ArchiveFile &achive, UInt64 sheetIdx);

    XlsxSheet *_CreateSheet(const LibString &sheetName, UInt64 sheetId);

private:
    bool _useThreadlocal;
    LibString _workBookName;
    LibString _workBookPath;
    std::map<LibString, XMLDocument *> _docPathRefXmlDoc;

    // 页数据
    std::map<LibString, XlsxSheet *> _sheetNameRefSheet;
};

ALWAYS_INLINE UInt64 XlsxWorkbook::GetSheetAmount() const
{
    return static_cast<UInt64>(_sheetNameRefSheet.size());
}

ALWAYS_INLINE const LibString &XlsxWorkbook::GetWorkbookFileName() const
{
    return _workBookName;
}

ALWAYS_INLINE const LibString &XlsxWorkbook::GetWorkbookPath() const
{
    return _workBookPath;
}

ALWAYS_INLINE const std::map<LibString, XlsxSheet *> &XlsxWorkbook::GetAllSheets() const
{
    return _sheetNameRefSheet;
}

ALWAYS_INLINE const XlsxSheet *XlsxWorkbook::GetSheet(const LibString &sheetName) const
{
    auto iter = _sheetNameRefSheet.find(sheetName);
    return iter == _sheetNameRefSheet.end() ? NULL : iter->second;   
}

ALWAYS_INLINE XMLDocument *XlsxWorkbook::_GetDocument(const LibString &docPath)
{
    auto iter = _docPathRefXmlDoc.find(docPath);
    return iter == _docPathRefXmlDoc.end() ? NULL : iter->second;
}

ALWAYS_INLINE const XMLDocument *XlsxWorkbook::_GetDocument(const LibString &docPath) const
{
    auto iter = _docPathRefXmlDoc.find(docPath);
    return iter == _docPathRefXmlDoc.end() ? NULL : iter->second;
}

KERNEL_END

#endif

