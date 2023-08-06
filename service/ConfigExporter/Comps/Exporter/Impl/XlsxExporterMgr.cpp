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
:_closeServiceStub(INVALID_LISTENER_STUB)
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

    _allConfigsHeader = "AllConfigs.h";
    _registerAllConfigs = "RegisterAllConfigs.hpp";

    return Status::Success;
}

void XlsxExporterMgr::_OnGlobalSysClose()
{
    _Clear();
}

Int32 XlsxExporterMgr::ExportConfigs(const std::map<KERNEL_NS::LibString, KERNEL_NS::LibString> &params)
{
    // ConfigExporter --config=xlsx --lang=S:cpp|C:C#,lua --source_dir=/xxx/ --target_dir=/xxx/ --data=/xx/ --meta=/xxx/ --base_path=xxx
    // 4.解析参数
    for(auto &iter : params)
    {
        auto &key = iter.first;
        auto &value = iter.second;

        if(key == "--lang")
        {// 生成的语言版本
            auto configTypeRefLangsArr = value.Split("@");
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

                    auto &ownType = configTypeLangPieces[0];
                    auto &langsStr = configTypeLangPieces[1];
                    ownType.strip();
                    langsStr.strip();
                    if(ownType.empty() || langsStr.empty())
                        continue;

                    auto langsPieces = langsStr.Split(",");
                    if(langsPieces.empty())
                        continue;

                    auto iter = _ownTypeRefLangTypes.find(ownType);
                    if(iter == _ownTypeRefLangTypes.end())
                        iter = _ownTypeRefLangTypes.insert(std::make_pair(ownType, std::unordered_set<KERNEL_NS::LibString>())).first;
                    auto &langsSet = iter->second; 

                    for(auto &lang : langsPieces)
                        langsSet.insert(lang.strip());    
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
        else if(key == "--base_path")
        {
            _baseDir = value;
        }
    }
    
    // 必须指定语言版本
    if(_ownTypeRefLangTypes.empty())
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

    // 分析配置
    if(!_AnalyzeExportConfigs())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("_AnalyzeExportConfigs fail sourceDir:%s, targetDir:%s, dataDir:%s metaDir:%s error."), 
                    _sourceDir.c_str(), _targetDir.c_str(), _dataDir.c_str(), _metaDir.c_str());
        return Status::Failed;
    }

    if(!_DoExportConfigs())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("_DoExportConfigs fail sourceDir:%s, targetDir:%s, dataDir:%s metaDir:%s error."), 
                    _sourceDir.c_str(), _targetDir.c_str(), _dataDir.c_str(), _metaDir.c_str());
        return Status::Failed;
    }

    // 更新metafile
    for(auto xlsxPath : _dirtyXlsxFiles)
    {
        if(!_ExportMetaFile(xlsxPath))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("_ExportMetaFile fail sourceDir:%s, targetDir:%s, dataDir:%s metaDir:%s xlsxPath:%s."), 
                        _sourceDir.c_str(), _targetDir.c_str(), _dataDir.c_str(), _metaDir.c_str(), xlsxPath.c_str());
            return Status::Failed;
        }
    }

    // 导出所有头文件
    for(auto iterOwnType : _ownTypeRefConfigTypeRefXlsxConfigTableInfo)
    {
        auto iterLang = _ownTypeRefLangTypes.find(iterOwnType.first);
        if(iterLang == _ownTypeRefLangTypes.end())
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("skip ownType:%s export."), iterOwnType.first.c_str());
            continue;
        }

        auto &langs = iterLang->second;
        for(auto &lang : langs)
        {
            const auto &lowwerLang = lang.tolower();
            if(lowwerLang == "c++" || lowwerLang == "cpp" || lowwerLang == "cxx")
            {// 导出c++代码和数据
                _ExportCppAllConfigHeaderFile(lowwerLang);
                _ExportCppRegisterConfigs(lowwerLang);
            }
        }
    }

    g_Log->Custom("export xlsx success.");
    return Status::Success;
}
   
// 测试点: 加载meta成功, meta中没有对应的xlsx文件名, 或者xlsx文件不存在则清理meta， md5为空则清理meta
// 注意:xlsx所在路径相对于xlsx的base路径的相对路径与meta文件相对于meta base 路径的相对路径是一致的
bool XlsxExporterMgr::_ScanMeta()
{
    g_Log->Custom("start scan meta files...");

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
            if(fullPath.empty() || fullPath.at(fullPath.length() - 1) != '/')
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
            KERNEL_NS::LibString srcDir = _sourceDir;
            if(srcDir.empty() || srcDir.at(srcDir.length() - 1) != '/')
                srcDir.AppendFormat("/");

            KERNEL_NS::LibString relationPath = metaInfo->_relationPath;
            if(relationPath.empty() || relationPath.at(srcDir.length() - 1) != '/')
                relationPath.AppendFormat("/");

            const auto xlsxFullPath = srcDir + relationPath + metaInfo->_xlsxFileName;
            if(!KERNEL_NS::FileUtil::IsFileExist(xlsxFullPath.c_str()))
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("xlsx file not found:%s, will remove meta file, metaFilePath:%s.")
                    , xlsxFullPath.c_str(), metaFilePath.c_str());   

                XlsxConfigMetaInfo::Delete_XlsxConfigMetaInfo(metaInfo);
                KERNEL_NS::FileUtil::DelFileCStyle(metaFilePath.c_str());
                iter = _metaNameRefConfigMetaInfo.erase(iter); 
                continue;
            }
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

    if(!isSuc)
    {
        return false;
    }

    g_Log->Custom("scan meta files success...");
    return true;
}

bool XlsxExporterMgr::_ScanXlsx()
{
    g_Log->Custom("start scan xlsx files source dir:%s...", _sourceDir.c_str());

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
            if(fullPath.empty() || fullPath.at(fullPath.length() - 1) != '/')
                fullPath.AppendFormat("/");

            const auto &fullFilePath = fullPath + fileInfo._fileName;
            
            const auto fileNameWithoutExtention = KERNEL_NS::FileUtil::ExtractFileWithoutExtension(fileInfo._fileName);

            const auto &relationDir = fullPath - _sourceDir;
            const auto metaFilePath = _metaDir + relationDir + fileNameWithoutExtention + ".meta";

            KERNEL_NS::SmartPtr<KERNEL_NS::XlsxWorkbook, KERNEL_NS::AutoDelMethods::CustomDelete> xlsxBook = KERNEL_NS::XlsxWorkbook::NewThreadLocal_XlsxWorkbook(true);
            xlsxBook.SetClosureDelegate([](void *p){
                auto ptr = reinterpret_cast<KERNEL_NS::XlsxWorkbook *>(p);
                KERNEL_NS::XlsxWorkbook::DeleteThreadLocal_XlsxWorkbook(ptr);
            });

            if(!xlsxBook->Parse(fullFilePath))
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("parse xlsx fail:%s"), fullFilePath.c_str());
                isParentDirContinue = true;
                isContinue = true;
                // isSuc = false;
                break;
            }

            auto &allSheet = xlsxBook->GetAllSheets();
            if(allSheet.empty())
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("have no any sheet:xlsx path:%s "), fullFilePath.c_str());
                break;
            }
                
            bool checkSuc = true;
            for(auto iter : allSheet)
            {
                auto sheet = iter.second;
                auto &sheetName = sheet->GetSheetName();
                const auto &configTypeName = _GetConfigTypeName(sheetName);
                if(!KERNEL_NS::StringUtil::CheckGeneralName(configTypeName))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("bad sheet config type: xlsx file path:%s, sheet name:%s, config type name:%s"), fullFilePath.c_str(), sheetName.c_str(), configTypeName.c_str());
                    isContinue = false;
                    isParentDirContinue = false;
                    checkSuc = false;
                    isSuc = false;
                    break;
                }

                // 首字母必须是英文且需要大写
                if(configTypeName[0] < 'A' || configTypeName[0] > 'Z')
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("bad first word:%c, must be upper. bad sheet config type: xlsx file path:%s, sheet name:%s, config type name:%s"), configTypeName[0], fullFilePath.c_str(), sheetName.c_str(), configTypeName.c_str());
                    isContinue = false;
                    isParentDirContinue = false;
                    checkSuc = false;
                    isSuc = false;
                    break;
                }

                auto iterSheet = _configTypeRefSheets.find(configTypeName);
                if(iterSheet == _configTypeRefSheets.end())
                    iterSheet = _configTypeRefSheets.insert(std::make_pair(configTypeName, std::set<KERNEL_NS::XlsxSheet *>())).first;

                iterSheet->second.insert(sheet);
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
            for(auto iter = allSheet.begin(); iter != allSheet.end(); ++iter)
            {
                auto sheet = iter->second;
                auto &sheetName = sheet->GetSheetName();
                const auto &configTypeName = _GetConfigTypeName(sheetName);
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

    g_Log->Custom("scan xlsx files success, xlsx file number:%llu, dirty file number:%llu, config type number:%llu, dirty config type number:%llu."
                , static_cast<UInt64>(_xlsxFileRefWorkbook.size()), static_cast<UInt64>(_dirtyXlsxFiles.size())
                , static_cast<UInt64>(_configTypeRefSheets.size()),  static_cast<UInt64>(_needExportConfigType.size()));

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
    KERNEL_NS::LibString md5;
    if(!_CalcMd5(xlsxFile, md5))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("calc md5 fail file:%s"), xlsxFile.c_str());
        return false;
    }

    // 3.若xlsxfile的md5和metafile的不同则说明需要导出
    return md5 != meta->_lastMd5;
}

bool XlsxExporterMgr::_CalcMd5(const KERNEL_NS::LibString &file, KERNEL_NS::LibString &md5) const
{
    // 2.生成xlsxfile的md5
    if(!KERNEL_NS::FileUtil::IsFileExist(file.c_str()))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("file not exists file:%s"), file.c_str());
        return false;
    }

    KERNEL_NS::LibString content;
    if(!KERNEL_NS::LibDigest::MakeFileMd5(file, content))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("MakeFileMd5 fail file:%s"), file.c_str());
        return false;
    }

    md5 = KERNEL_NS::LibBase64::Encode(content);

    return true;
}

bool XlsxExporterMgr::_IsOwnTypeNeedExport(const KERNEL_NS::LibString &ownType) const
{
    if(_ownTypeRefLangTypes.empty())
        return false;

    if(ownType.empty())
        return false;

    auto parts = ownType.Split(ConfigTableDefine::OWN_TYPE_SEP);
    if(parts.empty())
        return false;

    bool needExport = false;
    for(auto ownType : parts)
    {
        ownType.strip();
        auto iter = _ownTypeRefLangTypes.find(ownType);
        if(iter != _ownTypeRefLangTypes.end())
        {
            needExport = true;
            break;
        }
    }

    return needExport;
}

bool XlsxExporterMgr::_AnalyzeExportConfigs()
{
    g_Log->Custom("start analyze export configs...");

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

    g_Log->Custom("prepare xlsx config structure and datas...");

    if(!_PrepareConfigStructAndDatas(ownTypeRefConfigs, configTables.AsSelf()))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("_PrepareConfigStructAndDatas fail."));
        return false;
    }

    // 将configTables 按照ownType分类导出配置结构与数据
    g_Log->Custom("sort config tables with own type, then merge structure and datas...");
    for(auto &iterConfigTables : ownTypeRefConfigs)
    {
        const auto ownType = iterConfigTables.first;
        for(auto configTable : iterConfigTables.second)
        {
            auto iterConfigRefConfigTables = _ownTypeRefConfigTypeRefXlsxConfigTableInfo.find(ownType);
            if(iterConfigRefConfigTables == _ownTypeRefConfigTypeRefXlsxConfigTableInfo.end())
                iterConfigRefConfigTables = _ownTypeRefConfigTypeRefXlsxConfigTableInfo.insert(std::make_pair(ownType, std::map<KERNEL_NS::LibString, XlsxConfigTableInfo *>())).first;

            auto &configTypeRefConfigTables = iterConfigRefConfigTables->second;
            auto iterConfigTable = configTypeRefConfigTables.find(configTable->_tableClassName);
            XlsxConfigTableInfo *newConfigTable = NULL;
            if(iterConfigTable == configTypeRefConfigTables.end())
            {// 初次建立表数据
                // 解析所有属于ownType的字段
                newConfigTable = XlsxConfigTableInfo::New_XlsxConfigTableInfo();
                newConfigTable->_wholeSheetName = configTable->_wholeSheetName;
                newConfigTable->_tableClassName = configTable->_tableClassName;
                newConfigTable->_xlsxPath = configTable->_xlsxPath;
                newConfigTable->_rowIdRefFunctionBarColumn = configTable->_rowIdRefFunctionBarColumn;
                newConfigTable->_values = configTable->_values;
                configTypeRefConfigTables.insert(std::make_pair(configTable->_tableClassName, newConfigTable));

                // 定义移除列
                auto removeColumnFunc = [&newConfigTable](UInt64 columnId)
                {
                    for(auto iter = newConfigTable->_values.begin(); iter != newConfigTable->_values.end(); ++iter)
                        iter->erase(columnId);
                };

                // 生成只有ownType的字段
                if(!configTable->_fieldInfos.empty())
                    newConfigTable->_fieldInfos.resize(configTable->_fieldInfos.size());

                for(auto filedInfo : configTable->_fieldInfos)
                {
                    if(!filedInfo)
                    {
                        continue;
                    }

                    if(filedInfo->_ownType.empty())
                    {
                        continue;
                    }

                    auto ownTypes = filedInfo->_ownType.Split(ConfigTableDefine::OWN_TYPE_SEP);
                    if(ownTypes.empty())
                    {
                        removeColumnFunc(filedInfo->_columnId);
                        continue;
                    }

                    // 是不是ownType的字段
                    auto iterOwn = std::find_if(ownTypes.begin(), ownTypes.end(), [&ownType](const KERNEL_NS::LibString &item){
                        return item == ownType;
                    });

                    if(iterOwn == ownTypes.end())
                    {
                        removeColumnFunc(filedInfo->_columnId);
                        continue;
                    }

                    auto newFieldInfo = XlsxConfigFieldInfo::New_XlsxConfigFieldInfo(*filedInfo);
                    newFieldInfo->_owner = newConfigTable;
                    newConfigTable->_fieldInfos[newFieldInfo->_columnId] = newFieldInfo;
                    newConfigTable->_fieldNames.insert(newFieldInfo->_fieldName);
                }
            }
            else
            {// 已经存在config 说明需要合并(多文件，或者多页签,配表)
                // 需要先保证表头的一致性
                newConfigTable = iterConfigTable->second;
                KERNEL_NS::LibString errInfo;
                if(!newConfigTable->CheckHeaderSame(configTable, errInfo))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("config table:%s header not the same errInfo:%s")
                                , newConfigTable->_tableClassName.c_str(), errInfo.c_str());
                    return false;
                }

                newConfigTable->_xlsxPath.AppendFormat("\n%s", configTable->_xlsxPath.c_str());
                newConfigTable->_wholeSheetName.AppendFormat(";%s", configTable->_wholeSheetName.c_str());

                // 合并两表数据
                for(auto &rowValues : configTable->_values)
                {
                    bool isInsertLineFirst = true;
                    for(auto &iter : rowValues)
                    {
                        const auto otherColumnId = iter.first;
                        auto otherFiledInfo = configTable->_fieldInfos[otherColumnId];
                        if(!otherFiledInfo)
                        {
                            continue;
                        }

                        auto ownTypes = otherFiledInfo->_ownType.Split(ConfigTableDefine::OWN_TYPE_SEP);
                        if(ownTypes.empty())
                        {
                            continue;
                        }

                        // 是不是ownType的字段
                        auto iterOwn = std::find_if(ownTypes.begin(), ownTypes.end(), [&ownType](const KERNEL_NS::LibString &item){
                            return item == ownType;
                        });

                        if(iterOwn == ownTypes.end())
                        {
                            continue;
                        }

                        if(isInsertLineFirst)
                        {
                            newConfigTable->_values.push_back(std::map<UInt64, KERNEL_NS::LibString>());
                            isInsertLineFirst = false;
                        }

                        auto &configTableLine = newConfigTable->_values.back();
                        configTableLine.insert(std::make_pair(otherColumnId, iter.second));
                    }
                }
            }
        }
    }

    // 移除没有任何字段的配置
    for(auto iterConfigTypeRefConfigTable = _ownTypeRefConfigTypeRefXlsxConfigTableInfo.begin(); 
        iterConfigTypeRefConfigTable != _ownTypeRefConfigTypeRefXlsxConfigTableInfo.end();)
    {
        auto &configTypeRefConfigTable = iterConfigTypeRefConfigTable->second;
        for(auto iterConfigTable = configTypeRefConfigTable.begin(); iterConfigTable != configTypeRefConfigTable.end();)
        {
            auto configTable = iterConfigTable->second;
            if(configTable->_fieldNames.empty())
            {
                g_Log->Custom("xlsx path:%s config type:%s have no any field of own type:%s, will not be exported."
                            , configTable->_xlsxPath.c_str(), iterConfigTable->first.c_str(), iterConfigTypeRefConfigTable->first.c_str());

                XlsxConfigTableInfo::Delete_XlsxConfigTableInfo(configTable);
                iterConfigTable = configTypeRefConfigTable.erase(iterConfigTable);
            }
            else
            {
                ++iterConfigTable;
            }
        }

        if(configTypeRefConfigTable.empty())
        {
            iterConfigTypeRefConfigTable = _ownTypeRefConfigTypeRefXlsxConfigTableInfo.erase(iterConfigTypeRefConfigTable);
        }
        else
        {
            ++iterConfigTypeRefConfigTable;
        }
    }

    g_Log->Custom("analyze export configs success.");

    return true;
}

bool XlsxExporterMgr::_PrepareConfigStructAndDatas(std::unordered_map<KERNEL_NS::LibString, std::set<XlsxConfigTableInfo *>> &ownTypeRefConfigs, std::set<XlsxConfigTableInfo *> *configTables) const
{
    // 生成XlsxConfigTableInfo
    for(auto &configType : _needExportConfigType)
    {
        auto iterSheets = _configTypeRefSheets.find(configType);
        auto &sheets = iterSheets->second;
        for(auto &sheet : sheets)
        {
            if(sheet->GetTotalLine() < ConfigTableDefine::HEADER_ROW_NUMBER)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("have no enough line, total line:%llu, need line:%llu, and will skip exporting config:%s, xlsx path:%s")
                        ,sheet->GetTotalLine(), ConfigTableDefine::HEADER_ROW_NUMBER
                        , sheet->GetSheetName().c_str(), sheet->GetWorkbook()->GetWorkbookPath().c_str());
                continue;
            }

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
            configTableInfo->_xlsxPath = workbook->GetWorkbookPath();

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
            configTableInfo->_fieldInfos.resize(maxColumnId + 1);
            for(UInt64 idx = KERNEL_NS::XlsxCell::COLUMN_BEGIN + 1; idx <= maxColumnId; ++idx)
            {
                auto newConfigFieldInfo = XlsxConfigFieldInfo::New_XlsxConfigFieldInfo(configTableInfo);
                newConfigFieldInfo->_columnId = idx;
                configTableInfo->_fieldInfos[idx] = newConfigFieldInfo;

                auto &fieldsColumnCells = sheet->GetColumnCells(idx);
                if(fieldsColumnCells.empty())
                    continue;

                // 校验字段是否冲突
                {
                    const UInt64 fieldName = ConfigTableDefine::FIELD_NAME;
                    auto iterField = fieldsColumnCells.find(fieldName);
                    if(iterField == fieldsColumnCells.end())
                    {
                        g_Log->Error(LOGFMT_OBJ_TAG("have no field name sheet name:%s, xlsx path:%s")
                        , sheet->GetSheetName().c_str(), workbook->GetWorkbookPath().c_str());
                        return false;
                    }

                    if(configTableInfo->_fieldNames.find(iterField->second->_content) != configTableInfo->_fieldNames.end())
                    {
                        g_Log->Error(LOGFMT_OBJ_TAG("field name:%s duplicate column:%llu, row:%llu sheet name:%s, xlsx path:%s")
                        , iterField->second->_content.c_str(), iterField->second->_column, iterField->second->_row, sheet->GetSheetName().c_str()
                        , workbook->GetWorkbookPath().c_str());
                        return false;
                    }

                    configTableInfo->_fieldNames.insert(iterField->second->_content);
                }

                // 表头字段解析
                for(UInt64 rowId = KERNEL_NS::XlsxCell::ROW_BEGIN; rowId <= ConfigTableDefine::HEADER_ROW_NUMBER; ++rowId)
                {
                    switch (rowId)
                    {
                        case ConfigTableDefine::OWN_TYPE:
                        {
                            auto iterRowCell = fieldsColumnCells.find(rowId);
                            if(iterRowCell == fieldsColumnCells.end())
                            {// 可以缺省
                                // g_Log->Warn(LOGFMT_OBJ_TAG("have no row cell rowId:%llu, column:%llu, sheet name:%s, xlsx path:%s")
                                //                 , rowId, idx, sheet->GetSheetName().c_str(), workbook->GetWorkbookPath().c_str());
                                continue;
                            }

                            auto cell = iterRowCell->second;

                            newConfigFieldInfo->_ownType = cell->_content;
                            newConfigFieldInfo->_ownType.strip();
                            newConfigFieldInfo->_ownType.EraseAnyOf("\t\v\r\n\f");

                            // 分解ownType
                            auto ownTypes = newConfigFieldInfo->_ownType.Split(ConfigTableDefine::OWN_TYPE_SEP);
                            if(!ownTypes.empty())
                            {
                                for(auto &ownType : ownTypes)
                                {
                                    ownType.strip();

                                    // 只导出需要导出的版本
                                    auto iterConfig = _ownTypeRefLangTypes.find(ownType);
                                    if(iterConfig != _ownTypeRefLangTypes.end())
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
                            auto iterRowCell = fieldsColumnCells.find(rowId);
                            if(iterRowCell == fieldsColumnCells.end())
                            {// 不可缺省
                                g_Log->Warn(LOGFMT_OBJ_TAG("FIELD_NAME have no row cell rowId:%llu, column:%llu, sheet name:%s, xlsx path:%s")
                                                , rowId, idx, sheet->GetSheetName().c_str(), workbook->GetWorkbookPath().c_str());
                                return false;
                            }

                            auto cell = iterRowCell->second;
                            if(cell->_content.empty())
                            {
                                g_Log->Error(LOGFMT_OBJ_TAG("lack of field name sheet name:%s, xlsx path:%s")
                                            , sheet->GetSheetName().c_str(), workbook->GetWorkbookPath().c_str());
                                return false;
                            }

                            newConfigFieldInfo->_fieldName = cell->_content;
                            newConfigFieldInfo->_fieldName.strip();

                            if(!KERNEL_NS::StringUtil::CheckGeneralName(newConfigFieldInfo->_fieldName))
                            {
                                g_Log->Error(LOGFMT_OBJ_TAG("illigal field name:%s sheet name:%s, xlsx path:%s")
                                            ,newConfigFieldInfo->_fieldName.c_str(), sheet->GetSheetName().c_str(), workbook->GetWorkbookPath().c_str());
                                return false;
                            }

                        }
                        break;
                        case ConfigTableDefine::DATA_TYPE:
                        {
                            auto iterRowCell = fieldsColumnCells.find(rowId);
                            if(iterRowCell == fieldsColumnCells.end())
                            {// 不可缺省
                                g_Log->Warn(LOGFMT_OBJ_TAG("DATA_TYPE have no row cell rowId:%llu, column:%llu, sheet name:%s, xlsx path:%s")
                                                , rowId, idx, sheet->GetSheetName().c_str(), workbook->GetWorkbookPath().c_str());
                                return false;
                            }

                            auto cell = iterRowCell->second;
                            if(cell->_content.empty())
                            {
                                g_Log->Error(LOGFMT_OBJ_TAG("lack of data type sheet name:%s, xlsx path:%s")
                                            , sheet->GetSheetName().c_str(), workbook->GetWorkbookPath().c_str());
                                return false;
                            }
                            newConfigFieldInfo->_dataType = cell->_content;
                            newConfigFieldInfo->_dataType.strip();
                            newConfigFieldInfo->_dataType = newConfigFieldInfo->_dataType.tolower();
                            newConfigFieldInfo->_dataType.EraseAnyOf("\t\v\r\n\f");
                        }
                        break;
                        case ConfigTableDefine::CHECK:
                        {
                            auto iterRowCell = fieldsColumnCells.find(rowId);
                            if(iterRowCell == fieldsColumnCells.end())
                            {// 可以缺省
                                // g_Log->Warn(LOGFMT_OBJ_TAG("DATA_TYPE have no row cell rowId:%llu, column:%llu, sheet name:%s, xlsx path:%s")
                                //                 , rowId, idx, sheet->GetSheetName().c_str(), workbook->GetWorkbookPath().c_str());
                                continue;
                            }

                            auto cell = iterRowCell->second;
                            newConfigFieldInfo->_check = cell->_content;
                            newConfigFieldInfo->_check.strip();
                        }
                        break;
                        case ConfigTableDefine::FLAGS:
                        {
                            auto iterRowCell = fieldsColumnCells.find(rowId);
                            if(iterRowCell == fieldsColumnCells.end())
                            {// 可以缺省
                                // g_Log->Warn(LOGFMT_OBJ_TAG("DATA_TYPE have no row cell rowId:%llu, column:%llu, sheet name:%s, xlsx path:%s")
                                //                 , rowId, idx, sheet->GetSheetName().c_str(), workbook->GetWorkbookPath().c_str());
                                continue;
                            }

                            auto cell = iterRowCell->second;
                            newConfigFieldInfo->_flags = cell->_content;
                            newConfigFieldInfo->_flags.strip();
                        }
                        break;
                        case ConfigTableDefine::DEFAULT_VALUE:
                        {
                            auto iterRowCell = fieldsColumnCells.find(rowId);
                            if(iterRowCell == fieldsColumnCells.end())
                            {// 可以缺省
                                // g_Log->Warn(LOGFMT_OBJ_TAG("DATA_TYPE have no row cell rowId:%llu, column:%llu, sheet name:%s, xlsx path:%s")
                                //                 , rowId, idx, sheet->GetSheetName().c_str(), workbook->GetWorkbookPath().c_str());
                                continue;
                            }

                            auto cell = iterRowCell->second;
                            newConfigFieldInfo->_defaultValue = cell->_content;
                            newConfigFieldInfo->_defaultValue.strip();
                            newConfigFieldInfo->_defaultValue.EraseAnyOf("\t\v\r\n\f");
                        }
                        break;
                        case ConfigTableDefine::DESC:
                        {// 注释需要移除换行等
                            auto iterRowCell = fieldsColumnCells.find(rowId);
                            if(iterRowCell == fieldsColumnCells.end())
                            {// 可以缺省
                                // g_Log->Warn(LOGFMT_OBJ_TAG("DATA_TYPE have no row cell rowId:%llu, column:%llu, sheet name:%s, xlsx path:%s")
                                //                 , rowId, idx, sheet->GetSheetName().c_str(), workbook->GetWorkbookPath().c_str());
                                continue;
                            }

                            auto cell = iterRowCell->second;
                            newConfigFieldInfo->_desc = cell->_content;
                            newConfigFieldInfo->_desc.strip();
                            newConfigFieldInfo->_desc.EraseAnyOf("\t\v\r\n\f");
                        }
                        break;
                        default:
                        {
                            KERNEL_NS::XlsxCell *cell = NULL;
                            auto iterRowCell = fieldsColumnCells.find(rowId);
                            if(iterRowCell != fieldsColumnCells.end())
                            {// 可以缺省
                                // g_Log->Warn(LOGFMT_OBJ_TAG("DATA_TYPE have no row cell rowId:%llu, column:%llu, sheet name:%s, xlsx path:%s")
                                //                 , rowId, idx, sheet->GetSheetName().c_str(), workbook->GetWorkbookPath().c_str());
                                cell = iterRowCell->second;
                            }

                            g_Log->Warn(LOGFMT_OBJ_TAG("unknown config header line row id:%llu, columnId:%llu content:%s sheetName:%s, xlsx path:%s")
                                        , rowId, idx, (cell ? cell->_content.c_str() : ""), sheet->GetSheetName().c_str(), sheet->GetWorkbook()->GetWorkbookPath().c_str());
                            return false;
                        }
                        break;
                    }
                }
            }

            // 字典/数组默认值遵守json格式
            for(auto fieldInfo : configTableInfo->_fieldInfos)
            {
                if(!fieldInfo)
                    continue;

                if(SERVICE_COMMON_NS::DataTypeHelper::IsArray(fieldInfo->_dataType))
                {
                    if(fieldInfo->_defaultValue.empty())
                    {
                        fieldInfo->_defaultValue = "[]";
                        continue;
                    }

                    const auto jsonArray = nlohmann::json::parse(fieldInfo->_defaultValue.c_str(), NULL, false);
                    if(!jsonArray.is_array())
                    {
                        g_Log->Warn(LOGFMT_OBJ_TAG("bad default value, default value must be json array, please check field name:%s columnId:%llu default value:%s sheetName:%s, xlsx path:%s")
                                        , fieldInfo->_fieldName.c_str(), fieldInfo->_columnId, fieldInfo->_defaultValue.c_str(),  sheet->GetSheetName().c_str(), sheet->GetWorkbook()->GetWorkbookPath().c_str());
                        return false;
                    }

                    continue;
                }

                if(SERVICE_COMMON_NS::DataTypeHelper::IsDict(fieldInfo->_dataType))
                {
                    if(fieldInfo->_defaultValue.empty())
                    {
                        fieldInfo->_defaultValue = "{}";
                        continue;
                    }

                    const auto jsonObject = nlohmann::json::parse(fieldInfo->_defaultValue.c_str(), NULL, false);
                    if(!jsonObject.is_object())
                    {
                        g_Log->Warn(LOGFMT_OBJ_TAG("bad default value, default value must be json object, please check field name:%s columnId:%llu default value:%s sheetName:%s, xlsx path:%s")
                                        , fieldInfo->_fieldName.c_str(), fieldInfo->_columnId, fieldInfo->_defaultValue.c_str(),  sheet->GetSheetName().c_str(), sheet->GetWorkbook()->GetWorkbookPath().c_str());
                        return false;
                    }

                    continue;
                }

                if(SERVICE_COMMON_NS::DataTypeHelper::IsBool(fieldInfo->_dataType))
                {
                    if(fieldInfo->_defaultValue.empty())
                    {
                        fieldInfo->_defaultValue = SERVICE_COMMON_NS::DataTypeHelper::GetTypeDefaultValue(fieldInfo->_dataType);
                        continue;
                    }

                    if(fieldInfo->_defaultValue.isdigit())
                    {
                        fieldInfo->_defaultValue = KERNEL_NS::StringUtil::StringToInt32(fieldInfo->_defaultValue.c_str()) != 0 ? "true" : "false";
                        continue;
                    }

                    // 转小写
                    if(fieldInfo->_defaultValue == "TRUE" || fieldInfo->_defaultValue == "FALSE")
                        fieldInfo->_defaultValue = fieldInfo->_defaultValue.tolower();

                    if((fieldInfo->_defaultValue != "true") && (fieldInfo->_defaultValue != "false"))
                    {
                        g_Log->Warn(LOGFMT_OBJ_TAG("bad default value, default value must be true or false, please check field name:%s columnId:%llu default value:%s sheetName:%s, xlsx path:%s")
                                        , fieldInfo->_fieldName.c_str(), fieldInfo->_columnId, fieldInfo->_defaultValue.c_str(),  sheet->GetSheetName().c_str(), sheet->GetWorkbook()->GetWorkbookPath().c_str());
                        return false;
                    }

                    continue;
                }

                if(SERVICE_COMMON_NS::DataTypeHelper::IsString(fieldInfo->_dataType))
                {
                    if(fieldInfo->_defaultValue.empty())
                    {
                        fieldInfo->_defaultValue = "\"\"";
                        continue;
                    }
                }

                // 其他数据类型的默认值
                if(fieldInfo->_defaultValue.empty())
                    fieldInfo->_defaultValue = SERVICE_COMMON_NS::DataTypeHelper::GetTypeDefaultValue(fieldInfo->_dataType);
            }
            
            // 解析表数据并校验数据合法性
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

                configTableInfo->_values.push_back(std::map<UInt64, KERNEL_NS::LibString>());
                auto &finalLine = configTableInfo->_values.back();

                // 行数据
                for(UInt64 scanColumn = KERNEL_NS::XlsxCell::COLUMN_BEGIN + 1; scanColumn <= maxColumnId; ++scanColumn)
                {
                    auto fieldInfo = configTableInfo->_fieldInfos[scanColumn];
                    if(!fieldInfo)
                    {
                        continue;
                    }

                    if(!_IsOwnTypeNeedExport(fieldInfo->_ownType))
                    {
                        continue;
                    }

                    auto iterCell = rowCells.find(scanColumn);
                    if(iterCell == rowCells.end())
                    {// 使用默认值
                        KERNEL_NS::LibString errInfo;
                        if(!SERVICE_COMMON_NS::DataTypeHelper::CheckData(fieldInfo->_dataType, fieldInfo->_defaultValue, errInfo))
                        {
                            g_Log->Error(LOGFMT_OBJ_TAG("check data fail columnId:%llu, field name:%s config:%s:data type:%s, defaultValue:%s, errInfo:%s, sheet name:%s, path:%s, index(maybe not row id in sometimes):%llu, row data:\n")
                                        , fieldInfo->_columnId, fieldInfo->_fieldName.c_str(), fieldInfo->_owner->_tableClassName.c_str()
                                        ,  fieldInfo->_dataType.c_str(), fieldInfo->_defaultValue.c_str(), errInfo.c_str()
                                        , fieldInfo->_owner->_wholeSheetName.c_str(), fieldInfo->_owner->_xlsxPath.c_str(), idx);

                            // TODO:转义? 需要让日志支持直接打出不需要转义
                            // sheet->ToRowString(idx).escape()
                            const auto &lineData = sheet->ToRowString(idx);
                            g_Log->Error2(LOGFMT_OBJ_TAG_NO_FMT(), lineData);

                            return false;
                        }

                        finalLine.insert(std::make_pair(scanColumn, fieldInfo->_defaultValue));
                    }
                    else
                    {
                        if(iterCell->second->_content.empty())
                        {// 默认值
                            // 校验值
                            KERNEL_NS::LibString errInfo;
                            if(!SERVICE_COMMON_NS::DataTypeHelper::CheckData(fieldInfo->_dataType, fieldInfo->_defaultValue, errInfo))
                            {
                                g_Log->Error(LOGFMT_OBJ_TAG("check data fail columnId:%llu, field name:%s config:%s:data type:%s, defaultValue:%s, errInfo:%s, sheet name:%s, path:%s")
                                            , fieldInfo->_columnId, fieldInfo->_fieldName.c_str(), fieldInfo->_owner->_tableClassName.c_str()
                                            ,  fieldInfo->_dataType.c_str(), fieldInfo->_defaultValue.c_str(), errInfo.c_str()
                                            , fieldInfo->_owner->_wholeSheetName.c_str(), fieldInfo->_owner->_xlsxPath.c_str());

                                return false;
                            }


                            finalLine.insert(std::make_pair(scanColumn, fieldInfo->_defaultValue));
                        }
                        else
                        {
                            // 校验值
                            KERNEL_NS::LibString errInfo;
                            if(!SERVICE_COMMON_NS::DataTypeHelper::CheckData(fieldInfo->_dataType, iterCell->second->_content, errInfo))
                            {
                                g_Log->Error(LOGFMT_OBJ_TAG("check data fail columnId:%llu, field name:%s config:%s:data type:%s, content:%s, errInfo:%s, sheet name:%s, path:%s, index:%llu, line data:")
                                            , fieldInfo->_columnId, fieldInfo->_fieldName.c_str(), fieldInfo->_owner->_tableClassName.c_str()
                                            ,  fieldInfo->_dataType.c_str(), iterCell->second->_content.c_str(), errInfo.c_str()
                                            , fieldInfo->_owner->_wholeSheetName.c_str(), fieldInfo->_owner->_xlsxPath.c_str(), idx);

                                g_Log->Error2(LOGFMT_OBJ_TAG_NO_FMT(), sheet->ToRowString(idx));

                                return false;
                            }

                            // bool类型需要转成小写且如果是数值的需要转成bool
                            if(SERVICE_COMMON_NS::DataTypeHelper::IsBool(fieldInfo->_dataType))
                            {
                                if(iterCell->second->_content.isdigit())
                                {
                                    iterCell->second->_content = KERNEL_NS::StringUtil::StringToInt64(iterCell->second->_content.GetRaw().substr(0, 20).c_str()) != 0 ? "true" : "false";
                                }
                                else
                                {
                                    iterCell->second->_content = iterCell->second->_content.tolower();
                                }
                            }

                            finalLine.insert(std::make_pair(scanColumn, iterCell->second->_content));
                        }
                    }
                }
            }

            // 过滤空表 第一列是特殊功能区
            if(maxColumnId >= KERNEL_NS::XlsxCell::COLUMN_BEGIN + 1)
                configTables->insert(configTableInfo.pop());
        }
    }

    return true;
}

bool XlsxExporterMgr::_DoExportConfigs() const
{
    // 要么全部生成,要么都不生成没有生成一半的, 所以在导出的代码结构和数据需要先缓存在内存或者先生成出临时文件，成功后再转成正式文件
    for(auto iterOwnType : _ownTypeRefConfigTypeRefXlsxConfigTableInfo)
    {
        auto &configTypeRefConfigTable = iterOwnType.second;
        auto iterLang = _ownTypeRefLangTypes.find(iterOwnType.first);
        if(iterLang == _ownTypeRefLangTypes.end())
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("skip ownType:%s export."), iterOwnType.first.c_str());
            continue;
        }

        auto &langs = iterLang->second;
        for(auto &lang : langs)
        {
            const auto &lowwerLang = lang.tolower();
            if(lowwerLang == "c++" || lowwerLang == "cpp" || lowwerLang == "cxx")
            {// 导出c++代码和数据
                if(!_ExportCpp(lowwerLang, configTypeRefConfigTable))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("export cpp code fail."));
                    return false;
                }
            }
            else if(lowwerLang == "csharp" || lowwerLang == "c#")
            {// 导出csharp代码和数据
                if(!_ExportCSharpCode(configTypeRefConfigTable))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("export csharp code fail."));
                    return false;
                }

                if(!_ExportCSharpDatas(configTypeRefConfigTable))
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("export csharp data fail."));
                    return false;
                }
            }
            else
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("unknown language type export:%s"), lang.c_str());
            }
        }
    }

    return true;
}

bool XlsxExporterMgr::_ExportCpp(const KERNEL_NS::LibString &langName, const std::map<KERNEL_NS::LibString, XlsxConfigTableInfo *> &configTypeRefConfigTableInfo) const
{
    for(auto iter : configTypeRefConfigTableInfo)
    {
        // 导出头文件内容
        KERNEL_NS::LibString shortPath;
        auto path = iter.second->_xlsxPath;
        const auto pathParts = path.Split("\n");
        for(auto &part : pathParts)
            shortPath.AppendFormat("%s,", KERNEL_NS::DirectoryUtil::GetFileNameInPath(part.c_str()).c_str());
        g_Log->Custom("[GeneratorConfig]:[%sConfig] - %s", iter.second->_tableClassName.c_str(), shortPath.c_str());

        KERNEL_NS::LibString headerContent;
        if(!_ExportCppCodeHeader(iter.second, headerContent))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("export cpp header fail config class name:%s."), iter.second->_tableClassName.c_str());
            return false;
        }

        // 导出实现内容
        KERNEL_NS::LibString implementContent;
        auto configTableInfo = iter.second;
        if(!_ExportCppCodeImpl(configTableInfo, implementContent))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("export cpp implement fail config class name:%s."), iter.second->_tableClassName.c_str());
            return false;
        }

        // 导出数据
        KERNEL_NS::LibString configDataContent;
        if(!_ExportCppDatas(configTableInfo, configDataContent))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("export cpp data fail config class name:%s."), iter.second->_tableClassName.c_str());
            return false;
        }

        const auto className = iter.second->_tableClassName + "Config";

        const auto headerFileName = _targetDir + "/" + langName + "/" + className + ".h";
        if(KERNEL_NS::FileUtil::IsFileExist(headerFileName.c_str()))
            KERNEL_NS::FileUtil::DelFileCStyle(headerFileName.c_str());
        {// 头文件创建路径
            const auto dir =  KERNEL_NS::DirectoryUtil::GetFileDirInPath(headerFileName);
            if(dir.empty())
            {
                g_Log->Error(LOGFMT_OBJ_TAG("bad file name:%s"), headerFileName.c_str());
                return false;
            }
            if(!KERNEL_NS::DirectoryUtil::CreateDir(dir))
            {
                return false;
            }
        }

        const auto cppFileName = _targetDir + "/" + langName + "/" + className + ".cpp";
        if(KERNEL_NS::FileUtil::IsFileExist(cppFileName.c_str()))
            KERNEL_NS::FileUtil::DelFileCStyle(cppFileName.c_str());
        {// 实现创建路径
            const auto dir =  KERNEL_NS::DirectoryUtil::GetFileDirInPath(cppFileName);
            if(dir.empty())
            {
                g_Log->Error(LOGFMT_OBJ_TAG("bad file name:%s"), cppFileName.c_str());
                return false;
            }
            if(!KERNEL_NS::DirectoryUtil::CreateDir(dir))
            {
                return false;
            }
        }

        const auto dataFileName = _dataDir + "/" + langName + "/" + className + ".data";
        if(KERNEL_NS::FileUtil::IsFileExist(dataFileName.c_str()))
            KERNEL_NS::FileUtil::DelFileCStyle(dataFileName.c_str());
        {// 实现创建路径
            const auto dir =  KERNEL_NS::DirectoryUtil::GetFileDirInPath(dataFileName);
            if(dir.empty())
            {
                g_Log->Error(LOGFMT_OBJ_TAG("bad file name:%s"), dataFileName.c_str());
                return false;
            }
            if(!KERNEL_NS::DirectoryUtil::CreateDir(dir))
            {
                return false;
            }
        }

        {// 写入头文件内容
            // 创建文件
            KERNEL_NS::SmartPtr<FILE, KERNEL_NS::AutoDelMethods::CustomDelete> fp =  KERNEL_NS::FileUtil::OpenFile(headerFileName.c_str(), true);
            if(!fp)
            {
                g_Log->Error(LOGFMT_OBJ_TAG("open header file fail when export cpp config class name:%s fileName:%s.")
                            , iter.second->_tableClassName.c_str(), headerFileName.c_str());
                return false;
            }

            fp.SetClosureDelegate([](void *p){
                auto ptr = reinterpret_cast<FILE *>(p);
                KERNEL_NS::FileUtil::CloseFile(*ptr);
            });

            auto bytesChange = KERNEL_NS::FileUtil::WriteFile(*fp, headerContent);
            if(bytesChange != static_cast<Int64>(headerContent.size()))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("write header file bytesChange:%lld, headerContent size:%llu,  when export cpp config class name:%s fileName:%s.")
                            , bytesChange, static_cast<Int64>(headerContent.size()), iter.second->_tableClassName.c_str(), headerFileName.c_str());
                return false;
            }
        }

        {// 写入实现内容
            KERNEL_NS::SmartPtr<FILE, KERNEL_NS::AutoDelMethods::CustomDelete> fp =  KERNEL_NS::FileUtil::OpenFile(cppFileName.c_str(), true);
            if(!fp)
            {
                g_Log->Error(LOGFMT_OBJ_TAG("open implement file fail when export cpp config class name:%s fileName:%s.")
                            , iter.second->_tableClassName.c_str(), cppFileName.c_str());
                return false;
            }

            fp.SetClosureDelegate([](void *p){
                auto ptr = reinterpret_cast<FILE *>(p);
                KERNEL_NS::FileUtil::CloseFile(*ptr);
            });

            auto bytesChange = KERNEL_NS::FileUtil::WriteFile(*fp, implementContent);
            if(bytesChange != static_cast<Int64>(implementContent.size()))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("write implement file bytesChange:%lld, implementContent size:%llu,  when export cpp config class name:%s fileName:%s.")
                            , bytesChange, static_cast<Int64>(implementContent.size()), iter.second->_tableClassName.c_str(), cppFileName.c_str());
                return false;
            }
        }

        {// 写入数据文件
            KERNEL_NS::SmartPtr<FILE, KERNEL_NS::AutoDelMethods::CustomDelete> fp =  KERNEL_NS::FileUtil::OpenFile(dataFileName.c_str(), true);
            if(!fp)
            {
                g_Log->Error(LOGFMT_OBJ_TAG("open data file fail when export cpp config class name:%s fileName:%s.")
                            , iter.second->_tableClassName.c_str(), dataFileName.c_str());
                return false;
            }

            fp.SetClosureDelegate([](void *p){
                auto ptr = reinterpret_cast<FILE *>(p);
                KERNEL_NS::FileUtil::CloseFile(*ptr);
            });

            auto bytesChange = KERNEL_NS::FileUtil::WriteFile(*fp, configDataContent);
            if(bytesChange != static_cast<Int64>(configDataContent.size()))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("write data file bytesChange:%lld, configDataContent size:%llu,  when export cpp config class name:%s fileName:%s.")
                            , bytesChange, static_cast<Int64>(configDataContent.size()), iter.second->_tableClassName.c_str(), dataFileName.c_str());
                return false;
            }
        }
    }

    return true;
}

bool XlsxExporterMgr::_ExportCppCodeHeader(const XlsxConfigTableInfo *configInfo, KERNEL_NS::LibString &fileContent) const
{
    // 变量定义
    const auto className = configInfo->_tableClassName.FirstCharToUpper() + "Config";
    const auto mgrClassName = className + "Mgr";
    const auto mgrFactoryClassName = mgrClassName + "Factory";
    std::map<KERNEL_NS::LibString, KERNEL_NS::LibString> fieldNameRefDataType;

    // 生成文件头注释
    {
        fileContent.AppendFormat("// Generate by %s, Dont modify it!!!\n", GetApp()->GetAppName().c_str());

        // 文件路径
        const auto multiLinePaths = configInfo->_xlsxPath.Split("\n");
        for(auto &path : multiLinePaths)
            fileContent.AppendFormat("// file path:%s\n", path.c_str());

        // 页签名
        const auto multiLineSheet = configInfo->_wholeSheetName.Split("\n");
        for(auto &sheet : multiLineSheet)
            fileContent.AppendFormat("// sheet name:%s\n", sheet.c_str());
    }
    fileContent.AppendFormat("\n");

    // 文件宏
    KERNEL_NS::LibString macroStr;
    macroStr.AppendFormat("__CONFIG_%s_CONFIG_H__", configInfo->_tableClassName.toupper().c_str());

    {

        fileContent.AppendFormat("#ifndef %s\n", macroStr.c_str());
        fileContent.AppendFormat("#define %s\n", macroStr.c_str());

        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("#pragma once\n");
        fileContent.AppendFormat("\n");

        fileContent.AppendFormat("#include <kernel/kernel.h>\n");
        fileContent.AppendFormat("#include <service_common/config/config.h>\n");
        fileContent.AppendFormat("#include <service/common/common.h>\n");
        fileContent.AppendFormat("\n");

        fileContent.AppendFormat("SERVICE_BEGIN\n");
        fileContent.AppendFormat("\n");
    }

    {// 导出所有枚举
        bool hasEnums = false;
        for(auto fieldInfo :  configInfo->_fieldInfos)
        {
            if(fieldInfo == NULL)
                continue;

            if(!SERVICE_COMMON_NS::DataTypeHelper::IsEnum(fieldInfo->_dataType))
                continue;

            // enum 名: 表名 + 字段名 + Enums
            fileContent.AppendFormat("class %s%sEnums\n", className.FirstCharToUpper().c_str()
            , fieldInfo->_fieldName.FirstCharToUpper().c_str());

            fileContent.AppendFormat("{\npublic:\n");
            fileContent.AppendFormat("    enum ENUMS\n");
            fileContent.AppendFormat("    {\n");
            fileContent.AppendFormat("        __UNKNOWN_ENUM = 0,\n\n");

            // TODO:数据
            std::set<KERNEL_NS::LibString> checkRepeat;
            const Int32 count = static_cast<Int32>(configInfo->_values.size());
            for(Int32 idx = 0; idx < count; ++idx)
            {
                auto &columRefData = configInfo->_values[idx];
                auto iter = columRefData.find(fieldInfo->_columnId);
                if(iter == columRefData.end())
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("cant find field data, column id:%llu config:%s, sheet name:%s file name:%s, field name:%s, field data type:%s")
                            , fieldInfo->_columnId, configInfo->_tableClassName.c_str(), configInfo->_wholeSheetName.c_str(), configInfo->_xlsxPath.c_str()
                            , fieldInfo->_fieldName.c_str(), fieldInfo->_dataType.c_str());
                    return false;
                }

                // 去重
                if(checkRepeat.find(iter->second.strip()) != checkRepeat.end())
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("enum name repeat please check:%s, column id:%llu config:%s, sheet name:%s file name:%s, field name:%s, field data type:%s")
                            , iter->second.c_str(), fieldInfo->_columnId, configInfo->_tableClassName.c_str(), configInfo->_wholeSheetName.c_str(), configInfo->_xlsxPath.c_str()
                            , fieldInfo->_fieldName.c_str(), fieldInfo->_dataType.c_str());
                    return false;
                }

                checkRepeat.insert(iter->second.strip());

                if(fieldInfo->_desc.empty())
                {
                    fileContent.AppendFormat("        %s,\n\n", iter->second.strip().c_str());
                }
                else
                {
                    fileContent.AppendFormat("        %s,    // %s\n\n", iter->second.strip().c_str(), fieldInfo->_desc.c_str());
                }
            }

            fileContent.AppendFormat("        __ENUM_MAX,\n\n");
            fileContent.AppendFormat("    };\n");
            fileContent.AppendFormat("};\n");

            hasEnums = true;
        }

        if(hasEnums)
            fileContent.AppendFormat("\n");
    }

    {// 配置类
        fileContent.AppendFormat("class %s\n", className.c_str());
        fileContent.AppendFormat("{\n");
        
        fileContent.AppendFormat("    POOL_CREATE_OBJ_DEFAULT(%s);\n", className.c_str());
        fileContent.AppendFormat("\n");

        // 构造析构
        fileContent.AppendFormat("public:\n");
        fileContent.AppendFormat("    %s();\n", className.c_str());
        fileContent.AppendFormat("    ~%s(){}\n", className.c_str());
        fileContent.AppendFormat("\n");

        // 序列化反序列化接口
        fileContent.AppendFormat("    bool Parse(const KERNEL_NS::LibString &lineData);\n");
        fileContent.AppendFormat("    void Serialize(KERNEL_NS::LibString &lineData) const;\n");
        fileContent.AppendFormat("\n");

        // 成员变量
        fileContent.AppendFormat("public:\n");

        // 解析成员变量元素
        for(auto fieldInfo :  configInfo->_fieldInfos)
        {
            if(fieldInfo == NULL)
                continue;

            KERNEL_NS::LibString dataType;
            KERNEL_NS::LibString errInfo;
            if(!SERVICE_COMMON_NS::DataTypeHelper::Parse(fieldInfo->_dataType, dataType, errInfo))
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("parse data type fail config:%s, sheet name:%s file name:%s, field name:%s, field data type:%s errInfo:%s")
                        ,  configInfo->_tableClassName.c_str(), configInfo->_wholeSheetName.c_str(), configInfo->_xlsxPath.c_str()
                        , fieldInfo->_fieldName.c_str(), fieldInfo->_dataType.c_str(), errInfo.c_str());

                return false;
            }

            fieldNameRefDataType.insert(std::make_pair(fieldInfo->_fieldName, dataType));

            if(!fieldInfo->_desc.empty())
            {
                fileContent.AppendFormat("    %s _%s;    // %s", dataType.c_str(), fieldInfo->_fieldName.FirstCharToLower().c_str(), fieldInfo->_desc.c_str());
                fileContent.AppendFormat("\n");
                fileContent.AppendFormat("\n");
            }
            else
            {
                fileContent.AppendFormat("    %s _%s;", dataType.c_str(), fieldInfo->_fieldName.FirstCharToLower().c_str());
                fileContent.AppendFormat("\n");
                fileContent.AppendFormat("\n");
            }
        }

        fileContent.AppendFormat("\n");

        fileContent.AppendFormat("};\n");
    }
    
    {// 配置管理类
        fileContent.AppendFormat("\n");


        fileContent.AppendFormat("class %s : public SERVICE_COMMON_NS::IConfigMgr\n", mgrClassName.c_str());
        fileContent.AppendFormat("{\n");
        fileContent.AppendFormat("    POOL_CREATE_OBJ_DEFAULT_P1(IConfigMgr, %s)\n", mgrClassName.c_str());
        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("public:\n");
        fileContent.AppendFormat("    // Empty configs define\n");
        fileContent.AppendFormat("    static const std::vector<%s *> s_empty;\n", className.c_str());
        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("    %s();\n", mgrClassName.c_str());
        fileContent.AppendFormat("    ~%s();\n", mgrClassName.c_str());
        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("    virtual void Release() override;\n");
        fileContent.AppendFormat("    virtual void Clear() override;\n");
        fileContent.AppendFormat("    virtual KERNEL_NS::LibString ToString() const override;\n");
        fileContent.AppendFormat("    virtual Int32 Load() override;\n");
        fileContent.AppendFormat("    virtual Int32 Reload() override;\n");
        fileContent.AppendFormat("    virtual const KERNEL_NS::LibString & GetConfigDataMd5() const override;\n");
        fileContent.AppendFormat("    const std::vector<%s *> &GetAllConfigs() const;\n", className.c_str());
        
        // 字典的
        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("    // dict configs\n");

        {// 索引方法
            for(auto fieldInfo : configInfo->_fieldInfos)
            {
                if(!fieldInfo)
                    continue;

                if(!fieldInfo->_flags.empty())
                {
                    auto flags = fieldInfo->_flags.Split("|");
                    for(auto flag : flags)
                    {
                        flag.strip();
                        flag = flag.tolower();
                        const auto &fieldName = fieldInfo->_fieldName.FirstCharToUpper();

                        auto iterDataType = fieldNameRefDataType.find(fieldInfo->_fieldName);
                        if(flag == "unique")
                        {// 唯一索引
                            // 必须是简单数据类型
                            if(!SERVICE_COMMON_NS::DataTypeHelper::IsSimpleType(fieldInfo->_dataType))
                            {
                                g_Log->Error(LOGFMT_OBJ_TAG("unique flag field must be a simple type: config:%s, sheet name:%s file name:%s, field name:%s, field data type:%s")
                                ,  configInfo->_tableClassName.c_str(), configInfo->_wholeSheetName.c_str(), configInfo->_xlsxPath.c_str()
                                , fieldInfo->_fieldName.c_str(), fieldInfo->_dataType.c_str());
                                return false;
                            }

                            fileContent.AppendFormat("    // by %s\n", fieldInfo->_fieldName.c_str());
                            fileContent.AppendFormat("    const std::map<%s, %s *> &GetAll%sRefConfigs() const;\n", iterDataType->second.c_str(), className.c_str(), fieldName.c_str());

                            fileContent.AppendFormat("    const %s * GetConfigBy%s(const %s &key) const;\n"
                                        , className.c_str(), fieldName.c_str(), iterDataType->second.c_str());
                            fileContent.AppendFormat("\n");
                        }
                        else if(flag == "index")
                        {// 普通索引
                            // 必须是简单数据类型
                            if(!SERVICE_COMMON_NS::DataTypeHelper::IsSimpleType(fieldInfo->_dataType))
                            {
                                g_Log->Error(LOGFMT_OBJ_TAG("index flag field must be a simple type: config:%s, sheet name:%s file name:%s, field name:%s, field data type:%s")
                                ,  configInfo->_tableClassName.c_str(), configInfo->_wholeSheetName.c_str(), configInfo->_xlsxPath.c_str()
                                , fieldInfo->_fieldName.c_str(), fieldInfo->_dataType.c_str());
                                return false;
                            }

                            fileContent.AppendFormat("    // by %s\n", fieldInfo->_fieldName.c_str());
                            fileContent.AppendFormat("    const std::map<%s, std::vector<%s *>> &GetAll%sRefConfigs() const;\n", iterDataType->second.c_str(), className.c_str(), fieldName.c_str());

                            fileContent.AppendFormat("    const std::vector<%s *> &GetConfigsBy%s(const %s &key) const;\n"
                                        , className.c_str(), fieldName.c_str(), iterDataType->second.c_str());

                            fileContent.AppendFormat("\n");
                        }
                    }
                }
            }
        }

        fileContent.AppendFormat("private:\n");
        fileContent.AppendFormat("    virtual void _OnClose() override;\n");
        fileContent.AppendFormat("    void _Clear();\n");
        fileContent.AppendFormat("    Int64 _ReadConfigData(FILE &fp, KERNEL_NS::LibString &configData, Int32 totalLine, Int32 curLine) const;\n");
        fileContent.AppendFormat("\n");

        {// 数据成员
            fileContent.AppendFormat("private:\n");
            fileContent.AppendFormat("    std::vector<%s *> _configs;\n", className.c_str());
            fileContent.AppendFormat("    KERNEL_NS::LibString _dataMd5;\n");

            // 索引字典
            for(auto fieldInfo : configInfo->_fieldInfos)
            {
                if(!fieldInfo)
                {
                    continue;
                }

                if(!fieldInfo->_flags.empty())
                {
                    auto flags = fieldInfo->_flags.Split("|");
                    for(auto flag : flags)
                    {
                        flag.strip();
                        flag = flag.tolower();
                        const auto &fieldName = fieldInfo->_fieldName.FirstCharToLower();
                        auto iterDataType = fieldNameRefDataType.find(fieldInfo->_fieldName);
                        if(flag == "unique")
                        {// 唯一索引
                            fileContent.AppendFormat("    std::map<%s, %s *> _%sRefConfig;\n"
                                        , iterDataType->second.c_str(), className.c_str(), fieldName.c_str());
                        }
                        else if(flag == "index")
                        {// 普通索引
                            fileContent.AppendFormat("    std::map<%s, std::vector<%s *>> _%sRefConfigs;\n"
                                        , iterDataType->second.c_str(), className.c_str(), fieldName.c_str());
                        }
                    }
                }
            }
        }

        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("};\n");
        fileContent.AppendFormat("\n");
    }

    {// 内联接口
        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("ALWAYS_INLINE const std::vector<%s *> &%s::GetAllConfigs() const\n", className.c_str(), mgrClassName.c_str());
        fileContent.AppendFormat("{\n");
        fileContent.AppendFormat("    return _configs;\n");
        fileContent.AppendFormat("}\n");

        fileContent.AppendFormat("\n");
        for(auto fieldInfo : configInfo->_fieldInfos)
        {
            if(!fieldInfo)
                continue;

            if(!fieldInfo->_flags.empty())
            {
                auto flags = fieldInfo->_flags.Split("|");
                for(auto flag : flags)
                {
                    flag.strip();
                    flag = flag.tolower();
                    const auto &fieldName = fieldInfo->_fieldName.FirstCharToUpper();
                    const auto &lowerFieldName =  fieldInfo->_fieldName.FirstCharToLower();

                    auto iterDataType = fieldNameRefDataType.find(fieldInfo->_fieldName);
                    if(flag == "unique")
                    {// 唯一索引
                        fileContent.AppendFormat("ALWAYS_INLINE const std::map<%s, %s *> &%s::GetAll%sRefConfigs() const\n"
                                    , iterDataType->second.c_str(), className.c_str(), mgrClassName.c_str(), fieldName.c_str());
                        fileContent.AppendFormat("{\n");
                        fileContent.AppendFormat("    return _%sRefConfig;\n", lowerFieldName.c_str());
                        fileContent.AppendFormat("}\n");
                        fileContent.AppendFormat("\n");

                        fileContent.AppendFormat("ALWAYS_INLINE const %s * %s::GetConfigBy%s(const %s &key) const\n"
                                    , className.c_str(), mgrClassName.c_str(), fieldName.c_str(), iterDataType->second.c_str());
                        fileContent.AppendFormat("{\n");
                        fileContent.AppendFormat("    auto iter = _%sRefConfig.find(key);\n", lowerFieldName.c_str());
                        fileContent.AppendFormat("    return iter == _%sRefConfig.end() ? NULL : iter->second;\n", lowerFieldName.c_str());
                        fileContent.AppendFormat("}\n");
                        fileContent.AppendFormat("\n");

                    }
                    else if(flag == "index")
                    {// 普通索引
                        fileContent.AppendFormat("ALWAYS_INLINE const std::map<%s, std::vector<%s *>> &%s::GetAll%sRefConfigs() const\n", iterDataType->second.c_str(), className.c_str(), mgrClassName.c_str(), fieldName.c_str());
                        fileContent.AppendFormat("{\n");
                        fileContent.AppendFormat("    return _%sRefConfigs;\n", lowerFieldName.c_str());
                        fileContent.AppendFormat("}\n");
                        fileContent.AppendFormat("\n");

                        fileContent.AppendFormat("ALWAYS_INLINE const std::vector<%s *> &%s::GetConfigsBy%s(const %s &key) const\n"
                                    , className.c_str(), mgrClassName.c_str(), fieldName.c_str(), iterDataType->second.c_str());
                        fileContent.AppendFormat("{\n");
                        fileContent.AppendFormat("    auto iterConfigs = _%sRefConfigs.find(key);\n", lowerFieldName.c_str());
                        fileContent.AppendFormat("    return iterConfigs == _%sRefConfigs.end() ? s_empty : iterConfigs->second;\n", lowerFieldName.c_str());
                        fileContent.AppendFormat("}\n");
                        fileContent.AppendFormat("\n");
                    }
                }
            }
        }
    }

    {// 配置管理类工厂
        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("class %s : public KERNEL_NS::CompFactory\n", mgrFactoryClassName.c_str());
        fileContent.AppendFormat("{\n");
        fileContent.AppendFormat("public:\n");
        fileContent.AppendFormat("    static constexpr KERNEL_NS::_Build::MT _buildType{};\n");
        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("    static KERNEL_NS::CompFactory *FactoryCreate()\n");
        fileContent.AppendFormat("    {\n");
        fileContent.AppendFormat("        return KERNEL_NS::ObjPoolWrap<%s>::NewByAdapter(_buildType.V);\n", mgrFactoryClassName.c_str());
        fileContent.AppendFormat("    }\n");
        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("    virtual void Release()\n");
        fileContent.AppendFormat("    {\n");
        fileContent.AppendFormat("        KERNEL_NS::ObjPoolWrap<%s>::DeleteByAdapter(_buildType.V, this);\n", mgrFactoryClassName.c_str());
        fileContent.AppendFormat("    }\n");
        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("    virtual KERNEL_NS::CompObject *Create() const\n");
        fileContent.AppendFormat("    {\n");
        fileContent.AppendFormat("        return %s::NewByAdapter_%s(_buildType.V);\n", mgrClassName.c_str(), mgrClassName.c_str());
        fileContent.AppendFormat("    }\n");
        fileContent.AppendFormat("};\n");
        fileContent.AppendFormat("\n");
    }

    {// 文件宏末尾
        fileContent.AppendFormat("SERVICE_END\n");
        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("#endif // %s\n", macroStr.c_str());
    }

    return true;
}

bool XlsxExporterMgr::_ExportCppCodeImpl(const XlsxConfigTableInfo *configInfo, KERNEL_NS::LibString &fileContent) const
{
    const auto className = configInfo->_tableClassName.FirstCharToUpper() + "Config";
    const auto mgrClassName = className + "Mgr";
    const auto mgrClassFactory = mgrClassName + "Factory";

    {// 生成文件头注释
        fileContent.AppendFormat("// Generate by %s, Dont modify it!!!\n", GetApp()->GetAppName().c_str());

        // 文件路径
        const auto multiLinePaths = configInfo->_xlsxPath.Split("\n");
        for(auto &path : multiLinePaths)
            fileContent.AppendFormat("// file path:%s\n", path.c_str());

        // 页签名
        const auto multiLineSheet = configInfo->_wholeSheetName.Split("\n");
        for(auto &sheet : multiLineSheet)
            fileContent.AppendFormat("// sheet name:%s\n", sheet.c_str());
    }

    fileContent.AppendFormat("\n");
    fileContent.AppendFormat("#include <pch.h>\n");
    fileContent.AppendFormat("#include \"%s.h\"\n", className.c_str());

    fileContent.AppendFormat("\n");
    fileContent.AppendFormat("SERVICE_BEGIN\n");

    fileContent.AppendFormat("\n");
    fileContent.AppendFormat("POOL_CREATE_OBJ_DEFAULT_IMPL(%s);\n", className.c_str());


    // 初始化列表
    fileContent.AppendFormat("%s::%s()\n", className.c_str(), className.c_str());
    bool isFirstField = true;
    Int32 totalFieldNum = 0;
    for(auto fieldInfo : configInfo->_fieldInfos)
    {
        if(!fieldInfo)
        {
            continue;
        }

        ++totalFieldNum;
        if(!SERVICE_COMMON_NS::DataTypeHelper::IsSimpleType(fieldInfo->_dataType))
            continue;

        if(SERVICE_COMMON_NS::DataTypeHelper::IsString(fieldInfo->_dataType))
            continue;

        if(isFirstField)
        {
            isFirstField = false;
            fileContent.AppendFormat(":");
        }
        else
        {
            fileContent.AppendFormat(",");
        }
        const auto &memberName =  _MakeConfigMemberName(fieldInfo->_fieldName);
        const auto &defaultValue = SERVICE_COMMON_NS::DataTypeHelper::GetTypeDefaultValue(fieldInfo->_dataType);
        fileContent.AppendFormat("%s(%s)\n", memberName.c_str(), defaultValue.c_str());
    }
    fileContent.AppendFormat("{\n");
    fileContent.AppendFormat("}\n");
    fileContent.AppendFormat("\n");

    {// 反序列化
        fileContent.AppendFormat("bool %s::Parse(const KERNEL_NS::LibString &lineData)\n", className.c_str());
        fileContent.AppendFormat("{\n");

        fileContent.AppendFormat("// use json serialize text.\n");
        fileContent.AppendFormat("// format:column_{column id}_{data_len}:{json text}|...\n");
        fileContent.AppendFormat("// example:column_1_10:{json text}|...\n");
        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("    const Int32 fieldNum = %d;\n", totalFieldNum);
        fileContent.AppendFormat("    Int32 countFieldNum = 0;\n");
        fileContent.AppendFormat("    Int32 startPos = 0;\n");
        fileContent.AppendFormat("\n");

        // 解析字段数据
        for(auto fieldInfo : configInfo->_fieldInfos)
        {
            if(!fieldInfo)
                continue;

            const auto &memberName = _MakeConfigMemberName(fieldInfo->_fieldName);
            fileContent.AppendFormat("    {// %s\n", memberName.c_str());
            fileContent.AppendFormat("        auto pos = lineData.GetRaw().find_first_of(\"column_\", startPos);\n");
            fileContent.AppendFormat("        if(pos == std::string::npos)\n");
            fileContent.AppendFormat("        {\n");
            fileContent.AppendFormat("            g_Log->Error(LOGFMT_OBJ_TAG(\"parse field:%s, data format error: have no column_ prefix, lineData:%%s, startPos:%%d, countFieldNum:%%d\"), lineData.c_str(), startPos, countFieldNum);\n", fieldInfo->_fieldName.c_str());
            fileContent.AppendFormat("            return false;\n");
            fileContent.AppendFormat("        }\n");
            fileContent.AppendFormat("\n");
            fileContent.AppendFormat("       auto headerTailPos = lineData.GetRaw().find_first_of(\":\", pos);\n");
            fileContent.AppendFormat("       if(headerTailPos == std::string::npos)\n");
            fileContent.AppendFormat("       {\n");
            fileContent.AppendFormat("            g_Log->Error(LOGFMT_OBJ_TAG(\"parse field:%s, bad line data not find : symbol after column_ line data:%%s, startPos:%%d, countFieldNum:%%d\"), lineData.c_str(), startPos, countFieldNum);\n", fieldInfo->_fieldName.c_str());
            fileContent.AppendFormat("            return false;\n");
            fileContent.AppendFormat("       }\n");
            fileContent.AppendFormat("\n");
            fileContent.AppendFormat("       // parse data\n");
            fileContent.AppendFormat("       const KERNEL_NS::LibString headerInfo = lineData.GetRaw().substr(pos, headerTailPos - pos);\n");
            fileContent.AppendFormat("       const auto headParts = headerInfo.Split('_');\n");
            fileContent.AppendFormat("       if(headParts.empty())\n");
            fileContent.AppendFormat("       {\n");
            fileContent.AppendFormat("            g_Log->Error(LOGFMT_OBJ_TAG(\"parse field:%s, bad line data not find sep symbol:_ in header info line data:%%s, pos:%%d, headerTailPos:%%d, startPos:%%d, countFieldNum:%%d,\"),\n", fieldInfo->_fieldName.c_str());
            fileContent.AppendFormat("            lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);\n");
            fileContent.AppendFormat("            return false;\n");
            fileContent.AppendFormat("       }\n");
            fileContent.AppendFormat("\n");
            fileContent.AppendFormat("       // check column id");
            fileContent.AppendFormat("\n");
            fileContent.AppendFormat("\n");
            fileContent.AppendFormat("       const auto &columnIdString = headParts[1];\n");
            fileContent.AppendFormat("       if(columnIdString.length() == 0)\n");
            fileContent.AppendFormat("       {\n");
            fileContent.AppendFormat("           g_Log->Error(LOGFMT_OBJ_TAG(\"parse field:%s have no column id, bad line data header len info line data:%%s, pos:%%d, headerTailPos:%%d, startPos:%%d, countFieldNum:%%d,\"), lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);\n", fieldInfo->_fieldName.c_str());
            fileContent.AppendFormat("           return false;\n");
            fileContent.AppendFormat("       }\n");
            fileContent.AppendFormat("       const UInt64 columnId = KERNEL_NS::StringUtil::StringToUInt64(columnIdString.c_str());\n");
            fileContent.AppendFormat("       if(columnId != %llu)\n", fieldInfo->_columnId);
            fileContent.AppendFormat("       {\n");
            fileContent.AppendFormat("           g_Log->Error(LOGFMT_OBJ_TAG(\"parse field:%s, fail: bad comumn id, columnId:%%llu, real column id:%llu, please check if config data is old version, line data header len info line data:%%s, pos:%%d, headerTailPos:%%d, startPos:%%d, countFieldNum:%%d\"), columnId, lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);\n", fieldInfo->_fieldName.c_str(), fieldInfo->_columnId);
            fileContent.AppendFormat("           return false;\n");
            fileContent.AppendFormat("       }\n");
            fileContent.AppendFormat("\n");
            fileContent.AppendFormat("       // data len\n");
            fileContent.AppendFormat("       const auto &lenInfo = headParts[2];\n");
            fileContent.AppendFormat("       if(lenInfo.length() == 0)\n");
            fileContent.AppendFormat("       {\n");
            fileContent.AppendFormat("           g_Log->Error(LOGFMT_OBJ_TAG(\"parse field:%s fail: bad line data header len info line data:%%s, pos:%%d, headerTailPos:%%d, startPos:%%d, countFieldNum:%%d\"), lineData.c_str(), \n", fieldInfo->_fieldName.c_str());
            fileContent.AppendFormat("           static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);\n");
            fileContent.AppendFormat("           return false;\n");
            fileContent.AppendFormat("       }\n");
            fileContent.AppendFormat("\n");
            fileContent.AppendFormat("       const Int32 dataLen = KERNEL_NS::StringUtil::StringToInt32(lenInfo.c_str());\n");
            fileContent.AppendFormat("       const auto dataEndPos = headerTailPos + static_cast<decltype(headerTailPos)>(dataLen);\n");
            fileContent.AppendFormat("       KERNEL_NS::LibString dataPart = lineData.GetRaw().substr(headerTailPos + 1, dataEndPos - headerTailPos);\n");
            fileContent.AppendFormat("\n");
            fileContent.AppendFormat("      // parse data through\n");

            fileContent.AppendFormat("      KERNEL_NS::LibString errInfo;\n");
            fileContent.AppendFormat("      if(!SERVICE_COMMON_NS::DataTypeHelper::Assign(%s, dataPart, errInfo))\n", memberName.c_str());
            fileContent.AppendFormat("      {\n");
            fileContent.AppendFormat("          g_Log->Error(LOGFMT_OBJ_TAG(\"%%s, assign fail field name:%s, data part:%%s, errInfo:%%s  line data:%%s, pos:%%d, headerTailPos:%%d, dataEndPos:%%d\"), KERNEL_NS::RttiUtil::GetByObj(this), dataPart.c_str(), errInfo.c_str(), lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), static_cast<Int32>(dataEndPos));\n", memberName.c_str());
            fileContent.AppendFormat("          return false;\n");
            fileContent.AppendFormat("      }\n");
            fileContent.AppendFormat("\n");

            fileContent.AppendFormat("      startPos = static_cast<Int32>(dataEndPos);\n");
            fileContent.AppendFormat("      ++countFieldNum;\n");

            fileContent.AppendFormat("    }// %s\n", memberName.c_str());

            fileContent.AppendFormat("\n");
        }

        // 结束
        fileContent.AppendFormat("    if(countFieldNum != fieldNum)\n");
        fileContent.AppendFormat("    {\n");
        fileContent.AppendFormat("        g_Log->Error(LOGFMT_OBJ_TAG(\"field num not enough countFieldNum:%%d, need fieldNum:%%d lineData:%%s\"), countFieldNum, fieldNum, lineData.c_str());\n");
        fileContent.AppendFormat("        return false;\n");
        fileContent.AppendFormat("    }\n");
        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("    return true;\n");
        fileContent.AppendFormat("}\n");
    }

    fileContent.AppendFormat("\n");

    {// 序列化
        fileContent.AppendFormat("void %s::Serialize(KERNEL_NS::LibString &lineData) const\n", className.c_str());
        fileContent.AppendFormat("{\n");
        fileContent.AppendFormat("    const Int32 fieldNum = %d;\n", totalFieldNum);
        fileContent.AppendFormat("    Int32 countFieldNum = 0;\n");
        fileContent.AppendFormat("\n");

        // 序列化每个字段
        for(auto fieldInfo : configInfo->_fieldInfos)
        {
            if(!fieldInfo)
                continue;

            const auto &memberName = _MakeConfigMemberName(fieldInfo->_fieldName);
            fileContent.AppendFormat("    {// %s\n", memberName.c_str());
            fileContent.AppendFormat("        KERNEL_NS::LibString data;\n");
            fileContent.AppendFormat("        SERVICE_COMMON_NS::DataTypeHelper::ToString(%s, data);\n", memberName.c_str());
            fileContent.AppendFormat("        lineData.AppendFormat(\"column_%llu_%%d:%%s|\", static_cast<Int32>(data.size()), data.c_str());\n", fieldInfo->_columnId);
            fileContent.AppendFormat("        ++countFieldNum;\n");
            fileContent.AppendFormat("    }// %s\n", memberName.c_str());
            fileContent.AppendFormat("\n");
        }
        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("    if(countFieldNum != fieldNum)\n");
        fileContent.AppendFormat("    {\n");
        fileContent.AppendFormat("        g_Log->Error(LOGFMT_OBJ_TAG(\"field num not enough countFieldNum:%%d, need fieldNum:%%d\"), countFieldNum, fieldNum);\n");
        fileContent.AppendFormat("    }\n");

        fileContent.AppendFormat("}\n");
    }

    {// ConfigMgr
        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("POOL_CREATE_OBJ_DEFAULT_IMPL(%s);\n", mgrClassName.c_str());
        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("const std::vector<%s *> %s::s_empty;\n", className.c_str(), mgrClassName.c_str());
        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("%s::%s()\n", mgrClassName.c_str(), mgrClassName.c_str());
        fileContent.AppendFormat("{\n");
        fileContent.AppendFormat("}\n");

        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("%s::~%s()\n", mgrClassName.c_str(), mgrClassName.c_str());
        fileContent.AppendFormat("{\n");
        fileContent.AppendFormat("    _Clear();\n");
        fileContent.AppendFormat("}\n");

        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("void %s::Release()\n", mgrClassName.c_str());
        fileContent.AppendFormat("{\n");
        fileContent.AppendFormat("    %s::DeleteByAdapter_%s(%s::_buildType.V, this);\n", mgrClassName.c_str(), mgrClassName.c_str(), mgrClassFactory.c_str());
        fileContent.AppendFormat("}\n");

        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("void %s::Clear()\n", mgrClassName.c_str());
        fileContent.AppendFormat("{\n");
        fileContent.AppendFormat("    _Clear();\n");
        fileContent.AppendFormat("}\n");

        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("KERNEL_NS::LibString %s::ToString() const\n", mgrClassName.c_str());
        fileContent.AppendFormat("{\n");
        fileContent.AppendFormat("    return GetObjName();\n");
        fileContent.AppendFormat("}\n");

        
        {// Load
            fileContent.AppendFormat("\n");
            fileContent.AppendFormat("Int32 %s::Load()\n", mgrClassName.c_str());
            fileContent.AppendFormat("{\n");
            fileContent.AppendFormat("    KERNEL_NS::SmartPtr<std::vector<%s *>, KERNEL_NS::AutoDelMethods::CustomDelete> configs = new std::vector<%s *>;\n", className.c_str(), className.c_str());
            fileContent.AppendFormat("    configs.SetClosureDelegate([](void *p){\n");
            fileContent.AppendFormat("        std::vector<%s *> *ptr = reinterpret_cast<std::vector<%s *> *>(p);\n", className.c_str(), className.c_str());
            fileContent.AppendFormat("        KERNEL_NS::ContainerUtil::DelContainer(*ptr, [](%s *ptr){\n", className.c_str());
            fileContent.AppendFormat("            %s::Delete_%s(ptr);\n", className.c_str(), className.c_str());
            fileContent.AppendFormat("        });\n");
            fileContent.AppendFormat("        delete ptr;\n");
            fileContent.AppendFormat("    });\n");
            fileContent.AppendFormat("\n");
            fileContent.AppendFormat("    const auto basePath = GetLoader()->GetBasePath();\n");
            fileContent.AppendFormat("    const auto wholePath = basePath + \"/\" + \"%s.data\";\n", className.c_str());
            fileContent.AppendFormat("    KERNEL_NS::SmartPtr<FILE, KERNEL_NS::AutoDelMethods::CustomDelete> fp = KERNEL_NS::FileUtil::OpenFile(wholePath.c_str(), false, \"rb\");\n");
            fileContent.AppendFormat("    if(!fp)\n");
            fileContent.AppendFormat("    {\n");
            fileContent.AppendFormat("        g_Log->Error(LOGFMT_OBJ_TAG(\"data file not found wholePath:%%s\"), wholePath.c_str());\n");
            fileContent.AppendFormat("        return Status::Failed;\n");
            fileContent.AppendFormat("    }\n");
            fileContent.AppendFormat("\n");
            fileContent.AppendFormat("    fp.SetClosureDelegate([](void *p){\n");
            fileContent.AppendFormat("        auto fpPtr = reinterpret_cast<FILE *>(p);\n");
            fileContent.AppendFormat("        KERNEL_NS::FileUtil::CloseFile(*fpPtr);\n");
            fileContent.AppendFormat("    });\n");
            fileContent.AppendFormat("\n");
            fileContent.AppendFormat("    Int32 line = 0;\n");
            fileContent.AppendFormat("    MD5_CTX ctx;\n");
            fileContent.AppendFormat("    if(!KERNEL_NS::LibDigest::MakeMd5Init(ctx))\n");
            fileContent.AppendFormat("    {\n");
            fileContent.AppendFormat("        g_Log->Error(LOGFMT_OBJ_TAG(\"make md5 init fail wholePath:%%s\"), wholePath.c_str());\n");
            fileContent.AppendFormat("        return Status::Failed;\n");
            fileContent.AppendFormat("    }\n");
            fileContent.AppendFormat("\n");
            fileContent.AppendFormat("    // 去重");
            fileContent.AppendFormat("\n");

            // 字段类型解析
            std::map<KERNEL_NS::LibString, KERNEL_NS::LibString> fieldNameRefDataType;
            for(auto fieldInfo :  configInfo->_fieldInfos)
            {
                if(fieldInfo == NULL)
                    continue;

                KERNEL_NS::LibString dataType;
                KERNEL_NS::LibString errInfo;
                if(!SERVICE_COMMON_NS::DataTypeHelper::Parse(fieldInfo->_dataType, dataType, errInfo))
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("parse data type fail config:%s, sheet name:%s file name:%s, field name:%s, field data type:%s errInfo:%s")
                            ,  configInfo->_tableClassName.c_str(), configInfo->_wholeSheetName.c_str(), configInfo->_xlsxPath.c_str()
                            , fieldInfo->_fieldName.c_str(), fieldInfo->_dataType.c_str(), errInfo.c_str());

                    return false;
                }

                fieldNameRefDataType.insert(std::make_pair(fieldInfo->_fieldName, dataType));
            }

            // 去重的字段容器定义 以及字段字符串拼接
            KERNEL_NS::LibString fieldNames;
            KERNEL_NS::LibString fieldNamesWrap;
            fieldNamesWrap.AppendFormat("\"");
            for(auto fieldInfo : configInfo->_fieldInfos)
            {
                if(!fieldInfo)
                    continue;

                fieldNames.AppendFormat("%s|", fieldInfo->_fieldName.c_str());
                if(!fieldInfo->_flags.empty())
                {
                    auto flags = fieldInfo->_flags.Split("|");
                    for(auto flag : flags)
                    {
                        flag.strip();
                        flag = flag.tolower();
                        const auto &fieldName = fieldInfo->_fieldName.FirstCharToLower();
                        auto iterDataType = fieldNameRefDataType.find(fieldInfo->_fieldName);
                        if(flag == "unique")
                        {// 唯一索引
                            fileContent.AppendFormat("    std::set<%s> unique_%ss;\n", iterDataType->second.c_str(), fieldName.c_str());
                        }
                    }
                }
            }
            fieldNamesWrap += fieldNames;
            fieldNamesWrap.AppendFormat("\"");

            fileContent.AppendFormat("    Int32 totalLine = 0;\n");
            fileContent.AppendFormat("\n");
            fileContent.AppendFormat("    while(true)\n");
            fileContent.AppendFormat("    {\n");
            fileContent.AppendFormat("        KERNEL_NS::LibString lineData;\n");

            // 读第一行数据
            fileContent.AppendFormat("        if(line < 2)\n");
            fileContent.AppendFormat("        {\n");
            fileContent.AppendFormat("            Int64 retBytes = KERNEL_NS::FileUtil::ReadUtf8OneLine(*fp, lineData);\n");
            fileContent.AppendFormat("            if(retBytes == 0)\n");
            fileContent.AppendFormat("            {\n");
            fileContent.AppendFormat("                g_Log->Error(LOGFMT_OBJ_TAG(\"ReadUtf8OneLine fail wholePath:%%s, line:%%d, lineData:%%s\"), wholePath.c_str(), line, lineData.c_str());\n");
            fileContent.AppendFormat("                KERNEL_NS::LibDigest::MakeMd5Clean(ctx);\n");
            fileContent.AppendFormat("                return Status::Failed;\n");
            fileContent.AppendFormat("            }\n");
            fileContent.AppendFormat("            ++line;\n");
            fileContent.AppendFormat("            if(!KERNEL_NS::LibDigest::MakeMd5Continue(ctx, lineData.data(), static_cast<UInt64>(lineData.size())))\n");
            fileContent.AppendFormat("            {\n");
            fileContent.AppendFormat("                g_Log->Error(LOGFMT_OBJ_TAG(\"MakeMd5Continue fail wholePath:%%s, line:%%d, lineData:%%s\"), wholePath.c_str(), line, lineData.c_str());\n");
            fileContent.AppendFormat("                KERNEL_NS::LibDigest::MakeMd5Clean(ctx);\n");
            fileContent.AppendFormat("                return Status::Failed;\n");
            fileContent.AppendFormat("            }\n");
            fileContent.AppendFormat("\n");
            fileContent.AppendFormat("            // first line data is field names\n");
            fileContent.AppendFormat("            if(line == 1)\n");
            fileContent.AppendFormat("            {\n");
            fileContent.AppendFormat("                if(lineData != %s)\n", fieldNamesWrap.c_str());
            fileContent.AppendFormat("                {\n");
            fileContent.AppendFormat("                    g_Log->Error(LOGFMT_OBJ_TAG(\"current data not match this config data wholePath:%%s, current data columns:%%s, this config columns:%s\"), wholePath.c_str(), lineData.c_str());\n", fieldNames.c_str());
            fileContent.AppendFormat("                    return Status::Failed;\n");
            fileContent.AppendFormat("                }\n");
            fileContent.AppendFormat("            }\n");

            fileContent.AppendFormat("            // total line data\n");
            fileContent.AppendFormat("            if(line == 2)\n");
            fileContent.AppendFormat("            {\n");
            fileContent.AppendFormat("                lineData.strip();\n");
            fileContent.AppendFormat("                totalLine = KERNEL_NS::StringUtil::StringToInt32(lineData.c_str());\n");
            fileContent.AppendFormat("            }\n");
            fileContent.AppendFormat("\n");
            fileContent.AppendFormat("            continue;\n");
            fileContent.AppendFormat("        }\n");
            fileContent.AppendFormat("\n");
            fileContent.AppendFormat("        // have no data\n");
            fileContent.AppendFormat("        if(totalLine <= 2)\n");
            fileContent.AppendFormat("            break;\n");
            fileContent.AppendFormat("\n");
            fileContent.AppendFormat("        if(totalLine <= line)\n");
            fileContent.AppendFormat("          break;\n");
            fileContent.AppendFormat("\n");
            fileContent.AppendFormat("        Int64 readBytes = _ReadConfigData(*fp, lineData, totalLine, line + 1);\n");
            fileContent.AppendFormat("        if(readBytes < 0)\n");
            fileContent.AppendFormat("        {\n");
            fileContent.AppendFormat("            g_Log->Error(LOGFMT_OBJ_TAG(\"Read a line config data fail wholePath:%%s, line:%%d, \"), wholePath.c_str(), line);\n");
            fileContent.AppendFormat("            g_Log->Error2(LOGFMT_OBJ_TAG_NO_FMT(), KERNEL_NS::LibString(\"lineData:\"), lineData);\n");
            fileContent.AppendFormat("            KERNEL_NS::LibDigest::MakeMd5Clean(ctx);\n");
            fileContent.AppendFormat("            return Status::Failed;\n");
            fileContent.AppendFormat("        }\n");
            fileContent.AppendFormat("\n");
            fileContent.AppendFormat("        if(readBytes == 0)\n");
            fileContent.AppendFormat("            break;\n");
            fileContent.AppendFormat("\n");
            fileContent.AppendFormat("        ++line;\n");
            fileContent.AppendFormat("\n");
            fileContent.AppendFormat("        if(!KERNEL_NS::LibDigest::MakeMd5Continue(ctx, lineData.data(), static_cast<UInt64>(lineData.size())))\n");
            fileContent.AppendFormat("        {\n");
            fileContent.AppendFormat("            g_Log->Error(LOGFMT_OBJ_TAG(\"MakeMd5Continue fail wholePath:%%s, line:%%d, lineData:%%s\"), wholePath.c_str(), line, lineData.c_str());\n");
            fileContent.AppendFormat("            KERNEL_NS::LibDigest::MakeMd5Clean(ctx);\n");
            fileContent.AppendFormat("            return Status::Failed;\n");
            fileContent.AppendFormat("        }\n");
            fileContent.AppendFormat("\n");
            fileContent.AppendFormat("        KERNEL_NS::SmartPtr<%s, KERNEL_NS::AutoDelMethods::CustomDelete> config = %s::New_%s();\n", className.c_str(), className.c_str(), className.c_str());
            fileContent.AppendFormat("        config.SetClosureDelegate([](void *p){\n");
            fileContent.AppendFormat("            auto ptr = reinterpret_cast<%s *>(p);\n", className.c_str());
            fileContent.AppendFormat("            %s::Delete_%s(ptr);\n", className.c_str(), className.c_str());
            fileContent.AppendFormat("        });\n");
            fileContent.AppendFormat("\n");
            fileContent.AppendFormat("        if(!config->Parse(lineData))\n");
            fileContent.AppendFormat("        {\n");
            fileContent.AppendFormat("            g_Log->Warn(LOGFMT_OBJ_TAG(\"parse %s fail data path:%%s line:%%d, lineData:%%s\"), wholePath.c_str(), line, lineData.c_str());\n", className.c_str());
            fileContent.AppendFormat("            return Status::Failed;\n");
            fileContent.AppendFormat("        }\n");
            fileContent.AppendFormat("\n");

            // unique唯一性校验
            for(auto fieldInfo : configInfo->_fieldInfos)
            {
                if(!fieldInfo)
                    continue;

                if(!fieldInfo->_flags.empty())
                {
                    auto flags = fieldInfo->_flags.Split("|");
                    for(auto flag : flags)
                    {
                        flag.strip();
                        flag = flag.tolower();
                        const auto &fieldName = fieldInfo->_fieldName.FirstCharToLower();
                        if(flag == "unique")
                        {// 唯一索引
                            const auto &memberName = _MakeConfigMemberName(fieldInfo->_fieldName);
                            const auto &uniqueCheckFieldName = "unique_" + fieldName + "s";
                            fileContent.AppendFormat("        // check unique\n");
                            fileContent.AppendFormat("        if(%s.find(config->%s) != %s.end())\n", uniqueCheckFieldName.c_str(), memberName.c_str(), uniqueCheckFieldName.c_str());
                            fileContent.AppendFormat("        {\n");
                            fileContent.AppendFormat("            g_Log->Warn(LOGFMT_OBJ_TAG(\"duplicate %s:%%d data path:%%s line:%%d, lineData:%%s\"), config->%s, wholePath.c_str(), line, lineData.c_str());\n", fieldInfo->_fieldName.c_str(), memberName.c_str());
                            fileContent.AppendFormat("            return Status::Failed;\n");
                            fileContent.AppendFormat("        }\n");
                            fileContent.AppendFormat("\n");
                            fileContent.AppendFormat("        %s.insert(config->%s);\n", uniqueCheckFieldName.c_str(), memberName.c_str());
                            fileContent.AppendFormat("\n");
                        }
                    }
                }
            }

            fileContent.AppendFormat("        configs->push_back(config.pop());\n");
            fileContent.AppendFormat("    }// while(true)\n");

            fileContent.AppendFormat("\n");
            fileContent.AppendFormat("    KERNEL_NS::LibString dataMd5;\n");
            fileContent.AppendFormat("    if(!KERNEL_NS::LibDigest::MakeMd5Final(ctx, dataMd5))\n");
            fileContent.AppendFormat("    {\n");
            fileContent.AppendFormat("        g_Log->Error(LOGFMT_OBJ_TAG(\"MakeMd5Final fail wholePath:%%s\"), wholePath.c_str());\n");
            fileContent.AppendFormat("        KERNEL_NS::LibDigest::MakeMd5Clean(ctx);\n");
            fileContent.AppendFormat("        return Status::Failed;\n");
            fileContent.AppendFormat("    }\n");
            fileContent.AppendFormat("    KERNEL_NS::LibDigest::MakeMd5Clean(ctx);\n");
            fileContent.AppendFormat("\n");

            fileContent.AppendFormat("    _dataMd5 = KERNEL_NS::LibBase64::Encode(dataMd5);\n");
            fileContent.AppendFormat("    _configs.swap(*configs.AsSelf());\n");
            fileContent.AppendFormat("\n");

            // 清空映射字典
            for(auto fieldInfo : configInfo->_fieldInfos)
            {
                if(!fieldInfo)
                    continue;

                if(!fieldInfo->_flags.empty())
                {
                    auto flags = fieldInfo->_flags.Split("|");
                    for(auto flag : flags)
                    {
                        flag.strip();
                        flag = flag.tolower();
                        const auto &fieldName = fieldInfo->_fieldName.FirstCharToLower();
                        const auto &memberName = _MakeConfigMemberName(fieldInfo->_fieldName);
                        if(flag == "unique")
                        {// 唯一索引
                            fileContent.AppendFormat("    _%sRefConfig.clear();\n", fieldName.c_str());
                        }
                        else if(flag == "index")
                        {// 普通索引
                            fileContent.AppendFormat("    _%sRefConfigs.clear();\n", fieldName.c_str());
                        }
                    }
                }
            }
            fileContent.AppendFormat("\n");

            // 字典映射
            fileContent.AppendFormat("    for(auto config : _configs)\n");
            fileContent.AppendFormat("    {\n");

            // 字典映射数据填充
            for(auto fieldInfo : configInfo->_fieldInfos)
            {
                if(!fieldInfo)
                    continue;

                if(!fieldInfo->_flags.empty())
                {
                    auto flags = fieldInfo->_flags.Split("|");
                    for(auto flag : flags)
                    {
                        flag.strip();
                        flag = flag.tolower();
                        const auto &fieldName = fieldInfo->_fieldName.FirstCharToLower();
                        const auto &memberName = _MakeConfigMemberName(fieldInfo->_fieldName);

                        if(flag == "unique")
                        {// 唯一索引
                            fileContent.AppendFormat("        // key:%s \n", fieldInfo->_fieldName.c_str());
                            fileContent.AppendFormat("        {\n");
                            fileContent.AppendFormat("            _%sRefConfig.insert(std::make_pair(config->%s, config));\n", fieldName.c_str(), memberName.c_str());
                            fileContent.AppendFormat("        }\n");
                            fileContent.AppendFormat("\n");
                        }
                        else if(flag == "index")
                        {// 普通索引
                            fileContent.AppendFormat("        // key:%s \n", fieldInfo->_fieldName.c_str());
                            fileContent.AppendFormat("        {\n");

                            fileContent.AppendFormat("            auto iterConfigs = _%sRefConfigs.find(config->%s);\n", fieldName.c_str(), memberName.c_str());
                            fileContent.AppendFormat("            if(iterConfigs == _%sRefConfigs.end())\n", fieldName.c_str());
                            fileContent.AppendFormat("                iterConfigs = _%sRefConfigs.insert(std::make_pair(config->%s, std::vector<%s *>())).first;\n", fieldName.c_str(), memberName.c_str(), className.c_str());
                            fileContent.AppendFormat("            iterConfigs->second.push_back(config);\n");
                            fileContent.AppendFormat("        }\n");
                            fileContent.AppendFormat("\n");
                        }
                    }
                }
            }
            fileContent.AppendFormat("    }\n");

            fileContent.AppendFormat("\n");
            fileContent.AppendFormat("    return Status::Success;\n");
            fileContent.AppendFormat("}\n");
        }
        
        {// reload
            fileContent.AppendFormat("\n");
            fileContent.AppendFormat("Int32 %s::Reload()\n", mgrClassName.c_str());
            fileContent.AppendFormat("{\n");
            fileContent.AppendFormat("    return Load();\n");
            fileContent.AppendFormat("}\n");
        }

        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("const KERNEL_NS::LibString &%s::GetConfigDataMd5() const\n", mgrClassName.c_str());
        fileContent.AppendFormat("{\n");
        fileContent.AppendFormat("    return _dataMd5;\n");
        fileContent.AppendFormat("}\n");

        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("void %s::_OnClose()\n", mgrClassName.c_str());
        fileContent.AppendFormat("{\n");
        fileContent.AppendFormat("    _Clear();\n");
        fileContent.AppendFormat("}\n");

        {// _Clear()
            fileContent.AppendFormat("\n");
            fileContent.AppendFormat("void %s::_Clear()\n", mgrClassName.c_str());
            fileContent.AppendFormat("{\n");
            fileContent.AppendFormat("    _dataMd5.clear();\n");

            // 字典释放
            for(auto fieldInfo : configInfo->_fieldInfos)
            {
                if(!fieldInfo)
                    continue;

                if(!fieldInfo->_flags.empty())
                {
                    auto flags = fieldInfo->_flags.Split("|");
                    for(auto flag : flags)
                    {
                        flag.strip();
                        flag = flag.tolower();
                        const auto &fieldName = fieldInfo->_fieldName.FirstCharToLower();
                        if(flag == "unique")
                        {// 唯一索引
                            fileContent.AppendFormat("    _%sRefConfig.clear();\n", fieldName.c_str());
                            fileContent.AppendFormat("\n");
                        }
                        else if(flag == "index")
                        {// 普通索引
                            fileContent.AppendFormat("    _%sRefConfigs.clear();\n", fieldName.c_str());
                            fileContent.AppendFormat("\n");
                        }
                    }
                }
            }

            fileContent.AppendFormat("\n");
            fileContent.AppendFormat("    KERNEL_NS::ContainerUtil::DelContainer(_configs, [](%s *ptr){\n", className.c_str());
            fileContent.AppendFormat("        %s::Delete_%s(ptr);\n", className.c_str(), className.c_str());
            fileContent.AppendFormat("    });\n");
            fileContent.AppendFormat("}\n");
        }
    }

    {// _ReadConfigData
        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("Int64 %s::_ReadConfigData(FILE &fp, KERNEL_NS::LibString &configData, Int32 totalLine, Int32 curLine) const\n", mgrClassName.c_str());
        fileContent.AppendFormat("{\n");
        fileContent.AppendFormat("    // read all column data as one line data, it need check if lack. error data format return -1, return zeror if have no data, return > 0 as readBytes\n");
        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("    Int64 readBytes = 0;\n");
        fileContent.AppendFormat("    KERNEL_NS::LibString content;\n");
        fileContent.AppendFormat("    std::set<Int32> needFieldIds = ");
        fileContent.AppendFormat("{");

        bool isFirst = true;
        Int32 fieldNum = 0;
        for(auto fieldInfo : configInfo->_fieldInfos)
        {
            if(!fieldInfo)
                continue;

            ++fieldNum;
            if(isFirst)
            {
                fileContent.AppendFormat("%llu", fieldInfo->_columnId);
                isFirst = false;
            }
            else
            {
                fileContent.AppendFormat(", %llu", fieldInfo->_columnId);
            }
        }
        fileContent.AppendFormat("};\n");

        fileContent.AppendFormat("    const Int32 fieldNum = %d;\n", fieldNum);
        fileContent.AppendFormat("    Int32 count = 0;\n");
        fileContent.AppendFormat("    while(true)\n");
        fileContent.AppendFormat("    {\n");
        fileContent.AppendFormat("        Int64 bytesOnce = static_cast<Int64>(KERNEL_NS::FileUtil::ReadFile(fp, content, 1));\n");
        fileContent.AppendFormat("        if(bytesOnce == 0)\n");
        fileContent.AppendFormat("            break;\n");
        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("        readBytes += bytesOnce;\n");
        fileContent.AppendFormat("        if(content.Contain(\":\"))\n");
        fileContent.AppendFormat("        {\n");
        fileContent.AppendFormat("            const auto symbolPos = content.GetRaw().find_first_of(\":\", 0);\n");
        fileContent.AppendFormat("            const KERNEL_NS::LibString fieldHeader = content.GetRaw().substr(0, symbolPos);\n");
        fileContent.AppendFormat("            const auto &headerCache = fieldHeader.strip();\n");
        fileContent.AppendFormat("            const auto &headerParts = headerCache.Split(\'_\');\n");
        fileContent.AppendFormat("            if(headerParts.size() < 3)\n");
        fileContent.AppendFormat("            {\n");
        fileContent.AppendFormat("                g_Log->Warn(LOGFMT_OBJ_TAG(\"column field header format error\"));\n");
        fileContent.AppendFormat("                g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), KERNEL_NS::LibString(\"headerCache:\"), headerCache, KERNEL_NS::LibString(\", content:\"), content);\n");
        fileContent.AppendFormat("                return -1;\n");
        fileContent.AppendFormat("            }\n");
        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("            const Int32 fieldColumnId = KERNEL_NS::StringUtil::StringToInt32(headerParts[1].c_str());\n");
        fileContent.AppendFormat("            const Int64 fieldDataLen = KERNEL_NS::StringUtil::StringToInt64(headerParts[2].c_str());\n");
        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("            bytesOnce = static_cast<Int64>(KERNEL_NS::FileUtil::ReadFile(fp, content, fieldDataLen));\n");
        fileContent.AppendFormat("            if(bytesOnce != fieldDataLen)\n");
        fileContent.AppendFormat("            {\n");
        fileContent.AppendFormat("                g_Log->Warn(LOGFMT_OBJ_TAG(\"column data error:\"));\n");
        fileContent.AppendFormat("                g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), KERNEL_NS::LibString(\"headerCache:\"), headerCache, KERNEL_NS::LibString(\", content:\"), content, KERNEL_NS::LibString(\", fieldDataLen:\"), fieldDataLen, KERNEL_NS::LibString(\", real len:\"), bytesOnce, KERNEL_NS::LibString(\", not enough.\"));\n");
        fileContent.AppendFormat("                return -1;\n");
        fileContent.AppendFormat("            }\n");
        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("            readBytes += bytesOnce;\n");
        fileContent.AppendFormat("            configData += content;\n");
        fileContent.AppendFormat("            content.clear();\n");
        fileContent.AppendFormat("            needFieldIds.erase(fieldColumnId);\n");
        fileContent.AppendFormat("            ++count;\n");
        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("            if(((count != fieldNum) && needFieldIds.empty()) ||\n");
        fileContent.AppendFormat("                ((count == fieldNum) && !needFieldIds.empty()))\n");
        fileContent.AppendFormat("            {\n");
        fileContent.AppendFormat("                g_Log->Warn(LOGFMT_OBJ_TAG(\"column data error: field maybe changed count:%%d, need fieldNum:%%d column fieldIds not empty left:%%d\"), count, fieldNum, static_cast<Int32>(needFieldIds.size()));\n");
        fileContent.AppendFormat("                g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), KERNEL_NS::LibString(\"configData:\"), configData);\n");
        fileContent.AppendFormat("                return -1;\n");
        fileContent.AppendFormat("            }\n");
        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("            // read final line data all\n");
        fileContent.AppendFormat("            if(curLine == totalLine && ((count == fieldNum) && needFieldIds.empty()))\n");
        fileContent.AppendFormat("            {\n");
        fileContent.AppendFormat("                bytesOnce = static_cast<Int64>(KERNEL_NS::FileUtil::ReadFile(fp, content));\n");
        fileContent.AppendFormat("                readBytes += bytesOnce;\n");
        fileContent.AppendFormat("                configData += content;\n");
        fileContent.AppendFormat("                content.clear();\n");
        fileContent.AppendFormat("            };\n");
        fileContent.AppendFormat("\n");

        fileContent.AppendFormat("            if((count == fieldNum) && needFieldIds.empty())\n");
        fileContent.AppendFormat("            {\n");
        fileContent.AppendFormat("                return readBytes;\n");
        fileContent.AppendFormat("            }\n");

        fileContent.AppendFormat("        }\n");
        fileContent.AppendFormat("    }\n");
        fileContent.AppendFormat("    g_Log->Warn(LOGFMT_OBJ_TAG(\"column data error: field maybe changed count:%%d, need fieldNum:%%d column fieldIds not empty left:%%d\"), count, fieldNum, static_cast<Int32>(needFieldIds.size()));\n");
        fileContent.AppendFormat("    g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), KERNEL_NS::LibString(\"configData:\"), configData);\n");
        fileContent.AppendFormat("\n");
        fileContent.AppendFormat("    return -1;\n");

        fileContent.AppendFormat("}\n");
    }

    fileContent.AppendFormat("\n");
    fileContent.AppendFormat("SERVICE_END\n");

    return true;
}

bool XlsxExporterMgr::_ExportCppDatas(const XlsxConfigTableInfo *configInfo, KERNEL_NS::LibString &configDataContent) const
{
    // 每一行数据都是文本数据
    // 每个单元格的数据前缀：column_id_datalen:xxx|
    // column_前缀，列id，数据长度:数据|column_前缀,....
    // column_1_10:1010105555|column_2_10:aaaaaaaaaa|...

    // 第一行是字段拼接:
    KERNEL_NS::LibString dataParts;
    for(auto fieldInfo : configInfo->_fieldInfos)
    {
        if(!fieldInfo)
        {
            continue;
        }

        configDataContent.AppendFormat("%s|", fieldInfo->_fieldName.c_str());
    }
    configDataContent.AppendFormat("\n");

    // 枚举类型转数值
    std::map<KERNEL_NS::LibString, std::map<KERNEL_NS::LibString, Int32>> fieldNameRefEnumNameRefValue;
    for(auto fieldInfo :  configInfo->_fieldInfos)
    {
        if(fieldInfo == NULL)
            continue;

        const Int32 count = static_cast<Int32>(configInfo->_values.size());
        Int32 enumaValue = 0;
        for(Int32 idx = 0; idx < count; ++idx)
        {
            auto &columnRefContent = configInfo->_values[idx];
            auto iter = columnRefContent.find(fieldInfo->_columnId);
            if(iter == columnRefContent.end())
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("cant find field data, column id:%llu config:%s, sheet name:%s file name:%s, field name:%s, field data type:%s")
                        , fieldInfo->_columnId, configInfo->_tableClassName.c_str(), configInfo->_wholeSheetName.c_str(), configInfo->_xlsxPath.c_str()
                        , fieldInfo->_fieldName.c_str(), fieldInfo->_dataType.c_str());
                continue;
            }

            auto iterEnumRefValue = fieldNameRefEnumNameRefValue.find(fieldInfo->_fieldName);
            if(iterEnumRefValue == fieldNameRefEnumNameRefValue.end())
                iterEnumRefValue = fieldNameRefEnumNameRefValue.insert(std::make_pair(fieldInfo->_fieldName, std::map<KERNEL_NS::LibString, Int32>())).first;

            iterEnumRefValue->second.insert(std::make_pair(iter->second.strip(), ++enumaValue));
        }
    }
    
    const Int32 lineCount = static_cast<Int32>(configInfo->_values.size());
    Int32 totalLine = 2;
    for(Int32 idx = 0; idx < lineCount; ++idx)
    {
        auto &columnIdRefContent = configInfo->_values[idx];
        if(columnIdRefContent.empty())
            continue;

        for(auto iterContent : columnIdRefContent)
        {
            const UInt64 columnId = iterContent.first;
            const auto &content = iterContent.second;
            auto fieldInfo = configInfo->_fieldInfos[columnId];
            if(!fieldInfo)
            {
                g_Log->Error(LOGFMT_OBJ_TAG("line:%d, columnId:%llu, have no fieldinfo content:%s, _tableClassName:%s"), idx, columnId, content.c_str(), configInfo->_tableClassName.c_str());
                return false;
            }

            // 导出数据
            KERNEL_NS::LibString dataContent;
            if(SERVICE_COMMON_NS::DataTypeHelper::IsSimpleType(fieldInfo->_dataType))
            {
                KERNEL_NS::LibString value;
                if(content.length() == 0)
                {
                    value = SERVICE_COMMON_NS::DataTypeHelper::GetTypeDefaultValue(fieldInfo->_dataType);
                }
                else
                {
                    value = content;
                }

                if(SERVICE_COMMON_NS::DataTypeHelper::IsEnum(fieldInfo->_dataType))
                {
                    auto iterEnumRefValue = fieldNameRefEnumNameRefValue.find(fieldInfo->_fieldName);
                    auto &enumRefValue = iterEnumRefValue->second;
                    auto iterValue = enumRefValue.find(content.strip());
                    value = KERNEL_NS::StringUtil::Num2Str(iterValue->second);
                }

                SERVICE_COMMON_NS::DataTypeHelper::ToSimpleTypeString(fieldInfo->_dataType, value, dataContent);
            }
            else
            {
                if(SERVICE_COMMON_NS::DataTypeHelper::IsArray(fieldInfo->_dataType))
                {
                    if(content.length() == 0)
                    {
                        dataContent = "[]";
                    }
                    else
                    {
                        dataContent = content;
                    }
                }
                else if(SERVICE_COMMON_NS::DataTypeHelper::IsDict(fieldInfo->_dataType))
                {
                    if(content.length() == 0)
                    {
                        dataContent = "{}";
                    }
                    else
                    {
                        dataContent = content;
                    }
                }
                else
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("unknown data type:%s line:%d, columnId:%llu, content:%s _tableClassName:%s")
                            , fieldInfo->_dataType.c_str(), idx, columnId, content.c_str(), configInfo->_tableClassName.c_str());

                    return false;
                }
            }

            // 拼装数据
            dataParts.AppendFormat("column_%llu_%llu:", columnId, static_cast<UInt64>(dataContent.size()));
            dataParts += dataContent;
            dataParts += "|";
        }

        dataParts.AppendFormat("\n");
        ++totalLine;
    }

    configDataContent.AppendFormat("%d\n", totalLine);
    configDataContent += dataParts;

    return true;
}

bool XlsxExporterMgr::_ExportMetaFile(const KERNEL_NS::LibString &xlsxPath) const
{
    const auto xlsxFile = KERNEL_NS::DirectoryUtil::GetFileNameInPath(xlsxPath.c_str());
    const auto xlsxWithoutExtention = KERNEL_NS::FileUtil::ExtractFileWithoutExtension(xlsxFile);
    const auto xlsxDir = KERNEL_NS::DirectoryUtil::GetFileDirInPath(xlsxPath.c_str());
    auto relationPath = xlsxDir - _sourceDir;
    if(relationPath.empty() || relationPath.at(relationPath.length() - 1) != '/')
        relationPath.AppendFormat("/");

    const auto metaFile = _metaDir + relationPath + xlsxWithoutExtention + ".meta";
    if(KERNEL_NS::FileUtil::IsFileExist(metaFile.c_str()))
        KERNEL_NS::FileUtil::DelFileCStyle(metaFile.c_str());

    {// 实现创建路径
        const auto dir =  KERNEL_NS::DirectoryUtil::GetFileDirInPath(metaFile);
        if(dir.empty())
        {
            g_Log->Error(LOGFMT_OBJ_TAG("bad file name:%s"), metaFile.c_str());
            return false;
        }
        if(!KERNEL_NS::DirectoryUtil::CreateDir(dir))
        {
            return false;
        }
    }

    // XlsxFile:xxxfile.xlsx\n Md5:xxx

    KERNEL_NS::LibString md5;
    if(!_CalcMd5(xlsxPath, md5))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("calc xlsx md5 fail xlsx path:%s, metaFile:%s"), xlsxPath.c_str(), metaFile.c_str());
        return false;
    }

    KERNEL_NS::LibString metaFileRefContent;
    metaFileRefContent.AppendFormat("XlsxFile:%s", xlsxFile.c_str());
    metaFileRefContent += "\n";
    metaFileRefContent.AppendFormat("Md5:%s", md5.c_str());

    {// 写入meta文件
        KERNEL_NS::SmartPtr<FILE, KERNEL_NS::AutoDelMethods::CustomDelete> fp =  KERNEL_NS::FileUtil::OpenFile(metaFile.c_str(), true);
        if(!fp)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("open data file fail when export cpp config xlsxPath:%s, metaFile:%s content:%s.")
                        , xlsxPath.c_str(), metaFile.c_str(), metaFileRefContent.c_str());
            return false;
        }

        fp.SetClosureDelegate([](void *p){
            auto ptr = reinterpret_cast<FILE *>(p);
            KERNEL_NS::FileUtil::CloseFile(*ptr);
        });

        auto bytesChange = KERNEL_NS::FileUtil::WriteFile(*fp, metaFileRefContent);
        if(bytesChange != static_cast<Int64>(metaFileRefContent.size()))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("write meta file bytesChange:%lld, content size:%llu,  xlsxPath:%s, metaFile:%s content:%s.")
                        , bytesChange, static_cast<Int64>(metaFileRefContent.size()), xlsxPath.c_str(), metaFile.c_str(), metaFileRefContent.c_str());
            return false;
        }
    }

    return true;
}

void XlsxExporterMgr::_ExportCppAllConfigHeaderFile(const KERNEL_NS::LibString &lang) const
{
    if(_dirtyXlsxFiles.empty())
        return;

    // 导出allconfigs.h
    const auto rootPath = _targetDir + "/" + lang + "/";
     g_Log->Custom("start export AllConfigs.h file to dir:%s...", rootPath.c_str());

    bool isSuc = true;
    KERNEL_NS::LibString content;
    content.AppendFormat("// Generate by %s, Dont modify it!!!\n", GetApp()->GetAppName().c_str());

    const auto allConfigsName = KERNEL_NS::FileUtil::ExtractFileWithoutExtension(_allConfigsHeader);
    content.AppendFormat("#ifndef __CONFIG_%s_H__\n", allConfigsName.toupper().c_str());
    content.AppendFormat("#define __CONFIG_%s_H__\n", allConfigsName.toupper().c_str());

    content.AppendFormat("\n");

    KERNEL_NS::DirectoryUtil::TraverseDirRecursively(rootPath, [this, &content, &isSuc](const KERNEL_NS::FindFileInfo &fileInfo, bool &isParentDirContinue){
        
        bool isContinue = true;
        do
        {
            // 过滤目录
            if(KERNEL_NS::FileUtil::IsDir(fileInfo))
                break;

            // 过滤.h文件
            if(KERNEL_NS::FileUtil::ExtractFileExtension(fileInfo._fileName) != KERNEL_NS::LibString(".h"))
                break;

            // 过滤AllConfigs.h和RegesterAllConfigs.h
            if(fileInfo._fileName.Contain(_allConfigsHeader) || fileInfo._fileName.Contain(_registerAllConfigs))
                continue;

            KERNEL_NS::LibString fullPath = fileInfo._rootPath;
            auto targetDir = _targetDir;
            if(targetDir.empty() || targetDir[targetDir.length() - 1] != '/')
                targetDir.AppendFormat("/");

            auto &&relationDir = fullPath - targetDir;
            if(!relationDir.empty() && relationDir.at(relationDir.length() - 1) != '/')
            {
                relationDir.AppendFormat("/");
            }

            content.AppendFormat("#include <%s%s>\n", relationDir.c_str(), fileInfo._fileName.c_str());
        } while (false);
        
        return isContinue;
    });

    if(!isSuc)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("export allconfigs.h fail targetDir:%s."), _targetDir.c_str());
        return;
    }

    content.AppendFormat("#endif\n");

    const auto allConfigs = rootPath + _allConfigsHeader;
    if(KERNEL_NS::FileUtil::IsFileExist(allConfigs.c_str()))
        KERNEL_NS::FileUtil::DelFileCStyle(allConfigs.c_str());

    {// 写入文件
        KERNEL_NS::SmartPtr<FILE, KERNEL_NS::AutoDelMethods::CustomDelete> fp =  KERNEL_NS::FileUtil::OpenFile(allConfigs.c_str(), true);
        if(!fp)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("open data file fail when export cpp config allConfigs:%s.")
                        , allConfigs.c_str());
            return;
        }

        fp.SetClosureDelegate([](void *p){
            auto ptr = reinterpret_cast<FILE *>(p);
            KERNEL_NS::FileUtil::CloseFile(*ptr);
        });

        auto bytesChange = KERNEL_NS::FileUtil::WriteFile(*fp, content);
        if(bytesChange != static_cast<Int64>(content.size()))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("write allconfigs file:[%s] bytesChange:%lld, content size:%llu")
                        , allConfigs.c_str(), bytesChange, static_cast<Int64>(content.size()));
            return;
        }
    }
}

void XlsxExporterMgr::_ExportCppRegisterConfigs(const KERNEL_NS::LibString &lang) const
{
    if(_dirtyXlsxFiles.empty())
        return;

    // 导出allconfigs.h
    const auto rootPath = _targetDir + "/" + lang + "/";
     g_Log->Custom("start export %s file to dir:%s...", _registerAllConfigs.c_str(), rootPath.c_str());

    bool isSuc = true;
    KERNEL_NS::LibString content;
    content.AppendFormat("// Generate by %s, Dont modify it!!!\n", GetApp()->GetAppName().c_str());

    const auto registerConfigsName = KERNEL_NS::FileUtil::ExtractFileWithoutExtension(_registerAllConfigs);
    content.AppendFormat("\n");

    KERNEL_NS::DirectoryUtil::TraverseDirRecursively(rootPath, [this, &content, &isSuc](const KERNEL_NS::FindFileInfo &fileInfo, bool &isParentDirContinue){
        
        bool isContinue = true;
        do
        {
            // 过滤目录
            if(KERNEL_NS::FileUtil::IsDir(fileInfo))
                break;

            // 过滤.h文件
            if(KERNEL_NS::FileUtil::ExtractFileExtension(fileInfo._fileName) != KERNEL_NS::LibString(".h"))
                break;

            // 过滤AllConfigs.h和RegesterAllConfigs.h
            if(fileInfo._fileName.Contain(_allConfigsHeader) || fileInfo._fileName.Contain(_registerAllConfigs))
                continue;

            const auto config = KERNEL_NS::FileUtil::ExtractFileWithoutExtension(fileInfo._fileName);

            content.AppendFormat("RegisterComp<SERVICE_NS::%sMgrFactory>();\n", config.c_str());
        } while (false);
        
        return isContinue;
    });

    if(!isSuc)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("export allconfigs.h fail targetDir:%s."), _targetDir.c_str());
        return;
    }

    const auto file = rootPath + _registerAllConfigs;
    if(KERNEL_NS::FileUtil::IsFileExist(file.c_str()))
        KERNEL_NS::FileUtil::DelFileCStyle(file.c_str());

    {// 写入文件
        KERNEL_NS::SmartPtr<FILE, KERNEL_NS::AutoDelMethods::CustomDelete> fp =  KERNEL_NS::FileUtil::OpenFile(file.c_str(), true);
        if(!fp)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("open data file fail when export cpp config allConfigs:%s.")
                        , file.c_str());
            return;
        }

        fp.SetClosureDelegate([](void *p){
            auto ptr = reinterpret_cast<FILE *>(p);
            KERNEL_NS::FileUtil::CloseFile(*ptr);
        });

        auto bytesChange = KERNEL_NS::FileUtil::WriteFile(*fp, content);
        if(bytesChange != static_cast<Int64>(content.size()))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("write allconfigs file:[%s] bytesChange:%lld, content size:%llu")
                        , file.c_str(), bytesChange, static_cast<Int64>(content.size()));
            return;
        }
    }
}

bool XlsxExporterMgr::_ExportCSharpCode(const std::map<KERNEL_NS::LibString, XlsxConfigTableInfo *> &configTypeRefConfigTableInfo) const
{
    return true;
}

bool XlsxExporterMgr::_ExportCSharpDatas(const std::map<KERNEL_NS::LibString, XlsxConfigTableInfo *> &configTypeRefConfigTableInfo) const
{
    return true;
}

bool XlsxExporterMgr::_UpdateMetas()
{
    return true;
}

void XlsxExporterMgr::_Clear()
{
    _UnRegisterEvents();

    _ownTypeRefLangTypes.clear();
    KERNEL_NS::ContainerUtil::DelContainer(_metaNameRefConfigMetaInfo, [](XlsxConfigMetaInfo *ptr){
        XlsxConfigMetaInfo::Delete_XlsxConfigMetaInfo(ptr);
    });
    _needExportConfigType.clear();
    _dirtyXlsxFiles.clear();

    _configTypeRefSheets.clear();
    KERNEL_NS::ContainerUtil::DelContainer(_xlsxFileRefWorkbook, [](KERNEL_NS::XlsxWorkbook *ptr){
        KERNEL_NS::XlsxWorkbook::DeleteThreadLocal_XlsxWorkbook(ptr);
    });

    KERNEL_NS::ContainerUtil::DelContainer(_ownTypeRefConfigTypeRefXlsxConfigTableInfo, [](std::map<KERNEL_NS::LibString, XlsxConfigTableInfo *> &configTypeRefTable){
        KERNEL_NS::ContainerUtil::DelContainer(configTypeRefTable, [](XlsxConfigTableInfo *ptr){
            XlsxConfigTableInfo::Delete_XlsxConfigTableInfo(ptr);
        });
    });
}

void XlsxExporterMgr::_RegisterEvents()
{
    if(_closeServiceStub == INVALID_LISTENER_STUB)
        _closeServiceStub = GetEventMgr()->AddListener(EventEnums::QUIT_SERVICE_EVENT, this, &XlsxExporterMgr::_CloseServiceEvent);
}

void XlsxExporterMgr::_UnRegisterEvents()
{
    if(_closeServiceStub != INVALID_LISTENER_STUB)
        GetEventMgr()->RemoveListenerX(_closeServiceStub);
}

void XlsxExporterMgr::_CloseServiceEvent(KERNEL_NS::LibEvent *ev)
{
    GetService()->MaskServiceModuleQuitFlag(this);
}


SERVICE_END