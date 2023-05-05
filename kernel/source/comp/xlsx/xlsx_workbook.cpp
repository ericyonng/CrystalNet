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
#include <kernel/comp/SmartPtr.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(XlsxWorkbook);

XlsxWorkbook::XlsxWorkbook(bool useThreadlocal)
:_useThreadlocal(useThreadlocal)
{

}

XlsxWorkbook::~XlsxWorkbook()
{
    Clear();
}

void XlsxWorkbook::Clear()
{
    _workBookName.clear();
    ContainerUtil::DelContainer(_docPathRefXmlDoc);

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

bool XlsxWorkbook::Parse(const LibString &xlsxPath)
{
    // 先清理
    Clear();

    SmartPtr<ArchiveFile, AutoDelMethods::CustomDelete> achive;
    // zip解压_fileRefContent
    if(_useThreadlocal)
    {
        achive = ArchiveFile::NewThreadLocal_ArchiveFile();
    }
    else
    {
        achive = ArchiveFile::New_ArchiveFile();
    }

    achive.SetClosureDelegate([this](void *p){
        ArchiveFile *ptr = reinterpret_cast<ArchiveFile *>(p);
        if(_useThreadlocal)
        {
            ArchiveFile::DeleteThreadLocal_ArchiveFile(ptr);
        }
        else
        {
            ArchiveFile::Delete_ArchiveFile(ptr);
        }
    });

    if(!achive->ExtractToMem(xlsxPath))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("achive extract to mem fail. xlsxPath:%s"), xlsxPath.c_str());
        return false;
    }

    // 1.拿到所有的公共数据
    std::vector<LibString> allSharedStrings;
    if(!_ParseAllSharedStrings(*achive, allSharedStrings))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("parse all shared string from achive:%s fail"), xlsxPath.c_str());
        return false;
    }

    // 2.拿到所有页信息 页信息在workbook.xml中且与顺序强相关 解析sheet name, sheet id, r:id
    // allSheetRelations的下标索引用于索引sheetXX.xml文件
    std::vector<std::tuple<LibString, UInt64, LibString>> allSheetRelations;
    if(!_ParseAllSheetRelation(*achive, allSheetRelations))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("parse all sheet from achive:%s fail"), xlsxPath.c_str());
        return false;
    }

    allSharedStrings.shrink_to_fit();

    // 3.解析所有页数据
    {
        const UInt64 sz = static_cast<UInt64>(allSheetRelations.size());
        for(UInt64 idx = 0; idx < sz; ++idx)
        {
            std::vector<std::tuple<UInt64, UInt64, LibString>> cells;
            // idx + 1用来索引sheetXXX.xml
            if(!_ParseCells(*achive, idx + 1, allSharedStrings, cells))
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("have no cells in this sheet:%llu, achive:%s"), idx, achive->GetFilePath().c_str());
                continue;
            }

            auto &sheetRelation = allSheetRelations[idx];
            auto &sheetName = std::get<0>(sheetRelation);
            auto &sheetId = std::get<1>(sheetRelation);
            auto newSheet = _CreateSheet(sheetName, sheetId);

            for(auto &tupleCell : cells)
            {
                auto rowId = std::get<0>(tupleCell);
                auto columnId = std::get<1>(tupleCell);
                auto &content = std::get<2>(tupleCell);
                newSheet->AddCell(rowId, columnId, content);
            }
        }
    }
	
    _workBookName = DirectoryUtil::GetFileNameInPath(achive->GetFilePath());
    _workBookPath = achive->GetFilePath();
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
            allSharedString.push_back(value);
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

// sheet name, sheetId, sheet r id, sheetOrderId
// sheetorderId用于索引sheetxx.xml用
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

bool XlsxWorkbook::_ParseCells(ArchiveFile &achive, UInt64 sheetIdx, const std::vector<LibString> &allSharedStrings, std::vector<std::tuple<UInt64, UInt64, LibString>> &newCells)
{
    auto doc = _GetDocumentBy(achive, sheetIdx);
    if(!doc)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("get document by sheet id:%llu fail, achive path:%s"), sheetIdx, achive.GetFilePath().c_str());
        return false;
    }

    auto worksheetNode = doc->FirstChildElement("worksheet");
    if(worksheetNode == NULL)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("document have no any worksheet sheet id:%llu, achive path:%s"), sheetIdx, achive.GetFilePath().c_str());
        return false;
    }

    auto sheetDataNode = worksheetNode->FirstChildElement("sheetData");
    if(sheetDataNode == NULL)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("have no any sheet data sheet id:%llu, achieve:%s"), sheetIdx, achive.GetFilePath().c_str());
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
            auto typeAttr = cellNode->Attribute("t");

            // shared string index
            // typeAttr == s说明数据在shareStrings.xml中
            if(typeAttr && *typeAttr == 's')
            {
				// 由于我们在前面放了一个空字符串 所以后续的字符串索引都要加1
                currentValue.strip();
                currentValue = allSharedStrings[StringUtil::StringToInt64(currentValue.c_str())];
            }

            cellNode = cellNode->NextSiblingElement("c");
            newCells.emplace_back(rowIndex, columeIdx, currentValue);
        }

        rowNode = rowNode->NextSiblingElement("row");
    }

    return true;
}

XlsxSheet *XlsxWorkbook::_CreateSheet(const LibString &sheetName, UInt64 sheetId)
{
    XlsxSheet *newSheet = NULL;
    if(_useThreadlocal)
    {
        newSheet = XlsxSheet::NewThreadLocal_XlsxSheet(this, sheetName, sheetId);
    }
    else
    {
        newSheet = XlsxSheet::New_XlsxSheet(this, sheetName, sheetId);
    }

    _sheetNameRefSheet.insert(std::make_pair(sheetName, newSheet));
    return newSheet;
}


KERNEL_END