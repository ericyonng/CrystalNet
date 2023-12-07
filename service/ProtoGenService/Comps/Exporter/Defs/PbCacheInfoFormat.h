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
 * Date: 2022-10-23 15:40:59
 * Author: Eric Yonng
 * Description: 
 * 协议格式：[ProtoName:xxx,// 需要注意是排除ProtoPath后的路径proto名， MessageName: 协议名, Opcode: 协议号]
 * proto文件信息格式:/// ProtoName:xxx,// 需要注意是排除ProtoPath后的路径proto名, md5:,ModifyTime:,ProtoPath:,
*/

#pragma once

#include <kernel/kernel.h>
#include <service/common/macro.h>

SERVICE_BEGIN

struct PbCaheInfo
{
    POOL_CREATE_OBJ_DEFAULT(PbCaheInfo);

    PbCaheInfo();
    ~PbCaheInfo() {}

    bool CheckValid() const;

    KERNEL_NS::LibString ToPbChacheString() const;

    KERNEL_NS::LibString GetAnnotationValue(const KERNEL_NS::LibString &annotationKey) const;
    
    KERNEL_NS::LibString _messageName;  // message名
    KERNEL_NS::LibString _protoName;    // 文件名
    KERNEL_NS::LibString _protoPath;    // 文件完整的路径
    Int32 _opcode;
    Int32 _line;
    bool _noLog;    // 不打印日志, 和opcode互相配合
    bool _isXorEncrypt;
    bool _isKeyBase64;

    // 开启存储
    bool _enableStorage;
};

// 排序
class PbCacheInfoCompare
{
public:
    bool operator()(const PbCaheInfo *l, const PbCaheInfo *r) const;
};

struct PbCacheFileInfo
{
    POOL_CREATE_OBJ_DEFAULT(PbCacheFileInfo);
    PbCacheFileInfo();

    bool CheckValid() const;

    KERNEL_NS::LibString ToPbChacheString() const;

    KERNEL_NS::LibString _protoName;    // 文件名
    KERNEL_NS::LibString _protoPath;    // 完整的路径
    KERNEL_NS::LibString _md5;          // md5
    Int64 _modifyTime;                  // 文件修改时间
    Int32 _line;                        // 行号
};

// 排序
class PbCacheFileInfoCompare
{
public:
    bool operator()(const PbCacheFileInfo *l, const PbCacheFileInfo *r) const;
};

struct PbCacheFileContent
{
    POOL_CREATE_OBJ_DEFAULT(PbCacheFileContent);

    PbCacheFileContent() {}
    ~PbCacheFileContent()
    {
        KERNEL_NS::ContainerUtil::DelContainer(_lineRefMessageInfo, [](PbCaheInfo *ptr){
            PbCaheInfo::Delete_PbCaheInfo(ptr);
        });
        
        KERNEL_NS::ContainerUtil::DelContainer(_lineRefProtoFileInfo, [](PbCacheFileInfo *ptr){
            PbCacheFileInfo::Delete_PbCacheFileInfo(ptr);
        });
    }

    bool LoadPbCache(const KERNEL_NS::LibString &pbcacheFile, Int32 &maxOpcode);
    bool _LoadMessageInfo(Int32 currentLine, KERNEL_NS::LibString &lineData, bool &isContinue, Int32 &maxOpcode, std::set<Int32> &opcodeFilter);
    bool _LoadProtoFileInfo(Int32 currentLine, KERNEL_NS::LibString &lineData, bool &isContinue);
    bool UpdatePbCache(const KERNEL_NS::LibString &pbcacheFile);

    bool IsMessageCacheExists(const KERNEL_NS::LibString &filePath, const KERNEL_NS::LibString &messageName) const;
    void UpdateMessageCache(const PbCaheInfo &pbCache);
    void AddMessageCache(const PbCaheInfo &pbCache);
    KERNEL_NS::LibString GetMessageAnnotationValue(const KERNEL_NS::LibString &filePath, const KERNEL_NS::LibString &messageName, const KERNEL_NS::LibString &annotationKey) const;

    bool IsProtoFileCacheExists(const KERNEL_NS::LibString &filePath) const;
    void UpdateProtoFileCache(const PbCacheFileInfo &cache);
    void AddProtoFileCache(const PbCacheFileInfo &cache);

    void RemoveInvalidFile(PbCacheFileInfo *protoFile);
    void RemoveInvalidMessagesBy(const KERNEL_NS::LibString &protoPath, std::function<bool(const KERNEL_NS::LibString &messageName)> &&checkValidMessage);

    Int32 GetMaxLine() const;

    std::map<Int32, KERNEL_NS::LibString> _lineRefContent;
    std::map<Int32, PbCaheInfo *> _lineRefMessageInfo;
    std::map<Int32, PbCacheFileInfo *> _lineRefProtoFileInfo;

    std::map<KERNEL_NS::LibString, std::map<KERNEL_NS::LibString, PbCaheInfo *>> _protoPathRefMessageNameCacheInfo;
    std::map<KERNEL_NS::LibString, PbCacheFileInfo *> _protoPathRefFileInfo;
};

SERVICE_END