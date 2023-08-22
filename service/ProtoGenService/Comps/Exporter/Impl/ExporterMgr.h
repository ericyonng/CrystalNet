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
 * Date: 2022-10-11 23:42:04
 * Author: Eric Yonng
 * Description: 
*/


#pragma once

#include <service/ProtoGenService/Comps/Exporter/Interface/IExporterMgr.h>

SERVICE_BEGIN

struct ProtoContentInfo;
struct PbCacheFileContent;
struct PbCacheFileInfo;

class ExporterMgr : public IExporterMgr
{
    POOL_CREATE_OBJ_DEFAULT_P1(IExporterMgr, ExporterMgr);

public:
    ExporterMgr();
    ~ExporterMgr();

    void Release() override;

public:
    virtual KERNEL_NS::LibString ToString() const override;

protected:
    Int32 _OnGlobalSysInit() override;
    Int32 _OnHostStart() override;
    void _OnGlobalSysClose() override;

    // 分析proto并提取注解
    bool _AnalyzeProtoAnnotation();

    // 生成cpp
    bool _GenCplusplus();
    bool _ModifyCppPbHeader(const KERNEL_NS::LibString &pbHeaderName, std::vector<KERNEL_NS::LibString> &lines, const ProtoContentInfo *protoFileInfo, std::vector<KERNEL_NS::LibString> &newClassFactoryNames);
    void _CollectCppClassAdds(const KERNEL_NS::LibString &className, KERNEL_NS::LibString &addLines);
    void _CollectCppClassAdds(const KERNEL_NS::LibString &className, std::vector<KERNEL_NS::LibString> &addLines);
    void _CollectCppClassFactoryDeclearAdds(const KERNEL_NS::LibString &packageName, const KERNEL_NS::LibString &className, std::vector<KERNEL_NS::LibString> &addLines, std::vector<KERNEL_NS::LibString> &newClassFactoryNames);
    bool _ModifyCppPbCC(const KERNEL_NS::LibString &pbCCName, std::vector<KERNEL_NS::LibString> &lines, const ProtoContentInfo *protoFileInfo, const std::vector<KERNEL_NS::LibString> &newFactoryNames);
    void _GenOpcodeEnums();
    void _GenOpcodeInfo();
    void _GenAllPbs();
    // void _AddPchHeaderToGoogleCC();

    // 生成csharp
    bool _GenCSharp();
    void _AddCsharpNamespace(std::vector<KERNEL_NS::LibString> &lines);
    void _ProtoMessageAttribute(const ProtoContentInfo *protoFile, std::vector<KERNEL_NS::LibString> &lines);

    // 生成ts
    bool _GenTs();

    // pbcache file加载
    bool _LoadPbCache();

    bool _ScanProtos();
    bool _ScanAProto(const KERNEL_NS::FindFileInfo &fileInfo, const KERNEL_NS::LibString &fullFilePath, bool &isParentPathContinue);
    
    bool _UpdateProtoCache();
    void _RemoveInvalidFiles(PbCacheFileInfo *cacheFile);

    KERNEL_NS::LibString _FileHeader(const KERNEL_NS::LibTime &time, const KERNEL_NS::LibString &desc);

private:
    void _Clear();
    void _RegisterEvents();
    void _UnRegisterEvents();

    KERNEL_NS::LibString _basePath;         // 工程根目录相对于工具的路径
    KERNEL_NS::LibString _protocolsPath;    // 协议的工作根目录（相对于工具）
    KERNEL_NS::LibString _protoPath;        // proto路径（相对于工具）
    KERNEL_NS::LibString _cppOutPath;       // cpp out路径（相对于工具）
    KERNEL_NS::LibString _csharpOutPath;    // csharp生成路径（相对于工具）
    KERNEL_NS::LibString _tsOutPath;        // ts生成路径 (相对于工具)
    KERNEL_NS::LibString _cppProtocPath;    // cpp生成工具路径（相对于工具）
    KERNEL_NS::LibString _csharpProtocPath; // csharp生成工具路径（相对于工具）
    KERNEL_NS::LibString _googleProtoIncludePath;   // proto库目录
    bool _forceGenAll;                      // 强制全部重新生成

    // 当前最大opcode
    Int32 _maxOpcode;
    // 都是有变化的proto
    std::map<KERNEL_NS::LibString, ProtoContentInfo *> _protoNameRefProtoInfo;

    // pb cache file内容
    PbCacheFileContent *_pbCacheContent;

    KERNEL_NS::ListenerStub _closeServiceStub;
};

SERVICE_END