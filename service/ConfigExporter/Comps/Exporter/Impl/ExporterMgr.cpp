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
#include <service/ConfigExporter/Comps/Exporter/Impl/ExporterMgr.h>
#include <service/ConfigExporter/Comps/Exporter/Impl/ExporterMgrFactory.h>
#include <service/ConfigExporter/Comps/Exporter/Impl/ConfigInfo.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(IExporterMgr);

POOL_CREATE_OBJ_DEFAULT_IMPL(ExporterMgr);

ExporterMgr::ExporterMgr()
{

}

ExporterMgr::~ExporterMgr()
{
    _Clear();
}

void ExporterMgr::Release()
{
    ExporterMgr::DeleteByAdapter_ExporterMgr(ExporterMgrFactory::_buildType.V, this);
}

KERNEL_NS::LibString ExporterMgr::ToString() const
{
    return IExporterMgr::ToString();
}

Int32 ExporterMgr::_OnGlobalSysInit() 
{
    _RegisterEvents();

    auto timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
    timer->SetTimeOutHandler(this, &ExporterMgr::_OnExporter);
    timer->Schedule(0);

    // 1.读取所有配置表数据
    // 2.读取表头
    // 3.生成配置

    return Status::Success;
}

void ExporterMgr::_OnGlobalSysClose()
{
    _Clear();
}

void ExporterMgr::_OnExporter(KERNEL_NS::LibTimer *t)
{
    bool genSuc = false;
    auto app = GetApp();
    const auto &appArgs = app->GetAppArgs();
    
    // ConfigExporter --lang=S:cpp|C:C#,lua --source_dir=/xxx/ --target_dir=/xxx/ --data=/xx/ --meta=/xxx/

    // target字典
    KERNEL_NS::LibString sourceDir;
    KERNEL_NS::LibString targetDir;
    KERNEL_NS::LibString dataDir;
    KERNEL_NS::LibString metaDir;
    std::unordered_map<KERNEL_NS::LibString, std::unordered_set<KERNEL_NS::LibString>> configTypeRefLangTypes;

    // 1.传入的参数
    const Int32 argCount = static_cast<Int32>(appArgs.size());
    for(Int32 idx = 0; idx < argCount; ++idx)
    {
        const auto &arg = appArgs[idx];
        auto kv = arg.Split("=");
        if(kv.empty())
            continue;

        if(kv.size() < 2)
            continue;

        auto &k = kv[0];
        k.strip();
        KERNEL_NS::LibString &v = kv[1];
        if(k.empty())
            continue;

        v.strip();

        // 生成的语言版本
        if(k == "--lang")
        {
            auto configTypeRefLangsArr = v.Split("|");
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

                    auto iter = configTypeRefLangTypes.find(configType);
                    if(iter == configTypeRefLangTypes.end())
                        iter = configTypeRefLangTypes.insert(std::make_pair(configType, std::unordered_set<KERNEL_NS::LibString>())).first;
                    auto &langsSet = iter->second; 

                    for(auto &lang : langsPieces)
                        langsSet.insert(lang);    
                }
            }
        }
        else if(k == "--source_dir")
        {
            sourceDir = v;
        }
        else if(k == "target_dir")
        {
            targetDir = v;
        }
        else if(k == "--data")
        {
            dataDir = v;
        }
        else if(k == "--meta")
        {
            metaDir = v;
        }
        else
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("unknown param:k:%s, v:%s"), k.c_str(), v.c_str());
        }
    }
    
    
    // 1.扫描所有xlsx文件以页签的配置类型名为key生成需要处理文件的字典
    do
    {
        if(configTypeRefLangTypes.empty())
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("have no lang types"));
            genSuc = false;
            break;
        }

        if(sourceDir.empty() || targetDir.empty() || dataDir.empty() || metaDir.empty())
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("sourceDir:%s, targetDir:%s, dataDir:%s metaDir:%s error.")
                        , sourceDir.c_str(), targetDir.c_str(), dataDir.c_str(), metaDir.c_str());
            genSuc = false;
            break;
        }

        if(!_ScanMeta(metaDir, sourceDir))
        {
            genSuc = false;
            break;
        }


        // auto nowTs = KERNEL_NS::LibTime::Now();
        // auto traverseCallback = [this, &nowTs] (const KERNEL_NS::FindFileInfo &fileInfo, bool &isParentPathContinue) -> bool {

    //     bool isContinue = true;
    //     do
    //     {
    //         // 过滤目录
    //         if(KERNEL_NS::FileUtil::IsDir(fileInfo))
    //             break;

    //         // 过滤非xlsx文件
    //         if(KERNEL_NS::FileUtil::ExtractFileExtension(fileInfo._fileName) != KERNEL_NS::LibString(".xlsx"))
    //             break;

    //         KERNEL_NS::LibString xlsxRootPath = fileInfo._rootPath;
    //         if(fileInfo._rootPath.at(fileInfo._rootPath.length() - 1) != '/')
    //             xlsxRootPath.AppendFormat("/");

    //         const auto &fullFilePath = xlsxRootPath + fileInfo._fileName;

    //         // 拿pbcache中的缓存数据
    //         auto iterPbCacheFile = _pbCacheContent->_protoPathRefFileInfo.find(fullFilePath);
    //         if(iterPbCacheFile != _pbCacheContent->_protoPathRefFileInfo.end())
    //         {
    //             auto pbCacheFile = iterPbCacheFile->second;
    //             // 修改时间没变则不需要重新解析
    //             if(!_forceGenAll)
    //             {
    //                 if(pbCacheFile->_modifyTime == fileInfo._modifyTime)
    //                     break;
    //             }
    //         }

    //         // md5有变化才会被扫描到
    //         if(!_ScanAProto(fileInfo, fullFilePath, isParentPathContinue))
    //         {
    //             isContinue = false;
    //             break;
    //         }

    //     } while (false);
        
    //     return isContinue;
    // };

    // auto delg = KERNEL_CREATE_CLOSURE_DELEGATE(traverseCallback, bool, const KERNEL_NS::FindFileInfo &, bool &);

    //     // 2.加载xlsx文件
    //     // KERNEL_NS::DirectoryUtil::TraverseDirRecursively(sourceDir, )

    //     g_Log->Custom("[CONFIG GEN] START.");

    //     // 1.读取所有配置数据
        
    //     g_Log->Custom("[CONFIG GEN] END.");

     }while (false);

    // 4.关闭app
    Int32 err = genSuc ? Status::Success : Status::Failed;
    GetServiceProxy()->CloseApp(err);
    
    KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
}

// 测试点: 加载meta成功, meta中没有对应的xlsx文件名, 或者xlsx文件不存在则清理meta， md5为空则清理meta
// 注意:xlsx所在路径相对于xlsx的base路径的相对路径与meta文件相对于meta base 路径的相对路径是一致的
bool ExporterMgr::_ScanMeta(const KERNEL_NS::LibString &metaDir, const KERNEL_NS::LibString &xlsxBasePath)
{
    bool isSuc = true;
    KERNEL_NS::DirectoryUtil::TraverseDirRecursively(&metaDir, [this, &isSuc](const KERNEL_NS::FindFileInfo &fileInfo, bool &isParentDirContinue){
        
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
            auto newMeta = ConfigMetaInfo::New_ConfigMetaInfo();
            auto ptr = KERNEL_NS::FileUtil::OpenFile(fullFilePath.c_str());
            if(!ptr)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("open meta file fail:%s"), fullFilePath.c_str());
                isContinue = false;
                isParentDirContinue = false;
                isSuc = false;
                break;
            }

            KERNEL_NS::SmartPtr<FILE> fp(ptr);
            fp.SetClosureDelegate([](void *p){
                auto ptr = reinterpret_cast<FILE *>(p);
                KERNEL_NS::FileUtil::CloseFile(*ptr);
            });

            newMeta->_metaRootPath = fileInfo._rootPath;
            newMeta->_metaFileName = fileInfo._fileName;
            newMeta->_relationPath = fileInfo._rootPath - metaDir;
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
                    , xlsxFullPath.c_str(), metaFilePath.c_str()); 

            KERNEL_NS::FileUtil::DelFileCStyle(metaFilePath.c_str());
            ConfigMetaInfo::Delete_ConfigMetaInfo(metaInfo);
            iter = _metaNameRefConfigMetaInfo.erase(iter);
            continue;
        }
        else
        {
            const auto xlsxFullPath = xlsxBasePath + metaInfo->_relationPath + metaInfo->_xlsxFileName;
            if(!KERNEL_NS::FileUtil::IsFileExist(xlsxFullPath.c_str()))
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("xlsx file not found:%s, will remove meta file, metaFilePath:%s.")
                    , xlsxFullPath.c_str(), metaFilePath.c_str());    
            }

            ConfigMetaInfo::Delete_ConfigMetaInfo(metaInfo);
            KERNEL_NS::FileUtil::DelFileCStyle(metaFilePath.c_str());
            iter = _metaNameRefConfigMetaInfo.erase(iter);

            continue;
        }
        
        // 2.md5是空的则删除meta文件
        if(metaInfo->_lastMd5.empty())
        {
            ConfigMetaInfo::Delete_ConfigMetaInfo(metaInfo);
            KERNEL_NS::FileUtil::DelFileCStyle(metaFilePath.c_str());
            iter = _metaNameRefConfigMetaInfo.erase(iter);
            continue;
        }

        ++iter;
    }

    return isSuc;
}

void ExporterMgr::_Clear()
{
    _UnRegisterEvents();

    KERNEL_NS::ContainerUtil::DelContainer(_metaNameRefConfigMetaInfo, [](ConfigMetaInfo *ptr){
        ConfigMetaInfo::Delete_ConfigMetaInfo(ptr);
    });
}

void ExporterMgr::_RegisterEvents()
{

}

void ExporterMgr::_UnRegisterEvents()
{

}


SERVICE_END