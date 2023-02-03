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

template<typename WorksheetType>
class XlsxWorkbook
{
    POOL_CREATE_TEMPLATE_OBJ_DEFAULT(XlsxWorkbook, WorksheetType);

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

    // xml 中share string 解析内容以及映射字典
    std::vector<LibString> _allXmlSharedStrings;
    std::unordered_map<LibString, UInt64> _sharedStringIndexs;

    // 页解析
    std::vector<std::tuple<LibString, UInt64, LibString>> _allSheetRelations;

    // 单元格 tuple:row_index, column_index, share_string_index
    std::unordered_map<UInt64, std::vector<std::tuple<UInt64, UInt64, UInt64>>> _sheetIdxRefAllCells;

    // 页
    std::vector<WorksheetType *> _sheets;
    std::unordered_map<LibString, WorksheetType *> _sheetNameRefSheet;
    std::unordered_map<LibString, UInt64> _sheetNameRefSheetRelationVectorIndex;

    std::unordered_map<LibString, XlsxSheet *> _sheetIdRefSheet;   // xlsx中的sheetId
};

template<typename WorksheetType>
POOL_CREATE_TEMPLATE_OBJ_DEFAULT_IMPL(XlsxWorkbook, WorksheetType);

template<typename WorksheetType>
POOL_CREATE_TEMPLATE_OBJ_DEFAULT_TL_IMPL(XlsxWorkbook, WorksheetType);

template<typename WorksheetType>
ALWAYS_INLINE XlsxWorkbook<WorksheetType>::XlsxWorkbook(bool useThreadlocal)
:_useThreadlocal(useThreadlocal)
,_achive(NULL)
{

}

template<typename WorksheetType>
ALWAYS_INLINE XlsxWorkbook<WorksheetType>::~XlsxWorkbook()
{
    Clear();
}

template<typename WorksheetType>
ALWAYS_INLINE void XlsxWorkbook<WorksheetType>::Clear()
{
    _workBookName.clear();
    if(_achive)
    {
        if(_useThreadlocal)
        {
            ArchiveFile::DeleteThreadLocal_ArchiveFile(_achive);  
        }
        else
        {
            ArchiveFile::Delete_ArchiveFile(_achive);
        }

        _achive = NULL;
    }

    ContainerUtil::DelContainer(_docPathRefXmlDoc);

    _allXmlSharedStrings.clear();
    _sharedStringIndexs.clear();
    _allSheetRelations.clear();
    _sheetIdxRefAllCells.clear();

    ContainerUtil::DelContainer(_sheets, [](WorksheetType *ptr){
        WorksheetType::FactoryRelease(ptr);
    });
    _sheetNameRefSheet.clear();
    _sheetNameRefSheetRelationVectorIndex.clear();
}

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
ALWAYS_INLINE bool XlsxWorkbook<WorksheetType>::Parse(const LibString &xlsxPath)
{
    // 先清理
    Clear();

    // zip解压_fileRefContent
    if(_useThreadlocal)
    {
        _achive = ArchiveFile::NewThreadLocal_ArchiveFile();
    }
    else
    {
        _achive = ArchiveFile::New_ArchiveFile();
    }
    if(!_achive->ExtractToMem(xlsxPath))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("achive extract to mem fail. xlsxPath:%s"), xlsxPath.c_str());
        return false;
    }

    // xml解析
    if(!_ParseAllSharedStrings(*_achive, _allXmlSharedStrings))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("parse all shared string from achive:%s fail"), xlsxPath.c_str());
        return false;
    }
    _allXmlSharedStrings.insert(_allXmlSharedStrings.begin(), LibString());

    // share shring 的字典映射
    {
        const UInt64 sz = static_cast<UInt64>(_allXmlSharedStrings.size());
        for(UInt64 idx = 0; idx < sz; ++idx)
            _sharedStringIndexs[_allXmlSharedStrings[idx]] = idx;
    }

    // 解析sheet
    if(!_ParseAllSheetRelation(*_achive, _allSheetRelations))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("parse all sheet from achive:%s fail"), xlsxPath.c_str());
        return false;
    }
    {
        const UInt64 sz = static_cast<UInt64>(_allSheetRelations.size());
        for(UInt64 idx = 0; idx < sz; ++idx)
        {
            std::vector<std::tuple<UInt64, UInt64, UInt64>> *cells = NULL;
            if(!_ParseCells(idx + 1, cells))
                g_Log->Warn(LOGFMT_OBJ_TAG("have no cells in this sheet:%llu, achive:%s"), idx, _achive->GetFilePath().c_str());
        }
    }
    _allXmlSharedStrings.shrink_to_fit();

    {
        const UInt64 sz = static_cast<UInt64>(_allSheetRelations.size());
        for(UInt64 idx = 0; idx < sz; ++idx)
        {
            std::vector<std::tuple<UInt64, UInt64, UInt64>> *cells = NULL;
            if(!_ParseCells(idx + 1, cells))
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("have no cells in this sheet:%llu, achive:%s"), idx, _achive->GetFilePath().c_str());
                continue;
            }

            auto &sheetRelation = _allSheetRelations[idx];
            auto &sheetName = std::get<0>(sheetRelation);
            auto &sheetId = std::get<1>(sheetRelation);
            auto curSheet = WorksheetType::FactoryCreate(this, sheetName, sheetId, *cells);
            const auto &errString = curSheet->LoadCells();
            if(!errString.empty())
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("sheet load cells fail errString:%s, sheet name:%s, sheetId:%llu, achive:%s"), errString.c_str(), sheetName.c_str(), sheetId, _achive->GetFilePath().c_str());
                WorksheetType::FactoryRelease(curSheet);
                continue;
            }

            if(_sheets.size() <= idx)
                _sheets.resize(idx + 1);

            _sheets[idx] = curSheet;
            _sheetNameRefSheet.insert(std::make_pair(sheetName, curSheet));
            _sheetNameRefSheetRelationVectorIndex.insert(std::make_pair(sheetName, idx));
        }
    }
	
    _workBookName = DirectoryUtil::GetFileNameInPath(_achive->GetFilePath());
    return true;
}

template<typename WorksheetType>
ALWAYS_INLINE bool XlsxWorkbook<WorksheetType>::_ParseAllSharedStrings(const ArchiveFile &achive, std::vector<LibString> &allSharedString)
{
    LibString tablePath = "xl/sharedStrings.xml";
    XMLDocument *doc = NULL;
    if(!_ParseXml(achive, tablePath, doc))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("parse xml document fail achive path:%s."), achive.GetFilePath().c_str());
        return false;
    }

    auto shareTotalNode = doc->FirstChildElement("sst");
    if(shareTotalNode == NULL)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("have no any child element with sst achive path:%s"), achive.GetFilePath().c_str());
        return false;
    }

    auto shareStringBegin = shareTotalNode->FirstChildElement("si");
    while(shareStringBegin)
    {
        auto currentValue = shareStringBegin->FirstChildElement("t")->GetText();
        g_Log->Debug(LOGFMT_OBJ_TAG("current value:%s"), currentValue ? currentValue : "NULL");
        if(currentValue)
        {
            LibString value(currentValue);
            _StripBlank(value);
            auto valueLen = value.size();
            if(valueLen)
            {
                allSharedString.push_back(value);
            }
            else
            {
                allSharedString.push_back(LibString());
            }
        }
        else
        {
            allSharedString.push_back(LibString());
        }

        shareStringBegin = shareStringBegin->NextSiblingElement("si");
    }

    return true;
}

template<typename WorksheetType>
ALWAYS_INLINE bool XlsxWorkbook<WorksheetType>::_ParseAllSheetRelation(const ArchiveFile &achive, std::vector<std::tuple<LibString, UInt64, LibString>> &allSheetReleations)
{
    LibString tablePath = "xl/workbook.xml";
    XMLDocument *doc = NULL;
    if(!_ParseXml(achive, tablePath, doc))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("parse xml document fail achive path:%s."), achive.GetFilePath().c_str());
        return false;
    }

    auto workbookNode = doc->FirstChildElement("workbook");
    if(workbookNode == NULL)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("have no any child element with workbook achive path:%s"), achive.GetFilePath().c_str());
        return false;
    }

    auto sheetNode = workbookNode->FirstChildElement("sheets");
    if(sheetNode == NULL)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("have no any child element with sheets achive path:%s"), achive.GetFilePath().c_str());
        return false;
    }

    auto sheetsBegin = sheetNode->FirstChildElement("sheet");
    while(sheetsBegin)
    {
        LibString curSheetName(sheetsBegin->Attribute("name"));
        LibString curSheetId(sheetsBegin->Attribute("sheetId"));
        LibString curRelationId(sheetsBegin->Attribute("r:id"));

        allSheetReleations.emplace_back(std::make_tuple(curSheetName, StringUtil::StringToUInt64(curSheetId.c_str()), curRelationId));
        sheetsBegin = sheetsBegin->NextSiblingElement("sheet");
    }
    
    return true;
}

template<typename WorksheetType>
ALWAYS_INLINE bool XlsxWorkbook<WorksheetType>::_ParseCells(UInt64 sheetIdx, std::vector<std::tuple<UInt64, UInt64, UInt64>> *&allCells)
{
    auto iter = _sheetIdxRefAllCells.find(sheetIdx);
    if(iter != _sheetIdxRefAllCells.end())
    {
        allCells = &(iter->second);
        return true;
    }

    std::vector<std::tuple<UInt64, UInt64, UInt64>> newCells;
    auto doc = _GetDocumentBy(sheetIdx);
    if(!doc)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("get document by sheet id:%llu fail, achive path:%s"), sheetIdx, _achive->GetFilePath().c_str());
        return false;
    }

    auto worksheetNode = doc->FirstChildElement("worksheet");
    if(worksheetNode == NULL)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("document have no any worksheet sheet id:%llu, achive path:%s"), sheetIdx, _achive->GetFilePath().c_str());
        return false;
    }

    auto sheetDataNode = worksheetNode->FirstChildElement("sheetData");
    if(sheetDataNode == NULL)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("have no any sheet data sheet id:%llu, achieve:%s"), sheetIdx, _achive->GetFilePath().c_str());
        iter = _sheetIdxRefAllCells.insert(std::make_pair(sheetIdx, newCells)).first;
        allCells = &(iter->second);

        return true;
    }

    auto rowNode = sheetDataNode->FirstChildElement("row");
    while (rowNode)
    {
        UInt64 rowIndex = StringUtil::StringToUInt64(rowNode->Attribute("r"));
        auto cellNode = rowNode->FirstChildElement("c");
        while(cellNode)
        {
            UInt64 columeIdx = _ParseRowColumnFrom(cellNode->Attribute("r")).second;
            auto vNode = cellNode->FirstChildElement("v");
            if(!vNode)
            {
                break;
            }

            LibString currentValue = cellNode->FirstChildElement("v")->GetText();
            _StripBlank(currentValue);
            auto typeAttr = cellNode->Attribute("t");

            // shared string index
            UInt64 ssIdx = 0;
            if(typeAttr && *typeAttr == 's')
            {
				// 由于我们在前面放了一个空字符串 所以后续的字符串索引都要加1
                ssIdx = StringUtil::StringToUInt64(currentValue.c_str()) + 1;
            }
            else
            {
                ssIdx = _GetIndexFromString(currentValue);
            }

            cellNode = cellNode->NextSiblingElement("c");
            newCells.emplace_back(rowIndex, columeIdx, ssIdx);
        }

        rowNode = rowNode->NextSiblingElement("row");
    }

    auto newIter = _sheetIdxRefAllCells.insert(std::make_pair(sheetIdx, newCells)).first;
    allCells = &(newIter->second);

    return true;
}

template<typename WorksheetType>
ALWAYS_INLINE bool XlsxWorkbook<WorksheetType>::_ParseXml(const ArchiveFile &achive, const LibString &docPath, XMLDocument *&docXml)
{
    docXml = _GetDocument(docPath);
    if(docXml)
    {
        return true;
    }

    SmartPtr<XMLDocument> newDoc = new XMLDocument();
    const auto &docContent = achive.GetContentBy(docPath);
    if(docContent.first == NULL)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("have no xml content doc path:%s, achive path:%s"), docPath.c_str(), achive.GetFilePath().c_str());
        return false;
    }

    auto err = newDoc->Parse(docContent.first, static_cast<size_t>(docContent.second));
    if(err != XMLError::XML_SUCCESS)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("Parse doc xml fail :%d, achive path:%s"), static_cast<Int32>(err), achive.GetFilePath().c_str());
        return false;
    }

    _docPathRefXmlDoc.insert(std::make_pair(docPath, docXml = newDoc.pop()));

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

