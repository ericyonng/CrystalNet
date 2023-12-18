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
#include <service/ProtoGenService/Comps/Exporter/Defs/PbRawInfo.h>
#include <service/ProtoGenService/Comps/Exporter/Defs/Orm.h>

KERNEL_BEGIN

class CodeUnit;

KERNEL_END


SERVICE_BEGIN

struct ProtoContentInfo;
struct PbCacheFileContent;
struct PbCacheFileInfo;
struct CodeUnitTopologyTreeNode;

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
    Int32 _OnGlobalSysCompsCreated() override;
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
    bool _GenTsExtends();

    // 生成ORM
    bool _LoadOrmCache();
    bool _GenORM();
    bool _AddDependence(KERNEL_NS::SmartPtr<KERNEL_NS::CodeUnit, KERNEL_NS::AutoDelMethods::Release> &codeUnit, std::vector<KERNEL_NS::SmartPtr<KERNEL_NS::CodeUnit, KERNEL_NS::AutoDelMethods::Release>> &dependences);
    bool _GenOrmHeader(const KERNEL_NS::LibString &ormRootPath, const KERNEL_NS::LibString &appName, KERNEL_NS::SmartPtr<KERNEL_NS::CodeUnit, KERNEL_NS::AutoDelMethods::Release> &codeUnit);
    bool _GenOrmHeaderInterface(const KERNEL_NS::LibString &nameSapce, const KERNEL_NS::LibString &ormRootPath, const KERNEL_NS::LibString &appName
    , KERNEL_NS::SmartPtr<KERNEL_NS::CodeUnit, KERNEL_NS::AutoDelMethods::Release> &codeUnit, std::vector<KERNEL_NS::LibString> &headerCodeLines
    , std::set<Int32> &preDeclareTypes, std::vector<KERNEL_NS::LibString> &googlePre, std::vector<KERNEL_NS::LibString> &protobufPreDeclare, std::vector<KERNEL_NS::LibString> &serviceCommonPreDeclare
    ,  std::vector<KERNEL_NS::LibString> &stdLibs);
    
    bool _GenOrmImpl(const KERNEL_NS::LibString &ormRootPath, const KERNEL_NS::LibString &appName, KERNEL_NS::SmartPtr<KERNEL_NS::CodeUnit, KERNEL_NS::AutoDelMethods::Release> &codeUnit);
    void _CreateFieldOrmData(const KERNEL_NS::LibString &fieldName, const KERNEL_NS::LibString &fieldDataType, std::vector<KERNEL_NS::LibString> &lines, bool needRelease = true, bool needCheckHasCustom = true) const;
    void _CreateVarOrmData(const KERNEL_NS::LibString &varName, const KERNEL_NS::LibString &pbName, const KERNEL_NS::LibString &varDataType, std::vector<KERNEL_NS::LibString> &lines, bool needCheckHasCustom = true) const;
    void _CreateFieldOrmDataArray(const KERNEL_NS::LibString &fieldName, const KERNEL_NS::LibString &fieldDataType, std::vector<KERNEL_NS::LibString> &lines) const;
    bool _GenOrmHeaderInterfaceImpl(const KERNEL_NS::LibString &nameSapce, const KERNEL_NS::LibString &ormRootPath, const KERNEL_NS::LibString &appName, KERNEL_NS::SmartPtr<KERNEL_NS::CodeUnit, KERNEL_NS::AutoDelMethods::Release> &codeUnit, std::vector<KERNEL_NS::LibString> &implCodeLines);
    void _UpdateOrmCache();
    void _GenAllOrms();
    void _GenOrmEnums();
    void _GenOrmMgr();

    // protobuf语法分析
    bool _GrammarAnalyze();
    bool _ScanProtoDataType(const KERNEL_NS::FindFileInfo &fileInfo, const KERNEL_NS::LibString &fullFilePath, bool &isParentPathContinue);

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
    KERNEL_NS::LibString _ormOutPath;        // orm生成路径 (相对于工具)

    bool _forceGenAll;                      // 强制全部重新生成

    // 当前最大opcode
    Int32 _maxOpcode;
    // 都是有变化的proto
    std::map<KERNEL_NS::LibString, ProtoContentInfo *> _protoNameRefProtoInfo;

    // pb cache file内容
    PbCacheFileContent *_pbCacheContent;

    // 数据类型全名 => 数据类型 , 类型全名 = 完整的域名 + 类型名 中间没有任何符号
    std::map<KERNEL_NS::LibString, KERNEL_NS::SmartPtr<ProtobufDataTypeInfo, KERNEL_NS::AutoDelMethods::Release>> _dataTypeRefPbDataTypeInfo;

    KERNEL_NS::ListenerStub _closeServiceStub;

    // protobuf基本数据类型
    std::set<KERNEL_NS::LibString> _protobufBaseDataType;

    // 文件的import依赖
    std::map<KERNEL_NS::LibString, std::vector<KERNEL_NS::LibString>> _fullPathRefImportFilePath;

    // orm信息
    std::map<KERNEL_NS::LibString, OrmInfo> _typeNameRefOrmInfo;

    Int64 _maxOrmId;

    bool _isOrmCacheDirty;

    // 扫描所有需要storage的
    std::vector<KERNEL_NS::SmartPtr<KERNEL_NS::CodeUnit, KERNEL_NS::AutoDelMethods::Release>> _codeUnits;
    // 文件为单位的类型拓扑
    std::map<KERNEL_NS::LibString, std::vector<KERNEL_NS::SmartPtr<CodeUnitTopologyTreeNode>>> _fileRefTreeNode;
    // 所有
    std::map<KERNEL_NS::LibString, KERNEL_NS::SmartPtr<CodeUnitTopologyTreeNode>> _classRefTreeNode;
};

SERVICE_END