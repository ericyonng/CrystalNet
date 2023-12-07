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
 * Date: 2022-10-11 23:42:09
 * Author: Eric Yonng
 * Description: 
*/


#include "pch.h"
#include <regex>
#include <service_common/ServiceCommon.h>
#include <service/common/common.h>
#include <service/ProtoGenService/Common/ServiceCommon.h>

#include <service/ProtoGenService/Comps/Exporter/Impl/ExporterMgrFactory.h>
#include <service/ProtoGenService/Comps/Exporter/Defs/Defs.h>
#include <service/ProtoGenService/Comps/Exporter/Defs/ProtoContentInfo.h>
#include <service/ProtoGenService/Comps/Exporter/Defs/MessageInfo.h>
#include <service/ProtoGenService/Comps/Exporter/Defs/PbCacheInfoFormat.h>
#include <service/ProtoGenService/Comps/Exporter/Impl/ProtobuffHelper.h>

#include <service/ProtoGenService/Comps/Exporter/Impl/ExporterMgr.h>

#include <OptionComp/CodeAnalyze/CodeAnalyze.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(IExporterMgr);

POOL_CREATE_OBJ_DEFAULT_IMPL(ExporterMgr);

ExporterMgr::ExporterMgr()
:
_forceGenAll(false)
,_maxOpcode(0)
,_pbCacheContent(NULL)
,_closeServiceStub(INVALID_LISTENER_STUB)
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
    // 解析参数
    auto &appArgs = GetServiceProxy()->GetApp()->GetAppArgs();
    for(auto &arg : appArgs)
    {
        g_Log->Info(LOGFMT_OBJ_TAG("arg:%s"), arg.c_str());
        auto seps = arg.Split("=");
        if(seps.empty())
            continue;

        if(seps.size() != 2)
        {
            g_Log->Debug(LOGFMT_OBJ_TAG("protogentool param format error, arg:%s"), arg.c_str());
            continue;
        }

        auto key = seps[0].strip();
        auto value = seps[1].strip();

        if(key == "--proto_path")
            _protoPath = value;
        else if(key == "--cpp_out")
            _cppOutPath = value;
        else if(key == "--csharp_out")
            _csharpOutPath = value;
        else if(key == "--cpp_protoc")
        {
            _cppProtocPath = value;
        }
        else if(key == "--csharp_protoc")
        {
            _csharpProtocPath = value;
        }
        else if(key == "--ts_out")
        {
            _tsOutPath = value;
        }
        else if(key == "--force_gen_all")
        {
            _forceGenAll = true;
        }
        else if(key == "--base_path")
        {
            _basePath = value;
        }
        else if(key == "--protocols_path")
        {
            _protocolsPath = value;
        }
        else if(key == "--google_proto_include_path")
        {
            _googleProtoIncludePath = value;
        }
        else
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("unknown key:%s, value:%s"), key.c_str(), value.c_str());
        }
    }

    if(_basePath.empty())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("have no base path"));
        return Status::Failed;
    }

    if (_protocolsPath.empty())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("have no protocols work path"));
        return Status::Failed;
    }

    if(_protoPath.empty() || !KERNEL_NS::DirectoryUtil::IsDirExists(_protoPath))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("proto path not exists:%s"), _protoPath.c_str());
        return Status::Failed;
    }

    if(_cppOutPath.empty() && _csharpOutPath.empty())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("lack cpp out and csharp out path"));
        return Status::Failed;
    }

    if(!_cppOutPath.empty() && !KERNEL_NS::DirectoryUtil::IsDirExists(_cppOutPath))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("cpp out path not exists:%s"), _cppOutPath.c_str());
        return Status::Failed;
    }

    if(!_csharpOutPath.empty() && !KERNEL_NS::DirectoryUtil::IsDirExists(_csharpOutPath))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("csharp out path not exists:%s"), _csharpOutPath.c_str());
        return Status::Failed;
    }

    if(_cppProtocPath.empty() && _csharpProtocPath.empty())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("have no protoc path"));
        return Status::Failed;
    }

    _pbCacheContent = PbCacheFileContent::New_PbCacheFileContent();

    _protobufBaseDataType.insert("double");
    _protobufBaseDataType.insert("float");
    _protobufBaseDataType.insert("int32");
    _protobufBaseDataType.insert("uint32");
    _protobufBaseDataType.insert("uint64");
    _protobufBaseDataType.insert("sint32");
    _protobufBaseDataType.insert("sint64");
    _protobufBaseDataType.insert("fixed32");
    _protobufBaseDataType.insert("fixed64");
    _protobufBaseDataType.insert("sfixed32");
    _protobufBaseDataType.insert("sfixed64");
    _protobufBaseDataType.insert("bool");
    _protobufBaseDataType.insert("string");
    _protobufBaseDataType.insert("bytes");

    return Status::Success;
}

Int32 ExporterMgr::_OnGlobalSysCompsCreated()
{
    return Status::Success;
}

Int32 ExporterMgr::_OnHostStart()
{
    // 下一帧执行
    auto nextFrame = [this](KERNEL_NS::LibTimer *t)
    {
        bool genSuc = false;
        do
        {
            g_Log->Custom("[PROTO GEN] START.");

            // 1.扫描注解
            if(!_AnalyzeProtoAnnotation())
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("anylyze proto annotation fail."));
                break;
            }

            // 2.生成cpp
            if(!_GenCplusplus())
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("gen c plus plus fail."));
                break;
            }

            // 3.生成csharp
            if(!_GenCSharp())
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("gen csharp fail."));
                break;
            }

            // 4.更新pb cache
            if(!_UpdateProtoCache())
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("update proto cache fail."));
                break;
            }

            _GenOpcodeEnums();
            _GenOpcodeInfo();
            _GenAllPbs();

            // 语法分析
            _GrammarAnalyze();
            
            // 5.生成ts信息
            if(!_GenTs())
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("gen ts fail."));
                break;
            }

            // 生成ts
            _GenTsExtends();

            // 生成ORM相关代码
            _GenORM();

            // 强制更新以及带谷歌proto目录的需要给每个cc添加pch.h 不需要了
            // if(!_googleProtoIncludePath.empty() && _forceGenAll)
            // {
            //     _AddPchHeaderToGoogleCC();
            // }

            genSuc = true;

            g_Log->Custom("[PROTO GEN] END.");

        }while (false);

        // 4.关闭app
        Int32 err = genSuc ? Status::Success : Status::Failed;
        GetServiceProxy()->CloseApp(err);
        
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
    };

    auto timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
    timer->GetMgr()->TakeOverLifeTime(timer, [](KERNEL_NS::LibTimer *t){
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
    });
    timer->SetTimeOutHandler(KERNEL_CREATE_CLOSURE_DELEGATE(nextFrame, void, KERNEL_NS::LibTimer *));
    timer->Schedule(1);

    return Status::Success;
}

void ExporterMgr::_OnGlobalSysClose() 
{
    _Clear();
}

bool ExporterMgr::_AnalyzeProtoAnnotation()
{
    if(_protoPath.empty())
    {
        g_Log->Error(LOGFMT_OBJ_TAG("have no proto path."));
        return false;
    }

    // 打开pbcache 文件 并加载缓存信息
    if(!_LoadPbCache())
    {
        g_Log->Error(LOGFMT_OBJ_TAG("load pb cache fail."));
        return false;
    }

    // 扫描所有proto
    if(!_ScanProtos())
    {
        g_Log->Error(LOGFMT_OBJ_TAG("scan protos fail."));
        return false;
    }

    return true;
}

bool ExporterMgr::_GenCplusplus()
{
    if(_cppOutPath.empty())
        return true;

    if(_protoNameRefProtoInfo.empty())
    {
        g_Log->Custom("None.");
        return true;
    }

    const auto appFullPath = GetServiceProxy()->GetApp()->GetAppPath();
    const auto appPath = KERNEL_NS::DirectoryUtil::GetFileDirInPath(appFullPath);
    const auto appName = GetServiceProxy()->GetApp()->GetAppName();
    auto protocPath = KERNEL_NS::DirectoryUtil::GetFileDirInPath(_cppProtocPath);
    auto protocName = KERNEL_NS::DirectoryUtil::GetFileNameInPath(_cppProtocPath);

    {// 添加执行权限
        Int32 err = 0;
        KERNEL_NS::LibString cmd;

        // 2.0的proto需要绝对路径的proto
        #if CRYSTAL_TARGET_PLATFORM_LINUX
            cmd.AppendFormat("sudo chmod a+x %s"
            , (appPath + protocPath + protocName).c_str());

            KERNEL_NS::LibString outInfo;
            if(!KERNEL_NS::SystemUtil::Exec(cmd, err, outInfo))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("give executable ability to %s fail, cmd:%s, err:%d, outInfo:%s")
                , (appPath + protocPath + protocName).c_str(), cmd.c_str(), err, outInfo.c_str());
                return false;
            }
        #endif
    }

    // 1.protoc 生成原始文件
    const auto coutPath = (appPath + _cppOutPath);
    KERNEL_NS::DirectoryUtil::CreateDir(coutPath + "/");
    for(auto iter : _protoNameRefProtoInfo)
    {
        auto protoInfo = iter.second;
        Int32 err = 0;
        KERNEL_NS::LibString cmd;

        g_Log->Custom("[PROTO GEN CPP] %s ...", protoInfo->_protoInfo._fileName.c_str());

        const auto fullProtoPath = appPath + _protoPath;
        auto rootPath = appPath + protoInfo->_protoInfo._rootPath;
        auto fullPath = appPath + protoInfo->_fullPathName;
        auto relationPath = KERNEL_NS::DirectoryUtil::GetFileDirInPath(rootPath - fullProtoPath + "/");

        KERNEL_NS::DirectoryUtil::CreateDir(appPath + _cppOutPath + relationPath);

        // 2.0的proto需要绝对路径的proto
        #if CRYSTAL_TARGET_PLATFORM_WINDOWS
            cmd.AppendFormat("cd %s && %s --cpp_out=%s --proto_path=%s --proto_path=%s %s"
            , (appPath + protocPath).c_str()
            , (protocName).c_str()
            , (appPath + _cppOutPath + relationPath).c_str()
            , fullProtoPath.c_str()
            , (appPath + protoInfo->_protoInfo._rootPath).c_str()
            // , (appPath + protoInfo->_fullPathName).c_str());
            , protoInfo->_protoInfo._fileName.c_str());
        #else
            cmd.AppendFormat("%s --cpp_out=%s --proto_path=%s --proto_path=%s %s"
            , (appPath + protocPath + protocName).c_str()
            , (appPath + _cppOutPath + relationPath).c_str()
            , fullProtoPath.c_str()
            , (appPath + protoInfo->_protoInfo._rootPath).c_str()
            // , (appPath + protoInfo->_fullPathName).c_str());
            , protoInfo->_protoInfo._fileName.c_str());
        #endif

        KERNEL_NS::LibString outInfo;
        if(!KERNEL_NS::SystemUtil::Exec(cmd, err, outInfo))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("protoc gen fail proto file:%s cmd:%s, err:%d, outInfo:%s")
            , protoInfo->_fullPathName.c_str(), cmd.c_str(), err, outInfo.c_str());
            return false;
        }

        if(err != 0)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("[PROTO GEN:%s FAILED]:%s"), protoInfo->_fullPathName.c_str(), outInfo.c_str());
            return false;
        }

        std::vector<KERNEL_NS::LibString> newFactoryNames;
        {// .pb.h
            // 新的路径 = coutPath + 相对路径名 + 文件名
            auto rootPath = appPath + protoInfo->_protoInfo._rootPath;
            auto fullPath = appPath + protoInfo->_fullPathName;
            const auto fullProtoPath = appPath + _protoPath;
            auto relationPath = KERNEL_NS::DirectoryUtil::GetFileDirInPath(rootPath - fullProtoPath + "/");

            KERNEL_NS::DirectoryUtil::CreateDir(KERNEL_NS::DirectoryUtil::GetFileDirInPath(coutPath + relationPath));
            auto pbHeaderName = KERNEL_NS::FileUtil::ExtractFileWithoutExtension(coutPath + relationPath + protoInfo->_protoInfo._fileName) + ".pb.h";
            KERNEL_NS::SmartPtr<FILE, KERNEL_NS::AutoDelMethods::CustomDelete> fp = KERNEL_NS::FileUtil::OpenFile(pbHeaderName.c_str());
            if(!fp)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("cant open pb header file:%s"), pbHeaderName.c_str());
                return false;
            }

            fp.SetClosureDelegate([](void *ptr){
                auto fpPtr = reinterpret_cast<FILE *>(ptr);
                KERNEL_NS::FileUtil::CloseFile(*fpPtr);
            });


            // 2.1扫描头文件内容
            std::vector<KERNEL_NS::LibString> lineContents;
            auto line = KERNEL_NS::FileUtil::ReadUtf8File(*fp, lineContents);
            g_Log->Info(LOGFMT_OBJ_TAG("pb header:%s line:%lld"), pbHeaderName.c_str(), line);

            if(!_ModifyCppPbHeader(pbHeaderName, lineContents, protoInfo, newFactoryNames))
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("modify cpp line contents fail pb file:%s"), pbHeaderName.c_str());
                return false;
            }

            // 3.关闭文件
            KERNEL_NS::FileUtil::CloseFile(*fp);
            fp.pop();

            // 4.用新内容替换文件
            if(!KERNEL_NS::FileUtil::ReplaceFile(pbHeaderName, lineContents))
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("replace file fail:%s"), pbHeaderName.c_str());
                return false;
            }
        }

        {// .pb.cc
            // 新的路径 = coutPath + 相对路径名 + 文件名
            auto rootPath = appPath + protoInfo->_protoInfo._rootPath;
            auto fullPath = appPath + protoInfo->_fullPathName;
            const auto fullProtoPath = appPath + _protoPath;
            auto relationPath = KERNEL_NS::DirectoryUtil::GetFileDirInPath(rootPath - fullProtoPath + "/");

            auto pbCCName = KERNEL_NS::FileUtil::ExtractFileWithoutExtension(coutPath + relationPath + protoInfo->_protoInfo._fileName) + ".pb.cc";
            KERNEL_NS::SmartPtr<FILE, KERNEL_NS::AutoDelMethods::CustomDelete> fp = KERNEL_NS::FileUtil::OpenFile(pbCCName.c_str());
            if(!fp)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("cant open pb cc file:%s"), pbCCName.c_str());
                return false;
            }

            fp.SetClosureDelegate([](void *ptr){
                auto fpPtr = reinterpret_cast<FILE *>(ptr);
                KERNEL_NS::FileUtil::CloseFile(*fpPtr);
            });

            // 2.扫描文件内容
            std::vector<KERNEL_NS::LibString> lineContents;
            auto line = KERNEL_NS::FileUtil::ReadUtf8File(*fp, lineContents);
            g_Log->Info(LOGFMT_OBJ_TAG("pb cc:%s line:%lld"), pbCCName.c_str(), line);

            if(!_ModifyCppPbCC(pbCCName, lineContents, protoInfo, newFactoryNames))
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("modify cpp line contents fail cc file:%s"), pbCCName.c_str());
                return false;
            }

            // 3.关闭文件
            KERNEL_NS::FileUtil::CloseFile(*fp);
            fp.pop();

            // 4.用新内容替换文件
            if(!KERNEL_NS::FileUtil::ReplaceFile(pbCCName, lineContents))
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("replace file fail:%s"), pbCCName.c_str());
                return false;
            }
        }

        g_Log->Custom("[PROTO GEN CPP] %s SUCCESS!", protoInfo->_protoInfo._fileName.c_str());
    }

    return true;
}

bool ExporterMgr::_ModifyCppPbHeader(const KERNEL_NS::LibString &pbHeaderName, std::vector<KERNEL_NS::LibString> &lines, const ProtoContentInfo *protoFileInfo, std::vector<KERNEL_NS::LibString> &newClassFactoryNames)
{
    // 声明匹配模式
    const auto &pbHeaderDefinePatternRegex = std::regex("^#define GOOGLE_PROTOBUF_INCLUDED.*");

    // 1.添加头文件
    {
        std::vector<KERNEL_NS::LibString> addLines 
        {
            "",
            "// KERNEL_INCLUDED",
            "#include <kernel/kernel.h>", 
            "#include <google/protobuf/util/json_util.h>",
            "#include <google/protobuf/text_format.h>",
            "",
            "#ifdef GetMessage",
            " #undef GetMessage",
            "#endif",
            "",
        };
        // 匹配#define xxx的下一行开始插入数据
        ProtobuffHelper::Modifylines(lines, addLines, [&pbHeaderDefinePatternRegex](Int32 curLine, KERNEL_NS::LibString &lineData
        , std::vector<KERNEL_NS::LibString> &addDatasBefore
        , std::vector<KERNEL_NS::LibString> &addDatasAfter
        , bool &isContinue)->bool{
            return std::regex_match(lineData.GetRaw(), pbHeaderDefinePatternRegex);
        });
    }

    // 2.处理类(添加注解, 对象池声明, Release接口, 编解码接口等)
    std::vector<KERNEL_NS::LibString> classNames;
    {
        const auto &classPatternRegex = std::regex("^class .* :");
        std::vector<KERNEL_NS::LibString> addLines;
        ProtobuffHelper::Modifylines(lines, addLines, 
        [&pbHeaderName, &classPatternRegex, protoFileInfo, this, &lines, &classNames] (Int32 curLine, KERNEL_NS::LibString &lineData
        , std::vector<KERNEL_NS::LibString> &addLineDatasBefore
        , std::vector<KERNEL_NS::LibString> &addLineDatasAfter
        , bool &isContinue) -> bool
        {
            bool isMatched = std::regex_match(lineData.GetRaw(), classPatternRegex);
            if(isMatched)
            {
                // 1.获取类名
                const auto &className = ProtobuffHelper::DragClass(lineData);
                auto messageInfo = protoFileInfo->GetMessageInfo(className);
                if(!messageInfo)
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("message info not found message name:%s, proto info:%s"), className.c_str(), protoFileInfo->ToString().c_str());
                    return false;
                }

                classNames.push_back(className);

                // 2.类前注解
                const auto annotationInfo = KERNEL_NS::LibString().AppendFormat("// AnnotaionInfo[opcode(%d), nolog(%s), XorEncrypt(%s), KeyBase64(%s), EnableStorage:(%s)]"
                , messageInfo->_opcode, messageInfo->_noLog ? "true" : "false", messageInfo->_isXorEncrypt ? "true" : "false"
                , messageInfo->_isKeyBase64 ? "true" : "false", messageInfo->_enableStorage ? "true" : "false");
                addLineDatasBefore.push_back(annotationInfo);

                // 3.添加基类
                // lineData.findFirstAppendFormat(":", "\n    public KERNEL_NS::ICoder, ");
                const Int32 nextMaxLine = static_cast<Int32>(lines.size());
                for(Int32 nextIdx = curLine + 1; nextIdx < nextMaxLine; ++nextIdx)
                {
                    auto &nextLineData = lines[nextIdx];
                    auto success = ProtobuffHelper::SearchClassBodyBegin(nextLineData, [](Int32 curIdx, KERNEL_NS::LibString &lineData){
                        lineData.GetRaw().insert(curIdx, ", public KERNEL_NS::ICoder ");
                    });

                    if(success)
                        break;
                }
                
                // 4.添加类成员(对象池声明, Release, 编解码等)
                if(lineData.GetRaw().find("{") != std::string::npos)
                {// 类声明必然是以 {结尾 请遵循规则
                    _CollectCppClassAdds(className, addLineDatasAfter);
                    lineData.RemoveZeroTail();
                }
                else
                {
                    const Int32 maxLine = static_cast<Int32>(lines.size());
                    bool isFound = false;
                    for(Int32 idx = curLine + 1; idx < maxLine; ++idx)
                    {
                        auto &nexLine = lines[idx];
                        auto foundPos = nexLine.GetRaw().find("{");
                        if(foundPos != std::string::npos)
                        {
                            nexLine.AppendFormat("\n");
                            if(idx + 1 == maxLine)
                            {
                                _CollectCppClassAdds(className, nexLine);
                            }
                            else
                            {
                                KERNEL_NS::LibString cacheNext;
                                _CollectCppClassAdds(className, cacheNext);
                                nexLine.GetRaw().reserve(nexLine.GetRaw().size() + cacheNext.GetRaw().size());
                                nexLine.GetRaw().insert(foundPos + 1, cacheNext.GetRaw());
                            }

                            nexLine.RemoveZeroTail();
                            
                            isFound = true;
                            break;
                        }
                    }

                    if(!isFound)
                    {
                        g_Log->Warn(LOGFMT_OBJ_TAG("add object pool declear fail proto pb header:%s message name:%s,  begin line:%d "), pbHeaderName.c_str(), className.c_str(), curLine);
                    }
                }
            }

            isContinue = true;
            return isMatched;
        });

        // // 替换uint64_t
        // ProtobuffHelper::Modifylines(lines, 
        // [] (Int32 curLine, KERNEL_NS::LibString &lineData, bool &isContinue) -> bool
        // {
        //     isContinue = true;
        //     const std::string replaceFlag = "UInt64";
        //     const std::string preReplaceFlag = " UInt64";
        //     const std::string afterReplaceFlag = "UInt64 ";
        //     const std::string doubleReplaceFlag = " UInt64 ";
        //     const std::string initReplaceFlag = " UInt64{";
        //     const std::string initReplaceFlag2 = " UInt64(";
        //     const std::string initReplaceFlag3 = "(UInt64{";    // 函数
        //     const std::string initReplaceFlag4 = "(UInt64(";    // 函数
        //     const std::string replaceFlag5 = "*/UInt64{";    // 注释后
        //     const std::string replaceFlag6 = "*/UInt64(";    // 注释后
        //     const std::string replaceFlag7 = "{UInt64{";
        //     const std::string replaceFlag8 = "(UInt64 ";

        //     std::string flag = "uint64_t";
        //     std::string afterFix = "uint64_t ";
        //     std::string preFix = " uint64_t";
        //     std::string doubleFix = " uint64_t ";
        //     std::string initFix = " uint64_t{";
        //     std::string initFix2 = " uint64_t(";
        //     std::string initFix3 = "(uint64_t{";
        //     std::string initFix4 = "(uint64_t(";
        //     std::string fix5 = "*/uint64_t{";
        //     std::string fix6 = "*/uint64_t(";
        //     std::string fix7 = "{uint64_t{";
        //     std::string fix8 = "(uint64_t ";

        //     // 判断是否完全匹配
        //     if(lineData == flag)
        //     {
        //         lineData = replaceFlag;
        //     }
            
        //     // 先替换首部
        //     auto pos = lineData.GetRaw().find(afterFix);
        //     if(pos == 0)
        //     {
        //         lineData = replaceFlag + " " + lineData.GetRaw().substr(pos + afterFix.size());
        //     }

        //     // 再替换末尾
        //     pos = lineData.GetRaw().find(preFix);
        //     if(pos != std::string::npos)
        //     {
        //         if((pos + preFix.size()) == lineData.size())
        //         {
        //             lineData = lineData.GetRaw().substr(0, pos + 1) + " " + replaceFlag;
        //         }
        //     }

        //     // 再替换中间部位
        //     lineData.findreplace(doubleFix, doubleReplaceFlag);
        //     lineData.findreplace(initFix, initReplaceFlag);
        //     lineData.findreplace(initFix2, initReplaceFlag2);
        //     lineData.findreplace(initFix3, initReplaceFlag3);
        //     lineData.findreplace(initFix4, initReplaceFlag4);
        //     lineData.findreplace(fix5, replaceFlag5);
        //     lineData.findreplace(fix6, replaceFlag6);
        //     lineData.findreplace(fix7, replaceFlag7);
        //     lineData.findreplace(fix8, replaceFlag8);
            
        //     return true;
        // });

        // // 替换 int64_t
        // ProtobuffHelper::Modifylines(lines, 
        // [] (Int32 curLine, KERNEL_NS::LibString &lineData, bool &isContinue) -> bool
        // {
        //     isContinue = true;
        //     const std::string replaceFlag = "Int64";
        //     const std::string preReplaceFlag = " Int64";
        //     const std::string afterReplaceFlag = "Int64 ";
        //     const std::string doubleReplaceFlag = " Int64 ";
        //     const std::string initReplaceFlag = " Int64{";
        //     const std::string initReplaceFlag2 = " Int64(";
        //     const std::string initReplaceFlag3 = "(Int64{";    // 函数
        //     const std::string initReplaceFlag4 = "(Int64(";    // 函数
        //     const std::string replaceFlag5 = "*/Int64{";    // 注释后
        //     const std::string replaceFlag6 = "*/Int64(";    // 注释后
        //     const std::string replaceFlag7 = "{Int64{";
        //     const std::string replaceFlag8 = "(Int64 ";

        //     std::string flag = "int64_t";
        //     std::string afterFix = "int64_t ";
        //     std::string preFix = " int64_t";
        //     std::string doubleFix = " int64_t ";
        //     std::string initFix = " int64_t{";
        //     std::string initFix2 = " int64_t(";
        //     std::string initFix3 = "(int64_t{";
        //     std::string initFix4 = "(int64_t(";
        //     std::string fix5 = "*/int64_t{";
        //     std::string fix6 = "*/int64_t(";
        //     std::string fix7 = "{int64_t{";
        //     std::string fix8 = "(int64_t ";

        //     // 判断是否完全匹配
        //     if(lineData == flag)
        //     {
        //         lineData = replaceFlag;
        //     }
            
        //     // 先替换首部
        //     auto pos = lineData.GetRaw().find(afterFix);
        //     if(pos == 0)
        //     {
        //         lineData = replaceFlag + " " + lineData.GetRaw().substr(pos + afterFix.size());
        //     }

        //     // 再替换末尾
        //     pos = lineData.GetRaw().find(preFix);
        //     if(pos != std::string::npos)
        //     {
        //         if((pos + preFix.size()) == lineData.size())
        //         {
        //             lineData = lineData.GetRaw().substr(0, pos + 1) + " " + replaceFlag;
        //         }
        //     }

        //     // 再替换中间部位
        //     lineData.findreplace(doubleFix, doubleReplaceFlag);
        //     lineData.findreplace(initFix, initReplaceFlag);
        //     lineData.findreplace(initFix2, initReplaceFlag2);
        //     lineData.findreplace(initFix3, initReplaceFlag3);
        //     lineData.findreplace(initFix4, initReplaceFlag4);
        //     lineData.findreplace(fix5, replaceFlag5);
        //     lineData.findreplace(fix6, replaceFlag6);
        //     lineData.findreplace(fix7, replaceFlag7);
        //     lineData.findreplace(fix8, replaceFlag8);
            
        //     return true;
        // });

        // // 误伤 RepeatedField< UInt64 >
        // // 误伤 RepeatedField< Int64 >
        // // 替换uint64_t
        // ProtobuffHelper::Modifylines(lines, 
        // [] (Int32 curLine, KERNEL_NS::LibString &lineData, bool &isContinue) -> bool
        // {
        //     isContinue = true;
        //     const std::string replaceFlag = "RepeatedField< uint64_t >";

        //     std::string flag = "RepeatedField< UInt64 >";

        //     lineData.findreplace(flag, replaceFlag);
            
        //     return true;
        // });

        // // 替换 int64_t
        // ProtobuffHelper::Modifylines(lines, 
        // [] (Int32 curLine, KERNEL_NS::LibString &lineData, bool &isContinue) -> bool
        // {
        //     isContinue = true;
        //     const std::string replaceFlag = "RepeatedField< int64_t >";

        //     std::string flag = "RepeatedField< Int64 >";

        //     lineData.findreplace(flag, replaceFlag);
            
        //     return true;
        // });
    }
    
    // 3.添加工厂类
    {
        if(!classNames.empty())
        {
            const auto &endifOfFilePatternRegex = std::regex("^#endif.*");
            const Int32 maxLine = static_cast<Int32>(lines.size());
            for(Int32 idx = maxLine - 1; idx >= 0; --idx)
            {
                auto &lineData = lines[idx];
                if(std::regex_match(lineData.GetRaw(), endifOfFilePatternRegex))
                {
                    if(idx > 0)
                    {
                        std::vector<KERNEL_NS::LibString> addFactoryLines;
                        for(auto &className : classNames)
                            _CollectCppClassFactoryDeclearAdds(protoFileInfo->_packageName, className, addFactoryLines, newClassFactoryNames);

                        lines.insert(lines.begin() + idx, addFactoryLines.begin(), addFactoryLines.end());
                        break;
                    }
                }
            }
        }
    }

    return true;
}

void ExporterMgr::_CollectCppClassAdds(const KERNEL_NS::LibString &className, KERNEL_NS::LibString &addLines)
{
    std::vector<KERNEL_NS::LibString> lines;
    _CollectCppClassAdds(className, lines);
    KERNEL_NS::StringUtil::MergerMultiLine(lines, addLines);
}

void ExporterMgr::_CollectCppClassAdds(const KERNEL_NS::LibString &className, std::vector<KERNEL_NS::LibString> &addLines)
{
    // 1.暂时不做池化,因为protobuf 默认使用new/delete 若要池化需要全部替换这些,有点复杂
    // {
    //     addLines.push_back(KERNEL_NS::LibString().AppendFormat("POOL_CREATE_OBJ_DEFAULT(%s)", className.c_str()));
    // }
    
    // 2.添加访问控制
    addLines.push_back("");
    addLines.push_back("public:");

    // 3.添加Release方法
    {
        addLines.push_back("virtual void Release() override {");
        addLines.push_back("    delete this;");
        addLines.push_back("}");
    }
    addLines.push_back("");

    // 4.添加MT Encode方法
    {
        addLines.push_back("virtual bool Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) const override {");
        addLines.push_back("    if (UNLIKELY(!IsInitialized()))");
        addLines.push_back("    {");
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("      g_Log->Error(LOGFMT_OBJ_TAG(\"Encode message %s failed, error: %%s\"), InitializationErrorString().c_str());",
         className.c_str()));
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("      return false;"
        ));
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("    }"
        ));
        addLines.push_back("");
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("    size_t payloadSize = ByteSizeLong();"
        )); 
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("    if (payloadSize == 0)"
        )); 
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("      return true;"
        )); 
        addLines.push_back("");
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("    if(UNLIKELY(stream.GetBuffer() == NULL))"
        ));
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("        stream.Init(payloadSize);"
        ));
        addLines.push_back("");
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("    auto writableSize = stream.GetWritableSize();"
        ));
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("    if (writableSize < static_cast<Int64>(payloadSize))"
        ));
        addLines.push_back("    {");
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("        if(UNLIKELY(!stream.AppendCapacity(static_cast<Int64>(payloadSize) - writableSize)))"
        ));
        addLines.push_back("        {");
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("            g_Log->Error(LOGFMT_OBJ_TAG(\"stream append capacity fail IsAttach:%%d\"), stream.IsAttach());"
        ));
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("            return false;"
        ));
        addLines.push_back("        }");
        addLines.push_back("    }");
        addLines.push_back("");
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("    if (UNLIKELY(!SerializeToArray(stream.GetWriteBegin(), static_cast<Int32>(stream.GetWritableSize()))))"
        ));
        addLines.push_back("    {");
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("        g_Log->Error(LOGFMT_OBJ_TAG(\"Encode message %s failed, error: %%s\"), InitializationErrorString().c_str());"
        , className.c_str()));
        addLines.push_back("        return false;");
        addLines.push_back("    }");
        addLines.push_back("");
        addLines.push_back("    stream.ShiftWritePos(payloadSize);");
        addLines.push_back("    return true;");
        addLines.push_back("}");
    }
    addLines.push_back("");

    {// 5.添加TL Encode 方法
        addLines.push_back("virtual bool Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) const override {");
        addLines.push_back("    if (UNLIKELY(!IsInitialized()))");
        addLines.push_back("    {");
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("      g_Log->Error(LOGFMT_OBJ_TAG(\"Encode message %s failed, error: %%s\"), InitializationErrorString().c_str());",
         className.c_str()));
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("      return false;"
        ));
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("    }"
        ));
        addLines.push_back("");
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("    size_t payloadSize = ByteSizeLong();"
        )); 
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("    if (payloadSize == 0)"
        )); 
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("      return true;"
        )); 
        addLines.push_back("");
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("    if(UNLIKELY(stream.GetBuffer() == NULL))"
        ));
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("        stream.Init(payloadSize);"
        ));
        addLines.push_back("");
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("    auto writableSize = stream.GetWritableSize();"
        ));
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("    if (writableSize < static_cast<Int64>(payloadSize))"
        ));
        addLines.push_back("    {");
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("        if(UNLIKELY(!stream.AppendCapacity(static_cast<Int64>(payloadSize) - writableSize)))"
        ));
        addLines.push_back("        {");
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("            g_Log->Error(LOGFMT_OBJ_TAG(\"stream append capacity fail IsAttach:%%d\"), stream.IsAttach());"
        ));
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("            return false;"
        ));
        addLines.push_back("        }");
        addLines.push_back("    }");
        addLines.push_back("");
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("    if (UNLIKELY(!SerializeToArray(stream.GetWriteBegin(), static_cast<Int32>(stream.GetWritableSize()))))"
        ));
        addLines.push_back("    {");
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("        g_Log->Error(LOGFMT_OBJ_TAG(\"Encode message %s failed, error: %%s\"), InitializationErrorString().c_str());"
        , className.c_str()));
        addLines.push_back("        return false;");
        addLines.push_back("    }");
        addLines.push_back("");
        addLines.push_back("    stream.ShiftWritePos(payloadSize);");
        addLines.push_back("    return true;");
        addLines.push_back("}");
    }
    addLines.push_back("");

    {// 6.添加Decode MT方法
        addLines.push_back("virtual bool Decode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) override {");
        addLines.push_back("    if (stream.GetReadableSize() == 0)");
        addLines.push_back("    {");
        addLines.push_back("        Clear();");
        addLines.push_back("        return true;");
        addLines.push_back("    }");
        addLines.push_back("");
        addLines.push_back("    if (UNLIKELY(!ParseFromArray(stream.GetReadBegin(), static_cast<Int32>(stream.GetReadableSize()))))");
        addLines.push_back("    {");
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("        g_Log->Error(LOGFMT_OBJ_TAG(\"Decode message %s failed, error: %%s\"), InitializationErrorString().c_str());"
        , className.c_str()));
        addLines.push_back("        return false;");
        addLines.push_back("    }");
        addLines.push_back("");
        addLines.push_back("    return true;");
        addLines.push_back("}");
    }
    addLines.push_back("");

    {// 7.添加Decode TL方法
        addLines.push_back("virtual bool Decode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) override {");
        addLines.push_back("    if (stream.GetReadableSize() == 0)");
        addLines.push_back("    {");
        addLines.push_back("        Clear();");
        addLines.push_back("        return true;");
        addLines.push_back("    }");
        addLines.push_back("");
        addLines.push_back("    if (UNLIKELY(!ParseFromArray(stream.GetReadBegin(), static_cast<Int32>(stream.GetReadableSize()))))");
        addLines.push_back("    {");
        addLines.push_back(KERNEL_NS::LibString().AppendFormat
        ("        g_Log->Error(LOGFMT_OBJ_TAG(\"Decode message %s failed, error: %%s\"), InitializationErrorString().c_str());"
        , className.c_str()));
        addLines.push_back("        return false;");
        addLines.push_back("    }");
        addLines.push_back("");
        addLines.push_back("    return true;");
        addLines.push_back("}");
    }
    addLines.push_back("");

    {// 8.添加 ToJsonString 方法
        addLines.push_back("virtual KERNEL_NS::LibString ToJsonString() const override {");
        addLines.push_back("    KERNEL_NS::LibString data;");
        addLines.push_back("    if(!::google::protobuf::util::MessageToJsonString(*this, &data.GetRaw()).ok())");
        addLines.push_back("    {");
        addLines.push_back("        g_Log->Warn(LOGFMT_OBJ_TAG(\"Turn JsonString fail:%s\"), KERNEL_NS::RttiUtil::GetByObj(this));");
        addLines.push_back("        return \"\";");
        addLines.push_back("    }");
        addLines.push_back("");
        addLines.push_back("    return data;");
        addLines.push_back("}");
    }
    addLines.push_back("");

    {// 9.添加 ToJsonString 方法
        addLines.push_back("virtual bool ToJsonString(std::string *data) const override {");
        addLines.push_back("    if(!::google::protobuf::util::MessageToJsonString(*this, data).ok())");
        addLines.push_back("    {");
        addLines.push_back("        g_Log->Warn(LOGFMT_OBJ_TAG(\"Turn JsonString fail:%s\"), KERNEL_NS::RttiUtil::GetByObj(this));");
        addLines.push_back("        return false;");
        addLines.push_back("    }");
        addLines.push_back("");
        addLines.push_back("    return true;");
        addLines.push_back("}");
    }
    addLines.push_back("");

    {// 10.添加 FromJsonString 方法
        addLines.push_back("virtual bool FromJsonString(const Byte8 *data, size_t len) override {");
        addLines.push_back("    auto &&jsonString = ::google::protobuf::StringPiece(data, len);");
        addLines.push_back("    if(!::google::protobuf::util::JsonStringToMessage(jsonString, this).ok())");
        addLines.push_back("    {");
        addLines.push_back("        g_Log->Warn(LOGFMT_OBJ_TAG(\"SimpleInfo field JsonStringToMessage fail jsonString:%s, message name:%s\"), jsonString.as_string().c_str(), KERNEL_NS::RttiUtil::GetByObj(this));");
        addLines.push_back("        return false;");
        addLines.push_back("    }");
        addLines.push_back("");
        addLines.push_back("    return true;");
        addLines.push_back("}");
    }
    addLines.push_back("");
}

void ExporterMgr::_CollectCppClassFactoryDeclearAdds(const KERNEL_NS::LibString &packageName, const KERNEL_NS::LibString &className, std::vector<KERNEL_NS::LibString> &addLines, std::vector<KERNEL_NS::LibString> &newClassFactoryNames)
{
    addLines.push_back("");

    const auto factoryName = className + "Factory";
    newClassFactoryNames.push_back(factoryName);
    addLines.push_back(KERNEL_NS::LibString().AppendFormat("class %s : public KERNEL_NS::ICoderFactory {", factoryName.c_str()));
    addLines.push_back(KERNEL_NS::LibString().AppendFormat("    POOL_CREATE_OBJ_DEFAULT_P1(ICoderFactory, %s);", factoryName.c_str()));
    addLines.push_back(KERNEL_NS::LibString().AppendFormat("public:"));
    addLines.push_back(KERNEL_NS::LibString());

    // release
    addLines.push_back(KERNEL_NS::LibString().AppendFormat("    virtual void Release() override {"));
    addLines.push_back(KERNEL_NS::LibString().AppendFormat("        %s::Delete_%s(this);", factoryName.c_str(), factoryName.c_str()));
    addLines.push_back(KERNEL_NS::LibString().AppendFormat("    }"));
    addLines.push_back(KERNEL_NS::LibString());

    // CreateFactory
    addLines.push_back(KERNEL_NS::LibString().AppendFormat("    static %s *CreateFactory() {", factoryName.c_str()));
    addLines.push_back(KERNEL_NS::LibString().AppendFormat("        return %s::New_%s();", factoryName.c_str(), factoryName.c_str()));
    addLines.push_back(KERNEL_NS::LibString().AppendFormat("    }"));
    addLines.push_back(KERNEL_NS::LibString());

    // Create
    addLines.push_back(KERNEL_NS::LibString().AppendFormat("    virtual KERNEL_NS::ICoder *Create() const override {"));
    addLines.push_back(KERNEL_NS::LibString().AppendFormat("        return new ::%s::%s();", packageName.c_str(), className.c_str()));
    addLines.push_back(KERNEL_NS::LibString().AppendFormat("    }"));
    addLines.push_back(KERNEL_NS::LibString());

    // virtual ICoder *Create(const ICoder *coder) = 0;
    // Create
    addLines.push_back(KERNEL_NS::LibString().AppendFormat("    virtual KERNEL_NS::ICoder *Create(const KERNEL_NS::ICoder *coder) const override {"));
    addLines.push_back(KERNEL_NS::LibString().AppendFormat("        return new ::%s::%s(*dynamic_cast<const ::%s::%s *>(coder));", packageName.c_str(), className.c_str(), packageName.c_str(), className.c_str()));
    addLines.push_back(KERNEL_NS::LibString().AppendFormat("    }"));
    addLines.push_back(KERNEL_NS::LibString());

    // end
    addLines.push_back("};");
    addLines.push_back("");
}

bool ExporterMgr::_ModifyCppPbCC(const KERNEL_NS::LibString &pbCCName, std::vector<KERNEL_NS::LibString> &lines, const ProtoContentInfo *protoFileInfo, const std::vector<KERNEL_NS::LibString> &newFactoryNames)
{
    // 1.添加pch.h
    {
        // 在第一行插入
        if(lines.empty())
        {
            lines.push_back("#include <pch.h>");
        }
        else
        {
            lines.insert(lines.begin() + 0, "#include <pch.h>");
        }
    }

    {// 替换头文件
        const auto protoNameWithoutExtention = KERNEL_NS::FileUtil::ExtractFileWithoutExtension(protoFileInfo->_protoInfo._fileName);
        auto appPath = GetServiceProxy()->GetApp()->GetAppPath();
        appPath = KERNEL_NS::DirectoryUtil::GetFileDirInPath(appPath);

        auto rootPath = appPath + protoFileInfo->_protoInfo._rootPath;
        auto fullPath = appPath + protoFileInfo->_fullPathName;
        const auto fullProtoPath = appPath + _protoPath;
        auto relationPath = KERNEL_NS::DirectoryUtil::GetFileDirInPath(rootPath - fullProtoPath + "/");
        
        auto cppOutRelationPath = _cppOutPath - _basePath;

        // cc相对于工程根目录的相对路径
        auto headerRelationPath = KERNEL_NS::DirectoryUtil::GetFileDirInPath(cppOutRelationPath + relationPath);
    
        KERNEL_NS::LibString pattenString;
        pattenString.AppendFormat("^#include .*%s\\.pb\\.h.*", protoNameWithoutExtention.c_str());
        const auto &replaceIncludeRegex = std::regex(pattenString.c_str());

        ProtobuffHelper::Modifylines(lines, [&replaceIncludeRegex, &headerRelationPath, &protoNameWithoutExtention](Int32 curLine, KERNEL_NS::LibString &lineData, bool &isContinue)->bool
        {
            isContinue = false;
            auto isMatch = std::regex_match(lineData.GetRaw(), replaceIncludeRegex);
            if(isMatch)
            {
                lineData = KERNEL_NS::LibString().AppendFormat("#include <%s%s.pb.h>", headerRelationPath.c_str(), protoNameWithoutExtention.c_str());
            }

            return isMatch;
        });
    }

    {
        std::vector<KERNEL_NS::LibString> addFactoryImpls;
        for(auto &factoryName : newFactoryNames)
            addFactoryImpls.push_back(KERNEL_NS::LibString().AppendFormat("POOL_CREATE_OBJ_DEFAULT_IMPL(%s);", factoryName.c_str()));

        const auto protoNameWithoutExtention = KERNEL_NS::FileUtil::ExtractFileWithoutExtension(protoFileInfo->_protoInfo._fileName);
        auto appPath = GetServiceProxy()->GetApp()->GetAppPath();
        appPath = KERNEL_NS::DirectoryUtil::GetFileDirInPath(appPath);

        auto rootPath = appPath + protoFileInfo->_protoInfo._rootPath;
        auto fullPath = appPath + protoFileInfo->_fullPathName;
        const auto fullProtoPath = appPath + _protoPath;
        auto relationPath = KERNEL_NS::DirectoryUtil::GetFileDirInPath(rootPath - fullProtoPath + "/");
        auto cppOutRelationPath = _cppOutPath - _basePath;
        auto headerRelationPath = KERNEL_NS::DirectoryUtil::GetFileDirInPath(cppOutRelationPath + relationPath);
        
        KERNEL_NS::LibString pattenString = KERNEL_NS::LibString().AppendFormat("^#include.*%s%s\\.pb\\.h.*", headerRelationPath.c_str(), protoNameWithoutExtention.c_str());
        const auto &includeRegex = std::regex(pattenString.c_str());

        ProtobuffHelper::Modifylines(lines, addFactoryImpls, [&includeRegex](Int32 curLine, KERNEL_NS::LibString &lineData
                , std::vector<KERNEL_NS::LibString> &addLinesBefore // 默认为空
                , std::vector<KERNEL_NS::LibString> &addLinesAfter  // 默认把 addLines 添加到 addLinesAfter
                , bool &isContinue) -> bool
        {
            return std::regex_match(lineData.GetRaw(), includeRegex);
        });
    }

    // {
    //    // 替换uint64_t
    //     ProtobuffHelper::Modifylines(lines, 
    //     [] (Int32 curLine, KERNEL_NS::LibString &lineData, bool &isContinue) -> bool
    //     {
    //         isContinue = true;
    //         const std::string replaceFlag = "UInt64";
    //         const std::string preReplaceFlag = " UInt64";
    //         const std::string afterReplaceFlag = "UInt64 ";
    //         const std::string doubleReplaceFlag = " UInt64 ";
    //         const std::string initReplaceFlag = " UInt64{";
    //         const std::string initReplaceFlag2 = " UInt64(";
    //         const std::string initReplaceFlag3 = "(UInt64{";    // 函数
    //         const std::string initReplaceFlag4 = "(UInt64(";    // 函数
    //         const std::string replaceFlag5 = "*/UInt64{";    // 注释后
    //         const std::string replaceFlag6 = "*/UInt64(";    // 注释后
    //         const std::string replaceFlag7 = "{UInt64{";
    //         const std::string replaceFlag8 = "(UInt64 ";

    //         std::string flag = "uint64_t";
    //         std::string afterFix = "uint64_t ";
    //         std::string preFix = " uint64_t";
    //         std::string doubleFix = " uint64_t ";
    //         std::string initFix = " uint64_t{";
    //         std::string initFix2 = " uint64_t(";
    //         std::string initFix3 = "(uint64_t{";
    //         std::string initFix4 = "(uint64_t(";
    //         std::string fix5 = "*/uint64_t{";
    //         std::string fix6 = "*/uint64_t(";
    //         std::string fix7 = "{uint64_t{";
    //         std::string fix8 = "(uint64_t ";

    //         // 判断是否完全匹配
    //         if(lineData == flag)
    //         {
    //             lineData = replaceFlag;
    //         }
            
    //         // 先替换首部
    //         auto pos = lineData.GetRaw().find(afterFix);
    //         if(pos == 0)
    //         {
    //             lineData = replaceFlag + " " + lineData.GetRaw().substr(pos + afterFix.size());
    //         }

    //         // 再替换末尾
    //         pos = lineData.GetRaw().find(preFix);
    //         if(pos != std::string::npos)
    //         {
    //             if((pos + preFix.size()) == lineData.size())
    //             {
    //                 lineData = lineData.GetRaw().substr(0, pos + 1) + " " + replaceFlag;
    //             }
    //         }

    //         // 再替换中间部位
    //         lineData.findreplace(doubleFix, doubleReplaceFlag);
    //         lineData.findreplace(initFix, initReplaceFlag);
    //         lineData.findreplace(initFix2, initReplaceFlag2);
    //         lineData.findreplace(initFix3, initReplaceFlag3);
    //         lineData.findreplace(initFix4, initReplaceFlag4);
    //         lineData.findreplace(fix5, replaceFlag5);
    //         lineData.findreplace(fix6, replaceFlag6);
    //         lineData.findreplace(fix7, replaceFlag7);
    //         lineData.findreplace(fix8, replaceFlag8);
            
    //         return true;
    //     });

    //     // 替换 int64_t
    //     ProtobuffHelper::Modifylines(lines, 
    //     [] (Int32 curLine, KERNEL_NS::LibString &lineData, bool &isContinue) -> bool
    //     {
    //         isContinue = true;
    //         const std::string replaceFlag = "Int64";
    //         const std::string preReplaceFlag = " Int64";
    //         const std::string afterReplaceFlag = "Int64 ";
    //         const std::string doubleReplaceFlag = " Int64 ";
    //         const std::string initReplaceFlag = " Int64{";
    //         const std::string initReplaceFlag2 = " Int64(";
    //         const std::string initReplaceFlag3 = "(Int64{";    // 函数
    //         const std::string initReplaceFlag4 = "(Int64(";    // 函数
    //         const std::string replaceFlag5 = "*/Int64{";    // 注释后
    //         const std::string replaceFlag6 = "*/Int64(";    // 注释后
    //         const std::string replaceFlag7 = "{Int64{";
    //         const std::string replaceFlag8 = "(Int64 ";

    //         std::string flag = "int64_t";
    //         std::string afterFix = "int64_t ";
    //         std::string preFix = " int64_t";
    //         std::string doubleFix = " int64_t ";
    //         std::string initFix = " int64_t{";
    //         std::string initFix2 = " int64_t(";
    //         std::string initFix3 = "(int64_t{";
    //         std::string initFix4 = "(int64_t(";
    //         std::string fix5 = "*/int64_t{";
    //         std::string fix6 = "*/int64_t(";
    //         std::string fix7 = "{int64_t{";
    //         std::string fix8 = "(int64_t ";

    //         // 判断是否完全匹配
    //         if(lineData == flag)
    //         {
    //             lineData = replaceFlag;
    //         }
            
    //         // 先替换首部
    //         auto pos = lineData.GetRaw().find(afterFix);
    //         if(pos == 0)
    //         {
    //             lineData = replaceFlag + " " + lineData.GetRaw().substr(pos + afterFix.size());
    //         }

    //         // 再替换末尾
    //         pos = lineData.GetRaw().find(preFix);
    //         if(pos != std::string::npos)
    //         {
    //             if((pos + preFix.size()) == lineData.size())
    //             {
    //                 lineData = lineData.GetRaw().substr(0, pos + 1) + " " + replaceFlag;
    //             }
    //         }

    //         // 再替换中间部位
    //         lineData.findreplace(doubleFix, doubleReplaceFlag);
    //         lineData.findreplace(initFix, initReplaceFlag);
    //         lineData.findreplace(initFix2, initReplaceFlag2);
    //         lineData.findreplace(initFix3, initReplaceFlag3);
    //         lineData.findreplace(initFix4, initReplaceFlag4);
    //         lineData.findreplace(fix5, replaceFlag5);
    //         lineData.findreplace(fix6, replaceFlag6);
    //         lineData.findreplace(fix7, replaceFlag7);
    //         lineData.findreplace(fix8, replaceFlag8);
            
    //         return true;
    //     });

    //     // 误伤 RepeatedField< UInt64 >
    //     // 误伤 RepeatedField< Int64 >
    //     // 替换uint64_t
    //     ProtobuffHelper::Modifylines(lines, 
    //     [] (Int32 curLine, KERNEL_NS::LibString &lineData, bool &isContinue) -> bool
    //     {
    //         isContinue = true;
    //         const std::string replaceFlag = "RepeatedField< uint64_t >";

    //         std::string flag = "RepeatedField< UInt64 >";

    //         lineData.findreplace(flag, replaceFlag);
            
    //         return true;
    //     });

    //     // 替换 int64_t
    //     ProtobuffHelper::Modifylines(lines, 
    //     [] (Int32 curLine, KERNEL_NS::LibString &lineData, bool &isContinue) -> bool
    //     {
    //         isContinue = true;
    //         const std::string replaceFlag = "RepeatedField< int64_t >";

    //         std::string flag = "RepeatedField< Int64 >";

    //         lineData.findreplace(flag, replaceFlag);
            
    //         return true;
    //     });
    // }

    return true;
}

void ExporterMgr::_GenOpcodeEnums()
{
    if(_protoNameRefProtoInfo.empty())
        return;

    const auto appName = GetServiceProxy()->GetApp()->GetAppName();
    KERNEL_NS::BinaryArray<PbCaheInfo *, PbCacheInfoCompare> sortedArray;
    for(auto kv : _pbCacheContent->_lineRefMessageInfo)
    {
        auto messageInfo = kv.second;
        if(messageInfo->_opcode <= 0)
            continue;

        sortedArray.insert(kv.second);
    }

    {// 生成cpp的opcode
        std::vector<KERNEL_NS::LibString> lines;
        g_Log->Custom("[PROTO GEN CPP] OPCODE ENUMS...");

        // 文件头
        const auto fileHeader = _FileHeader(KERNEL_NS::LibTime::Now(), KERNEL_NS::LibString().AppendFormat("Generated By %s, Dont Modify This File!!!", appName.c_str()));
        lines.push_back(fileHeader);
        lines.push_back("");

        lines.push_back("class OpcodeConst");
        lines.push_back("{");
        lines.push_back("public:");
        lines.push_back("    static constexpr Int32 OPCODE_BEGIN = 0;");

        const Int64 arrSize = static_cast<Int64>(sortedArray.size());
        Int32 maxOpcode = 0;
        for(Int64 idx = 0; idx < arrSize; ++idx)
        {
            auto messageInfo = sortedArray[idx];

            lines.push_back(KERNEL_NS::LibString().AppendFormat("    static constexpr Int32 OPCODE_%s = %d;    // %s"
                        , messageInfo->_messageName.c_str()
                        , messageInfo->_opcode
                        , messageInfo->_protoName.c_str()));

            if(maxOpcode == 0)
                maxOpcode = messageInfo->_opcode;
            else if(maxOpcode < messageInfo->_opcode)
                maxOpcode = messageInfo->_opcode;
        }

        lines.push_back(KERNEL_NS::LibString().AppendFormat("    static constexpr Int32 OPCODE_MAX = %d;", maxOpcode));
        lines.push_back("};");
        lines.push_back("");

        const auto opcodeEnums = _protocolsPath + "/OpcodeEnums.h";
        if(!KERNEL_NS::FileUtil::ReplaceFile(opcodeEnums, lines))
        {
            g_Log->Custom("[PROTO GEN CPP] OPCODE ENUMS FAILED.");
        }
        else
        {
            g_Log->Custom("[PROTO GEN CPP] OPCODE ENUMS SUCCESS.");
        }
    }

    if(!_csharpOutPath.empty())
    {// 生成csharp的opcode
        std::vector<KERNEL_NS::LibString> lines;
        g_Log->Custom("[PROTO GEN CSHARP] OPCODE ENUMS...");

        // 文件头
        const auto fileHeader = KERNEL_NS::LibString().AppendFormat("// Generated By %s, Dont Modify This File!!!", appName.c_str());
        lines.push_back(fileHeader);
        lines.push_back("");

        lines.push_back("public static class PacketOpcode");
        lines.push_back("{");
        lines.push_back("    public enum ENUMS");
        lines.push_back("    {");
        lines.push_back("        OpcodeBegin = 0,");

        const Int64 arrSize = static_cast<Int64>(sortedArray.size());
        Int32 maxOpcode = 0;
        for(Int64 idx = 0; idx < arrSize; ++idx)
        {
            auto messageInfo = sortedArray[idx];

            lines.push_back(KERNEL_NS::LibString().AppendFormat("        %s = %d,    // %s"
                        , messageInfo->_messageName.c_str()
                        , messageInfo->_opcode
                        , messageInfo->_protoName.c_str()));

            if(maxOpcode == 0)
                maxOpcode = messageInfo->_opcode;
            else if(maxOpcode < messageInfo->_opcode)
                maxOpcode = messageInfo->_opcode;
        }

        lines.push_back(KERNEL_NS::LibString().AppendFormat("        OpcodeMax = %d,", maxOpcode));
        lines.push_back("    }");
        lines.push_back("}");
        lines.push_back("");

        const auto opcodeEnums = _protocolsPath + "/csharp_common/csharp_common/ProtoPackage/PacketOpcode.cs";
        if(!KERNEL_NS::FileUtil::ReplaceFile(opcodeEnums, lines))
        {
            g_Log->Custom("[PROTO GEN CSHARP] OPCODE ENUMS FAILED.");
        }
        else
        {
            g_Log->Custom("[PROTO GENC CSHARP] OPCODE ENUMS SUCCESS.");
        }
    }
}

void ExporterMgr::_GenOpcodeInfo()
{
    if(_protoNameRefProtoInfo.empty())
        return;

    g_Log->Custom("[PROTO GEN] OPCODE INFO...");
    const auto appName = GetServiceProxy()->GetApp()->GetAppName();
    const auto opcodeInfoHeader = _protocolsPath + "/OpcodeInfo.h";

    std::vector<KERNEL_NS::LibString> lines;

    // 文件头
    const auto fileHeader = _FileHeader(KERNEL_NS::LibTime::Now(), KERNEL_NS::LibString().AppendFormat("Generated By %s, Dont Modify This File!!!", appName.c_str()));
    lines.push_back(fileHeader);
    lines.push_back("");

    // 排序
    KERNEL_NS::BinaryArray<PbCaheInfo *, PbCacheInfoCompare> sortedArray;
    for(auto kv : _pbCacheContent->_lineRefMessageInfo)
    {
        auto messageInfo = kv.second;
        if(messageInfo->_opcode <= 0)
            continue;

        sortedArray.insert(kv.second);
    }

    const Int64 arrSize = static_cast<Int64>(sortedArray.size());
    for(Int64 idx = 0; idx < arrSize; ++idx)
    {
        auto messageInfo = sortedArray[idx];
        lines.push_back(KERNEL_NS::LibString().AppendFormat("    {// %s", messageInfo->_messageName.c_str()));

        lines.push_back("        auto info = OpcodeInfo();");
        lines.push_back(KERNEL_NS::LibString().AppendFormat("        info._opcode = %d;", messageInfo->_opcode));
        lines.push_back(KERNEL_NS::LibString().AppendFormat("        info._noLog = %s;", messageInfo->_noLog ? "true" : "false"));
        lines.push_back(KERNEL_NS::LibString().AppendFormat("        info._enableStorage = %s;", messageInfo->_enableStorage ? "true" : "false"));

        // 加密
        if(messageInfo->_isXorEncrypt)
            lines.push_back(KERNEL_NS::LibString().AppendFormat("        info._msgFlags |= SERVICE_COMMON_NS::MsgFlagsType::XOR_ENCRYPT_FLAG;"));
        if(messageInfo->_isKeyBase64)
            lines.push_back(KERNEL_NS::LibString().AppendFormat("        info._msgFlags |= SERVICE_COMMON_NS::MsgFlagsType::KEY_IN_BASE64_FLAG;"));

        lines.push_back(KERNEL_NS::LibString().AppendFormat("        info._opcodeName = \"%s\";", messageInfo->_messageName.c_str()));
        lines.push_back(KERNEL_NS::LibString().AppendFormat("        info._protoFile = \"%s\";", messageInfo->_protoName.c_str()));
        lines.push_back(KERNEL_NS::LibString().AppendFormat("        _allOpcodeInfo.push_back(info);"));
        
        // _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, TitleInfoResFactory::CreateFactory()));
        lines.push_back(KERNEL_NS::LibString().AppendFormat("        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, %sFactory::CreateFactory()));", messageInfo->_messageName.c_str()));

        lines.push_back("    }");
        lines.push_back("");
    }

    if(!KERNEL_NS::FileUtil::ReplaceFile(opcodeInfoHeader, lines))
    {
        g_Log->Custom("[PROTO GEN] OPCODE INFO FAILED.");
        return;
    }
    
    g_Log->Custom("[PROTO GEN] OPCODE INFO SUCCESS.");
}

void ExporterMgr::_GenAllPbs()
{
    if(_protoNameRefProtoInfo.empty())
        return;
        
    g_Log->Custom("[PROTO GEN] ALL PBS HEADER...");
    const auto appName = GetServiceProxy()->GetApp()->GetAppName();
    const auto opcodeInfoHeader = _protocolsPath + "/AllPbs.h";

    std::vector<KERNEL_NS::LibString> lines;

    // 文件头
    const auto fileHeader = _FileHeader(KERNEL_NS::LibTime::Now(), KERNEL_NS::LibString().AppendFormat("Generated By %s, Dont Modify This File!!!", appName.c_str()));
    lines.push_back(fileHeader);
    lines.push_back("");
    lines.push_back("#ifndef __PROTOCOLS_ALLPBS_H__");
    lines.push_back("#define __PROTOCOLS_ALLPBS_H__");
    lines.push_back("");
    lines.push_back("#pragma once");
    lines.push_back("");

    // 排序
    KERNEL_NS::BinaryArray<PbCacheFileInfo *, PbCacheFileInfoCompare> sortedArray;
    for(auto kv : _pbCacheContent->_lineRefProtoFileInfo)
        sortedArray.insert(kv.second);

    const auto appFullPath = GetServiceProxy()->GetApp()->GetAppPath();
    const auto appPath = KERNEL_NS::DirectoryUtil::GetFileDirInPath(appFullPath);
    
    const auto fullProtoPath = appPath + _protoPath;

    const Int64 arrSize = static_cast<Int64>(sortedArray.size());
    for(Int64 idx = 0; idx < arrSize; ++idx)
    {
        auto protoInfo = sortedArray[idx];
        auto rootPath = appPath + protoInfo->_protoPath;
        auto relationPath = _cppOutPath - _basePath + KERNEL_NS::DirectoryUtil::GetFileDirInPath(rootPath - fullProtoPath);

        const auto protoFileName = KERNEL_NS::FileUtil::ExtractFileWithoutExtension(protoInfo->_protoName);
        lines.push_back(KERNEL_NS::LibString().AppendFormat("#include <%s%s.pb.h>"
            , relationPath.c_str(), protoFileName.c_str()));
    }

    lines.push_back("");
    lines.push_back("#endif // __PROTOCOLS_ALLPBS_H__");
    lines.push_back("");

    if(!KERNEL_NS::FileUtil::ReplaceFile(opcodeInfoHeader, lines))
    {
        g_Log->Custom("[PROTO GEN] ALL PBS HEADER FAILED.");
        return;
    }
    
    g_Log->Custom("[PROTO GEN] ALL PBS HEADER SUCCESS.");
}

// void ExporterMgr::_AddPchHeaderToGoogleCC()
// {
//     if(_googleProtoIncludePath.empty())
//         return;

//    g_Log->Custom("SCAN GOOGLE PROTO PATH TO ADD PCH HEADER PATH:%s ...", _googleProtoIncludePath.c_str());

//     auto nowTs = KERNEL_NS::LibTime::Now();
//     auto traverseCallback = [this, &nowTs] (const KERNEL_NS::FindFileInfo &fileInfo, bool &isParentPathContinue) -> bool {

//         bool isContinue = true;
//         do
//         {
//             // 过滤目录
//             if(KERNEL_NS::FileUtil::IsDir(fileInfo))
//                 break;

//             // 过滤非cc
//             if(KERNEL_NS::FileUtil::ExtractFileExtension(fileInfo._fileName) != KERNEL_NS::LibString(".cc"))
//                 break;

//             KERNEL_NS::LibString fullPath = fileInfo._rootPath;
//             if(fileInfo._rootPath.at(fileInfo._rootPath.length() - 1) != '/')
//                 fullPath.AppendFormat("/");

//             const auto &fullFilePath = fullPath + fileInfo._fileName;

//             KERNEL_NS::SmartPtr<FILE, KERNEL_NS::AutoDelMethods::CustomDelete> fp = KERNEL_NS::FileUtil::OpenFile(fullFilePath.c_str());
//             if(!fp)
//             {
//                 g_Log->Warn(LOGFMT_OBJ_TAG("cant open cc file:%s"), fullFilePath.c_str());
//                 break;
//             }
//             fp.SetClosureDelegate([](void *p){
//                 auto fpPtr = reinterpret_cast<FILE *>(p);
//                 KERNEL_NS::FileUtil::CloseFile(*fpPtr);
//             });
            
//             std::vector<KERNEL_NS::LibString> lines;
//             KERNEL_NS::FileUtil::ReadUtf8File(*fp, lines);
//             KERNEL_NS::FileUtil::CloseFile(*fp);
//             fp.pop();

//             bool isFound = false;
//             for(auto &lineData : lines)
//             {
//                 if(std::regex_match(lineData.GetRaw(), std::regex(".*#include.*\\<pch\\.h\\>.*")))
//                     isFound = true;
//             }

//             if(!isFound && !lines.empty())
//                 lines.insert(lines.begin(), "#include <pch.h>");

//             if(!KERNEL_NS::FileUtil::ReplaceFile(fullFilePath, lines))
//                 g_Log->Warn(LOGFMT_OBJ_TAG("replace file with new lines fail:%s"), fullFilePath.c_str());

//         } while (false);
        
//         return isContinue;
//     };

//     auto delg = KERNEL_CREATE_CLOSURE_DELEGATE(traverseCallback, bool, const KERNEL_NS::FindFileInfo &, bool &);
//     KERNEL_NS::DirectoryUtil::TraverseDirRecursively(_googleProtoIncludePath, delg);

//    g_Log->Custom("SCAN GOOGLE PROTO PATH TO ADD PCH HEADER PATH:%s SUCCESS.", _googleProtoIncludePath.c_str());
// }

bool ExporterMgr::_GenCSharp()
{
    if(_csharpOutPath.empty())
        return true;

    const auto appFullPath = GetServiceProxy()->GetApp()->GetAppPath();
    const auto appPath = KERNEL_NS::DirectoryUtil::GetFileDirInPath(appFullPath);
    const auto appName = GetServiceProxy()->GetApp()->GetAppName();
    auto protocPath = KERNEL_NS::DirectoryUtil::GetFileDirInPath(_cppProtocPath);
    auto protocName = KERNEL_NS::DirectoryUtil::GetFileNameInPath(_cppProtocPath);

    {// 添加执行权限
        Int32 err = 0;
        KERNEL_NS::LibString cmd;

        // 2.0的proto需要绝对路径的proto
        #if CRYSTAL_TARGET_PLATFORM_LINUX
            cmd.AppendFormat("sudo chmod a+x %s"
            , (appPath + protocPath + protocName).c_str());

            KERNEL_NS::LibString outInfo;
            if(!KERNEL_NS::SystemUtil::Exec(cmd, err, outInfo))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("give executable ability to %s fail, cmd:%s, err:%d, outInfo:%s")
                , (appPath + protocPath + protocName).c_str(), cmd.c_str(), err, outInfo.c_str());
                return false;
            }
        #endif
    }

    // 1.protoc 生成原始文件
    const auto csharpOutPath = (appPath + _csharpOutPath);
    KERNEL_NS::DirectoryUtil::CreateDir(csharpOutPath + "/");

    for(auto iter : _protoNameRefProtoInfo)
    {
        auto protoInfo = iter.second;
        Int32 err = 0;
        KERNEL_NS::LibString cmd;

        g_Log->Custom("[PROTO GEN CSHARP] %s ...", protoInfo->_protoInfo._fileName.c_str());

        auto rootPath = appPath + protoInfo->_protoInfo._rootPath;
        auto fullPath = appPath + protoInfo->_fullPathName;
        const auto fullProtoPath = appPath + _protoPath;
        auto relationPath = KERNEL_NS::DirectoryUtil::GetFileDirInPath(rootPath - fullProtoPath + "/");
        KERNEL_NS::DirectoryUtil::CreateDir(appPath + _csharpOutPath + relationPath);

        // 2.0的proto需要绝对路径的proto
        #if CRYSTAL_TARGET_PLATFORM_WINDOWS
            cmd.AppendFormat("cd %s && %s --csharp_out=%s --proto_path=%s --proto_path=%s %s"
            , (appPath + protocPath).c_str()
            , (protocName).c_str()
            , (appPath + _csharpOutPath + relationPath).c_str()
            , fullProtoPath.c_str()
            , (appPath + protoInfo->_protoInfo._rootPath).c_str()
            // , (appPath + protoInfo->_fullPathName).c_str());
            , protoInfo->_protoInfo._fileName.c_str());
        #else
            cmd.AppendFormat("%s --csharp_out=%s --proto_path=%s --proto_path=%s %s"
            , (appPath + protocPath + protocName).c_str()
            , (appPath + _csharpOutPath + relationPath).c_str()
            , fullProtoPath.c_str()
            , (appPath + protoInfo->_protoInfo._rootPath).c_str()
            // , (appPath + protoInfo->_fullPathName).c_str());
            , protoInfo->_protoInfo._fileName.c_str());
        #endif

        KERNEL_NS::LibString outInfo;
        if(!KERNEL_NS::SystemUtil::Exec(cmd, err, outInfo))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("protoc gen csharp fail proto file:%s cmd:%s, err:%d, outInfo:%s")
            , protoInfo->_fullPathName.c_str(), cmd.c_str(), err, outInfo.c_str());
            return false;
        }

        if(err != 0)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("[PROTO GEN CSHARP:%s FAILED]:%s"), protoInfo->_fullPathName.c_str(), outInfo.c_str());
            return false;
        }

        // 新的路径 = coutPath + 相对路径名 + 文件名
        auto protoName = KERNEL_NS::FileUtil::ExtractFileWithoutExtension(KERNEL_NS::DirectoryUtil::GetFileNameInPath(protoInfo->_fullPathName));
        auto csharpFileName = ProtobuffHelper::TreatCsharpName(protoName);

        rootPath = appPath + protoInfo->_protoInfo._rootPath;
        fullPath = appPath + protoInfo->_fullPathName;
        relationPath = KERNEL_NS::DirectoryUtil::GetFileDirInPath(rootPath - fullProtoPath + "/");
        
        auto wholeCSharpName = csharpOutPath + relationPath + csharpFileName + ".cs";
        KERNEL_NS::SmartPtr<FILE, KERNEL_NS::AutoDelMethods::CustomDelete> fp = KERNEL_NS::FileUtil::OpenFile(wholeCSharpName.c_str());
        if(!fp)
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("cant open proto csharp file file:%s"), wholeCSharpName.c_str());
            return false;
        }

        fp.SetClosureDelegate([](void *ptr){
            auto fpPtr = reinterpret_cast<FILE *>(ptr);
            KERNEL_NS::FileUtil::CloseFile(*fpPtr);
        });

        // 扫描头文件内容
        std::vector<KERNEL_NS::LibString> lineContents;
        auto line = KERNEL_NS::FileUtil::ReadUtf8File(*fp, lineContents);
        g_Log->Info(LOGFMT_OBJ_TAG("csharp file:%s line:%lld"), wholeCSharpName.c_str(), line);


        // 1.生成到Package的命名空间中
        _AddCsharpNamespace(lineContents);

        // 2.添加特性
        _ProtoMessageAttribute(protoInfo, lineContents);

        // 3.关闭文件
        KERNEL_NS::FileUtil::CloseFile(*fp);
        fp.pop();

        // 4.用新内容替换文件
        if(!KERNEL_NS::FileUtil::ReplaceFile(wholeCSharpName, lineContents))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("replace file fail:%s"), wholeCSharpName.c_str());
            return false;
        }

        g_Log->Custom("[PROTO GEN CSHARP] %s SUCCESS!", protoInfo->_protoInfo._fileName.c_str());
    }


    return true;
}

void ExporterMgr::_AddCsharpNamespace(std::vector<KERNEL_NS::LibString> &lines)
{
    // const KERNEL_NS::LibString namespaceName = "namespace ProtoPackage;";

    // 1.找到第一个类所在行并跳过连续的注释行,添加命名空间
    const Int32 maxLine = static_cast<Int32>(lines.size());
    bool isMatch = false;
    Int32 idx = 0;
    for(; idx < maxLine; ++idx)
    {
        auto &lineData = lines[idx];
        if(std::regex_match(lineData.GetRaw(), std::regex(".* class .* .*")))
        {// 找到第一个类
            // 跳过注释
            Int32 backIdx = idx - 1;
            for(; backIdx >= 0; --backIdx)
            {
                if(!ProtobuffHelper::IsNoteLine(lines[backIdx]))
                    break;
            }

            // 都是注释行
            if(backIdx < 0)
            {
                lines.insert(lines.begin(), "");
                lines.insert(lines.begin(), "using ProtoPackage.Attributes;");
            }
            else
            {
                lines.insert(lines.begin() + backIdx + 1, "");
                lines.insert(lines.begin() + backIdx + 1, "using ProtoPackage.Attributes;");
            }

            isMatch = true;
            break;
        }
    }

    if(!isMatch)
        g_Log->Warn(LOGFMT_OBJ_TAG("not found any class."));
}

void ExporterMgr::_ProtoMessageAttribute(const ProtoContentInfo *protoFile, std::vector<KERNEL_NS::LibString> &lines)
{
    const auto &classPatternRegex = std::regex(".* class .* .*");
    std::vector<KERNEL_NS::LibString> addLines;
    ProtobuffHelper::Modifylines(lines, addLines, 
    [protoFile, &classPatternRegex, this, &lines] (Int32 curLine, KERNEL_NS::LibString &lineData
    , std::vector<KERNEL_NS::LibString> &addLineDatasBefore
    , std::vector<KERNEL_NS::LibString> &addLineDatasAfter
    , bool &isContinue) -> bool
    {
        bool isMatched = std::regex_match(lineData.GetRaw(), classPatternRegex);
        if(isMatched)
        {
            // 1.获取类名
            auto classPos = lineData.GetRaw().find(ProtobufMessageParam::ClassFlag.GetRaw());
            KERNEL_NS::LibString classSub = lineData.GetRaw().substr(classPos);
            
            const auto &className = ProtobuffHelper::DragClass(classSub);
            auto messageInfo = protoFile->GetMessageInfo(className);
            if(!messageInfo)
                return false;

            if (messageInfo->_opcode == 0)
                return false;

            // 2.类前添加特性
            const auto annotationInfo = KERNEL_NS::LibString().AppendFormat("[ProtoMessage(%d)]", messageInfo->_opcode);
            addLineDatasBefore.push_back(annotationInfo);
        }

        isContinue = true;
        return isMatched;
    });
}

bool ExporterMgr::_GenTs()
{
    if(_tsOutPath.empty())
        return true;

    if(_protoNameRefProtoInfo.empty())
        return true;

    g_Log->Custom("[PROTO GEN] TS INFO...");
    const auto appName = GetServiceProxy()->GetApp()->GetAppName();
    const auto &appFullPath = GetServiceProxy()->GetApp()->GetAppPath();
    const auto appPath = KERNEL_NS::DirectoryUtil::GetFileDirInPath(appFullPath);

    const auto tsPbInfoFile = appPath  + "/" + _tsOutPath + "/ts_pbinfo.ts";
    KERNEL_NS::DirectoryUtil::CreateDir(appPath + _tsOutPath + "/");

    std::vector<KERNEL_NS::LibString> lines;

    // 文件头
    const auto fileHeader = _FileHeader(KERNEL_NS::LibTime::Now(), KERNEL_NS::LibString().AppendFormat("Generated By %s, Dont Modify This File!!!", appName.c_str()));
    lines.push_back(fileHeader);
    lines.push_back("");

    // 排序
    KERNEL_NS::BinaryArray<PbCaheInfo *, PbCacheInfoCompare> sortedArray;
    for(auto kv : _pbCacheContent->_lineRefMessageInfo)
    {
        auto messageInfo = kv.second;
        if(messageInfo->_opcode <= 0)
            continue;

        sortedArray.insert(kv.second);
    }

    // 命名空间
    lines.push_back("export namespace ts_pbinfo{");
    lines.push_back("");

    const Int64 arrSize = static_cast<Int64>(sortedArray.size());
    for(Int64 idx = 0; idx < arrSize; ++idx)
    {
        auto messageInfo = sortedArray[idx];
        lines.push_back(KERNEL_NS::LibString().AppendFormat("    // %s ", messageInfo->_protoName.c_str()));
        lines.push_back(KERNEL_NS::LibString().AppendFormat("    export class %s {", messageInfo->_messageName.c_str()));
        lines.push_back(KERNEL_NS::LibString().AppendFormat("      getOpcode():number {return %s.OPCODE; }", messageInfo->_messageName.c_str()));
        lines.push_back(KERNEL_NS::LibString().AppendFormat("      getIsXorEncrypt():boolean {return %s.XorEncrypt; }", messageInfo->_messageName.c_str()));
        lines.push_back(KERNEL_NS::LibString().AppendFormat("      getIsKeyBase64():boolean {return %s.KeyBase64; }", messageInfo->_messageName.c_str()));
        lines.push_back(KERNEL_NS::LibString().AppendFormat("      getOpcodeName():string {return %s.OPCODE_NAME; }", messageInfo->_messageName.c_str()));


        lines.push_back(KERNEL_NS::LibString().AppendFormat("      static OPCODE:number = %d;", messageInfo->_opcode));
        lines.push_back(KERNEL_NS::LibString().AppendFormat("      static OPCODE_NAME:string = \"%s\";", messageInfo->_messageName.c_str()));
        lines.push_back(KERNEL_NS::LibString().AppendFormat("      static XorEncrypt:boolean = %s;", messageInfo->_isXorEncrypt ? "true" : "false"));
        lines.push_back(KERNEL_NS::LibString().AppendFormat("      static KeyBase64:boolean = %s;", messageInfo->_isKeyBase64 ? "true" : "false"));
        lines.push_back("    }");
        lines.push_back("");
        lines.push_back("");
    }

    lines.push_back(KERNEL_NS::LibString().AppendFormat("    export class TsPbDict {"));
    lines.push_back(KERNEL_NS::LibString().AppendFormat("      static pb_dict = {"));

    for(Int64 idx = 0; idx < arrSize; ++idx)
    {
        auto messageInfo = sortedArray[idx];

        KERNEL_NS::LibString endCh;
        if(idx != (arrSize - 1))
            endCh = ",";

        lines.push_back(KERNEL_NS::LibString().AppendFormat("          [%s.OPCODE]: new %s()%s", messageInfo->_messageName.c_str(), messageInfo->_messageName.c_str(), endCh.c_str()));
        lines.push_back("");
    }

    lines.push_back(KERNEL_NS::LibString());
    lines.push_back(KERNEL_NS::LibString().AppendFormat("      }"));


    lines.push_back("");
    lines.push_back("    }");
    lines.push_back("}");
    
    if(!KERNEL_NS::FileUtil::ReplaceFile(tsPbInfoFile, lines))
    {
        g_Log->Custom("[PROTO GEN] TS INFO FAILED.");
        return false;
    }
    
    g_Log->Custom("[PROTO GEN] TS INFO SUCCESS.");

    return true;
}

bool ExporterMgr::_GenTsExtends()
{
    g_Log->Custom("[PROTO GEN TYPESCRIPT] PROTO PATH:%s", _protoPath.c_str());

    // 获取命名空间
    auto codeAnalyzeMgr = GetService()->GetComp<KERNEL_NS::ICodeAnalyzeMgr>();
    auto &allCodeUnit = codeAnalyzeMgr->GetAllCodeUnits();
    KERNEL_NS::LibString namespaceStr;
    for(auto iter : allCodeUnit)
    {
        auto &codeUnit = iter.second;

        if(!KERNEL_NS::CodeUnitFlags::HasFlags(codeUnit->_flags, KERNEL_NS::CodeUnitFlags::NAMESPACE_FLAG))
            continue;

        namespaceStr = codeUnit->_unitName;
        break;
    }

    if(namespaceStr.empty())
    {
        namespaceStr = "CRYSTAL_NET.service";
    }

    namespaceStr.findreplace(".", "_");
    namespaceStr = namespaceStr.tolower();

    const auto appName = GetServiceProxy()->GetApp()->GetAppName();
    const auto &appFullPath = GetServiceProxy()->GetApp()->GetAppPath();
    const auto appPath = KERNEL_NS::DirectoryUtil::GetFileDirInPath(appFullPath);

    KERNEL_NS::DirectoryUtil::CreateDir(appPath + _tsOutPath + "/");

    g_Log->Custom("[PROTO GEN TYPESCRIPT] GEN TYPESCRIPT OBJECT FILE:%s.ts...", namespaceStr.c_str());
    {
        const auto tsPbInfoFile = appPath  + "/" + _tsOutPath + "/" + namespaceStr + ".ts";
        std::vector<KERNEL_NS::LibString> lines;

        // 文件头
        const auto fileHeader = _FileHeader(KERNEL_NS::LibTime::Now(), KERNEL_NS::LibString().AppendFormat("Generated By %s, Dont Modify This File!!!", appName.c_str()));
        lines.push_back(fileHeader);
        lines.push_back("");

        lines.push_back(KERNEL_NS::LibString().AppendFormat("export namespace %s", namespaceStr.c_str()));
        lines.push_back("{");
        lines.push_back("");

        // 剔除枚举
        std::map<KERNEL_NS::LibString, KERNEL_NS::SmartPtr<KERNEL_NS::CodeUnit, KERNEL_NS::AutoDelMethods::Release>> enumCodeUnits;
        for(auto iter : allCodeUnit)
        {
            auto &fullName = iter.first;
            auto &codeUnit = iter.second;

            // 命名空间过滤
            if(KERNEL_NS::CodeUnitFlags::HasFlags(codeUnit->_flags, KERNEL_NS::CodeUnitFlags::NAMESPACE_FLAG))
                continue;

            // 是否oneof
            if(KERNEL_NS::CodeUnitFlags::HasFlags(codeUnit->_flags, CodeUnitFlagsExt::ONEOF_FIELD_FLAG))
                continue;

            // 类名
            if(KERNEL_NS::CodeUnitFlags::HasFlags(codeUnit->_flags, KERNEL_NS::CodeUnitFlags::ENUM_DATA_TYPE_DEFINE_FLAG))
            {
                continue;
            }

            auto &subCodeUnits = codeUnit->_subCodeUnits;
            const Int32 count = static_cast<Int32>(subCodeUnits.size());
            for(Int32 idx = 0; idx < count; ++idx)
            {
                auto &subCodeUnit = subCodeUnits[idx];
                if(KERNEL_NS::CodeUnitFlags::HasFlags(subCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::ENUM_DATA_TYPE_DEFINE_FLAG))
                {
                    enumCodeUnits.insert(std::make_pair(fullName, codeUnit));
                    break;
                }
            }
        }

        // 生成枚举
        for(auto iter : enumCodeUnits)
        {
            auto &codeUnit = iter.second;
            auto &subCodeUnits = codeUnit->_subCodeUnits;
            const Int32 count = static_cast<Int32>(subCodeUnits.size());
            for(Int32 idx = 0; idx < count; ++idx)
            {
                auto &subCodeUnit = subCodeUnits[idx];

                // 是否枚举
                if(KERNEL_NS::CodeUnitFlags::HasFlags(subCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::ENUM_DATA_TYPE_DEFINE_FLAG))
                {
                    // 注释
                    for(auto &noteLine : subCodeUnit->_comments)
                    {
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("    %s", noteLine.c_str()));
                    }

                    // 文件
                    lines.push_back(KERNEL_NS::LibString().AppendFormat("    // %s", codeUnit->_fileName.c_str()));
                    lines.push_back(KERNEL_NS::LibString().AppendFormat("    export enum %s_%s {", codeUnit->_unitName.c_str(), subCodeUnit->_unitName.c_str()));

                    auto &enumFieldCodeUnits = subCodeUnit->_subCodeUnits;
                    const Int32 enumCounts = static_cast<Int32>(enumFieldCodeUnits.size());
                    for(Int32 enumFieldIdx = 0; enumFieldIdx < enumCounts; ++enumFieldIdx)
                    {
                        auto &enumFieldCodeUnit = enumFieldCodeUnits[enumFieldIdx];
                        auto &params = enumFieldCodeUnit->_params;

                        // 注释
                        for(auto &noteLine : enumFieldCodeUnit->_comments)
                        {
                            lines.push_back(KERNEL_NS::LibString().AppendFormat("        %s", noteLine.c_str()));
                        }
                        if(params.empty())
                        {
                            lines.push_back(KERNEL_NS::LibString().AppendFormat("        %s,", enumFieldCodeUnit->_unitName.c_str()));
                        }
                        else
                        {
                            Int32 value = KERNEL_NS::StringUtil::StringToInt32(params[0].c_str());
                            lines.push_back(KERNEL_NS::LibString().AppendFormat("        %s = %d,", enumFieldCodeUnit->_unitName.c_str(), value));
                        }
                    }

                    lines.push_back("");
                    lines.push_back("    }");

                    continue;
                }
            }
        }

        // 1.生成ts对象
        for(auto iter : allCodeUnit)
        {
            auto &codeUnit = iter.second;

            // 命名空间过滤
            if(KERNEL_NS::CodeUnitFlags::HasFlags(codeUnit->_flags, KERNEL_NS::CodeUnitFlags::NAMESPACE_FLAG))
                continue;

            // 是否oneof
            if(KERNEL_NS::CodeUnitFlags::HasFlags(codeUnit->_flags, CodeUnitFlagsExt::ONEOF_FIELD_FLAG))
                continue;

            // 类名
            if(KERNEL_NS::CodeUnitFlags::HasFlags(codeUnit->_flags, KERNEL_NS::CodeUnitFlags::ENUM_DATA_TYPE_DEFINE_FLAG))
            {
                // 注释
                for(auto &noteLine : codeUnit->_comments)
                {
                    lines.push_back(KERNEL_NS::LibString().AppendFormat("    %s", noteLine.c_str()));
                }

                // 文件
                lines.push_back(KERNEL_NS::LibString().AppendFormat("    // %s", codeUnit->_fileName.c_str()));
                lines.push_back(KERNEL_NS::LibString().AppendFormat("    export enum %s {", codeUnit->_unitName.c_str()));
            }
            else
            {
                // 注释
                for(auto &noteLine : codeUnit->_comments)
                {
                    lines.push_back(KERNEL_NS::LibString().AppendFormat("    %s", noteLine.c_str()));
                }

                // 文件
                lines.push_back(KERNEL_NS::LibString().AppendFormat("    // %s", codeUnit->_fileName.c_str()));
                
                lines.push_back(KERNEL_NS::LibString().AppendFormat("    export class %s {", codeUnit->_unitName.c_str()));
            }

            // 字段
            if(KERNEL_NS::CodeUnitFlags::HasFlags(codeUnit->_flags, KERNEL_NS::CodeUnitFlags::ENUM_DATA_TYPE_DEFINE_FLAG))
            {
                auto &subCodeUnits = codeUnit->_subCodeUnits;
                const Int32 count = static_cast<Int32>(subCodeUnits.size());
                for(Int32 idx = 0; idx < count; ++idx)
                {
                    auto &subCodeUnit = subCodeUnits[idx];
                    auto &params = subCodeUnit->_params;
                    // 注释
                    for(auto &noteLine : subCodeUnit->_comments)
                    {
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("        %s", noteLine.c_str()));
                    }
                    if(params.empty())
                    {
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("        %s,", subCodeUnit->_unitName.c_str()));
                    }
                    else
                    {
                        Int32 value = KERNEL_NS::StringUtil::StringToInt32(params[0].c_str());
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("        %s = %d,", subCodeUnit->_unitName.c_str(), value));
                    }

                    lines.push_back("");
                }
            }
            else
            {
                auto &subCodeUnits = codeUnit->_subCodeUnits;
                const Int32 count = static_cast<Int32>(subCodeUnits.size());
                for(Int32 idx = 0; idx < count; ++idx)
                {
                    auto &subCodeUnit = subCodeUnits[idx];

                    // 是否枚举
                    if(KERNEL_NS::CodeUnitFlags::HasFlags(subCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::ENUM_DATA_TYPE_DEFINE_FLAG))
                    {
                        continue;
                    }

                    // 注释
                    for(auto &noteLine : subCodeUnit->_comments)
                    {
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("        %s", noteLine.c_str()));
                    }
                    
                    KERNEL_NS::LibString dataType;
                    dataType.AppendFormat("        %s", subCodeUnit->_unitName.c_str());
                    // 是否可选
                    if(KERNEL_NS::CodeUnitFlags::HasFlags(subCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::OPTION_FILED_FLAG))
                    {
                        dataType.AppendFormat("?:");
                    }
                    else
                    {
                        dataType.AppendFormat(":");
                    }

                    // 数值类型类型
                    if(KERNEL_NS::CodeUnitFlags::HasFlags(subCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::NUMBER_FIELD_FLAG))
                    {
                        dataType.AppendFormat("number");
                    }
                    else if(KERNEL_NS::CodeUnitFlags::HasFlags(subCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::BOOL_FIELD_FLAG))
                    {
                        dataType.AppendFormat("boolean");
                    }
                    else if(KERNEL_NS::CodeUnitFlags::HasFlags(subCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::STRING_FIELD_FLAG))
                    {
                        dataType.AppendFormat("string");
                    }
                    else if(KERNEL_NS::CodeUnitFlags::HasFlags(subCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::CUSTOM_FIELD_FLAG))
                    {
                        auto &params = subCodeUnit->_params;
                        auto &codeUnitFullName = params[0];
                        auto customCodeUnit = codeAnalyzeMgr->GetCodeUnit(codeUnitFullName);
                        dataType.AppendFormat("%s", customCodeUnit->_unitName.c_str());
                    }

                    // 是否数组
                    if(KERNEL_NS::CodeUnitFlags::HasFlags(subCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::ARRAY_FIELD_FLAG))
                    {
                        dataType.AppendFormat("[]");
                    }

                    if(!KERNEL_NS::CodeUnitFlags::HasFlags(subCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::OPTION_FILED_FLAG))
                    {
                        if(KERNEL_NS::CodeUnitFlags::HasFlags(subCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::ARRAY_FIELD_FLAG))
                        {
                            dataType.AppendFormat(" = [];");
                        }
                        else
                        {
                            if(KERNEL_NS::CodeUnitFlags::HasFlags(subCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::NUMBER_FIELD_FLAG))
                            {
                                dataType.AppendFormat(" = 0;");
                            }
                            else if(KERNEL_NS::CodeUnitFlags::HasFlags(subCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::BOOL_FIELD_FLAG))
                            {
                                dataType.AppendFormat(" = false;");
                            }
                            else if(KERNEL_NS::CodeUnitFlags::HasFlags(subCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::STRING_FIELD_FLAG))
                            {
                                dataType.AppendFormat(" = \"\";");
                            }
                            else if(KERNEL_NS::CodeUnitFlags::HasFlags(subCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::CUSTOM_FIELD_FLAG))
                            {
                                auto &params = subCodeUnit->_params;
                                auto &codeUnitFullName = params[0];
                                auto customCodeUnit = codeAnalyzeMgr->GetCodeUnit(codeUnitFullName);
                                dataType.AppendFormat(" = new %s();", customCodeUnit->_unitName.c_str());
                            }
                        }
                    }
                    else
                    {
                        dataType += ";";
                    }

                    lines.push_back(dataType);
                    lines.push_back("");
                }
            }

            lines.push_back("");
            lines.push_back("    }");
        }

        lines.push_back("");
        lines.push_back("}");

        if(!KERNEL_NS::FileUtil::ReplaceFile(tsPbInfoFile, lines))
        {
            g_Log->Custom("[PROTO GEN TYPESCRIPT] %s.ts FAILED.", namespaceStr.c_str());
            return false;
        }
    }
    g_Log->Custom("[PROTO GEN TYPESCRIPT] GEN TYPESCRIPT OBJECT FILE:%s.ts SUCCESS.", namespaceStr.c_str());

    // 2.生成 parser文件
    {
        const auto ts_objModule = namespaceStr;
        namespaceStr = namespaceStr + "_parser";

        g_Log->Custom("[PROTO GEN TYPESCRIPT] GEN TYPESCRIPT OBJECT PARSER FILE:%s.ts...", namespaceStr.c_str());

        const auto tsPbInfoFile = appPath  + "/" + _tsOutPath + "/" + namespaceStr + ".ts";
        std::vector<KERNEL_NS::LibString> lines;

        // 文件头
        const auto fileHeader = _FileHeader(KERNEL_NS::LibTime::Now(), KERNEL_NS::LibString().AppendFormat("Generated By %s, Dont Modify This File!!!", appName.c_str()));
        lines.push_back(fileHeader);
        lines.push_back("");

        // 导入
        lines.push_back(KERNEL_NS::LibString().AppendFormat("import { %s } from \"./%s\";", ts_objModule.c_str(), ts_objModule.c_str()));
        lines.push_back("");

        lines.push_back(KERNEL_NS::LibString().AppendFormat("export namespace %s", namespaceStr.c_str()));
        lines.push_back("{");
        lines.push_back("");

        // parser
        lines.push_back(KERNEL_NS::LibString().AppendFormat("    export class AllParsers {"));
        lines.push_back(KERNEL_NS::LibString());

        // 1.字典
        lines.push_back(KERNEL_NS::LibString().AppendFormat("        AllParsersDict:Map<string, (jsonData:any)=>any> = new Map<string, (jsonData:any)=>any>();"));
        lines.push_back(KERNEL_NS::LibString());

        // 2.构造初始化parser字典 TODO:
        lines.push_back(KERNEL_NS::LibString().AppendFormat("        constructor()"));
        lines.push_back(KERNEL_NS::LibString().AppendFormat("        {"));

        // 1.构造注册parser方法
        for(auto iter : allCodeUnit)
        {
            auto &codeUnit = iter.second;

            // 命名空间过滤
            if(KERNEL_NS::CodeUnitFlags::HasFlags(codeUnit->_flags, KERNEL_NS::CodeUnitFlags::NAMESPACE_FLAG))
                continue;

            // 是否oneof
            if(KERNEL_NS::CodeUnitFlags::HasFlags(codeUnit->_flags, CodeUnitFlagsExt::ONEOF_FIELD_FLAG))
                continue;

            // 是否枚举
            if(KERNEL_NS::CodeUnitFlags::HasFlags(codeUnit->_flags, KERNEL_NS::CodeUnitFlags::ENUM_DATA_TYPE_DEFINE_FLAG))
                continue;

            if(!KERNEL_NS::CodeUnitFlags::HasFlags(codeUnit->_flags, KERNEL_NS::CodeUnitFlags::DATA_TYPE_DEFINE_FLAG))
                continue;

            lines.push_back(KERNEL_NS::LibString());
            lines.push_back(KERNEL_NS::LibString().AppendFormat("            this.add(\"%s\", (jsonData:any):any =>{", codeUnit->_unitName.c_str()));
            lines.push_back(KERNEL_NS::LibString().AppendFormat("                return this.%sParser(jsonData);", codeUnit->_unitName.c_str()));
            lines.push_back(KERNEL_NS::LibString().AppendFormat("            })"));
            lines.push_back(KERNEL_NS::LibString());
        }

        lines.push_back(KERNEL_NS::LibString().AppendFormat("        }"));
        lines.push_back(KERNEL_NS::LibString());

        // 3.添加add方法
        lines.push_back(KERNEL_NS::LibString().AppendFormat("        public add(pbType:string, cb:(jsonData:any)=>any)"));
        lines.push_back(KERNEL_NS::LibString().AppendFormat("        {"));
        lines.push_back(KERNEL_NS::LibString().AppendFormat("            this.AllParsersDict.set(pbType, cb);"));
        lines.push_back(KERNEL_NS::LibString().AppendFormat("        }"));
        lines.push_back(KERNEL_NS::LibString());

        // 4.添加getParser方法
        lines.push_back(KERNEL_NS::LibString().AppendFormat("        public getParser(pbType:string):((jsonData:any)=>any)|undefined"));
        lines.push_back(KERNEL_NS::LibString().AppendFormat("        {"));
        lines.push_back(KERNEL_NS::LibString().AppendFormat("            var cb = this.AllParsersDict.get(pbType);"));
        lines.push_back(KERNEL_NS::LibString().AppendFormat("            if(cb == undefined)"));
        lines.push_back(KERNEL_NS::LibString().AppendFormat("            {"));
        lines.push_back(KERNEL_NS::LibString().AppendFormat("                console.log(\"have no parser, please check pbType:\", pbType)"));
        lines.push_back(KERNEL_NS::LibString().AppendFormat("            }"));
        lines.push_back(KERNEL_NS::LibString().AppendFormat("            return cb;"));
        lines.push_back(KERNEL_NS::LibString().AppendFormat("        }"));
        lines.push_back(KERNEL_NS::LibString());

        // 5.添加具体parser的私有方法
        for(auto iter : allCodeUnit)
        {
            auto &codeUnit = iter.second;

            // 命名空间过滤
            if(KERNEL_NS::CodeUnitFlags::HasFlags(codeUnit->_flags, KERNEL_NS::CodeUnitFlags::NAMESPACE_FLAG))
                continue;

            // 是否oneof
            if(KERNEL_NS::CodeUnitFlags::HasFlags(codeUnit->_flags, CodeUnitFlagsExt::ONEOF_FIELD_FLAG))
                continue;

            // 是否枚举
            if(KERNEL_NS::CodeUnitFlags::HasFlags(codeUnit->_flags, KERNEL_NS::CodeUnitFlags::ENUM_DATA_TYPE_DEFINE_FLAG))
                continue;

            if(!KERNEL_NS::CodeUnitFlags::HasFlags(codeUnit->_flags, KERNEL_NS::CodeUnitFlags::DATA_TYPE_DEFINE_FLAG))
                continue;

            lines.push_back(KERNEL_NS::LibString());
            lines.push_back(KERNEL_NS::LibString().AppendFormat("            private %sParser(jsonData:any):any{", codeUnit->_unitName.c_str()));
            lines.push_back(KERNEL_NS::LibString().AppendFormat("                var newInfo = new %s.%s()", ts_objModule.c_str(), codeUnit->_unitName.c_str()));

            // 字段
            const Int32 objFieldCount = static_cast<Int32>(codeUnit->_subCodeUnits.size());
            for(Int32 objFieldIdx = 0; objFieldIdx < objFieldCount; ++objFieldIdx)
            {
                auto &objFieldCodeUnit = codeUnit->_subCodeUnits[objFieldIdx];

                // 过滤枚举
                if(KERNEL_NS::CodeUnitFlags::HasFlags(objFieldCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::ENUM_DATA_TYPE_DEFINE_FLAG))
                    continue;

                if(KERNEL_NS::CodeUnitFlags::HasFlags(objFieldCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::ARRAY_FIELD_FLAG))
                {// 数组
                    lines.push_back(KERNEL_NS::LibString());
                    lines.push_back(KERNEL_NS::LibString().AppendFormat("                if(jsonData.%s != undefined)", objFieldCodeUnit->_unitName.c_str()));
                    lines.push_back(KERNEL_NS::LibString().AppendFormat("                {"));
                    lines.push_back(KERNEL_NS::LibString().AppendFormat("                    var jsonArray = jsonData.%s.map((value, idx, arr)=>{", objFieldCodeUnit->_unitName.c_str()));
                    
                    if(KERNEL_NS::CodeUnitFlags::HasFlags(objFieldCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::NUMBER_FIELD_FLAG))
                    {
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                    return parseFloat(value);"));
                    }
                    else if(KERNEL_NS::CodeUnitFlags::HasFlags(objFieldCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::BOOL_FIELD_FLAG))
                    {
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                    return value == \"true\";"));
                    }
                    else if(KERNEL_NS::CodeUnitFlags::HasFlags(objFieldCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::STRING_FIELD_FLAG))
                    {
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                    return value;"));
                    }
                    else if(KERNEL_NS::CodeUnitFlags::HasFlags(objFieldCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::CUSTOM_FIELD_FLAG))
                    {
                        auto &dataTypeFullName = objFieldCodeUnit->_params[0];
                        auto &fieldDataTypeCodeUnit = codeAnalyzeMgr->GetCodeUnit(dataTypeFullName);

                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                    var parser = this.getParser(\"%s\");", fieldDataTypeCodeUnit->_unitName.c_str()));
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                    if(parser == undefined)"));
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                        return {}"));
                        lines.push_back(KERNEL_NS::LibString());
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                    return parser(value);"));
                    }

                    lines.push_back(KERNEL_NS::LibString().AppendFormat("                    });"));
                    lines.push_back(KERNEL_NS::LibString());
                    lines.push_back(KERNEL_NS::LibString().AppendFormat("                    newInfo.%s = jsonArray;", objFieldCodeUnit->_unitName.c_str()));
                    lines.push_back(KERNEL_NS::LibString().AppendFormat("                }"));
                    lines.push_back(KERNEL_NS::LibString());
                }

                // 非数组
                else
                {
                    if(KERNEL_NS::CodeUnitFlags::HasFlags(objFieldCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::NUMBER_FIELD_FLAG))
                    {// 数值
                        lines.push_back(KERNEL_NS::LibString());
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                if(jsonData.%s != undefined)", objFieldCodeUnit->_unitName.c_str()));
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                {"));
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                    newInfo.%s = parseFloat(jsonData.%s);", objFieldCodeUnit->_unitName.c_str(), objFieldCodeUnit->_unitName.c_str()));
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                }"));
                        lines.push_back(KERNEL_NS::LibString());
                    }
                    else if(KERNEL_NS::CodeUnitFlags::HasFlags(objFieldCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::STRING_FIELD_FLAG))
                    {
                        lines.push_back(KERNEL_NS::LibString());
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                if(jsonData.%s != undefined)", objFieldCodeUnit->_unitName.c_str()));
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                {"));
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                    newInfo.%s = jsonData.%s;", objFieldCodeUnit->_unitName.c_str(), objFieldCodeUnit->_unitName.c_str()));
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                }"));
                        lines.push_back(KERNEL_NS::LibString());
                    }
                    else if(KERNEL_NS::CodeUnitFlags::HasFlags(objFieldCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::BOOL_FIELD_FLAG))
                    {
                        lines.push_back(KERNEL_NS::LibString());
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                if(jsonData.%s != undefined)", objFieldCodeUnit->_unitName.c_str()));
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                {"));
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                    newInfo.%s = jsonData.%s == \"true\";", objFieldCodeUnit->_unitName.c_str(), objFieldCodeUnit->_unitName.c_str()));
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                }"));
                        lines.push_back(KERNEL_NS::LibString());
                    }
                    else if(KERNEL_NS::CodeUnitFlags::HasFlags(objFieldCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::CUSTOM_FIELD_FLAG))
                    {
                        lines.push_back(KERNEL_NS::LibString());
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                if(jsonData.%s != undefined)", objFieldCodeUnit->_unitName.c_str()));
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                {"));
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                    var jsonFieldInfo = jsonData.%s;", objFieldCodeUnit->_unitName.c_str()));

                        auto &dataTypeFullName = objFieldCodeUnit->_params[0];
                        auto &fieldDataTypeCodeUnit = codeAnalyzeMgr->GetCodeUnit(dataTypeFullName);

                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                    var cb = this.getParser(\"%s\");", fieldDataTypeCodeUnit->_unitName.c_str()));
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                    if(cb != undefined)"));
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                    {"));
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                        newInfo.%s = cb(jsonFieldInfo);", objFieldCodeUnit->_unitName.c_str()));
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                    }"));
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                    else"));
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                    {"));
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                        console.log(\"field %s have no %s parser\");", objFieldCodeUnit->_unitName.c_str(), fieldDataTypeCodeUnit->_unitName.c_str()));
                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                    }"));

                        lines.push_back(KERNEL_NS::LibString().AppendFormat("                }"));
                        lines.push_back(KERNEL_NS::LibString());
                    }
                }
            }
            lines.push_back(KERNEL_NS::LibString().AppendFormat("                 return newInfo;"));
            lines.push_back(KERNEL_NS::LibString().AppendFormat("            }"));
            lines.push_back(KERNEL_NS::LibString());
        }
        

        lines.push_back(KERNEL_NS::LibString());
        lines.push_back(KERNEL_NS::LibString().AppendFormat("    }"));

        lines.push_back("");
        lines.push_back("}");

        if(!KERNEL_NS::FileUtil::ReplaceFile(tsPbInfoFile, lines))
        {
            g_Log->Custom("[PROTO GEN TYPESCRIPT] %s.ts FAILED.", namespaceStr.c_str());
            return false;
        }

        g_Log->Custom("[PROTO GEN TYPESCRIPT] GEN TYPESCRIPT OBJECT PARSER FILE:%s.ts SUCCESS.", namespaceStr.c_str());
    }

    g_Log->Custom("[PROTO GEN TYPESCRIPT] SUCCESS.");

    // 1.获取所有message信息
    // 2.添加protobuf 数据类型的parser 
    // protobuf data type: double/float/int32/uint32/uint64/sint32/sint64/fixed32/fixed64/sfixed32/sfixed64/bool/string/bytes/
    // double/float/int32/uint32/uint64/sint32/sint64/fixed32/fixed64/sfixed32/sfixed64 => number parseFloat()
    // bool => true/false v == true
    // string => string
    // bytes => base64 string => string

    // 3.识别protobuf repeated/bytes
    return true;
}


void ExporterMgr::_GenORM()
{

}

bool ExporterMgr::_GrammarAnalyze()
{
    g_Log->Custom("[PROTO GRAMMER ANALYZE] PROTO PATH:%s", _protoPath.c_str());

    auto codeAnalyzeMgr = GetService()->GetComp<KERNEL_NS::ICodeAnalyzeMgr>();

    // 不支持Map
    auto &codeUnitStack = codeAnalyzeMgr->GetCodeUnitStack();
    KERNEL_NS::LibString packageName;
    codeAnalyzeMgr->ScanDir(_protoPath, {".proto"}, [codeAnalyzeMgr, &codeUnitStack, &packageName, this](KERNEL_NS::LibString &validData, Int32 currentLine, const KERNEL_NS::LibString &fullPath, const std::vector<KERNEL_NS::LibString> &lineDatas){
        validData.strip();
        if(ProtobuffHelper::HasSyntax(validData))
            return;

        if(ProtobuffHelper::IsImport(validData))
            return;
        
        // package
        auto &areaUnits = codeUnitStack->_codeUnits;
        if(packageName.empty())
        {
            ProtobuffHelper::GetPackageName(validData, packageName, "");

            if(!packageName.empty())
            {
                auto &packageCodeUnit = codeAnalyzeMgr->GetCodeUnit(packageName);
                if(!packageCodeUnit)
                {
                    auto &&newCodeUnit = KERNEL_NS::CodeUnit::CreateCodeUnit();
                    packageName.strip();
                    newCodeUnit->_unitName = packageName;
                    newCodeUnit->_line = currentLine + 1;
                    newCodeUnit->_fileName = KERNEL_NS::DirectoryUtil::GetFileNameInPath(fullPath);

                    if(!areaUnits.empty())
                    {
                        for(auto &unit : areaUnits)
                        {
                            newCodeUnit->_belongToArea.push_back(unit->_unitName);
                        }
                    }

                    KERNEL_NS::CodeUnitFlags::SetFlags(newCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::NAMESPACE_FLAG);

                    newCodeUnit->_isStarted = true;
                    newCodeUnit->_isEnd = true;

                    areaUnits.push_back(newCodeUnit);

                    codeAnalyzeMgr->AddCodeUnit(newCodeUnit);
                }
                else
                {
                    areaUnits.push_back(packageCodeUnit);
                }

                return;
            }
        }

        auto &currentCodeUnit = codeUnitStack->GetCurrentCodeUnit();

        {
            KERNEL_NS::LibString enumDataTypeName;
            if(ProtobuffHelper::HasMessage(validData))
            {
                size_t endPos = std::string::npos;
                auto &&messageName = ProtobuffHelper::DragMessageSeg(validData, endPos);
                if(!messageName.empty())
                {
                    auto &&newCodeUnit = KERNEL_NS::CodeUnit::CreateCodeUnit();
                    messageName.strip();
                    newCodeUnit->_unitName = messageName;
                    newCodeUnit->_line = currentLine + 1;
                    newCodeUnit->_fileName = KERNEL_NS::DirectoryUtil::GetFileNameInPath(fullPath);

                    if(!areaUnits.empty())
                    {
                        for(auto &unit : areaUnits)
                        {
                            newCodeUnit->_belongToArea.push_back(unit->_unitName);
                        }
                    }

                    // 注释
                    ProtobuffHelper::BuildComments(currentLine, lineDatas, newCodeUnit->_comments);

                    KERNEL_NS::CodeUnitFlags::SetFlags(newCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::DATA_TYPE_DEFINE_FLAG);
                    codeAnalyzeMgr->AddCodeUnit(newCodeUnit);
                    codeUnitStack->_codeUnits.push_back(newCodeUnit);
                    return;
                }
            }
            else if(ProtobuffHelper::GetEnumDataTypeDefine(validData, enumDataTypeName))
            {
                auto &&newCodeUnit = KERNEL_NS::CodeUnit::CreateCodeUnit();
                newCodeUnit->_unitName = enumDataTypeName;
                newCodeUnit->_line = currentLine + 1;
                newCodeUnit->_fileName = KERNEL_NS::DirectoryUtil::GetFileNameInPath(fullPath);

                if(!areaUnits.empty())
                {
                    for(auto &unit : areaUnits)
                    {
                        newCodeUnit->_belongToArea.push_back(unit->_unitName);
                    }
                }

                // 注释
                ProtobuffHelper::BuildComments(currentLine, lineDatas, newCodeUnit->_comments);
                KERNEL_NS::CodeUnitFlags::SetFlags(newCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::ENUM_DATA_TYPE_DEFINE_FLAG);
                if(KERNEL_NS::CodeUnitFlags::HasFlags(currentCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::NAMESPACE_FLAG))
                {
                    codeAnalyzeMgr->AddCodeUnit(newCodeUnit);
                }
                else
                {
                    currentCodeUnit->_subCodeUnits.push_back(newCodeUnit);
                }
                codeUnitStack->_codeUnits.push_back(newCodeUnit);
                return;
            }
            else if(ProtobuffHelper::IsField(validData))
            {
                if(!currentCodeUnit)
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("have no any code unit but analyze a field:%s, file:%s, line:%d")
                        , validData.c_str(), fullPath.c_str(), currentLine);
                    return;
                }

                if(!currentCodeUnit->_isStarted)
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("code unit not start code unit:%s but analyze a field:%s, file:%s, line:%d")
                        , currentCodeUnit->_unitName.c_str(), validData.c_str(), fullPath.c_str(), currentLine);
                    return;
                }

                if(currentCodeUnit->_isEnd)
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("code unit had finished code unit:%s but analyze a field:%s, file:%s, line:%d")
                        , currentCodeUnit->_unitName.c_str(), validData.c_str(), fullPath.c_str(), currentLine);
                    return;
                }

                validData.strip();
                validData.findreplace("=", " =", 1);
                if(ProtobuffHelper::IsEnumField(validData))
                {
                    auto &&parts = validData.Split(" ");
                    std::vector<KERNEL_NS::LibString> finalParts;
                    const Int32 count = static_cast<Int32>(parts.size());
                    for(Int32 idx = 0; idx < count; ++idx)
                    {
                        auto &itemPart = parts[idx];
                        itemPart.strip();
                        if(itemPart.empty())
                            continue;

                        if(itemPart == "=")
                            continue;

                        finalParts.push_back(itemPart);
                    }

                    if(!finalParts.empty())
                    {
                        if(finalParts[finalParts.size() - 1] == ";")
                            finalParts.pop_back();
                        else
                        {
                            auto &tagPart = finalParts[finalParts.size() - 1];
                            if(tagPart[tagPart.size() - 1] == ';')
                                tagPart.GetRaw().pop_back();
                            tagPart.strip();
                        }
                    }

                    if(finalParts.empty())
                        return;

                    if(finalParts.size() < 2)
                        return;

                    KERNEL_NS::LibString fieldName;
                    KERNEL_NS::LibString tagId;
                    fieldName = finalParts[0];
                    tagId = finalParts[1];

                    auto &&newCodeUnit = KERNEL_NS::CodeUnit::CreateCodeUnit();
                    newCodeUnit->_unitName = fieldName;
                    newCodeUnit->_line = currentLine + 1;
                    newCodeUnit->_fileName = KERNEL_NS::DirectoryUtil::GetFileNameInPath(fullPath);
                    if(!areaUnits.empty())
                    {
                        for(auto &unit : areaUnits)
                        {
                            newCodeUnit->_belongToArea.push_back(unit->_unitName);
                        }
                    }

                    // 注释
                    ProtobuffHelper::BuildComments(currentLine, lineDatas, newCodeUnit->_comments);
                    ProtobuffHelper::GetComment(validData, newCodeUnit->_commentSameLine);

                    KERNEL_NS::CodeUnitFlags::SetFlags(newCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::ENUM_FIELD_FLAG);
                    newCodeUnit->_params.push_back(tagId);

                    newCodeUnit->_isStarted = true;
                    newCodeUnit->_isEnd = true;

                    currentCodeUnit->_subCodeUnits.push_back(newCodeUnit);
                }
                else
                {
                    auto &&parts = validData.Split(" ");
                    std::vector<KERNEL_NS::LibString> finalParts;
                    const Int32 count = static_cast<Int32>(parts.size());
                    for(Int32 idx = 0; idx < count; ++idx)
                    {
                        auto &itemPart = parts[idx];
                        itemPart.strip();
                        if(itemPart.empty())
                            continue;

                        if(itemPart == "=")
                            continue;

                        finalParts.push_back(itemPart);
                    }

                    if(!finalParts.empty())
                    {
                        if(finalParts[finalParts.size() - 1] == ";")
                            finalParts.pop_back();
                        else
                        {
                            auto &tagPart = finalParts[finalParts.size() - 1];
                            if(tagPart[tagPart.size() - 1] == ';')
                                tagPart.GetRaw().pop_back();
                            tagPart.strip();
                        }
                    }

                    if(finalParts.empty())
                        return;

                    if(finalParts.size() < 3)
                        return;

                    KERNEL_NS::LibString dataType;
                    KERNEL_NS::LibString fieldName;
                    KERNEL_NS::LibString tagId;
                    KERNEL_NS::LibString prefix;
                    if(finalParts.size() == 4)
                    {
                        prefix = finalParts[0];
                        dataType = finalParts[1];
                        fieldName = finalParts[2];
                        tagId = finalParts[3];
                    }
                    else
                    {
                        dataType = finalParts[0];
                        fieldName = finalParts[1];
                        tagId = finalParts[2];
                    }

                    prefix.strip();
                    dataType.strip();
                    fieldName.strip();
                    tagId.strip();

                    auto &&newCodeUnit = KERNEL_NS::CodeUnit::CreateCodeUnit();
                    newCodeUnit->_unitName = fieldName;
                    newCodeUnit->_line = currentLine + 1;
                    newCodeUnit->_fileName = KERNEL_NS::DirectoryUtil::GetFileNameInPath(fullPath);
                    if(!areaUnits.empty())
                    {
                        if(KERNEL_NS::CodeUnitFlags::HasFlags(currentCodeUnit->_flags, CodeUnitFlagsExt::ONEOF_FIELD_FLAG))
                        {
                            KERNEL_NS::CodeUnitFlags::SetFlags(newCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::OPTION_FILED_FLAG);

                            const Int32 allUnitsCount = static_cast<Int32>(areaUnits.size());
                            for(Int32 idx = 0; idx < (allUnitsCount - 1); ++idx)
                            {
                                auto &codeUnitItem = areaUnits[idx];
                                newCodeUnit->_belongToArea.push_back(codeUnitItem->_unitName);
                            }
                        }
                        else
                        {
                            for(auto &unit : areaUnits)
                            {
                                newCodeUnit->_belongToArea.push_back(unit->_unitName);
                            }
                        }
                    }

                    // 注释
                    ProtobuffHelper::BuildComments(currentLine, lineDatas, newCodeUnit->_comments);
                    ProtobuffHelper::GetComment(validData, newCodeUnit->_commentSameLine);

                    if(ProtobuffHelper::IsMapDataType(dataType))
                    {
                        g_Log->Warn(LOGFMT_OBJ_TAG("cant support map data type file name:%s, line:%d"), fullPath.c_str(), currentLine);
                        return;
                    }

                    if(prefix == "repeated")
                    {
                        KERNEL_NS::CodeUnitFlags::SetFlags(newCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::ARRAY_FIELD_FLAG);
                    }

                    if(_protobufBaseDataType.find(dataType) != _protobufBaseDataType.end())
                    {// protobuf数据类型
                        if( (dataType == "double"   )|| 
                            (dataType == "float"    )||
                            (dataType == "int32"    )||
                            (dataType == "uint32"   )||
                            (dataType == "uint64"   )||
                            (dataType == "sint32"   )||
                            (dataType == "sint64"   )||
                            (dataType == "fixed32"  )||
                            (dataType == "fixed64"  )||
                            (dataType == "sfixed32" )||
                            (dataType == "sfixed64")
                        )
                        {
                            KERNEL_NS::CodeUnitFlags::SetFlags(newCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::NUMBER_FIELD_FLAG);
                        }
                        else if(dataType == "bool")
                        {
                            KERNEL_NS::CodeUnitFlags::SetFlags(newCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::BOOL_FIELD_FLAG);
                        }
                        else if((dataType == "string") || (dataType == "bytes"))
                        {
                            KERNEL_NS::CodeUnitFlags::SetFlags(newCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::STRING_FIELD_FLAG);
                        }
                    }
                    else
                    {
                        KERNEL_NS::CodeUnitFlags::SetFlags(newCodeUnit->_flags, KERNEL_NS::CodeUnitFlags::CUSTOM_FIELD_FLAG);
                        
                        KERNEL_NS::LibString dataTypeFullName;
                        dataTypeFullName.AppendData(packageName);
                        dataTypeFullName += dataType.findreplace(".", "");
                        newCodeUnit->_params.push_back(dataTypeFullName);
                    }

                    newCodeUnit->_isStarted = true;
                    newCodeUnit->_isEnd = true;

                    // 如果是oneof 字段, 则是可选字段
                    if(KERNEL_NS::CodeUnitFlags::HasFlags(currentCodeUnit->_flags, CodeUnitFlagsExt::ONEOF_FIELD_FLAG))
                    {
                        auto &allUnits = codeUnitStack->_codeUnits;
                        auto &realCodeUnit = allUnits[allUnits.size() - 2];
                        realCodeUnit->_subCodeUnits.push_back(newCodeUnit);
                    }
                    else
                    {
                        currentCodeUnit->_subCodeUnits.push_back(newCodeUnit);
                    }
                }
            }
            else if(ProtobuffHelper::HasUnitStart(validData))
            {
                currentCodeUnit->_isStarted = true;
            }
            else if(ProtobuffHelper::HasUnitEnd(validData))
            {
                if(currentCodeUnit)
                {
                    currentCodeUnit->_isEnd = true;
                    codeUnitStack->_codeUnits.pop_back();
                    return;
                }
            }
            else if(ProtobuffHelper::IsOneOf(validData))
            {
                KERNEL_NS::LibString oneofName;
                ProtobuffHelper::GetOneOfName(validData, oneofName);
                if(oneofName.empty())
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("oneof name is empty file name:%s, line:%d"), fullPath.c_str(), currentLine);
                    return;
                }

                auto &&newCodeUnit = KERNEL_NS::CodeUnit::CreateCodeUnit();
                newCodeUnit->_unitName = oneofName;
                newCodeUnit->_line = currentLine + 1;
                newCodeUnit->_fileName = KERNEL_NS::DirectoryUtil::GetFileNameInPath(fullPath);

                if(!areaUnits.empty())
                {
                    for(auto &unit : areaUnits)
                    {
                        newCodeUnit->_belongToArea.push_back(unit->_unitName);
                    }
                }

                // 注释
                ProtobuffHelper::BuildComments(currentLine, lineDatas, newCodeUnit->_comments);
                KERNEL_NS::CodeUnitFlags::SetFlags(newCodeUnit->_flags, CodeUnitFlagsExt::ONEOF_FIELD_FLAG);
                codeAnalyzeMgr->AddCodeUnit(newCodeUnit);
                codeUnitStack->_codeUnits.push_back(newCodeUnit);
            }
        }
    }, 
    
    // 文件扫描结束
    [&codeUnitStack, &packageName](){
        codeUnitStack->_codeUnits.pop_back();
        packageName.clear();
    });

    g_Log->Custom("[PROTO GRAMMER ANALYZE] SUCCESS.");

    return true;
}

bool ExporterMgr::_ScanProtoDataType(const KERNEL_NS::FindFileInfo &fileInfo, const KERNEL_NS::LibString &fullFilePath, bool &isParentPathContinue)
{
//     // 嵌套的数据类型会按照堆栈顺序变化域, 比如当前扫描到 message A { message B { message C { 则域message C 的所在域是 ::A::B:: 链表数据是：[A, B], 当生成C的数据类型时，数据C会携带当前属于它的域
//     std::list<KERNEL_NS::LibString> _currentArea;
    
//     // 当前的数据类型是链表的最后一个节点, 语法分析其实是一个堆栈结构, 当内部不再有内嵌的数据类型, 且数据类型符合message xxx {}, 那么这个数据类型封闭, 且会被弹栈, 接下来的部分就是弹栈后栈顶的数据类型部分
//     std::list<KERNEL_NS::SmartPtr<ProtobufDataTypeInfo, KERNEL_NS::AutoDelMethods::Release>> _handlingDataTypes;

//   // 加载文件内容到内存
//     KERNEL_NS::SmartPtr<FILE, KERNEL_NS::AutoDelMethods::CustomDelete> fp = KERNEL_NS::FileUtil::OpenFile(fullFilePath.c_str());
//     if(!fp)
//     {
//         g_Log->Warn(LOGFMT_OBJ_TAG("proto not found:%s"), fullFilePath.c_str());
//         isParentPathContinue = false;
//         return false;
//     }

//     fp.SetClosureDelegate([](void *ptr){
//         auto filePtr = reinterpret_cast<FILE *>(ptr);
//         KERNEL_NS::FileUtil::CloseFile(*filePtr);
//     });
    

//     // 扫描proto message/注解
//     Int32 currentLine = 0;

//     KERNEL_NS::LibString packageName;
//     KERNEL_NS::SmartPtr<ProtobufDataTypeInfo, KERNEL_NS::AutoDelMethods::Release> currentDataType;
//     while(!KERNEL_NS::FileUtil::IsEnd(*fp))
//     {
//         KERNEL_NS::LibString lineData;
//         KERNEL_NS::FileUtil::ReadUtf8OneLine(*fp, lineData);

//         lineData.strip();

//         // 跳过注释
//         if(ProtobuffHelper::IsNoteLine(lineData))
//             continue;

//         // 移除注释
//         ProtobuffHelper::RemoveNotePart(lineData);

//         if(packageName.empty())
//         {
//             ProtobuffHelper::GetPackageName(lineData, packageName, "_");

//             if(!packageName.empty())
//                 _currentArea.push_back(packageName);
//         }

//         // 判断是不是message
//         if(ProtobuffHelper::HasMessage(lineData))
//         {
//             // 提取message名 在message 之后 空格之前
//             auto dragMessageName = ProtobuffHelper::DragMessageSeg(lineData);
//             if(dragMessageName.empty())
//             {
//                 g_Log->Error(LOGFMT_OBJ_TAG("illegal message name :%s in proto file:%s"), dragMessageName.c_str(), fullFilePath.c_str());
//                 return false;
//             }

//             dragMessageName.strip();
//             // message 名称是连续的英文 + _ + 数字 且首字母是非数字构成
//             if(!ProtobuffHelper::CheckValidMessage(dragMessageName))
//             {
//                 g_Log->Error(LOGFMT_OBJ_TAG("invalid message name :%s in proto file:%s"), dragMessageName.c_str(), fullFilePath.c_str());
//                 return false;
//             }


//             // 类型冲突判断
//             KERNEL_NS::LibString fullArea;
//             for(auto &item : _currentArea)
//                 fullArea.AppendData(item);
//             const auto &fulllMessageName = fullArea + dragMessageName;
//             auto iter = _dataTypeRefPbDataTypeInfo.find(fulllMessageName);
//             if(iter != _dataTypeRefPbDataTypeInfo.end())
//             {
//                 g_Log->Error(LOGFMT_OBJ_TAG("repeate message:%s, full message name:%s, in proto file:%s"), dragMessageName.c_str(), fulllMessageName.c_str(), fullFilePath.c_str());
//                 return false;
//             }

//             // 创建新的数据处理
//             KERNEL_NS::SmartPtr<ProtobufDataTypeInfo, KERNEL_NS::AutoDelMethods::Release> dataType = ProtobufDataTypeInfo::Create();
//             dataType->_protoFileName = fileInfo._fileName;
//             dataType->_area = _currentArea;
//             dataType->_dataTypeName = dragMessageName;
//             _dataTypeRefPbDataTypeInfo.insert(std::make_pair(fulllMessageName, dataType));
//             _handlingDataTypes.push_back(dataType);
//             currentDataType = dataType;

//             continue;
//         }

//         // 不是消息也不是注释应该是类型的部分
//         if
        

//             KERNEL_NS::SmartPtr<MessageInfo, KERNEL_NS::AutoDelMethods::Release> messageInfo = MessageInfo::New_MessageInfo();
//             messageInfo->_messageName = dragMessageName;
//             auto &annotationParamNameRefValue = messageInfo->_annotationParamNameRefValue;

//             Int32 annnotationEnable = 0;
//             for(auto iter = lineRefProtoData.rbegin(); iter != lineRefProtoData.rend(); ++iter)
//             {
//                 if(iter->first == currentLine)
//                     continue;

//                 auto reverseLine = iter->second;

//                 reverseLine.lstrip();
//                 const Int32 oldAnnotationEnable = annnotationEnable;
//                 if(ProtobuffHelper::HasAnnotation(reverseLine))
//                     ++annnotationEnable;
//                 else if(!reverseLine.empty()) // 非空行则打断
//                     break;
//                 else
//                     annnotationEnable = 0;

//                 // 注解行不连续则停止解析
//                 if(oldAnnotationEnable != 0 && (annnotationEnable == 0))
//                     break;

//                 // 不是注解行 需要从第一个注解行开始解析
//                 if(annnotationEnable == 0)
//                     continue;
                
//                 auto dragParams = reverseLine.DragAfter(ProtobufMessageParam::ParamLineBegin);
//                 dragParams.strip();
//                 auto annotationParts = dragParams.Split(ProtobufMessageParam::CacheSegSepFlag);
//                 if(annotationParts.empty())
//                     continue;

//                 // 解析注解
//                 for(auto &annotationPairsStr : annotationParts)
//                 {
//                     auto annotationPairParts = annotationPairsStr.Split(ProtobufMessageParam::CacheKVSepFlag);
//                     if(annotationPairParts.empty())
//                         continue;

//                     annotationPairParts[0].strip();

//                     auto iterKey = annotationParamNameRefValue.find(annotationPairParts[0]);
//                     if(iterKey != annotationParamNameRefValue.end())
//                     {
//                         g_Log->Error(LOGFMT_OBJ_TAG("repeate annotation :%s in proto file:%s, annotationPairsStr:%s, reverseLine:%s, annotation key:%s")
//                                     , dragMessageName.c_str(), fullFilePath.c_str(), annotationPairsStr.c_str(), reverseLine.c_str(), annotationPairParts[0].c_str());
//                         return false;
//                     }

//                     if(annotationPairParts.size() > 1)
//                     {
//                         annotationPairParts[1].strip();
//                         iterKey = annotationParamNameRefValue.insert(std::make_pair(annotationPairParts[0], annotationPairParts[1])).first;
//                     }
//                     else
//                     {
//                         iterKey = annotationParamNameRefValue.insert(std::make_pair(annotationPairParts[0], KERNEL_NS::LibString())).first;
//                     }

//                     // 若注解没有值先从缓存中拿
//                     if(iterKey->second.empty())
//                         iterKey->second = _pbCacheContent->GetMessageAnnotationValue(fullFilePath,  messageInfo->_messageName, annotationPairParts[0]);
//                 }
//             }

//             // 放入字典
//             messageInfo->FieldsFromAnnotations(_maxOpcode);
//             messageInfoDict.insert(std::make_pair(messageInfo->_messageName, messageInfo.AsSelf()));
//             messageInfo.pop();
//         }
//     }
    
//     KERNEL_NS::LibString protoMd5;
//     if(!KERNEL_NS::LibDigest::MakeMd5Final(*ctx, protoMd5))
//     {
//         g_Log->Warn(LOGFMT_OBJ_TAG("md5 final fail fullFilePath:%s"), fullFilePath.c_str());
//         return false;
//     }
    
//     protoInfo->_md5 = KERNEL_NS::LibBase64::Encode(protoMd5);

//     // md5是否变化
//     auto iterPbCache = _pbCacheContent->_protoPathRefFileInfo.find(fullFilePath);
//     bool isProtoFileChanged = true;
//     if(iterPbCache != _pbCacheContent->_protoPathRefFileInfo.end())
//     {
//         auto pbCache = iterPbCache->second;
//         if(pbCache->_md5 == protoInfo->_md5)
//             isProtoFileChanged = false;
//     }
//     protoInfo->_isMd5Change = isProtoFileChanged;

//     // 文件变化或者强制重新生成
//     if(isProtoFileChanged || _forceGenAll)
//     {
//         g_Log->Custom("PROTO WILL GEN:%s...", protoInfo->_protoInfo._fileName.c_str());
//         _protoNameRefProtoInfo.insert(std::make_pair(protoInfo->_fullPathName, protoInfo.AsSelf()));
//     }

//     g_Log->Info(LOGFMT_OBJ_TAG("PROTO INFO:%s"), protoInfo->ToString().c_str());
    
//     protoInfo.pop();

    return true;
}


bool ExporterMgr::_LoadPbCache()
{
    const auto pbcacheFile = _protoPath + "/../" + ProtobufMessageParam::ProtoInfoCacheFile;
    return _pbCacheContent->LoadPbCache(pbcacheFile, _maxOpcode);
}

bool ExporterMgr::_ScanProtos()
{
    g_Log->Custom("SCAN PROTOS IN PROTO PATH:%s", _protoPath.c_str());

    auto nowTs = KERNEL_NS::LibTime::Now();
    auto traverseCallback = [this, &nowTs] (const KERNEL_NS::FindFileInfo &fileInfo, bool &isParentPathContinue) -> bool {

        bool isContinue = true;
        do
        {
            // 过滤目录
            if(KERNEL_NS::FileUtil::IsDir(fileInfo))
                break;

            // 过滤非proto
            if(KERNEL_NS::FileUtil::ExtractFileExtension(fileInfo._fileName) != KERNEL_NS::LibString(".proto"))
                break;

            KERNEL_NS::LibString fullPath = fileInfo._rootPath;
            if(fileInfo._rootPath.at(fileInfo._rootPath.length() - 1) != '/')
                fullPath.AppendFormat("/");

            const auto &fullFilePath = fullPath + fileInfo._fileName;

            // 拿pbcache中的缓存数据
            auto iterPbCacheFile = _pbCacheContent->_protoPathRefFileInfo.find(fullFilePath);
            if(iterPbCacheFile != _pbCacheContent->_protoPathRefFileInfo.end())
            {
                auto pbCacheFile = iterPbCacheFile->second;
                // 修改时间没变则不需要重新解析
                if(!_forceGenAll)
                {
                    if(pbCacheFile->_modifyTime == fileInfo._modifyTime)
                        break;
                }
            }

            // md5有变化才会被扫描到
            if(!_ScanAProto(fileInfo, fullFilePath, isParentPathContinue))
            {
                isContinue = false;
                break;
            }

        } while (false);
        
        return isContinue;
    };

    auto delg = KERNEL_CREATE_CLOSURE_DELEGATE(traverseCallback, bool, const KERNEL_NS::FindFileInfo &, bool &);
    KERNEL_NS::DirectoryUtil::TraverseDirRecursively(_protoPath, delg);
    delg->Release();

    g_Log->Custom("SCAN PROTOS IN PROTO PATH:%s END", _protoPath.c_str());

    return true;
}

bool ExporterMgr::_ScanAProto(const KERNEL_NS::FindFileInfo &fileInfo, const KERNEL_NS::LibString &fullFilePath, bool &isParentPathContinue)
{
    // 加载文件内容到内存
    KERNEL_NS::SmartPtr<FILE, KERNEL_NS::AutoDelMethods::CustomDelete> fp = KERNEL_NS::FileUtil::OpenFile(fullFilePath.c_str());
    if(!fp)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("proto not found:%s"), fullFilePath.c_str());
        isParentPathContinue = false;
        return false;
    }

    fp.SetClosureDelegate([](void *ptr){
        auto filePtr = reinterpret_cast<FILE *>(ptr);
        KERNEL_NS::FileUtil::CloseFile(*filePtr);
    });
    
    auto iterProtoContent = _protoNameRefProtoInfo.find(fullFilePath);
    if(iterProtoContent != _protoNameRefProtoInfo.end())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("repeat proto full path name:fullFilePath:%s"), fullFilePath.c_str());
        isParentPathContinue = false;
        return false;
    }

    KERNEL_NS::SmartPtr<ProtoContentInfo, KERNEL_NS::AutoDelMethods::Release> protoInfo = ProtoContentInfo::New_ProtoContentInfo();
    protoInfo->_fullPathName = fullFilePath;
    protoInfo->_protoInfo = fileInfo;
    auto &lineRefProtoData = protoInfo->_lineRefContent;
    auto &messageInfoDict = protoInfo->_messageNameRefMessageInfo;

    // 扫描proto message/注解
    Int32 currentLine = 0;
    KERNEL_NS::SmartPtr<MD5_CTX, KERNEL_NS::AutoDelMethods::CustomDelete> ctx = new MD5_CTX;
    bool isCtxInit = false;
    ctx.SetClosureDelegate([&isCtxInit](void *ptr)
    {
        auto ctxPtr = reinterpret_cast<MD5_CTX *>(ptr);
        if(isCtxInit)
           KERNEL_NS::LibDigest::MakeMd5Clean(ctxPtr); 

        delete ctxPtr;
    });

    if(!KERNEL_NS::LibDigest::MakeMd5Init(ctx.AsSelf()))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("init md5 fail fullFilePath:%s"), fullFilePath.c_str());
        return false;
    }
    isCtxInit = true;

    while(!KERNEL_NS::FileUtil::IsEnd(*fp))
    {
        KERNEL_NS::LibString lineData;
        KERNEL_NS::FileUtil::ReadUtf8OneLine(*fp, lineData);

        lineRefProtoData.insert(std::make_pair(++currentLine, lineData));

        auto lineCopy = lineData + "\n";
        if(!KERNEL_NS::LibDigest::MakeMd5Continue(ctx.AsSelf(), lineCopy.c_str(), lineCopy.length()))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("md5 continue fail fullFilePath:%s, proto line:%d, lineData:%s"), fullFilePath.c_str(), currentLine, lineCopy.c_str());
            return false;
        }
        
        lineData.strip();

        if(protoInfo->_packageName.empty())
            ProtobuffHelper::GetPackageName(lineData, protoInfo->_packageName);

        // 判断是不是message
        if(ProtobuffHelper::HasMessage(lineData))
        {
            // 提取message名 在message 之后 空格之前
            auto dragMessageName = ProtobuffHelper::DragMessageSeg(lineData);
            if(dragMessageName.empty())
            {
                g_Log->Error(LOGFMT_OBJ_TAG("illegal message name :%s in proto file:%s"), dragMessageName.c_str(), fullFilePath.c_str());
                return false;
            }

            // message 名称是连续的英文 + _ + 数字 且首字母是非数字构成
            if(!ProtobuffHelper::CheckValidMessage(dragMessageName))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("invalid message name :%s in proto file:%s"), dragMessageName.c_str(), fullFilePath.c_str());
                return false;
            }

            // 重复定义message
            if(messageInfoDict.find(dragMessageName) != messageInfoDict.end())
            {
                g_Log->Error(LOGFMT_OBJ_TAG("repeat message: proto file path:%s, message name:%s"), fullFilePath.c_str(), dragMessageName.c_str());
                return false;
            }

            KERNEL_NS::SmartPtr<MessageInfo, KERNEL_NS::AutoDelMethods::Release> messageInfo = MessageInfo::New_MessageInfo();
            messageInfo->_messageName = dragMessageName;
            auto &annotationParamNameRefValue = messageInfo->_annotationParamNameRefValue;

            Int32 annnotationEnable = 0;
            for(auto iter = lineRefProtoData.rbegin(); iter != lineRefProtoData.rend(); ++iter)
            {
                if(iter->first == currentLine)
                    continue;

                auto reverseLine = iter->second;

                reverseLine.lstrip();
                const Int32 oldAnnotationEnable = annnotationEnable;
                if(ProtobuffHelper::HasAnnotation(reverseLine))
                    ++annnotationEnable;
                else if(!reverseLine.empty()) // 非空行则打断
                    break;
                else
                    annnotationEnable = 0;

                // 注解行不连续则停止解析
                if(oldAnnotationEnable != 0 && (annnotationEnable == 0))
                    break;

                // 不是注解行 需要从第一个注解行开始解析
                if(annnotationEnable == 0)
                    continue;
                
                auto dragParams = reverseLine.DragAfter(ProtobufMessageParam::ParamLineBegin);
                dragParams.strip();
                auto annotationParts = dragParams.Split(ProtobufMessageParam::CacheSegSepFlag);
                if(annotationParts.empty())
                    continue;

                // 解析注解
                for(auto &annotationPairsStr : annotationParts)
                {
                    auto annotationPairParts = annotationPairsStr.Split(ProtobufMessageParam::CacheKVSepFlag);
                    if(annotationPairParts.empty())
                        continue;

                    annotationPairParts[0].strip();

                    auto iterKey = annotationParamNameRefValue.find(annotationPairParts[0]);
                    if(iterKey != annotationParamNameRefValue.end())
                    {
                        g_Log->Error(LOGFMT_OBJ_TAG("repeate annotation :%s in proto file:%s, annotationPairsStr:%s, reverseLine:%s, annotation key:%s")
                                    , dragMessageName.c_str(), fullFilePath.c_str(), annotationPairsStr.c_str(), reverseLine.c_str(), annotationPairParts[0].c_str());
                        return false;
                    }

                    if(annotationPairParts.size() > 1)
                    {
                        annotationPairParts[1].strip();
                        iterKey = annotationParamNameRefValue.insert(std::make_pair(annotationPairParts[0], annotationPairParts[1])).first;
                    }
                    else
                    {
                        iterKey = annotationParamNameRefValue.insert(std::make_pair(annotationPairParts[0], KERNEL_NS::LibString())).first;
                    }

                    // 若注解没有值先从缓存中拿
                    if(iterKey->second.empty())
                        iterKey->second = _pbCacheContent->GetMessageAnnotationValue(fullFilePath,  messageInfo->_messageName, annotationPairParts[0]);
                }
            }

            // 放入字典
            messageInfo->FieldsFromAnnotations(_maxOpcode);
            messageInfoDict.insert(std::make_pair(messageInfo->_messageName, messageInfo.AsSelf()));
            messageInfo.pop();
        }
    }
    
    KERNEL_NS::LibString protoMd5;
    if(!KERNEL_NS::LibDigest::MakeMd5Final(ctx.AsSelf(), protoMd5))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("md5 final fail fullFilePath:%s"), fullFilePath.c_str());
        return false;
    }
    
    protoInfo->_md5 = KERNEL_NS::LibBase64::Encode(protoMd5);

    // md5是否变化
    auto iterPbCache = _pbCacheContent->_protoPathRefFileInfo.find(fullFilePath);
    bool isProtoFileChanged = true;
    if(iterPbCache != _pbCacheContent->_protoPathRefFileInfo.end())
    {
        auto pbCache = iterPbCache->second;
        if(pbCache->_md5 == protoInfo->_md5)
            isProtoFileChanged = false;
    }
    protoInfo->_isMd5Change = isProtoFileChanged;

    // 文件变化或者强制重新生成
    if(isProtoFileChanged || _forceGenAll)
    {
        g_Log->Custom("PROTO WILL GEN:%s...", protoInfo->_protoInfo._fileName.c_str());
        _protoNameRefProtoInfo.insert(std::make_pair(protoInfo->_fullPathName, protoInfo.AsSelf()));
    }

    g_Log->Info(LOGFMT_OBJ_TAG("PROTO INFO:%s"), protoInfo->ToString().c_str());
    
    protoInfo.pop();

    return true;
}

bool ExporterMgr::_UpdateProtoCache()
{
    // 有变化的更新或者加入到缓存中
    for(auto &kv : _protoNameRefProtoInfo)
    {
        auto protoContent = kv.second;
        auto &messageInfoList = protoContent->_messageNameRefMessageInfo;
        for(auto iter : messageInfoList)
        {
            auto messageInfo = iter.second;
            const auto &updateCacheInfo = messageInfo->ToPbCache(protoContent->_protoInfo._fileName, protoContent->_fullPathName);
            // // 没有协议号的不用缓存
            // if(updateCacheInfo._opcode <= 0)
            //     continue;

            if(_pbCacheContent->IsMessageCacheExists(updateCacheInfo._protoPath, updateCacheInfo._messageName))
            {
                _pbCacheContent->UpdateMessageCache(updateCacheInfo);
            }
            else
            {
                _pbCacheContent->AddMessageCache(updateCacheInfo);
            }
        }

        // 移除无效的消息缓存
        _pbCacheContent->RemoveInvalidMessagesBy(protoContent->_fullPathName, [&messageInfoList](const KERNEL_NS::LibString &messageName)->bool
        {
            return messageInfoList.find(messageName) != messageInfoList.end();
        });

        const auto &pbFileCache = protoContent->ToPbCache();
        if(_pbCacheContent->IsProtoFileCacheExists(pbFileCache._protoPath))
        {
            _pbCacheContent->UpdateProtoFileCache(pbFileCache);
        }
        else
        {
            _pbCacheContent->AddProtoFileCache(pbFileCache);
        }
    }

    // 清除无效文件
    std::set<PbCacheFileInfo *> invalidFiles;
    for(auto kv : _pbCacheContent->_lineRefProtoFileInfo)
    {
        if(!KERNEL_NS::FileUtil::IsFileExist(kv.second->_protoPath.c_str()))
            invalidFiles.insert(kv.second);
    }

    for(auto protoFile : invalidFiles)
        _RemoveInvalidFiles(protoFile);

    // 有更新就重写缓存文件
    if(!_protoNameRefProtoInfo.empty())
    {
        const auto pbcacheFile = _protoPath + "/../" + ProtobufMessageParam::ProtoInfoCacheFile;    
        if(!_pbCacheContent->UpdatePbCache(pbcacheFile))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("update pb cache fail please check pbcacheFile:%s"), pbcacheFile.c_str());
            return false;
        }

        g_Log->Custom("[PROTO GEN] UPDATE PB CACHE %s SUCCESS.", pbcacheFile.c_str());
    }

    return true;
}

void ExporterMgr::_RemoveInvalidFiles(PbCacheFileInfo *cacheFile)
{
    // TODO: 移除.h, .cc, .cs等
    const auto &appFullPath = GetServiceProxy()->GetApp()->GetAppPath();
    const auto &appPath = KERNEL_NS::DirectoryUtil::GetFileDirInPath(appFullPath);

    // 相对路径 = 完整路径 - 完整protoPath路径
    const auto fullProtoPath = appPath + _protoPath;
    const auto protoFullPath = appPath + cacheFile->_protoPath;
    const auto relationPath = protoFullPath - fullProtoPath + "/";
    const auto protoName = KERNEL_NS::FileUtil::ExtractFileWithoutExtension(cacheFile->_protoName);

    // .pb.h完整路径 = 完整的cppOut路径 + 相对路径
    {
        const auto fullCppOutPath = appPath + _cppOutPath;
        const auto pbHeaderPath = KERNEL_NS::FileUtil::ExtractFileWithoutExtension(fullCppOutPath + relationPath + protoName) + ".pb.h";
        const auto pbCCPath = KERNEL_NS::FileUtil::ExtractFileWithoutExtension(fullCppOutPath + relationPath + protoName) + ".pb.cc";
        if(KERNEL_NS::FileUtil::IsFileExist(pbHeaderPath.c_str()))
            KERNEL_NS::FileUtil::DelFileCStyle(pbHeaderPath.c_str());
        if(KERNEL_NS::FileUtil::IsFileExist(pbCCPath.c_str()))
            KERNEL_NS::FileUtil::DelFileCStyle(pbCCPath.c_str());
    }


    // .cs 完整路径 = 完整的csharpOut路径 + 相对路径
    {
        const auto fullCSharpOutPath = appPath + _csharpOutPath;
        const auto extractPath = KERNEL_NS::DirectoryUtil::GetFileDirInPath(relationPath.c_str());
        const auto csharpName = ProtobuffHelper::TreatCsharpName(protoName);

        const auto csharpFullName = (fullCSharpOutPath + extractPath + csharpName) + ".cs";
        if(KERNEL_NS::FileUtil::IsFileExist(csharpFullName.c_str()))
            KERNEL_NS::FileUtil::DelFileCStyle(csharpFullName.c_str());
    }

    _pbCacheContent->RemoveInvalidFile(cacheFile);
}

KERNEL_NS::LibString ExporterMgr::_FileHeader(const KERNEL_NS::LibTime &time, const KERNEL_NS::LibString &desc)
{
    KERNEL_NS::LibString fileHeader;
    fileHeader.AppendFormat(""
    "/*!\n"
    "*  MIT License\n"
    "*  \n"
    "*  Copyright (c) 2020 ericyonng<120453674@qq.com>\n"
    "*  \n"
    "*  Permission is hereby granted, free of charge, to any person obtaining a copy\n"
    "*  of this software and associated documentation files (the \"Software\"), to deal\n"
    "*  in the Software without restriction, including without limitation the rights\n"
    "*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n"
    "*  copies of the Software, and to permit persons to whom the Software is\n"
    "*  furnished to do so, subject to the following conditions:\n"
    "*  \n"
    "*  The above copyright notice and this permission notice shall be included in all\n"
    "*  copies or substantial portions of the Software.\n"
    "*  \n"
    "*  THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
    "*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
    "*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n"
    "*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
    "*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n"
    "*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n"
    "*  SOFTWARE.\n"
    "* \n"
    "* Author: Eric Yonng\n"
    "* Description: %s\n"
    "*/\n"
    , desc.c_str());

    return fileHeader;
}

void ExporterMgr::_Clear()
{
    KERNEL_NS::ContainerUtil::DelContainer2(_protoNameRefProtoInfo);
    if(_pbCacheContent)
        PbCacheFileContent::Delete_PbCacheFileContent(_pbCacheContent);
    _pbCacheContent = NULL;
}

void ExporterMgr::_RegisterEvents()
{

}

void ExporterMgr::_UnRegisterEvents()
{
    
}

SERVICE_END