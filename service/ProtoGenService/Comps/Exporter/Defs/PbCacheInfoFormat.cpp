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
 * Date: 2022-10-23 16:00:28
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <service/ProtoGenService/Comps/Exporter/Defs/PbCacheInfoFormat.h>
#include <service/ProtoGenService/Comps/Exporter/Defs/Defs.h>
SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(PbCaheInfo);
POOL_CREATE_OBJ_DEFAULT_IMPL(PbCacheFileInfo);
POOL_CREATE_OBJ_DEFAULT_IMPL(PbCacheFileContent);

PbCaheInfo::PbCaheInfo()
:_opcode(0)
,_line(0)
{

}

bool PbCaheInfo::CheckValid() const
{
    if(_opcode <= 0)
        return false;

    if(_line <= 0)
        return false;

    if(_messageName.empty())
        return false;

    if(_protoName.empty())
        return false;

    if(_protoPath.empty())
        return false;

    return true;
}

KERNEL_NS::LibString PbCaheInfo::ToPbChacheString() const
{
    KERNEL_NS::LibString info;
    info.AppendFormat("%s", ProtobufMessageParam::MessageStartFlag.c_str())
        .AppendFormat("%s%s%s%s", ProtobufMessageParam::ProtoName.c_str(), ProtobufMessageParam::CacheKVSepFlag.c_str(), _protoName.c_str(), ProtobufMessageParam::CacheSegSepFlag.c_str())
        .AppendFormat("%s%s%s%s", ProtobufMessageParam::ProtoPath.c_str(), ProtobufMessageParam::CacheKVSepFlag.c_str(), _protoPath.c_str(), ProtobufMessageParam::CacheSegSepFlag.c_str())
        .AppendFormat("%s%s%s%s", ProtobufMessageParam::MessageName.c_str(), ProtobufMessageParam::CacheKVSepFlag.c_str(), _messageName.c_str(), ProtobufMessageParam::CacheSegSepFlag.c_str())
        .AppendFormat("%s%s%d%s", ProtobufMessageParam::Opcode.c_str(), ProtobufMessageParam::CacheKVSepFlag.c_str(), _opcode, ProtobufMessageParam::CacheSegSepFlag.c_str())
        .AppendFormat("%s", ProtobufMessageParam::MessageEndFlag.c_str())
        ;

    return info;
}

KERNEL_NS::LibString PbCaheInfo::GetAnnotationValue(const KERNEL_NS::LibString &annotationKey) const
{
    KERNEL_NS::LibString value;
    if(annotationKey == ProtobufMessageParam::Opcode)
    {
        value.AppendFormat("%d", _opcode);
        return value;
    }

    return value;
}

bool PbCacheInfoCompare::operator()(const PbCaheInfo *l, const PbCaheInfo *r) const
{
    if(!l || !r)
        return l < r;
    
    if(l == r)
        return false;
    
    if(l->_opcode == r->_opcode)
        return l < r;

    return l->_opcode < r->_opcode;
}

PbCacheFileInfo::PbCacheFileInfo()
:_modifyTime(0)
,_line(0)
{

}

bool PbCacheFileInfo::CheckValid() const
{
    if(_line <= 0)
        return false;

    if(_protoName.empty())
        return false;

    if(_modifyTime <= 0)
        return false;

    if(_protoPath.empty())
        return false;

    return true;
}

KERNEL_NS::LibString PbCacheFileInfo::ToPbChacheString() const
{
    KERNEL_NS::LibString info;
    info.AppendFormat("%s ", ProtobufMessageParam::ProtoFileStartFlag.c_str())
        .AppendFormat("%s%s%s%s", ProtobufMessageParam::ProtoName.c_str(), ProtobufMessageParam::CacheKVSepFlag.c_str(), _protoName.c_str(), ProtobufMessageParam::CacheSegSepFlag.c_str())
        .AppendFormat("%s%s%s%s", ProtobufMessageParam::ProtoPath.c_str(), ProtobufMessageParam::CacheKVSepFlag.c_str(), _protoPath.c_str(), ProtobufMessageParam::CacheSegSepFlag.c_str())
        .AppendFormat("%s%s%s%s", ProtobufMessageParam::Md5.c_str(), ProtobufMessageParam::CacheKVSepFlag.c_str(), _md5.c_str(), ProtobufMessageParam::CacheSegSepFlag.c_str())
        .AppendFormat("%s%s%lld%s", ProtobufMessageParam::ModifyTime.c_str(), ProtobufMessageParam::CacheKVSepFlag.c_str(), _modifyTime, ProtobufMessageParam::CacheSegSepFlag.c_str())
        ;

    return info;
}

bool PbCacheFileInfoCompare::operator()(const PbCacheFileInfo *l, const PbCacheFileInfo *r) const
{
    if(!l || !r)
        return l < r;
    
    if(l == r)
        return false;
    
    if(l->_protoName == r->_protoName)
        return l < r;

    return l->_protoName < r->_protoName;
}

bool PbCacheFileContent::LoadPbCache(const KERNEL_NS::LibString &pbcacheFile, Int32 &maxOpcode)
{
    // 不存在不必加载
    if(!KERNEL_NS::FileUtil::IsFileExist(pbcacheFile.c_str()))
        return true;

    auto fp = KERNEL_NS::FileUtil::OpenFile(pbcacheFile.c_str());
    if(!fp)
        return true;

    g_Log->Custom("LOAD PB CACHE %s", KERNEL_NS::DirectoryUtil::GetFileNameInPath(pbcacheFile).c_str());
    // 1.扫描所有message 信息
    // 2.扫描所有proto文件信息
    Int32 currentLine = 0;
    auto &lineContentDict = _lineRefContent;
    bool isSuc = true;
    while(!KERNEL_NS::FileUtil::IsEnd(*fp))
    {
        KERNEL_NS::LibString lineData;
        KERNEL_NS::FileUtil::ReadUtf8OneLine(*fp, lineData);
        ++currentLine;

        lineContentDict.insert(std::make_pair(currentLine, lineData));

        lineData.strip();
        bool isContinue = true;
        // message 信息
        if(lineData.IsStartsWith(ProtobufMessageParam::MessageStartFlag) && lineData.IsEndsWith(ProtobufMessageParam::MessageEndFlag))
        {// message 信息
            if(!_LoadMessageInfo(currentLine, lineData, isContinue, maxOpcode))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("load message info fail"));
                isSuc = false;
                break;
            }

            if(!isContinue)
                continue;
        }
        else if(lineData.IsStartsWith(ProtobufMessageParam::ProtoFileStartFlag))
        {// proto 文件信息
            if(!_LoadProtoFileInfo(currentLine, lineData, isContinue))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("load proto file info fail"));
                isSuc = false;
                break;
            }

            if(!isContinue)
                continue;
        }
    }

    KERNEL_NS::FileUtil::CloseFile(*fp);

    g_Log->Custom("LOAD PB CACHE %s END", KERNEL_NS::DirectoryUtil::GetFileNameInPath(pbcacheFile).c_str());

    return isSuc;
}

bool PbCacheFileContent::UpdatePbCache(const KERNEL_NS::LibString &pbcacheFile)
{
    return KERNEL_NS::FileUtil::ReplaceFile(pbcacheFile, _lineRefContent);
}

bool PbCacheFileContent::_LoadMessageInfo(Int32 currentLine, KERNEL_NS::LibString &lineData, bool &isContinue, Int32 &maxOpcode)
{
    auto &protoPathRefMessageNameCacheInfo = _protoPathRefMessageNameCacheInfo;
    const auto &messageContent = lineData.DragRange(ProtobufMessageParam::MessageStartFlag, ProtobufMessageParam::MessageEndFlag);
    if(messageContent.empty())
    {
        g_Log->Error(LOGFMT_OBJ_TAG("proto cache file format error:have no info between %s, %s, currentLine:%d, line data:%s")
                    , ProtobufMessageParam::MessageStartFlag.c_str(), ProtobufMessageParam::MessageEndFlag.c_str(), currentLine, lineData.c_str());

        return false;
    }

    const auto &segs = messageContent.Split(ProtobufMessageParam::CacheSegSepFlag);
    if(segs.empty())
    {
        g_Log->Error(LOGFMT_OBJ_TAG("proto cache file format error:when split with %s, messageContent:%s, currentLine:%d, line data:%s")
                    , ProtobufMessageParam::CacheSegSepFlag.c_str(), messageContent.c_str(), currentLine, lineData.c_str());

        return false;
    }

    std::map<KERNEL_NS::LibString, KERNEL_NS::LibString> fieldDict;
    for(auto &segInfo : segs)
    {
        if(segInfo.empty())
            continue;

        const auto &kvs = segInfo.Split(ProtobufMessageParam::CacheKVSepFlag);
        if(kvs.empty() || kvs.size() != 2)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("proto cache file format error:split kv sep fail segInfo:%s currentLine:%d, line data:%s")
                    , segInfo.c_str(), currentLine, lineData.c_str());

            return false;
        }

        auto k = kvs[0];
        auto v = kvs[1];
        k.strip();
        v.strip();
        fieldDict.insert(std::make_pair(k, v));
    }

    if(fieldDict.empty())
    {
        isContinue = false;
        return true;
    }

    auto pbCache = PbCaheInfo::New_PbCaheInfo();
    pbCache->_line = currentLine;
    for(auto &kv : fieldDict)
    {
        if(kv.first == ProtobufMessageParam::ProtoName)
        {
            pbCache->_protoName = kv.second;
        }
        else if(kv.first == ProtobufMessageParam::ProtoPath)
        {
            pbCache->_protoPath = kv.second;
        }
        else if(kv.first == ProtobufMessageParam::MessageName)
        {
            pbCache->_messageName = kv.second;
        }
        else if(kv.first == ProtobufMessageParam::Opcode)
        {
            pbCache->_opcode = KERNEL_NS::StringUtil::StringToInt32(kv.second.c_str());
            maxOpcode = std::max<Int32>(maxOpcode, pbCache->_opcode);
        }
    }

    if(!pbCache->CheckValid())
    {
        g_Log->Error(LOGFMT_OBJ_TAG("invalid pb cache line data:%s"), lineData.c_str());
        PbCaheInfo::Delete_PbCaheInfo(pbCache);
        return false;
    }

    _lineRefMessageInfo.insert(std::make_pair(currentLine, pbCache));

    {
        auto iterPb = protoPathRefMessageNameCacheInfo.find(pbCache->_protoPath);
        if(iterPb == protoPathRefMessageNameCacheInfo.end())
            iterPb = protoPathRefMessageNameCacheInfo.insert(std::make_pair(pbCache->_protoPath, std::map<KERNEL_NS::LibString, PbCaheInfo *>())).first;

        auto &messageRefPbCache = iterPb->second;
        auto iterCache = messageRefPbCache.find(pbCache->_messageName);
        if(iterCache != messageRefPbCache.end())
        {
            g_Log->Error(LOGFMT_OBJ_TAG("duplicate message lineData:%s"), lineData.c_str());
            return false;
        }

        messageRefPbCache.insert(std::make_pair(pbCache->_messageName, pbCache));
    }

    return true;
}

bool PbCacheFileContent::_LoadProtoFileInfo(Int32 currentLine, KERNEL_NS::LibString &lineData, bool &isContinue)
{
    const auto &messageContent = lineData.DragAfter(ProtobufMessageParam::ProtoFileStartFlag);
    if(messageContent.empty())
    {
        g_Log->Error(LOGFMT_OBJ_TAG("proto cache file format error:have no info after %s, currentLine:%d, line data:%s")
                    , ProtobufMessageParam::ProtoFileStartFlag.c_str(), currentLine, lineData.c_str());

        return false;
    }

    const auto &segs = messageContent.Split(ProtobufMessageParam::CacheSegSepFlag);
    if(segs.empty())
    {
        g_Log->Error(LOGFMT_OBJ_TAG("proto cache file format error:when split with %s, messageContent:%s, currentLine:%d, line data:%s")
                    , ProtobufMessageParam::CacheSegSepFlag.c_str(), messageContent.c_str(), currentLine, lineData.c_str());

        return false;
    }

    std::map<KERNEL_NS::LibString, KERNEL_NS::LibString> fieldDict;
    for(auto &segInfo : segs)
    {
        if(segInfo.empty())
            continue;

        const auto &kvs = segInfo.Split(ProtobufMessageParam::CacheKVSepFlag);
        if(kvs.empty() || kvs.size() != 2)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("proto cache file format error:split kv sep fail segInfo:%s currentLine:%d, line data:%s")
                    , segInfo.c_str(), currentLine, lineData.c_str());

            return false;
        }

        auto k = kvs[0];
        auto v = kvs[1];
        k.strip();
        v.strip();
        fieldDict.insert(std::make_pair(k, v));
    }

    if(fieldDict.empty())
    {
        isContinue = false;
        return true;
    }

    auto pbFileInfo = PbCacheFileInfo::New_PbCacheFileInfo();
    pbFileInfo->_line = currentLine;
    for(auto &kv : fieldDict)
    {
        if(kv.first == ProtobufMessageParam::ProtoName)
        {
            pbFileInfo->_protoName = kv.second;
        }
        else if(kv.first == ProtobufMessageParam::Md5)
        {
            pbFileInfo->_md5 = kv.second;
        }
        else if(kv.first == ProtobufMessageParam::ModifyTime)
        {
            pbFileInfo->_modifyTime = KERNEL_NS::StringUtil::StringToInt64(kv.second.c_str());
        }
        else if(kv.first == ProtobufMessageParam::ProtoPath)
        {
            pbFileInfo->_protoPath = kv.second;
        }
    }

    if(!pbFileInfo->CheckValid())
    {
        g_Log->Error(LOGFMT_OBJ_TAG("invalid pb file info line data:%s"), lineData.c_str());
        PbCacheFileInfo::Delete_PbCacheFileInfo(pbFileInfo);
        return false;
    }

    _lineRefProtoFileInfo.insert(std::make_pair(currentLine, pbFileInfo));

    {
        auto &protoPathRefFileInfo = _protoPathRefFileInfo;
        auto iterPb = protoPathRefFileInfo.find(pbFileInfo->_protoPath);
        if(iterPb != protoPathRefFileInfo.end())
        {
            g_Log->Error(LOGFMT_OBJ_TAG("duplicate proto file info lineData:%s"), lineData.c_str());
            return false;
        }

        protoPathRefFileInfo.insert(std::make_pair(pbFileInfo->_protoPath, pbFileInfo));
    }

    return true;
}


bool PbCacheFileContent::IsMessageCacheExists(const KERNEL_NS::LibString &filePath, const KERNEL_NS::LibString &messageName) const
{
    auto iterDict = _protoPathRefMessageNameCacheInfo.find(filePath);
    if(iterDict == _protoPathRefMessageNameCacheInfo.end())
        return false;

    auto iter = iterDict->second.find(messageName);
    return iter != iterDict->second.end();
}

void PbCacheFileContent::UpdateMessageCache(const PbCaheInfo &pbCache)
{
    auto iterDict = _protoPathRefMessageNameCacheInfo.find(pbCache._protoPath);
    if(iterDict == _protoPathRefMessageNameCacheInfo.end())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("bad pb cache not found proto path:%s"), pbCache.ToPbChacheString().c_str());
        return;
    }

    auto iter = iterDict->second.find(pbCache._messageName);
    if(iter == iterDict->second.end())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("bad pb cache not found message name:%s"), pbCache.ToPbChacheString().c_str());
        return;
    }

    auto line = iter->second->_line;
    *iter->second = pbCache;
    iter->second->_line = line;
    _lineRefContent[line] = iter->second->ToPbChacheString();
}

void PbCacheFileContent::AddMessageCache(const PbCaheInfo &pbCache)
{
    auto iterDict = _protoPathRefMessageNameCacheInfo.find(pbCache._protoPath);
    if(iterDict == _protoPathRefMessageNameCacheInfo.end())
        iterDict = _protoPathRefMessageNameCacheInfo.insert(std::make_pair(pbCache._protoPath, std::map<KERNEL_NS::LibString, PbCaheInfo *>())).first;

    auto &dict = iterDict->second;

    auto iter = dict.find(pbCache._messageName);
    if(iter != dict.end())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("bad pb cache message is already existed:%s\n old cache:%s"), pbCache.ToPbChacheString().c_str(), iter->second->ToPbChacheString().c_str());
        return;
    }

    auto newCache = PbCaheInfo::New_PbCaheInfo(pbCache);
    Int32 curMaxLine = GetMaxLine();
    ++curMaxLine;
    newCache->_line = curMaxLine;

    dict.insert(std::make_pair(pbCache._messageName, newCache));
    _lineRefContent.insert(std::make_pair(curMaxLine, newCache->ToPbChacheString()));
    _lineRefMessageInfo.insert(std::make_pair(curMaxLine, newCache));
}

KERNEL_NS::LibString PbCacheFileContent::GetMessageAnnotationValue(const KERNEL_NS::LibString &filePath, const KERNEL_NS::LibString &messageName, const KERNEL_NS::LibString &annotationKey) const
{
    KERNEL_NS::LibString value;
    auto iterDict = _protoPathRefMessageNameCacheInfo.find(filePath);
    if(iterDict == _protoPathRefMessageNameCacheInfo.end())
        return value;

    auto &dict = iterDict->second;
    auto iterMessage = dict.find(messageName);
    if(iterMessage == dict.end())
        return value;

    auto messageCache = iterMessage->second;
    return messageCache->GetAnnotationValue(annotationKey);
}

bool PbCacheFileContent::IsProtoFileCacheExists(const KERNEL_NS::LibString &filePath) const
{
    auto iter = _protoPathRefFileInfo.find(filePath);
    return iter != _protoPathRefFileInfo.end();
}

void PbCacheFileContent::UpdateProtoFileCache(const PbCacheFileInfo &cache)
{
    auto iter = _protoPathRefFileInfo.find(cache._protoPath);
    if(iter == _protoPathRefFileInfo.end())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("proto path not found cache:%s"), cache.ToPbChacheString().c_str());
        return;
    }

    auto line = iter->second->_line;
    *iter->second = cache;
    iter->second->_line = line;
    _lineRefContent[line] = iter->second->ToPbChacheString();
}

void PbCacheFileContent::AddProtoFileCache(const PbCacheFileInfo &cache)
{
    auto iter = _protoPathRefFileInfo.find(cache._protoPath);
    if(iter != _protoPathRefFileInfo.end())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("repeat proto cache:%s, old cache:%s"), cache.ToPbChacheString().c_str(), iter->second->ToPbChacheString().c_str());
        return;
    }

    Int32 curMaxLine = GetMaxLine();
    ++curMaxLine;
    auto newCache = PbCacheFileInfo::New_PbCacheFileInfo(cache);
    newCache->_line = curMaxLine;
    _protoPathRefFileInfo.insert(std::make_pair(newCache->_protoPath, newCache));
    _lineRefProtoFileInfo.insert(std::make_pair(newCache->_line, newCache));
    _lineRefContent.insert(std::make_pair(newCache->_line, newCache->ToPbChacheString()));

    // 需要添加空白行
    _lineRefContent.insert(std::make_pair(++curMaxLine, ""));
}

void PbCacheFileContent::RemoveInvalidFile(PbCacheFileInfo *protoFile)
{
    // 清理
    _lineRefContent.erase(protoFile->_line);
    _lineRefProtoFileInfo.erase(protoFile->_line);
    _protoPathRefFileInfo.erase(protoFile->_protoPath);

    auto iterMessageDict = _protoPathRefMessageNameCacheInfo.find(protoFile->_protoPath);
    if(iterMessageDict != _protoPathRefMessageNameCacheInfo.end())
    {
        auto &messageDict = iterMessageDict->second;
        for(auto kv : messageDict)
        {
            auto messageInfo = kv.second;
            _lineRefContent.erase(messageInfo->_line);
            _lineRefMessageInfo.erase(messageInfo->_line);
            PbCaheInfo::Delete_PbCaheInfo(messageInfo);
        }

        _protoPathRefMessageNameCacheInfo.erase(iterMessageDict);
    }

    PbCacheFileInfo::Delete_PbCacheFileInfo(protoFile);
}

void PbCacheFileContent::RemoveInvalidMessagesBy(const KERNEL_NS::LibString &protoPath, std::function<bool(const KERNEL_NS::LibString &messageName)> &&checkValidMessage)
{
    auto iterDict = _protoPathRefMessageNameCacheInfo.find(protoPath);
    if(iterDict == _protoPathRefMessageNameCacheInfo.end())
        return;

    auto &dict = iterDict->second;
    std::set<PbCaheInfo *> messageInfos;
    for(auto kv : dict)
    {
        if(!checkValidMessage(kv.first))
            messageInfos.insert(kv.second);
    }

    for(auto messageInfo : messageInfos)
    {
        dict.erase(messageInfo->_messageName);
        _lineRefMessageInfo.erase(messageInfo->_line);
        _lineRefContent.erase(messageInfo->_line);
        PbCaheInfo::Delete_PbCaheInfo(messageInfo);
    }
}


Int32 PbCacheFileContent::GetMaxLine() const
{
    Int32 curMaxLine = 0;
    if(!_lineRefContent.empty())
        curMaxLine = _lineRefContent.rbegin()->first;

    return curMaxLine;
}

SERVICE_END