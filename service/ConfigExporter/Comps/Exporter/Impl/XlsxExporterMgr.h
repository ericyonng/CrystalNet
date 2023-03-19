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

#include <service/ConfigExporter/Comps/Exporter/Interface/IXlsxExporterMgr.h>

SERVICE_BEGIN

class XlsxConfigMetaInfo;

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
    Int32 _OnGlobalSysInit() override;
    void _OnGlobalSysClose() override;

    // 扫描meta文件
    bool _ScanMeta();

    // 扫描xlsx文件
    bool _ScanXlsx();

    // 是否需要导出
    bool _IsNeedExport(const KERNEL_NS::LibString &metaFile, const KERNEL_NS::LibString &xlsxFile) const;

    // meta file
    const XlsxConfigMetaInfo *_GetMetaFile(const KERNEL_NS::LibString &metaFile) const;

private:
    void _Clear();

    void _RegisterEvents();
    void _UnRegisterEvents();

private:
    // target字典
    KERNEL_NS::LibString _sourceDir;
    KERNEL_NS::LibString _targetDir;
    KERNEL_NS::LibString _dataDir;
    KERNEL_NS::LibString _metaDir;
    std::unordered_map<KERNEL_NS::LibString, std::unordered_set<KERNEL_NS::LibString>> _configTypeRefLangTypes;

    std::unordered_map<KERNEL_NS::LibString, XlsxConfigMetaInfo *> _metaNameRefConfigMetaInfo;
};

ALWAYS_INLINE  const XlsxConfigMetaInfo *XlsxExporterMgr::_GetMetaFile(const KERNEL_NS::LibString &metaFile) const
{
    auto iter = _metaNameRefConfigMetaInfo.find(metaFile);
    return iter == _metaNameRefConfigMetaInfo.end() ? NULL : iter->second;
}


SERVICE_END