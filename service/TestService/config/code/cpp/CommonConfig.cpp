// Generate by ConfigExporter, Dont modify it!!!
// file path:../../service/TestService/config/xlsx/公共参数.xlsx
// sheet name:公共参数|Common

#include <pch.h>
#include "CommonConfig.h"

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(CommonConfig);
CommonConfig::CommonConfig()
:_id(0)
,_value(0)
,_int64Value(0)
{
}

bool CommonConfig::Parse(const KERNEL_NS::LibString &lineData)
{
// use json serialize text.
// format:column_{column id}_{data_len}:{json text}|...
// example:column_1_10:{json text}|...

    const Int32 fieldNum = 4;
    Int32 countFieldNum = 0;
    Int32 startPos = 0;

    {// _id
        auto pos = lineData.GetRaw().find_first_of("column_", startPos);
        if(pos == std::string::npos)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:Id, data format error: have no column_ prefix, lineData:%s, startPos:%d, countFieldNum:%d"), lineData.c_str(), startPos, countFieldNum);
            return false;
        }

       auto headerTailPos = lineData.GetRaw().find_first_of(":", pos);
       if(headerTailPos == std::string::npos)
       {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:Id, bad line data not find : symbol after column_ line data:%s, startPos:%d, countFieldNum:%d"), lineData.c_str(), startPos, countFieldNum);
            return false;
       }

       // parse data
       const KERNEL_NS::LibString headerInfo = lineData.GetRaw().substr(pos, headerTailPos - pos);
       const auto headParts = headerInfo.Split('_');
       if(headParts.empty())
       {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:Id, bad line data not find sep symbol:_ in header info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d,"),
            lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
            return false;
       }

       // check column id

       const auto &columnIdString = headParts[1];
       if(columnIdString.length() == 0)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:Id have no column id, bad line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d,"), lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }
       const UInt64 columnId = KERNEL_NS::StringUtil::StringToUInt64(columnIdString.c_str());
       if(columnId != 2)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:Id, fail: bad comumn id, columnId:%llu, real column id:2, please check if config data is old version, line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d"), columnId, lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }

       // data len
       const auto &lenInfo = headParts[2];
       if(lenInfo.length() == 0)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:Id fail: bad line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d"), lineData.c_str(), 
           static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }

       const Int32 dataLen = KERNEL_NS::StringUtil::StringToInt32(lenInfo.c_str());
       const auto dataEndPos = headerTailPos + static_cast<decltype(headerTailPos)>(dataLen);
       KERNEL_NS::LibString dataPart = lineData.GetRaw().substr(headerTailPos + 1, dataEndPos - headerTailPos);

      // parse data through
      KERNEL_NS::LibString errInfo;
      if(!SERVICE_COMMON_NS::DataTypeHelper::Assign(_id, dataPart, errInfo))
      {
          g_Log->Error(LOGFMT_OBJ_TAG("%s, assign fail field name:_id, data part:%s, errInfo:%s  line data:%s, pos:%d, headerTailPos:%d, dataEndPos:%d"), KERNEL_NS::RttiUtil::GetByObj(this), dataPart.c_str(), errInfo.c_str(), lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), static_cast<Int32>(dataEndPos));
          return false;
      }

      startPos = static_cast<Int32>(dataEndPos);
      ++countFieldNum;
    }// _id

    {// _value
        auto pos = lineData.GetRaw().find_first_of("column_", startPos);
        if(pos == std::string::npos)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:Value, data format error: have no column_ prefix, lineData:%s, startPos:%d, countFieldNum:%d"), lineData.c_str(), startPos, countFieldNum);
            return false;
        }

       auto headerTailPos = lineData.GetRaw().find_first_of(":", pos);
       if(headerTailPos == std::string::npos)
       {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:Value, bad line data not find : symbol after column_ line data:%s, startPos:%d, countFieldNum:%d"), lineData.c_str(), startPos, countFieldNum);
            return false;
       }

       // parse data
       const KERNEL_NS::LibString headerInfo = lineData.GetRaw().substr(pos, headerTailPos - pos);
       const auto headParts = headerInfo.Split('_');
       if(headParts.empty())
       {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:Value, bad line data not find sep symbol:_ in header info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d,"),
            lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
            return false;
       }

       // check column id

       const auto &columnIdString = headParts[1];
       if(columnIdString.length() == 0)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:Value have no column id, bad line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d,"), lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }
       const UInt64 columnId = KERNEL_NS::StringUtil::StringToUInt64(columnIdString.c_str());
       if(columnId != 3)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:Value, fail: bad comumn id, columnId:%llu, real column id:3, please check if config data is old version, line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d"), columnId, lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }

       // data len
       const auto &lenInfo = headParts[2];
       if(lenInfo.length() == 0)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:Value fail: bad line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d"), lineData.c_str(), 
           static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }

       const Int32 dataLen = KERNEL_NS::StringUtil::StringToInt32(lenInfo.c_str());
       const auto dataEndPos = headerTailPos + static_cast<decltype(headerTailPos)>(dataLen);
       KERNEL_NS::LibString dataPart = lineData.GetRaw().substr(headerTailPos + 1, dataEndPos - headerTailPos);

      // parse data through
      KERNEL_NS::LibString errInfo;
      if(!SERVICE_COMMON_NS::DataTypeHelper::Assign(_value, dataPart, errInfo))
      {
          g_Log->Error(LOGFMT_OBJ_TAG("%s, assign fail field name:_value, data part:%s, errInfo:%s  line data:%s, pos:%d, headerTailPos:%d, dataEndPos:%d"), KERNEL_NS::RttiUtil::GetByObj(this), dataPart.c_str(), errInfo.c_str(), lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), static_cast<Int32>(dataEndPos));
          return false;
      }

      startPos = static_cast<Int32>(dataEndPos);
      ++countFieldNum;
    }// _value

    {// _int64Value
        auto pos = lineData.GetRaw().find_first_of("column_", startPos);
        if(pos == std::string::npos)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:Int64Value, data format error: have no column_ prefix, lineData:%s, startPos:%d, countFieldNum:%d"), lineData.c_str(), startPos, countFieldNum);
            return false;
        }

       auto headerTailPos = lineData.GetRaw().find_first_of(":", pos);
       if(headerTailPos == std::string::npos)
       {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:Int64Value, bad line data not find : symbol after column_ line data:%s, startPos:%d, countFieldNum:%d"), lineData.c_str(), startPos, countFieldNum);
            return false;
       }

       // parse data
       const KERNEL_NS::LibString headerInfo = lineData.GetRaw().substr(pos, headerTailPos - pos);
       const auto headParts = headerInfo.Split('_');
       if(headParts.empty())
       {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:Int64Value, bad line data not find sep symbol:_ in header info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d,"),
            lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
            return false;
       }

       // check column id

       const auto &columnIdString = headParts[1];
       if(columnIdString.length() == 0)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:Int64Value have no column id, bad line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d,"), lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }
       const UInt64 columnId = KERNEL_NS::StringUtil::StringToUInt64(columnIdString.c_str());
       if(columnId != 4)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:Int64Value, fail: bad comumn id, columnId:%llu, real column id:4, please check if config data is old version, line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d"), columnId, lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }

       // data len
       const auto &lenInfo = headParts[2];
       if(lenInfo.length() == 0)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:Int64Value fail: bad line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d"), lineData.c_str(), 
           static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }

       const Int32 dataLen = KERNEL_NS::StringUtil::StringToInt32(lenInfo.c_str());
       const auto dataEndPos = headerTailPos + static_cast<decltype(headerTailPos)>(dataLen);
       KERNEL_NS::LibString dataPart = lineData.GetRaw().substr(headerTailPos + 1, dataEndPos - headerTailPos);

      // parse data through
      KERNEL_NS::LibString errInfo;
      if(!SERVICE_COMMON_NS::DataTypeHelper::Assign(_int64Value, dataPart, errInfo))
      {
          g_Log->Error(LOGFMT_OBJ_TAG("%s, assign fail field name:_int64Value, data part:%s, errInfo:%s  line data:%s, pos:%d, headerTailPos:%d, dataEndPos:%d"), KERNEL_NS::RttiUtil::GetByObj(this), dataPart.c_str(), errInfo.c_str(), lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), static_cast<Int32>(dataEndPos));
          return false;
      }

      startPos = static_cast<Int32>(dataEndPos);
      ++countFieldNum;
    }// _int64Value

    {// _stringValue
        auto pos = lineData.GetRaw().find_first_of("column_", startPos);
        if(pos == std::string::npos)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:StringValue, data format error: have no column_ prefix, lineData:%s, startPos:%d, countFieldNum:%d"), lineData.c_str(), startPos, countFieldNum);
            return false;
        }

       auto headerTailPos = lineData.GetRaw().find_first_of(":", pos);
       if(headerTailPos == std::string::npos)
       {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:StringValue, bad line data not find : symbol after column_ line data:%s, startPos:%d, countFieldNum:%d"), lineData.c_str(), startPos, countFieldNum);
            return false;
       }

       // parse data
       const KERNEL_NS::LibString headerInfo = lineData.GetRaw().substr(pos, headerTailPos - pos);
       const auto headParts = headerInfo.Split('_');
       if(headParts.empty())
       {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:StringValue, bad line data not find sep symbol:_ in header info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d,"),
            lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
            return false;
       }

       // check column id

       const auto &columnIdString = headParts[1];
       if(columnIdString.length() == 0)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:StringValue have no column id, bad line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d,"), lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }
       const UInt64 columnId = KERNEL_NS::StringUtil::StringToUInt64(columnIdString.c_str());
       if(columnId != 5)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:StringValue, fail: bad comumn id, columnId:%llu, real column id:5, please check if config data is old version, line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d"), columnId, lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }

       // data len
       const auto &lenInfo = headParts[2];
       if(lenInfo.length() == 0)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:StringValue fail: bad line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d"), lineData.c_str(), 
           static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }

       const Int32 dataLen = KERNEL_NS::StringUtil::StringToInt32(lenInfo.c_str());
       const auto dataEndPos = headerTailPos + static_cast<decltype(headerTailPos)>(dataLen);
       KERNEL_NS::LibString dataPart = lineData.GetRaw().substr(headerTailPos + 1, dataEndPos - headerTailPos);

      // parse data through
      KERNEL_NS::LibString errInfo;
      if(!SERVICE_COMMON_NS::DataTypeHelper::Assign(_stringValue, dataPart, errInfo))
      {
          g_Log->Error(LOGFMT_OBJ_TAG("%s, assign fail field name:_stringValue, data part:%s, errInfo:%s  line data:%s, pos:%d, headerTailPos:%d, dataEndPos:%d"), KERNEL_NS::RttiUtil::GetByObj(this), dataPart.c_str(), errInfo.c_str(), lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), static_cast<Int32>(dataEndPos));
          return false;
      }

      startPos = static_cast<Int32>(dataEndPos);
      ++countFieldNum;
    }// _stringValue

    if(countFieldNum != fieldNum)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("field num not enough countFieldNum:%d, need fieldNum:%d lineData:%s"), countFieldNum, fieldNum, lineData.c_str());
        return false;
    }

    return true;
}

void CommonConfig::Serialize(KERNEL_NS::LibString &lineData) const
{
    const Int32 fieldNum = 4;
    Int32 countFieldNum = 0;

    {// _id
        KERNEL_NS::LibString data;
        SERVICE_COMMON_NS::DataTypeHelper::ToString(_id, data);
        lineData.AppendFormat("column_2_%d:%s|", static_cast<Int32>(data.size()), data.c_str());
        ++countFieldNum;
    }// _id

    {// _value
        KERNEL_NS::LibString data;
        SERVICE_COMMON_NS::DataTypeHelper::ToString(_value, data);
        lineData.AppendFormat("column_3_%d:%s|", static_cast<Int32>(data.size()), data.c_str());
        ++countFieldNum;
    }// _value

    {// _int64Value
        KERNEL_NS::LibString data;
        SERVICE_COMMON_NS::DataTypeHelper::ToString(_int64Value, data);
        lineData.AppendFormat("column_4_%d:%s|", static_cast<Int32>(data.size()), data.c_str());
        ++countFieldNum;
    }// _int64Value

    {// _stringValue
        KERNEL_NS::LibString data;
        SERVICE_COMMON_NS::DataTypeHelper::ToString(_stringValue, data);
        lineData.AppendFormat("column_5_%d:%s|", static_cast<Int32>(data.size()), data.c_str());
        ++countFieldNum;
    }// _stringValue


    if(countFieldNum != fieldNum)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("field num not enough countFieldNum:%d, need fieldNum:%d"), countFieldNum, fieldNum);
    }
}

POOL_CREATE_OBJ_DEFAULT_IMPL(CommonConfigMgr);

const std::vector<CommonConfig *> CommonConfigMgr::s_empty;

CommonConfigMgr::CommonConfigMgr()
{
}

CommonConfigMgr::~CommonConfigMgr()
{
    _Clear();
}

void CommonConfigMgr::Release()
{
    CommonConfigMgr::DeleteByAdapter_CommonConfigMgr(CommonConfigMgrFactory::_buildType.V, this);
}

void CommonConfigMgr::Clear()
{
    _Clear();
}

KERNEL_NS::LibString CommonConfigMgr::ToString() const
{
    return GetObjName();
}

Int32 CommonConfigMgr::Load()
{
    KERNEL_NS::SmartPtr<std::vector<CommonConfig *>, KERNEL_NS::AutoDelMethods::CustomDelete> configs = new std::vector<CommonConfig *>;
    configs.SetClosureDelegate([](void *p){
        std::vector<CommonConfig *> *ptr = reinterpret_cast<std::vector<CommonConfig *> *>(p);
        KERNEL_NS::ContainerUtil::DelContainer(*ptr, [](CommonConfig *ptr){
            CommonConfig::Delete_CommonConfig(ptr);
        });
        delete ptr;
    });

    const auto basePath = GetLoader()->GetBasePath();
    const auto wholePath = basePath + "/" + "CommonConfig.data";
    KERNEL_NS::SmartPtr<FILE, KERNEL_NS::AutoDelMethods::CustomDelete> fp = KERNEL_NS::FileUtil::OpenFile(wholePath.c_str(), false, "rb");
    if(!fp)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("data file not found wholePath:%s"), wholePath.c_str());
        return Status::Failed;
    }

    fp.SetClosureDelegate([](void *p){
        auto fpPtr = reinterpret_cast<FILE *>(p);
        KERNEL_NS::FileUtil::CloseFile(*fpPtr);
    });

    Int32 line = 0;
    MD5_CTX ctx;
    if(!KERNEL_NS::LibDigest::MakeMd5Init(ctx))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("make md5 init fail wholePath:%s"), wholePath.c_str());
        return Status::Failed;
    }

    // 去重
    std::set<Int32> unique_ids;
    Int32 totalLine = 0;

    while(true)
    {
        KERNEL_NS::LibString lineData;
        if(line < 2)
        {
            Int64 retBytes = KERNEL_NS::FileUtil::ReadUtf8OneLine(*fp, lineData);
            if(retBytes == 0)
            {
                g_Log->Error(LOGFMT_OBJ_TAG("ReadUtf8OneLine fail wholePath:%s, line:%d, lineData:%s"), wholePath.c_str(), line, lineData.c_str());
                KERNEL_NS::LibDigest::MakeMd5Clean(ctx);
                return Status::Failed;
            }
            ++line;
            if(!KERNEL_NS::LibDigest::MakeMd5Continue(ctx, lineData.data(), static_cast<UInt64>(lineData.size())))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("MakeMd5Continue fail wholePath:%s, line:%d, lineData:%s"), wholePath.c_str(), line, lineData.c_str());
                KERNEL_NS::LibDigest::MakeMd5Clean(ctx);
                return Status::Failed;
            }

            // first line data is field names
            if(line == 1)
            {
                if(lineData != "Id|Value|Int64Value|StringValue|")
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("current data not match this config data wholePath:%s, current data columns:%s, this config columns:Id|Value|Int64Value|StringValue|"), wholePath.c_str(), lineData.c_str());
                    return Status::Failed;
                }
            }
            // total line data
            if(line == 2)
            {
                lineData.strip();
                totalLine = KERNEL_NS::StringUtil::StringToInt32(lineData.c_str());
            }

            continue;
        }

        // have no data
        if(totalLine <= 2)
            break;

        if(totalLine <= line)
          break;

        Int64 readBytes = _ReadConfigData(*fp, lineData, totalLine, line + 1);
        if(readBytes < 0)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("Read a line config data fail wholePath:%s, line:%d, "), wholePath.c_str(), line);
            g_Log->Error2(LOGFMT_OBJ_TAG_NO_FMT(), KERNEL_NS::LibString("lineData:"), lineData);
            KERNEL_NS::LibDigest::MakeMd5Clean(ctx);
            return Status::Failed;
        }

        if(readBytes == 0)
            break;

        ++line;

        if(!KERNEL_NS::LibDigest::MakeMd5Continue(ctx, lineData.data(), static_cast<UInt64>(lineData.size())))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("MakeMd5Continue fail wholePath:%s, line:%d, lineData:%s"), wholePath.c_str(), line, lineData.c_str());
            KERNEL_NS::LibDigest::MakeMd5Clean(ctx);
            return Status::Failed;
        }

        KERNEL_NS::SmartPtr<CommonConfig, KERNEL_NS::AutoDelMethods::CustomDelete> config = CommonConfig::New_CommonConfig();
        config.SetClosureDelegate([](void *p){
            auto ptr = reinterpret_cast<CommonConfig *>(p);
            CommonConfig::Delete_CommonConfig(ptr);
        });

        if(!config->Parse(lineData))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("parse CommonConfig fail data path:%s line:%d, lineData:%s"), wholePath.c_str(), line, lineData.c_str());
            return Status::Failed;
        }

        // check unique
        if(unique_ids.find(config->_id) != unique_ids.end())
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("duplicate Id:%d data path:%s line:%d, lineData:%s"), config->_id, wholePath.c_str(), line, lineData.c_str());
            return Status::Failed;
        }

        unique_ids.insert(config->_id);

        configs->push_back(config.pop());
    }// while(true)

    KERNEL_NS::LibString dataMd5;
    if(!KERNEL_NS::LibDigest::MakeMd5Final(ctx, dataMd5))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("MakeMd5Final fail wholePath:%s"), wholePath.c_str());
        KERNEL_NS::LibDigest::MakeMd5Clean(ctx);
        return Status::Failed;
    }
    KERNEL_NS::LibDigest::MakeMd5Clean(ctx);

    _dataMd5 = KERNEL_NS::LibBase64::Encode(dataMd5);
    _configs.swap(*configs.AsSelf());

    _idRefConfig.clear();

    for(auto config : _configs)
    {
        // key:Id 
        {
            _idRefConfig.insert(std::make_pair(config->_id, config));
        }

    }

    return Status::Success;
}

Int32 CommonConfigMgr::Reload()
{
    return Load();
}

const KERNEL_NS::LibString &CommonConfigMgr::GetConfigDataMd5() const
{
    return _dataMd5;
}

void CommonConfigMgr::_OnClose()
{
    _Clear();
}

void CommonConfigMgr::_Clear()
{
    _dataMd5.clear();
    _idRefConfig.clear();


    KERNEL_NS::ContainerUtil::DelContainer(_configs, [](CommonConfig *ptr){
        CommonConfig::Delete_CommonConfig(ptr);
    });
}

Int64 CommonConfigMgr::_ReadConfigData(FILE &fp, KERNEL_NS::LibString &configData, Int32 totalLine, Int32 curLine) const
{
    // read all column data as one line data, it need check if lack. error data format return -1, return zeror if have no data, return > 0 as readBytes

    Int64 readBytes = 0;
    KERNEL_NS::LibString content;
    std::set<Int32> needFieldIds = {2, 3, 4, 5};
    const Int32 fieldNum = 4;
    Int32 count = 0;
    while(true)
    {
        Int64 bytesOnce = static_cast<Int64>(KERNEL_NS::FileUtil::ReadFile(fp, content, 1));
        if(bytesOnce == 0)
            break;

        readBytes += bytesOnce;
        if(content.Contain(":"))
        {
            const auto symbolPos = content.GetRaw().find_first_of(":", 0);
            const KERNEL_NS::LibString fieldHeader = content.GetRaw().substr(0, symbolPos);
            const auto &headerCache = fieldHeader.strip();
            const auto &headerParts = headerCache.Split('_');
            if(headerParts.size() < 3)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("column field header format error"));
                g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), KERNEL_NS::LibString("headerCache:"), headerCache, KERNEL_NS::LibString(", content:"), content);
                return -1;
            }

            const Int32 fieldColumnId = KERNEL_NS::StringUtil::StringToInt32(headerParts[1].c_str());
            const Int64 fieldDataLen = KERNEL_NS::StringUtil::StringToInt64(headerParts[2].c_str());

            bytesOnce = static_cast<Int64>(KERNEL_NS::FileUtil::ReadFile(fp, content, fieldDataLen));
            if(bytesOnce != fieldDataLen)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("column data error:"));
                g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), KERNEL_NS::LibString("headerCache:"), headerCache, KERNEL_NS::LibString(", content:"), content, KERNEL_NS::LibString(", fieldDataLen:"), fieldDataLen, KERNEL_NS::LibString(", real len:"), bytesOnce, KERNEL_NS::LibString(", not enough."));
                return -1;
            }

            readBytes += bytesOnce;
            configData += content;
            content.clear();
            needFieldIds.erase(fieldColumnId);
            ++count;

            if(((count != fieldNum) && needFieldIds.empty()) ||
                ((count == fieldNum) && !needFieldIds.empty()))
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("column data error: field maybe changed count:%d, need fieldNum:%d column fieldIds not empty left:%d"), count, fieldNum, static_cast<Int32>(needFieldIds.size()));
                g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), KERNEL_NS::LibString("configData:"), configData);
                return -1;
            }

            // read final line data all
            if(curLine == totalLine && ((count == fieldNum) && needFieldIds.empty()))
            {
                bytesOnce = static_cast<Int64>(KERNEL_NS::FileUtil::ReadFile(fp, content));
                readBytes += bytesOnce;
                configData += content;
                content.clear();
            };

            if((count == fieldNum) && needFieldIds.empty())
            {
                return readBytes;
            }
        }
    }
    g_Log->Warn(LOGFMT_OBJ_TAG("column data error: field maybe changed count:%d, need fieldNum:%d column fieldIds not empty left:%d"), count, fieldNum, static_cast<Int32>(needFieldIds.size()));
    g_Log->Warn2(LOGFMT_OBJ_TAG_NO_FMT(), KERNEL_NS::LibString("configData:"), configData);

    return -1;
}

SERVICE_END