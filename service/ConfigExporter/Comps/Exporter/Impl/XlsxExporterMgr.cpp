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
 * Date: 2023-02-19 22:12:07
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <service/ConfigExporter/Comps/Exporter/Impl/XlsxExporterMgr.h>
#include <service/ConfigExporter/Comps/Exporter/Impl/XlsxExporterMgrFactory.h>
#include <service/ConfigExporter/Comps/Exporter/Impl/XlsxConfigInfo.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(IXlsxExporterMgr);

POOL_CREATE_OBJ_DEFAULT_IMPL(XlsxExporterMgr);

XlsxExporterMgr::XlsxExporterMgr()
{

}

XlsxExporterMgr::~XlsxExporterMgr()
{
    _Clear();
}

void XlsxExporterMgr::Release()
{
    XlsxExporterMgr::DeleteByAdapter_XlsxExporterMgr(XlsxExporterMgrFactory::_buildType.V, this);
}

KERNEL_NS::LibString XlsxExporterMgr::ToString() const
{
    return IXlsxExporterMgr::ToString();
}

Int32 XlsxExporterMgr::_OnGlobalSysInit() 
{
    _RegisterEvents();

    // 1.读取所有配置表数据
    // 2.读取表头
    // 3.生成配置

    return Status::Success;
}

void XlsxExporterMgr::_OnGlobalSysClose()
{
    _Clear();
}

Int32 XlsxExporterMgr::ExportConfigs(const std::map<KERNEL_NS::LibString, KERNEL_NS::LibString> &params)
{
    // ConfigExporter --config=xlsx --lang=S:cpp|C:C#,lua --source_dir=/xxx/ --target_dir=/xxx/ --data=/xx/ --meta=/xxx/
    // 4.解析参数
    for(auto &iter : params)
    {
        auto &key = iter.first;
        auto &value = iter.second;

        if(key == "--lang")
        {// 生成的语言版本
            auto configTypeRefLangsArr = value.Split("|");
            if(!configTypeRefLangsArr.empty())
            {
                for(auto &configTypeRefLangs : configTypeRefLangsArr)
                {
                    configTypeRefLangs.strip();
                    if(configTypeRefLangs.empty())
                        continue;
                    
                    auto configTypeLangPieces = configTypeRefLangs.Split(":");
                    if(configTypeLangPieces.empty())
                        continue;

                    if(configTypeLangPieces.size() < 2)
                        continue;

                    auto &configType = configTypeLangPieces[0];
                    auto &langsStr = configTypeLangPieces[1];
                    configType.strip();
                    langsStr.strip();
                    if(configType.empty() || langsStr.empty())
                        continue;

                    auto langsPieces = langsStr.Split(",");
                    if(langsPieces.empty())
                        continue;

                    auto iter = _configTypeRefLangTypes.find(configType);
                    if(iter == _configTypeRefLangTypes.end())
                        iter = _configTypeRefLangTypes.insert(std::make_pair(configType, std::unordered_set<KERNEL_NS::LibString>())).first;
                    auto &langsSet = iter->second; 

                    for(auto &lang : langsPieces)
                        langsSet.insert(lang);    
                }
            }
        }
        else if(key == "--source_dir")
        {
            _sourceDir = value;
        }
        else if(key == "--target_dir")
        {
            _targetDir = value;
        }
        else if(key == "--data")
        {
            _dataDir = value;
        }
        else if(key == "--meta")
        {
            _metaDir = value;
        }
        else
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("unknown param:k:%s, v:%s"), key.c_str(), value.c_str());
        }
    }
    
    // 必须指定语言版本
    if(_configTypeRefLangTypes.empty())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("have no lang types"));
        return Status::ParamError;
    }

    // 参数都不可缺省
    if(_sourceDir.empty() || _targetDir.empty() || _dataDir.empty() || _metaDir.empty())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("param error: sourceDir:%s, targetDir:%s, dataDir:%s metaDir:%s error.")
                    , _sourceDir.c_str(), _targetDir.c_str(), _dataDir.c_str(), _metaDir.c_str());
        return Status::ParamError;
    }

    // 扫描meta文件
    if(!_ScanMeta())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("_ScanMeta fail sourceDir:%s, targetDir:%s, dataDir:%s metaDir:%s error.")
                    , _sourceDir.c_str(), _targetDir.c_str(), _dataDir.c_str(), _metaDir.c_str());
        return Status::Failed;
    }

    // 扫描xlsx文件
    if(!_ScanXlsx())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("_ScanXlsx fail sourceDir:%s, targetDir:%s, dataDir:%s metaDir:%s error.")
                    , _sourceDir.c_str(), _targetDir.c_str(), _dataDir.c_str(), _metaDir.c_str());
        return Status::Failed;
    }

    // 导出配置
    if(!_DoExportConfigs())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("_DoExportConfigs fail sourceDir:%s, targetDir:%s, dataDir:%s metaDir:%s error."), 
                    _sourceDir.c_str(), _targetDir.c_str(), _dataDir.c_str(), _metaDir.c_str());
        return Status::Failed;
    }

    // 更新meta文件
    

    g_Log->Info(LOGFMT_OBJ_TAG("export xlsx success."));
    return Status::Success;
}
   
// 测试点: 加载meta成功, meta中没有对应的xlsx文件名, 或者xlsx文件不存在则清理meta， md5为空则清理meta
// 注意:xlsx所在路径相对于xlsx的base路径的相对路径与meta文件相对于meta base 路径的相对路径是一致的
bool XlsxExporterMgr::_ScanMeta()
{
    bool isSuc = true;
    KERNEL_NS::DirectoryUtil::TraverseDirRecursively(_metaDir, [this, &isSuc](const KERNEL_NS::FindFileInfo &fileInfo, bool &isParentDirContinue){
        
        bool isContinue = true;
        do
        {
            // 过滤目录
            if(KERNEL_NS::FileUtil::IsDir(fileInfo))
                break;

            // 过滤非meta
            if(KERNEL_NS::FileUtil::ExtractFileExtension(fileInfo._fileName) != KERNEL_NS::LibString(".meta"))
                break;

            KERNEL_NS::LibString fullPath = fileInfo._rootPath;
            if(fileInfo._rootPath.at(fileInfo._rootPath.length() - 1) != '/')
                fullPath.AppendFormat("/");

            const auto &fullFilePath = fullPath + fileInfo._fileName;

            // 拿pbcache中的缓存数据
            auto newMeta = XlsxConfigMetaInfo::New_XlsxConfigMetaInfo();
            auto ptr = KERNEL_NS::FileUtil::OpenFile(fullFilePath.c_str());
            if(!ptr)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("open meta file fail:%s"), fullFilePath.c_str());
                isContinue = false;
                isParentDirContinue = false;
                isSuc = false;
                break;
            }

            KERNEL_NS::SmartPtr<FILE, KERNEL_NS::AutoDelMethods::CustomDelete> fp(ptr);
            fp.SetClosureDelegate([](void *p){
                auto ptr = reinterpret_cast<FILE *>(p);
                KERNEL_NS::FileUtil::CloseFile(*ptr);
            });

            newMeta->_metaRootPath = fileInfo._rootPath;
            newMeta->_metaFileName = fileInfo._fileName;
            newMeta->_relationPath = fileInfo._rootPath - _metaDir;
            _metaNameRefConfigMetaInfo.insert(std::make_pair(fullFilePath, newMeta));

            std::vector<KERNEL_NS::LibString> lines;
            KERNEL_NS::FileUtil::ReadUtf8File(*fp, lines);
            if(lines.empty())
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("meta file have no any content:%s"), fullFilePath.c_str());
                isContinue = false;
                isParentDirContinue = false;
                isSuc = false;
                break;
            }

            // 填充内容
            for(auto &content : lines)
            {
                auto kv = content.Split(":");
                if(kv.empty() || (kv.size() < 2))
                    continue;

                auto k = kv[0].strip();
                auto v = kv[1].strip();
                if(k == "XlsxFile")
                {
                    if(v.empty())
                    {
                        g_Log->Warn(LOGFMT_OBJ_TAG("bad meta file content:%s, file:%s"), content.c_str(), fullFilePath.c_str());
                        break;
                    }

                    newMeta->_xlsxFileName = v;
                }
                else if(k == "Md5")
                {
                    if(v.empty())
                    {
                        g_Log->Warn(LOGFMT_OBJ_TAG("bad meta file content:%s, file:%s"), content.c_str(), fullFilePath.c_str());
                        break;
                    }

                    newMeta->_lastMd5 = v;
                }
            }
        } while (false);
        
        return isContinue;
    });

    // 不存在的xlsx需要清理掉, 清理无效的meta文件
    for(auto iter = _metaNameRefConfigMetaInfo.begin(); iter != _metaNameRefConfigMetaInfo.end();)
    {
        auto metaInfo = iter->second;
        const auto metaFilePath = metaInfo->_metaRootPath + metaInfo->_metaFileName;
        // 1.没有对应的xlsx文件或者对应的文件不存在则删除meta
        if(metaInfo->_xlsxFileName.empty())
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("have no xlsx file, will remove meta file, metaFilePath:%s.")
                    , metaFilePath.c_str()); 

            KERNEL_NS::FileUtil::DelFileCStyle(metaFilePath.c_str());
            XlsxConfigMetaInfo::Delete_XlsxConfigMetaInfo(metaInfo);
            iter = _metaNameRefConfigMetaInfo.erase(iter);
            continue;
        }
        else
        {
            const auto xlsxFullPath = _sourceDir + metaInfo->_relationPath + metaInfo->_xlsxFileName;
            if(!KERNEL_NS::FileUtil::IsFileExist(xlsxFullPath.c_str()))
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("xlsx file not found:%s, will remove meta file, metaFilePath:%s.")
                    , xlsxFullPath.c_str(), metaFilePath.c_str());    
            }

            XlsxConfigMetaInfo::Delete_XlsxConfigMetaInfo(metaInfo);
            KERNEL_NS::FileUtil::DelFileCStyle(metaFilePath.c_str());
            iter = _metaNameRefConfigMetaInfo.erase(iter);

            continue;
        }
        
        // 2.md5是空的则删除meta文件
        if(metaInfo->_lastMd5.empty())
        {
            XlsxConfigMetaInfo::Delete_XlsxConfigMetaInfo(metaInfo);
            KERNEL_NS::FileUtil::DelFileCStyle(metaFilePath.c_str());
            iter = _metaNameRefConfigMetaInfo.erase(iter);
            continue;
        }

        ++iter;
    }

    return isSuc;
}

bool XlsxExporterMgr::_ScanXlsx()
{
    bool isSuc = true;
    KERNEL_NS::DirectoryUtil::TraverseDirRecursively(_sourceDir, [this, &isSuc](const KERNEL_NS::FindFileInfo &fileInfo, bool &isParentDirContinue){
        
        bool isContinue = true;
        do
        {
            // 过滤目录
            if(KERNEL_NS::FileUtil::IsDir(fileInfo))
                break;

            // 过滤非xlsx
            if(KERNEL_NS::FileUtil::ExtractFileExtension(fileInfo._fileName) != KERNEL_NS::LibString(".xlsx"))
                break;

            KERNEL_NS::LibString fullPath = fileInfo._rootPath;
            if(fileInfo._rootPath.at(fileInfo._rootPath.length() - 1) != '/')
                fullPath.AppendFormat("/");

            const auto &fullFilePath = fullPath + fileInfo._fileName;
            
            const auto fileNameWithoutExtention = KERNEL_NS::FileUtil::ExtractFileWithoutExtension(fileInfo._fileName);

            const auto &relationDir = fullPath - _sourceDir;
            const auto metaFilePath = _metaDir + relationDir + fileNameWithoutExtention;

            KERNEL_NS::SmartPtr<KERNEL_NS::XlsxWorkbook, KERNEL_NS::AutoDelMethods::CustomDelete> xlsxBook = KERNEL_NS::XlsxWorkbook::NewThreadLocal_XlsxWorkbook(true);
            xlsxBook.SetClosureDelegate([](void *p){
                auto ptr = reinterpret_cast<KERNEL_NS::XlsxWorkbook *>(p);
                KERNEL_NS::XlsxWorkbook::DeleteThreadLocal_XlsxWorkbook(ptr);
            });

            if(!xlsxBook->Parse(fullFilePath))
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("parse xlsx fail:%s"), fullFilePath.c_str());
                isParentDirContinue = false;
                isContinue = false;
                isSuc = false;
                break;
            }

            auto &allSheet = xlsxBook->GetAllSheets();
            if(allSheet.empty())
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("have no any sheet:xlsx path:%s "), fullFilePath.c_str());
                break;
            }
                
            bool checkSuc = true;
            for(auto &iter : allSheet)
            {
                auto sheet = iter.second;
                auto &sheetName = sheet->GetSheetName();
                const auto &configTypeName = GetConfigTypeName(sheetName);
                if(!KERNEL_NS::StringUtil::CheckGeneralName(configTypeName))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("bad sheet config type: xlsx file path:%s, sheet name:%s, config type name:%s"), fullFilePath.c_str(), sheetName.c_str(), configTypeName.c_str());
                    isContinue = false;
                    isParentDirContinue = false;
                    checkSuc = false;
                    isSuc = false;
                    break;
                }

                auto iter = _configTypeRefSheets.find(configTypeName);
                if(iter == _configTypeRefSheets.end())
                    iter = _configTypeRefSheets.insert(std::make_pair(configTypeName, std::set<KERNEL_NS::XlsxSheet *>())).first;

                iter->second.insert(sheet);
            }

            if(!checkSuc)
                break;

            _xlsxFileRefWorkbook.insert(std::make_pair(fullFilePath, xlsxBook.pop()));

            // 需不需要导出
            if(!_IsNeedExport(metaFilePath, fullFilePath))
            {
                break;
            }

            // 要导出的配置类型汇总
            for(auto &iter : allSheet)
            {
                auto sheet = iter.second;
                auto &sheetName = sheet->GetSheetName();
                const auto &configTypeName = GetConfigTypeName(sheetName);
                _needExportConfigType.insert(configTypeName);
            }

            _dirtyXlsxFiles.insert(fullFilePath);

        } while (false);
        
        return isContinue;
    });

    if(!isSuc)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("scan xlsx fail source path:%s."), _sourceDir.c_str());
        return false;
    }

    return true;
}

bool XlsxExporterMgr::_IsNeedExport(const KERNEL_NS::LibString &metaFile, const KERNEL_NS::LibString &xlsxFile) const
{
    // 1.拿到metafile 若没有metafile说明需要导出
    auto meta = _GetMetaFile(metaFile);
    if(!meta)
    {
        return true;
    }

    // 2.生成xlsxfile的md5
    KERNEL_NS::SmartPtr<FILE, KERNEL_NS::AutoDelMethods::CustomDelete> fp = KERNEL_NS::FileUtil::OpenFile(xlsxFile.c_str()); 
    if(!fp)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("open file fail file:%s"), xlsxFile.c_str());
        return false;
    }

    fp.SetClosureDelegate([](void *p){
        auto ptr = reinterpret_cast<FILE *>(p);
        KERNEL_NS::FileUtil::CloseFile(*ptr);
    });

    KERNEL_NS::LibString md5;
    if(!KERNEL_NS::LibDigest::MakeFileMd5(xlsxFile, md5))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("MakeFileMd5 fail file:%s"), xlsxFile.c_str());
        return false;
    }
    md5 = KERNEL_NS::LibBase64::Encode(md5);

    // 3.若xlsxfile的md5和metafile的不同则说明需要导出
    return md5 != meta->_lastMd5;
}

bool XlsxExporterMgr::_DoExportConfigs()
{
    // 收集ownType的config table
    std::unordered_map<KERNEL_NS::LibString, std::set<XlsxConfigTableInfo *>> ownTypeRefConfigs;

    // 临时的configTable最后都要释放
    KERNEL_NS::SmartPtr<std::set<XlsxConfigTableInfo *>, KERNEL_NS::AutoDelMethods::CustomDelete> configTables = new std::set<XlsxConfigTableInfo *>;
    configTables.SetClosureDelegate([](void *p){
        auto ptr = reinterpret_cast<std::set<XlsxConfigTableInfo *> *>(p);
        KERNEL_NS::ContainerUtil::DelContainer(*ptr, [](XlsxConfigTableInfo *config){
            XlsxConfigTableInfo::Delete_XlsxConfigTableInfo(config);
        });
        delete ptr;
    });

    // 生成XlsxConfigTableInfo
    for(auto &configType : _needExportConfigType)
    {
        auto iterSheets = _configTypeRefSheets.find(configType);
        auto &sheets = iterSheets->second;
        for(auto &sheet : sheets)
        {
            // TODO:
            const auto maxRowId = sheet->GetMaxRow();
            const auto workbook = sheet->GetWorkbook();
            KERNEL_NS::SmartPtr<XlsxConfigTableInfo, KERNEL_NS::AutoDelMethods::CustomDelete> configTableInfo = XlsxConfigTableInfo::New_XlsxConfigTableInfo();
            configTableInfo.SetClosureDelegate([](void *p){
                auto ptr = reinterpret_cast<XlsxConfigTableInfo *>(p);
                XlsxConfigTableInfo::Delete_XlsxConfigTableInfo(ptr);
            });

            configTableInfo->_wholeSheetName = sheet->GetSheetName();
            configTableInfo->_tableClassName = configType;

            // 解析第一列功能区
            auto &firstColumnCells = sheet->GetColumnCells(KERNEL_NS::XlsxCell::COLUMN_BEGIN);
            if(!firstColumnCells.empty())
            {
                for(auto iter : firstColumnCells)
                {
                    if(iter.second->_content.empty())
                        continue;

                    configTableInfo->_rowIdRefFunctionBarColumn.insert(std::make_pair(iter.first, iter.second->_content));
                }
            }

            // 解析表头
            const UInt64 maxColumnId = sheet->GetMaxColumn();
            configTableInfo->_fieldInfos.resize(maxColumnId);
            for(UInt64 idx = KERNEL_NS::XlsxCell::COLUMN_BEGIN + 1; idx <= maxColumnId; ++idx)
            {
                auto newConfigFieldInfo = XlsxConfigFieldInfo::New_XlsxConfigFieldInfo(configTableInfo);
                configTableInfo->_fieldInfos[idx] = newConfigFieldInfo;

                auto &fieldsColumnCells = sheet->GetColumnCells(idx);
                if(fieldsColumnCells.empty())
                    continue;

                // 校验字段是否冲突
                {
                    auto iterField = fieldsColumnCells.find(ConfigTableDefine::FIELD_NAME);
                    if(iterField == fieldsColumnCells.end())
                    {
                        g_Log->Error(LOGFMT_OBJ_TAG("have no field name sheet name:%s, xlsx path:%s")
                        , sheet->GetSheetName().c_str(), workbook->GetWorkbookPath().c_str());
                        return false;
                    }

                    if(configTableInfo->_fieldNames.find(iterField->second->_content) != configTableInfo->_fieldNames.end())
                    {
                        g_Log->Error(LOGFMT_OBJ_TAG("field name:%s duplicate column:%llu, row:%llu sheet name:%s, xlsx path:%s")
                        , iterField->second->_content.c_str(), sheet->GetSheetName().c_str(), workbook->GetWorkbookPath().c_str());
                        return false;
                    }

                    configTableInfo->_fieldNames.insert(iterField->second->_content);
                }

                // 表头字段解析
                for(UInt64 rowId = KERNEL_NS::XlsxCell::ROW_BEGIN; rowId <= ConfigTableDefine::HEADER_ROW_NUMBER; ++rowId)
                {
                    auto iterRowCell = fieldsColumnCells.find(rowId);
                    if(iterRowCell == fieldsColumnCells.end())
                    {
                        g_Log->Warn(LOGFMT_OBJ_TAG("have no row cell rowId:%llu, column:%llu, sheet name:%s, xlsx path:%s")
                                        , rowId, idx, sheet->GetSheetName().c_str(), workbook->GetWorkbookPath().c_str());
                        return false;
                    }

                    auto cell = iterRowCell->second;
                    switch (cell->_row)
                    {
                        case ConfigTableDefine::OWN_TYPE:
                        {
                            newConfigFieldInfo->_ownType = cell->_content;

                            // 分解ownType
                            auto ownTypes = newConfigFieldInfo->_ownType.Split(ConfigTableDefine::OWN_TYPE_SEP);
                            if(!ownTypes.empty())
                            {
                                for(auto &ownType : ownTypes)
                                {
                                    ownType.strip();

                                    // 只导出需要导出的版本
                                    auto iterConfig = _configTypeRefLangTypes.find(ownType);
                                    if(iterConfig != _configTypeRefLangTypes.end())
                                    {
                                        auto iterOwnTypeConfigs = ownTypeRefConfigs.find(ownType);
                                        if(iterOwnTypeConfigs == ownTypeRefConfigs.end())
                                            iterOwnTypeConfigs = ownTypeRefConfigs.insert(std::make_pair(ownType, std::set<XlsxConfigTableInfo *>())).first;
                                        iterOwnTypeConfigs->second.insert(configTableInfo);
                                    }
                                }
                            }
                        }
                        break;
                        case ConfigTableDefine::FIELD_NAME:
                        {
                            if(cell->_content.empty())
                            {
                                g_Log->Error(LOGFMT_OBJ_TAG("lack of field name sheet name:%s, xlsx path:%s")
                                            , sheet->GetSheetName().c_str(), workbook->GetWorkbookPath().c_str());
                                return false;
                            }

                            newConfigFieldInfo->_fieldName = cell->_content;
                        }
                        break;
                        case ConfigTableDefine::DATA_TYPE:
                        {
                            if(cell->_content.empty())
                            {
                                g_Log->Error(LOGFMT_OBJ_TAG("lack of data type sheet name:%s, xlsx path:%s")
                                            , sheet->GetSheetName().c_str(), workbook->GetWorkbookPath().c_str());
                                return false;
                            }
                            newConfigFieldInfo->_dataType = cell->_content;
                        }
                        break;
                        case ConfigTableDefine::CHECK:
                        {
                            newConfigFieldInfo->_check = cell->_content;
                        }
                        break;
                        case ConfigTableDefine::FLAGS:
                        {
                            newConfigFieldInfo->_flags = cell->_content;
                        }
                        break;
                        case ConfigTableDefine::DEFAULT_VALUE:
                        {
                            newConfigFieldInfo->_defaultValue = cell->_content;
                        }
                        break;
                        case ConfigTableDefine::DESC:
                        {
                            newConfigFieldInfo->_desc = cell->_content;
                        }
                        break;
                        default:
                        {
                            g_Log->Warn(LOGFMT_OBJ_TAG("unknown config header line row id:%llu, columnId:%llu sheetName:%s, xlsx path:%s")
                                        , cell->_row, cell->_column, sheet->GetSheetName().c_str(), sheet->GetWorkbook()->GetWorkbookPath().c_str());
                        }
                        break;
                    }
                }
            }
            
            // 解析表数据
            bool isMultiDisableLine = false;
            for(UInt64 idx = ConfigTableDefine::HEADER_ROW_NUMBER + 1; idx <= maxRowId; ++idx)
            {
                auto &rowCells = sheet->GetRowCells(idx);

                // 功能区内容
                auto iterFunc = configTableInfo->_rowIdRefFunctionBarColumn.find(idx);

                // 是否是单行注释
                if(iterFunc != configTableInfo->_rowIdRefFunctionBarColumn.end() && 
                    iterFunc->second.strip() == ConfigTableDefine::SINGLE_DISABLE_FLAG)
                {
                    continue;
                }

                // 是否是多行注释
                if((iterFunc != configTableInfo->_rowIdRefFunctionBarColumn.end()) && 
                    iterFunc->second.strip() == ConfigTableDefine::MULTI_DISABLE_FLAG)
                {
                    isMultiDisableLine = !isMultiDisableLine;
                    continue;
                }

                // 处于多行注释中
                if(isMultiDisableLine)
                    continue;

                auto iter = configTableInfo->_values.find(idx);
                if(iter == configTableInfo->_values.end())
                    iter = configTableInfo->_values.insert(std::make_pair(idx, std::unordered_map<UInt64, KERNEL_NS::LibString>())).first;

                // 行数据
                auto &columnRefValue = iter->second;
                for(auto iterCell : rowCells)
                {
                    auto cell = iterCell.second;
                    columnRefValue.insert(std::make_pair(cell->_column, cell->_content));
                }
            }

            configTables->insert(configTableInfo.pop());
        }
    }

    // TODO: ownTypeRefConfigs 导出表
    return true;
}

bool XlsxExporterMgr::_UpdateMetas()
{
    return true;
}

void XlsxExporterMgr::_Clear()
{
    _UnRegisterEvents();

    KERNEL_NS::ContainerUtil::DelContainer(_metaNameRefConfigMetaInfo, [](XlsxConfigMetaInfo *ptr){
        XlsxConfigMetaInfo::Delete_XlsxConfigMetaInfo(ptr);
    });
}

void XlsxExporterMgr::_RegisterEvents()
{

}

void XlsxExporterMgr::_UnRegisterEvents()
{

}


SERVICE_END