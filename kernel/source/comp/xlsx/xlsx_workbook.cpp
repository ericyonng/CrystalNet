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
 * Date: 2023-01-23 18:30:52
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/xlsx/xlsx_workbook.h>
#include <kernel/comp/xlsx/xlsx_worksheet.h>

KERNEL_BEGIN

XlsxWorkbook::XlsxWorkbook(bool useThreadlocal)
:_useThreadlocal(useThreadlocal)
,_achive(NULL)
{

}

XlsxWorkbook::~XlsxWorkbook()
{
    Clear();
}

void XlsxWorkbook::Clear()
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

    _allSheetRelations.clear();
    _sheetIdxRefAllCells.clear();

    ContainerUtil::DelContainer(_sheetNameRefSheet, [this](XlsxSheet *ptr){
        if(_useThreadlocal)
        {
            XlsxSheet::DeleteThreadLocal_XlsxSheet(ptr);
        }
        else
        {
            XlsxSheet::Delete_XlsxSheet(ptr);
        }
    });
}

bool XlsxWorkbook:Parse(const LibString &xlsxPath)
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
    std::vector<LibString> allSharedStrings;
    if(!_ParseAllSharedStrings(*_achive, allSharedStrings))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("parse all shared string from achive:%s fail"), xlsxPath.c_str());
        return false;
    }
    allSharedStrings.insert(allSharedStrings.begin(), LibString());

    // 解析sheet name, sheet id, r:id
    std::vector<std::tuple<LibString, UInt64, LibString>> allSheetRelations;
    if(!_ParseAllSheetRelation(*_achive, allSheetRelations))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("parse all sheet from achive:%s fail"), xlsxPath.c_str());
        return false;
    }
    {
        const UInt64 sz = static_cast<UInt64>(allSheetRelations.size());
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


bool XlsxWorkbook::_ParseAllSharedStrings(const ArchiveFile &achive, std::vector<LibString> &allSharedString)
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

bool XlsxWorkbook::_ParseXml(const ArchiveFile &achive, const LibString &docPath, XMLDocument *&docXml)
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


bool XlsxWorkbook::_ParseAllSheetRelation(const ArchiveFile &achive, std::vector<std::tuple<LibString, UInt64, LibString>> &allSheetReleations)
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

bool XlsxWorkbook::_ParseCells(UInt64 sheetIdx, std::vector<std::tuple<UInt64, UInt64, UInt64>> *&allCells)
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

KERNEL_END