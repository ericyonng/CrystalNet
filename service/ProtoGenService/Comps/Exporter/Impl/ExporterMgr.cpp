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
#include <service/ProtoGenService/Comps/Exporter/Impl/ExporterMgrFactory.h>
#include <service/ProtoGenService/Comps/Exporter/Defs/Defs.h>
#include <service/ProtoGenService/Comps/Exporter/Defs/ProtoContentInfo.h>
#include <service/ProtoGenService/Comps/Exporter/Defs/MessageInfo.h>
#include <service/ProtoGenService/Comps/Exporter/Defs/PbCacheInfoFormat.h>
#include <service/ProtoGenService/Comps/Exporter/Impl/ProtobuffHelper.h>

#include <service/ProtoGenService/Comps/Exporter/Impl/ExporterMgr.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(IExporterMgr);

POOL_CREATE_OBJ_DEFAULT_IMPL(ExporterMgr);

ExporterMgr::ExporterMgr()
:
_forceGenAll(false)
,_maxOpcode(0)
,_pbCacheContent(NULL)
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
    // ????????????
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

    return Status::Success;
}

Int32 ExporterMgr::_OnHostStart()
{
    // ???????????????
    auto nextFrame = [this](KERNEL_NS::LibTimer *t)
    {
        bool genSuc = false;
        do
        {
            g_Log->Custom("[PROTO GEN] START.");

            // 1.????????????
            if(!_AnalyzeProtoAnnotation())
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("anylyze proto annotation fail."));
                break;
            }

            // 2.??????cpp
            if(!_GenCplusplus())
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("gen c plus plus fail."));
                break;
            }

            // 3.??????csharp
            if(!_GenCSharp())
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("gen csharp fail."));
                break;
            }

            // 4.??????pb cache
            if(!_UpdateProtoCache())
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("update proto cache fail."));
                break;
            }

            _GenOpcodeEnums();
            _GenOpcodeInfo();
            _GenAllPbs();

            // ???????????????????????????proto????????????????????????cc??????pch.h ????????????
            // if(!_googleProtoIncludePath.empty() && _forceGenAll)
            // {
            //     _AddPchHeaderToGoogleCC();
            // }

            genSuc = true;

            g_Log->Custom("[PROTO GEN] END.");

        }while (false);

        // 4.??????app
        Int32 err = genSuc ? Status::Success : Status::Failed;
        GetServiceProxy()->CloseApp(err);
        
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
    };

    auto timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer();
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

    // ??????pbcache ?????? ?????????????????????
    if(!_LoadPbCache())
    {
        g_Log->Error(LOGFMT_OBJ_TAG("load pb cache fail."));
        return false;
    }

    // ????????????proto
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

    {// ??????????????????
        Int32 err = 0;
        KERNEL_NS::LibString cmd;

        // 2.0???proto?????????????????????proto
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

    // 1.protoc ??????????????????
    const auto coutPath = (appPath + _cppOutPath);
    for(auto iter : _protoNameRefProtoInfo)
    {
        auto protoInfo = iter.second;
        Int32 err = 0;
        KERNEL_NS::LibString cmd;

        g_Log->Custom("[PROTO GEN CPP] %s ...", protoInfo->_protoInfo._fileName.c_str());

        // 2.0???proto?????????????????????proto
        #if CRYSTAL_TARGET_PLATFORM_WINDOWS
            cmd.AppendFormat("cd %s && %s --cpp_out=%s --proto_path=%s %s"
            , (appPath + protocPath).c_str()
            , (protocName).c_str()
            , (appPath + _cppOutPath).c_str()
            , (appPath + protoInfo->_protoInfo._rootPath).c_str()
            // , (appPath + protoInfo->_fullPathName).c_str());
            , protoInfo->_protoInfo._fileName.c_str());
        #else
            cmd.AppendFormat("%s --cpp_out=%s --proto_path=%s %s"
            , (appPath + protocPath + protocName).c_str()
            , (appPath + _cppOutPath).c_str()
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
            // ???????????? = coutPath + ??????????????? + ?????????
            auto rootPath = appPath + protoInfo->_protoInfo._rootPath;
            auto fullPath = appPath + protoInfo->_fullPathName;
            auto relationPath = fullPath - rootPath;
            auto pbHeaderName = KERNEL_NS::FileUtil::ExtractFileWithoutExtension(coutPath + relationPath) + ".pb.h";
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


            // 2.1?????????????????????
            std::vector<KERNEL_NS::LibString> lineContents;
            auto line = KERNEL_NS::FileUtil::ReadUtf8File(*fp, lineContents);
            g_Log->Info(LOGFMT_OBJ_TAG("pb header:%s line:%lld"), pbHeaderName.c_str(), line);

            if(!_ModifyCppPbHeader(pbHeaderName, lineContents, protoInfo, newFactoryNames))
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("modify cpp line contents fail pb file:%s"), pbHeaderName.c_str());
                return false;
            }

            // 3.????????????
            KERNEL_NS::FileUtil::CloseFile(*fp);
            fp.pop();

            // 4.????????????????????????
            if(!KERNEL_NS::FileUtil::ReplaceFile(pbHeaderName, lineContents))
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("replace file fail:%s"), pbHeaderName.c_str());
                return false;
            }
        }

        {// .pb.cc
            // ???????????? = coutPath + ??????????????? + ?????????
            auto rootPath = appPath + protoInfo->_protoInfo._rootPath;
            auto fullPath = appPath + protoInfo->_fullPathName;
            auto relationPath = fullPath - rootPath;
            auto pbCCName = KERNEL_NS::FileUtil::ExtractFileWithoutExtension(coutPath + relationPath) + ".pb.cc";
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

            // 2.??????????????????
            std::vector<KERNEL_NS::LibString> lineContents;
            auto line = KERNEL_NS::FileUtil::ReadUtf8File(*fp, lineContents);
            g_Log->Info(LOGFMT_OBJ_TAG("pb cc:%s line:%lld"), pbCCName.c_str(), line);

            if(!_ModifyCppPbCC(pbCCName, lineContents, protoInfo, newFactoryNames))
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("modify cpp line contents fail cc file:%s"), pbCCName.c_str());
                return false;
            }

            // 3.????????????
            KERNEL_NS::FileUtil::CloseFile(*fp);
            fp.pop();

            // 4.????????????????????????
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
    // ??????????????????
    const auto &pbHeaderDefinePatternRegex = std::regex("^#define GOOGLE_PROTOBUF_INCLUDED.*");

    // 1.???????????????
    {
        std::vector<KERNEL_NS::LibString> addLines 
        {
            "",
            "// KERNEL_INCLUDED",
            "#include <kernel/kernel.h>", 
            "#include <service_common/ServiceCommon.h>"
            "",
            "#ifdef GetMessage",
            " #undef GetMessage",
            "#endif",
            "",
        };
        // ??????#define xxx??????????????????????????????
        ProtobuffHelper::Modifylines(lines, addLines, [&pbHeaderDefinePatternRegex](Int32 curLine, KERNEL_NS::LibString &lineData
        , std::vector<KERNEL_NS::LibString> &addDatasBefore
        , std::vector<KERNEL_NS::LibString> &addDatasAfter
        , bool &isContinue)->bool{
            return std::regex_match(lineData.GetRaw(), pbHeaderDefinePatternRegex);
        });
    }

    // 2.?????????(????????????, ???????????????, Release??????, ??????????????????)
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
                // 1.????????????
                const auto &className = ProtobuffHelper::DragClass(lineData);
                auto messageInfo = protoFileInfo->GetMessageInfo(className);
                if(!messageInfo)
                {
                    g_Log->Warn(LOGFMT_OBJ_TAG("message info not found message name:%s, proto info:%s"), className.c_str(), protoFileInfo->ToString().c_str());
                    return false;
                }

                classNames.push_back(className);

                // 2.????????????
                const auto annotationInfo = KERNEL_NS::LibString().AppendFormat("// AnnotaionInfo[opcode(%d)]", messageInfo->_opcode);
                addLineDatasBefore.push_back(annotationInfo);

                // 3.????????????
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
                
                // 4.???????????????(???????????????, Release, ????????????)
                if(lineData.GetRaw().find("{") != std::string::npos)
                {// ????????????????????? {?????? ???????????????
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
    }
    
    // 3.???????????????
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
                            _CollectCppClassFactoryDeclearAdds(className, addFactoryLines, newClassFactoryNames);

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
    // 1.??????????????????,??????protobuf ????????????new/delete ????????????????????????????????????,????????????
    // {
    //     addLines.push_back(KERNEL_NS::LibString().AppendFormat("POOL_CREATE_OBJ_DEFAULT(%s)", className.c_str()));
    // }
    
    // 2.??????????????????
    addLines.push_back("");
    addLines.push_back("public:");

    // 3.??????Release??????
    {
        addLines.push_back("virtual void Release() override {");
        addLines.push_back("    delete this;");
        addLines.push_back("}");
    }
    addLines.push_back("");

    // 4.??????MT Encode??????
    {
        addLines.push_back("virtual bool Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) override {");
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

    {// 5.??????TL Encode ??????
        addLines.push_back("virtual bool Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) override {");
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

    {// 6.??????Decode MT??????
        addLines.push_back("virtual bool Decode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) override {");
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

    {// 7.??????Decode TL??????
        addLines.push_back("virtual bool Decode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) override {");
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
}

void ExporterMgr::_CollectCppClassFactoryDeclearAdds(const KERNEL_NS::LibString &className, std::vector<KERNEL_NS::LibString> &addLines, std::vector<KERNEL_NS::LibString> &newClassFactoryNames)
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
    addLines.push_back(KERNEL_NS::LibString().AppendFormat("        return new %s();", className.c_str()));
    addLines.push_back(KERNEL_NS::LibString().AppendFormat("    }"));
    addLines.push_back(KERNEL_NS::LibString());

    // virtual ICoder *Create(const ICoder *coder) = 0;
    // Create
    addLines.push_back(KERNEL_NS::LibString().AppendFormat("    virtual KERNEL_NS::ICoder *Create(const KERNEL_NS::ICoder *coder) const override {"));
    addLines.push_back(KERNEL_NS::LibString().AppendFormat("        return new %s(*dynamic_cast<const %s *>(coder));", className.c_str(), className.c_str()));
    addLines.push_back(KERNEL_NS::LibString().AppendFormat("    }"));
    addLines.push_back(KERNEL_NS::LibString());

    // end
    addLines.push_back("};");
    addLines.push_back("");
}

bool ExporterMgr::_ModifyCppPbCC(const KERNEL_NS::LibString &pbCCName, std::vector<KERNEL_NS::LibString> &lines, const ProtoContentInfo *protoFileInfo, const std::vector<KERNEL_NS::LibString> &newFactoryNames)
{
    // 1.??????pch.h
    {
        // ??????????????????
        if(lines.empty())
        {
            lines.push_back("#include <pch.h>");
        }
        else
        {
            lines.insert(lines.begin() + 0, "#include <pch.h>");
        }
    }

    {// ???????????????
        const auto protoNameWithoutExtention = KERNEL_NS::FileUtil::ExtractFileWithoutExtension(protoFileInfo->_protoInfo._fileName);
        auto appPath = GetServiceProxy()->GetApp()->GetAppPath();
        appPath = KERNEL_NS::DirectoryUtil::GetFileDirInPath(appPath);

        auto rootPath = appPath + protoFileInfo->_protoInfo._rootPath;
        auto fullPath = appPath + protoFileInfo->_fullPathName;
        auto relationPath = fullPath - rootPath;
        auto cppOutRelationPath = _cppOutPath - _basePath;

        // cc???????????????????????????????????????
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
        auto relationPath = fullPath - rootPath;
        auto cppOutRelationPath = _cppOutPath - _basePath;
        auto headerRelationPath = KERNEL_NS::DirectoryUtil::GetFileDirInPath(cppOutRelationPath + relationPath);
        
        KERNEL_NS::LibString pattenString = KERNEL_NS::LibString().AppendFormat("^#include.*%s%s\\.pb\\.h.*", headerRelationPath.c_str(), protoNameWithoutExtention.c_str());
        const auto &includeRegex = std::regex(pattenString.c_str());

        ProtobuffHelper::Modifylines(lines, addFactoryImpls, [&includeRegex](Int32 curLine, KERNEL_NS::LibString &lineData
                , std::vector<KERNEL_NS::LibString> &addLinesBefore // ????????????
                , std::vector<KERNEL_NS::LibString> &addLinesAfter  // ????????? addLines ????????? addLinesAfter
                , bool &isContinue) -> bool
        {
            return std::regex_match(lineData.GetRaw(), includeRegex);
        });
    }

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

    {// ??????cpp???opcode
        std::vector<KERNEL_NS::LibString> lines;
        g_Log->Custom("[PROTO GEN CPP] OPCODE ENUMS...");

        // ?????????
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

    {// ??????csharp???opcode
        std::vector<KERNEL_NS::LibString> lines;
        g_Log->Custom("[PROTO GEN CSHARP] OPCODE ENUMS...");

        // ?????????
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

    // ?????????
    const auto fileHeader = _FileHeader(KERNEL_NS::LibTime::Now(), KERNEL_NS::LibString().AppendFormat("Generated By %s, Dont Modify This File!!!", appName.c_str()));
    lines.push_back(fileHeader);
    lines.push_back("");

    // ??????
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

    // ?????????
    const auto fileHeader = _FileHeader(KERNEL_NS::LibTime::Now(), KERNEL_NS::LibString().AppendFormat("Generated By %s, Dont Modify This File!!!", appName.c_str()));
    lines.push_back(fileHeader);
    lines.push_back("");
    lines.push_back("#ifndef __PROTOCOLS_ALLPBS_H__");
    lines.push_back("#define __PROTOCOLS_ALLPBS_H__");
    lines.push_back("");
    lines.push_back("#pragma once");
    lines.push_back("");

    // ??????
    KERNEL_NS::BinaryArray<PbCacheFileInfo *, PbCacheFileInfoCompare> sortedArray;
    for(auto kv : _pbCacheContent->_lineRefProtoFileInfo)
        sortedArray.insert(kv.second);

    const auto relationPath = _cppOutPath - _basePath;
    const Int64 arrSize = static_cast<Int64>(sortedArray.size());
    for(Int64 idx = 0; idx < arrSize; ++idx)
    {
        auto protoInfo = sortedArray[idx];
        const auto protoFileName = KERNEL_NS::FileUtil::ExtractFileWithoutExtension(protoInfo->_protoName);
        lines.push_back(KERNEL_NS::LibString().AppendFormat("#include <%s/%s.pb.h>"
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
//             // ????????????
//             if(KERNEL_NS::FileUtil::IsDir(fileInfo))
//                 break;

//             // ?????????cc
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
    
    const auto appFullPath = GetServiceProxy()->GetApp()->GetAppPath();
    const auto appPath = KERNEL_NS::DirectoryUtil::GetFileDirInPath(appFullPath);
    const auto appName = GetServiceProxy()->GetApp()->GetAppName();
    auto protocPath = KERNEL_NS::DirectoryUtil::GetFileDirInPath(_cppProtocPath);
    auto protocName = KERNEL_NS::DirectoryUtil::GetFileNameInPath(_cppProtocPath);

    {// ??????????????????
        Int32 err = 0;
        KERNEL_NS::LibString cmd;

        // 2.0???proto?????????????????????proto
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

    // 1.protoc ??????????????????
    const auto csharpOutPath = (appPath + _csharpOutPath);
    for(auto iter : _protoNameRefProtoInfo)
    {
        auto protoInfo = iter.second;
        Int32 err = 0;
        KERNEL_NS::LibString cmd;

        g_Log->Custom("[PROTO GEN CSHARP] %s ...", protoInfo->_protoInfo._fileName.c_str());

        // 2.0???proto?????????????????????proto
        #if CRYSTAL_TARGET_PLATFORM_WINDOWS
            cmd.AppendFormat("cd %s && %s --csharp_out=%s --proto_path=%s %s"
            , (appPath + protocPath).c_str()
            , (protocName).c_str()
            , (appPath + _csharpOutPath).c_str()
            , (appPath + protoInfo->_protoInfo._rootPath).c_str()
            // , (appPath + protoInfo->_fullPathName).c_str());
            , protoInfo->_protoInfo._fileName.c_str());
        #else
            cmd.AppendFormat("%s --csharp_out=%s --proto_path=%s %s"
            , (appPath + protocPath + protocName).c_str()
            , (appPath + _csharpOutPath).c_str()
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

        // ???????????? = coutPath + ??????????????? + ?????????
        auto protoName = KERNEL_NS::FileUtil::ExtractFileWithoutExtension(KERNEL_NS::DirectoryUtil::GetFileNameInPath(protoInfo->_fullPathName));
        auto csharpFileName = ProtobuffHelper::TreatCsharpName(protoName);

        auto rootPath = appPath + protoInfo->_protoInfo._rootPath;
        auto fullPath = appPath + protoInfo->_fullPathName;
        auto relationPath = KERNEL_NS::DirectoryUtil::GetFileDirInPath(fullPath - rootPath);
        
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

        // ?????????????????????
        std::vector<KERNEL_NS::LibString> lineContents;
        auto line = KERNEL_NS::FileUtil::ReadUtf8File(*fp, lineContents);
        g_Log->Info(LOGFMT_OBJ_TAG("csharp file:%s line:%lld"), wholeCSharpName.c_str(), line);


        // 1.?????????Package??????????????????
        _AddCsharpNamespace(lineContents);

        // 2.????????????
        _ProtoMessageAttribute(protoInfo, lineContents);

        // 3.????????????
        KERNEL_NS::FileUtil::CloseFile(*fp);
        fp.pop();

        // 4.????????????????????????
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

    // 1.??????????????????????????????????????????????????????,??????????????????
    const Int32 maxLine = static_cast<Int32>(lines.size());
    bool isMatch = false;
    Int32 idx = 0;
    for(; idx < maxLine; ++idx)
    {
        auto &lineData = lines[idx];
        if(std::regex_match(lineData.GetRaw(), std::regex(".* class .* .*")))
        {// ??????????????????
            // ????????????
            Int32 backIdx = idx - 1;
            for(; backIdx >= 0; --backIdx)
            {
                if(!ProtobuffHelper::IsNoteLine(lines[backIdx]))
                    break;
            }

            // ???????????????
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
            // 1.????????????
            auto classPos = lineData.GetRaw().find(ProtobufMessageParam::ClassFlag.GetRaw());
            KERNEL_NS::LibString classSub = lineData.GetRaw().substr(classPos);
            
            const auto &className = ProtobuffHelper::DragClass(classSub);
            auto messageInfo = protoFile->GetMessageInfo(className);
            if(!messageInfo)
                return false;

            if (messageInfo->_opcode == 0)
                return false;

            // 2.??????????????????
            const auto annotationInfo = KERNEL_NS::LibString().AppendFormat("[ProtoMessage(%d)]", messageInfo->_opcode);
            addLineDatasBefore.push_back(annotationInfo);
        }

        isContinue = true;
        return isMatched;
    });
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
            // ????????????
            if(KERNEL_NS::FileUtil::IsDir(fileInfo))
                break;

            // ?????????proto
            if(KERNEL_NS::FileUtil::ExtractFileExtension(fileInfo._fileName) != KERNEL_NS::LibString(".proto"))
                break;

            KERNEL_NS::LibString fullPath = fileInfo._rootPath;
            if(fileInfo._rootPath.at(fileInfo._rootPath.length() - 1) != '/')
                fullPath.AppendFormat("/");

            const auto &fullFilePath = fullPath + fileInfo._fileName;

            // ???pbcache??????????????????
            auto iterPbCacheFile = _pbCacheContent->_protoPathRefFileInfo.find(fullFilePath);
            if(iterPbCacheFile != _pbCacheContent->_protoPathRefFileInfo.end())
            {
                auto pbCacheFile = iterPbCacheFile->second;
                // ??????????????????????????????????????????
                if(!_forceGenAll)
                {
                    if(pbCacheFile->_modifyTime == fileInfo._modifyTime)
                        break;
                }
            }

            // md5???????????????????????????
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

    g_Log->Custom("SCAN PROTOS IN PROTO PATH:%s END", _protoPath.c_str());

    return true;
}

bool ExporterMgr::_ScanAProto(const KERNEL_NS::FindFileInfo &fileInfo, const KERNEL_NS::LibString &fullFilePath, bool &isParentPathContinue)
{
    // ???????????????????????????
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

    // ??????proto message/??????
    Int32 currentLine = 0;
    KERNEL_NS::SmartPtr<MD5_CTX, KERNEL_NS::AutoDelMethods::CustomDelete> ctx = new MD5_CTX;
    bool isCtxInit = false;
    ctx.SetClosureDelegate([&isCtxInit](void *ptr)
    {
        auto ctxPtr = reinterpret_cast<MD5_CTX *>(ptr);
        if(isCtxInit)
           KERNEL_NS::LibDigest::MakeMd5Clean(*ctxPtr); 

        delete ctxPtr;
    });

    if(!KERNEL_NS::LibDigest::MakeMd5Init(*ctx))
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
        if(!KERNEL_NS::LibDigest::MakeMd5Continue(*ctx, lineCopy.c_str(), lineCopy.length()))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("md5 continue fail fullFilePath:%s, proto line:%d, lineData:%s"), fullFilePath.c_str(), currentLine, lineCopy.c_str());
            return false;
        }
        
        lineData.strip();

        // ???????????????message
        if(ProtobuffHelper::HasMessage(lineData))
        {
            // ??????message??? ???message ?????? ????????????
            auto dragMessageName = ProtobuffHelper::DragMessageSeg(lineData);
            if(dragMessageName.empty())
            {
                g_Log->Error(LOGFMT_OBJ_TAG("illegal message name :%s in proto file:%s"), dragMessageName.c_str(), fullFilePath.c_str());
                return false;
            }

            // message ???????????????????????? + _ + ?????? ??????????????????????????????
            if(!ProtobuffHelper::CheckValidMessage(dragMessageName))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("invalid message name :%s in proto file:%s"), dragMessageName.c_str(), fullFilePath.c_str());
                return false;
            }

            // ????????????message
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
                else if(!reverseLine.empty()) // ??????????????????
                    break;
                else
                    annnotationEnable = 0;

                // ?????????????????????????????????
                if(oldAnnotationEnable != 0 && (annnotationEnable == 0))
                    break;

                // ??????????????? ???????????????????????????????????????
                if(annnotationEnable == 0)
                    continue;
                
                auto dragParams = reverseLine.DragAfter(ProtobufMessageParam::ParamLineBegin);
                dragParams.strip();
                auto annotationParts = dragParams.Split(ProtobufMessageParam::CacheSegSepFlag);
                if(annotationParts.empty())
                    continue;

                // ????????????
                for(auto &annotationPairsStr : annotationParts)
                {
                    auto annotationPairParts = annotationPairsStr.Split(ProtobufMessageParam::CacheKVSepFlag);
                    if(annotationPairParts.empty())
                        continue;

                    auto iterKey = annotationParamNameRefValue.find(annotationPairParts[0]);
                    if(iterKey != annotationParamNameRefValue.end())
                    {
                        g_Log->Error(LOGFMT_OBJ_TAG("repeate annotation :%s in proto file:%s, annotationPairsStr:%s, reverseLine:%s, annotation key:%s")
                                    , dragMessageName.c_str(), fullFilePath.c_str(), annotationPairsStr.c_str(), reverseLine.c_str(), annotationPairParts[0].c_str());
                        return false;
                    }

                    if(annotationPairParts.size() > 1)
                    {
                        iterKey = annotationParamNameRefValue.insert(std::make_pair(annotationPairParts[0], annotationPairParts[1])).first;
                    }
                    else
                    {
                        iterKey = annotationParamNameRefValue.insert(std::make_pair(annotationPairParts[0], KERNEL_NS::LibString())).first;
                    }

                    // ????????????????????????????????????
                    if(iterKey->second.empty())
                        iterKey->second = _pbCacheContent->GetMessageAnnotationValue(fullFilePath,  messageInfo->_messageName, annotationPairParts[0]);
                }
            }

            // ????????????
            messageInfo->FieldsFromAnnotations(_maxOpcode);
            messageInfoDict.insert(std::make_pair(messageInfo->_messageName, messageInfo.AsSelf()));
            messageInfo.pop();
        }
    }
    
    KERNEL_NS::LibString protoMd5;
    if(!KERNEL_NS::LibDigest::MakeMd5Final(*ctx, protoMd5))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("md5 final fail fullFilePath:%s"), fullFilePath.c_str());
        return false;
    }
    
    protoInfo->_md5 = KERNEL_NS::LibBase64::Encode(protoMd5);

    // md5????????????
    auto iterPbCache = _pbCacheContent->_protoPathRefFileInfo.find(fullFilePath);
    bool isProtoFileChanged = true;
    if(iterPbCache != _pbCacheContent->_protoPathRefFileInfo.end())
    {
        auto pbCache = iterPbCache->second;
        if(pbCache->_md5 == protoInfo->_md5)
            isProtoFileChanged = false;
    }
    protoInfo->_isMd5Change = isProtoFileChanged;

    // ????????????????????????????????????
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
    // ??????????????????????????????????????????
    for(auto &kv : _protoNameRefProtoInfo)
    {
        auto protoContent = kv.second;
        auto &messageInfoList = protoContent->_messageNameRefMessageInfo;
        for(auto iter : messageInfoList)
        {
            auto messageInfo = iter.second;
            const auto &updateCacheInfo = messageInfo->ToPbCache(protoContent->_protoInfo._fileName, protoContent->_fullPathName);
            // ??????????????????????????????
            if(updateCacheInfo._opcode <= 0)
                continue;

            if(_pbCacheContent->IsMessageCacheExists(updateCacheInfo._protoPath, updateCacheInfo._messageName))
            {
                _pbCacheContent->UpdateMessageCache(updateCacheInfo);
            }
            else
            {
                _pbCacheContent->AddMessageCache(updateCacheInfo);
            }
        }

        // ???????????????????????????
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

    // ??????????????????
    std::set<PbCacheFileInfo *> invalidFiles;
    for(auto kv : _pbCacheContent->_lineRefProtoFileInfo)
    {
        if(!KERNEL_NS::FileUtil::IsFileExist(kv.second->_protoPath.c_str()))
            invalidFiles.insert(kv.second);
    }

    for(auto protoFile : invalidFiles)
        _RemoveInvalidFiles(protoFile);

    // ??????????????????????????????
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
    // TODO: ??????.h, .cc, .cs???
    const auto &appFullPath = GetServiceProxy()->GetApp()->GetAppPath();
    const auto &appPath = KERNEL_NS::DirectoryUtil::GetFileDirInPath(appFullPath);

    // ???????????? = ???????????? - ??????protoPath??????
    const auto fullProtoPath = appPath + _protoPath;
    const auto protoFullPath = appPath + cacheFile->_protoPath;
    const auto relationPath = protoFullPath - fullProtoPath;
    const auto protoName = KERNEL_NS::FileUtil::ExtractFileWithoutExtension(cacheFile->_protoName);

    // .pb.h???????????? = ?????????cppOut?????? + ????????????
    {
        const auto fullCppOutPath = appPath + _cppOutPath;
        const auto pbHeaderPath = KERNEL_NS::FileUtil::ExtractFileWithoutExtension(fullCppOutPath + relationPath) + ".pb.h";
        const auto pbCCPath = KERNEL_NS::FileUtil::ExtractFileWithoutExtension(fullCppOutPath + relationPath) + ".pb.cc";
        if(KERNEL_NS::FileUtil::IsFileExist(pbHeaderPath.c_str()))
            KERNEL_NS::FileUtil::DelFileCStyle(pbHeaderPath.c_str());
        if(KERNEL_NS::FileUtil::IsFileExist(pbCCPath.c_str()))
            KERNEL_NS::FileUtil::DelFileCStyle(pbCCPath.c_str());
    }


    // .cs ???????????? = ?????????csharpOut?????? + ????????????
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


SERVICE_END