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

#pragma once

#include <ConfigExporter/Exporter/Interface/IXlsxExporterMgr.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/xlsx/xlsx.h>

#include <unordered_map>
#include <set>
#include <unordered_set>

class XlsxConfigMetaInfo;
class XlsxConfigTableInfo;

class XlsxExporterMgr : public IXlsxExporterMgr
{
    POOL_CREATE_OBJ_DEFAULT_P1(IXlsxExporterMgr, XlsxExporterMgr);

public:
    XlsxExporterMgr();
    ~XlsxExporterMgr();

    void Release() override;



    virtual Int32 ExportConfigs(const std::map<KERNEL_NS::LibString, KERNEL_NS::LibString> &params) override;

public:
    virtual KERNEL_NS::LibString ToString() const override;

protected:
    Int32 _OnInit() override;
    void _OnWillClose() override;

    // 扫描meta文件
    bool _ScanMeta();

    // 扫描xlsx文件
    bool _ScanXlsx();

    // 是否需要导出
    bool _IsNeedExport(const KERNEL_NS::LibString &metaFile, const KERNEL_NS::LibString &xlsxFile) const;
    bool _CalcMd5(const KERNEL_NS::LibString &file, KERNEL_NS::LibString &md5) const;

    bool _IsOwnTypeNeedExport(const KERNEL_NS::LibString &ownType) const;

    // 分析配置结构和数据
    bool _AnalyzeExportConfigs();
    bool _PrepareConfigStructAndDatas(std::unordered_map<KERNEL_NS::LibString, std::set<XlsxConfigTableInfo *>> &ownTypeRefConfigs, std::set<XlsxConfigTableInfo *> *configTables) const;

    // 导出配置代码
    bool _DoExportConfigs() const;
    bool _ExportCpp(const KERNEL_NS::LibString &langName, const std::map<KERNEL_NS::LibString, XlsxConfigTableInfo *> &configTypeRefConfigTableInfo) const;
    bool _ExportCppCodeHeader(const XlsxConfigTableInfo *configInfo, KERNEL_NS::LibString &fileContent) const;
    bool _ExportCppCodeImpl(const XlsxConfigTableInfo *configInfo, KERNEL_NS::LibString &fileContent) const;
    bool _ExportCppDatas(const XlsxConfigTableInfo *configInfo, KERNEL_NS::LibString &configDataContent) const;

    bool _ExportMetaFile(const KERNEL_NS::LibString &xlsxPath) const;

    void _ExportCppAllConfigHeaderFile(const KERNEL_NS::LibString &lang) const;
    void _ExportCppRegisterConfigs(const KERNEL_NS::LibString &lang) const;

    // 导出配置数据
    bool _ExportCSharpCode(const std::map<KERNEL_NS::LibString, XlsxConfigTableInfo *> &configTypeRefConfigTableInfo) const;
    bool _ExportCSharpDatas(const std::map<KERNEL_NS::LibString, XlsxConfigTableInfo *> &configTypeRefConfigTableInfo) const;
    
    // 更新meta文件
    bool _UpdateMetas();

    // meta file
    const XlsxConfigMetaInfo *_GetMetaFile(const KERNEL_NS::LibString &metaFile) const;

    // 通过sheet name 获取configType
    KERNEL_NS::LibString _GetConfigTypeName(const KERNEL_NS::LibString &sheetName) const;

    // 获取member成员名
    KERNEL_NS::LibString _MakeConfigMemberName(const KERNEL_NS::LibString &fieldName) const;

private:
    void _Clear();

private:
    // target字典
    KERNEL_NS::LibString _sourceDir;
    KERNEL_NS::LibString _targetDir;
    KERNEL_NS::LibString _dataDir;
    KERNEL_NS::LibString _metaDir;
    KERNEL_NS::LibString _baseDir;
    std::unordered_map<KERNEL_NS::LibString, std::unordered_set<KERNEL_NS::LibString>> _ownTypeRefLangTypes;

    std::unordered_map<KERNEL_NS::LibString, XlsxConfigMetaInfo *> _metaNameRefConfigMetaInfo;

    std::set<KERNEL_NS::LibString> _needExportConfigType;     // 需要导出的配置 sheet 标签指定的配置
    std::set<KERNEL_NS::LibString> _dirtyXlsxFiles;             // 脏配置表,会生成新的meta文件
    std::unordered_map<KERNEL_NS::LibString, std::set<KERNEL_NS::XlsxSheet *>> _configTypeRefSheets;  // 配置类型与xlsx 用于合并同类配置
    std::unordered_map<KERNEL_NS::LibString, KERNEL_NS::XlsxWorkbook *> _xlsxFileRefWorkbook;   // 所有的配置xlsx

    std::map<KERNEL_NS::LibString, std::map<KERNEL_NS::LibString, XlsxConfigTableInfo *>> _ownTypeRefConfigTypeRefXlsxConfigTableInfo;    // 每个配置类型对应的配置信息

    KERNEL_NS::LibString _allConfigsHeader;
    KERNEL_NS::LibString _registerAllConfigs;
};

ALWAYS_INLINE  const XlsxConfigMetaInfo *XlsxExporterMgr::_GetMetaFile(const KERNEL_NS::LibString &metaFile) const
{
    auto iter = _metaNameRefConfigMetaInfo.find(metaFile);
    return iter == _metaNameRefConfigMetaInfo.end() ? NULL : iter->second;
}

// sheetname: xxx名字|xxx类型名(英文, 数字, 下划线, 首字母非数字)
ALWAYS_INLINE KERNEL_NS::LibString XlsxExporterMgr::_GetConfigTypeName(const KERNEL_NS::LibString &sheetName) const
{
    const auto &parts = sheetName.Split('|');
    if(parts.size() < 2)
    {
        return parts[0];
    }

    return parts[1];
}

ALWAYS_INLINE KERNEL_NS::LibString XlsxExporterMgr::_MakeConfigMemberName(const KERNEL_NS::LibString &fieldName) const
{
    return "_" + fieldName.FirstCharToLower();
}