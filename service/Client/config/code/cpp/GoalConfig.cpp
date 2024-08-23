// Generate by ConfigExporter, Dont modify it!!!
// file path:../../service/Client/config/xlsx/example.xlsx
// sheet name:目标|Goal

#include <pch.h>
#include <kernel/kernel.h>
#include <service_common/config/DataTypeHelper.h>
#include <openssl/md5.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/buffer.h>
#include <openssl/sha.h>
#include <openssl/bn.h>
#include "GoalConfig.h"

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(GoalConfig);
GoalConfig::GoalConfig()
:_id(0)
,_type(0)
,_isLucky(false)
{
}

bool GoalConfig::Parse(const KERNEL_NS::LibString &lineData)
{
// use json serialize text.
// format:column_{column id}_{data_len}:{json text}|...
// example:column_1_10:{json text}|...

    const Int32 fieldNum = 8;
    Int32 countFieldNum = 0;
    Int32 startPos = 0;

    {// _id
        auto pos = lineData.GetRaw().find("column_", startPos);
        if(pos == std::string::npos)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:Id, data format error: have no column_ prefix, lineData:%s, startPos:%d, countFieldNum:%d"), lineData.c_str(), startPos, countFieldNum);
            return false;
        }

       auto headerTailPos = lineData.GetRaw().find(":", pos);
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
          g_Log->Error(LOGFMT_OBJ_TAG("%s, assign fail field name:_id, data part:%s, errInfo:%s  line data:%s, pos:%d, headerTailPos:%d, dataEndPos:%d"), KERNEL_NS::RttiUtil::GetByObj(this).c_str(), dataPart.c_str(), errInfo.c_str(), lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), static_cast<Int32>(dataEndPos));
          return false;
      }

      startPos = static_cast<Int32>(dataEndPos);
      ++countFieldNum;
    }// _id

    {// _type
        auto pos = lineData.GetRaw().find("column_", startPos);
        if(pos == std::string::npos)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:Type, data format error: have no column_ prefix, lineData:%s, startPos:%d, countFieldNum:%d"), lineData.c_str(), startPos, countFieldNum);
            return false;
        }

       auto headerTailPos = lineData.GetRaw().find(":", pos);
       if(headerTailPos == std::string::npos)
       {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:Type, bad line data not find : symbol after column_ line data:%s, startPos:%d, countFieldNum:%d"), lineData.c_str(), startPos, countFieldNum);
            return false;
       }

       // parse data
       const KERNEL_NS::LibString headerInfo = lineData.GetRaw().substr(pos, headerTailPos - pos);
       const auto headParts = headerInfo.Split('_');
       if(headParts.empty())
       {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:Type, bad line data not find sep symbol:_ in header info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d,"),
            lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
            return false;
       }

       // check column id

       const auto &columnIdString = headParts[1];
       if(columnIdString.length() == 0)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:Type have no column id, bad line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d,"), lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }
       const UInt64 columnId = KERNEL_NS::StringUtil::StringToUInt64(columnIdString.c_str());
       if(columnId != 3)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:Type, fail: bad comumn id, columnId:%llu, real column id:3, please check if config data is old version, line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d"), columnId, lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }

       // data len
       const auto &lenInfo = headParts[2];
       if(lenInfo.length() == 0)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:Type fail: bad line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d"), lineData.c_str(), 
           static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }

       const Int32 dataLen = KERNEL_NS::StringUtil::StringToInt32(lenInfo.c_str());
       const auto dataEndPos = headerTailPos + static_cast<decltype(headerTailPos)>(dataLen);
       KERNEL_NS::LibString dataPart = lineData.GetRaw().substr(headerTailPos + 1, dataEndPos - headerTailPos);

      // parse data through
      KERNEL_NS::LibString errInfo;
      if(!SERVICE_COMMON_NS::DataTypeHelper::Assign(_type, dataPart, errInfo))
      {
          g_Log->Error(LOGFMT_OBJ_TAG("%s, assign fail field name:_type, data part:%s, errInfo:%s  line data:%s, pos:%d, headerTailPos:%d, dataEndPos:%d"), KERNEL_NS::RttiUtil::GetByObj(this).c_str(), dataPart.c_str(), errInfo.c_str(), lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), static_cast<Int32>(dataEndPos));
          return false;
      }

      startPos = static_cast<Int32>(dataEndPos);
      ++countFieldNum;
    }// _type

    {// _title
        auto pos = lineData.GetRaw().find("column_", startPos);
        if(pos == std::string::npos)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:title, data format error: have no column_ prefix, lineData:%s, startPos:%d, countFieldNum:%d"), lineData.c_str(), startPos, countFieldNum);
            return false;
        }

       auto headerTailPos = lineData.GetRaw().find(":", pos);
       if(headerTailPos == std::string::npos)
       {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:title, bad line data not find : symbol after column_ line data:%s, startPos:%d, countFieldNum:%d"), lineData.c_str(), startPos, countFieldNum);
            return false;
       }

       // parse data
       const KERNEL_NS::LibString headerInfo = lineData.GetRaw().substr(pos, headerTailPos - pos);
       const auto headParts = headerInfo.Split('_');
       if(headParts.empty())
       {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:title, bad line data not find sep symbol:_ in header info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d,"),
            lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
            return false;
       }

       // check column id

       const auto &columnIdString = headParts[1];
       if(columnIdString.length() == 0)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:title have no column id, bad line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d,"), lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }
       const UInt64 columnId = KERNEL_NS::StringUtil::StringToUInt64(columnIdString.c_str());
       if(columnId != 4)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:title, fail: bad comumn id, columnId:%llu, real column id:4, please check if config data is old version, line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d"), columnId, lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }

       // data len
       const auto &lenInfo = headParts[2];
       if(lenInfo.length() == 0)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:title fail: bad line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d"), lineData.c_str(), 
           static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }

       const Int32 dataLen = KERNEL_NS::StringUtil::StringToInt32(lenInfo.c_str());
       const auto dataEndPos = headerTailPos + static_cast<decltype(headerTailPos)>(dataLen);
       KERNEL_NS::LibString dataPart = lineData.GetRaw().substr(headerTailPos + 1, dataEndPos - headerTailPos);

      // parse data through
      KERNEL_NS::LibString errInfo;
      if(!SERVICE_COMMON_NS::DataTypeHelper::Assign(_title, dataPart, errInfo))
      {
          g_Log->Error(LOGFMT_OBJ_TAG("%s, assign fail field name:_title, data part:%s, errInfo:%s  line data:%s, pos:%d, headerTailPos:%d, dataEndPos:%d"), KERNEL_NS::RttiUtil::GetByObj(this).c_str(), dataPart.c_str(), errInfo.c_str(), lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), static_cast<Int32>(dataEndPos));
          return false;
      }

      startPos = static_cast<Int32>(dataEndPos);
      ++countFieldNum;
    }// _title

    {// _goal
        auto pos = lineData.GetRaw().find("column_", startPos);
        if(pos == std::string::npos)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:Goal, data format error: have no column_ prefix, lineData:%s, startPos:%d, countFieldNum:%d"), lineData.c_str(), startPos, countFieldNum);
            return false;
        }

       auto headerTailPos = lineData.GetRaw().find(":", pos);
       if(headerTailPos == std::string::npos)
       {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:Goal, bad line data not find : symbol after column_ line data:%s, startPos:%d, countFieldNum:%d"), lineData.c_str(), startPos, countFieldNum);
            return false;
       }

       // parse data
       const KERNEL_NS::LibString headerInfo = lineData.GetRaw().substr(pos, headerTailPos - pos);
       const auto headParts = headerInfo.Split('_');
       if(headParts.empty())
       {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:Goal, bad line data not find sep symbol:_ in header info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d,"),
            lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
            return false;
       }

       // check column id

       const auto &columnIdString = headParts[1];
       if(columnIdString.length() == 0)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:Goal have no column id, bad line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d,"), lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }
       const UInt64 columnId = KERNEL_NS::StringUtil::StringToUInt64(columnIdString.c_str());
       if(columnId != 5)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:Goal, fail: bad comumn id, columnId:%llu, real column id:5, please check if config data is old version, line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d"), columnId, lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }

       // data len
       const auto &lenInfo = headParts[2];
       if(lenInfo.length() == 0)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:Goal fail: bad line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d"), lineData.c_str(), 
           static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }

       const Int32 dataLen = KERNEL_NS::StringUtil::StringToInt32(lenInfo.c_str());
       const auto dataEndPos = headerTailPos + static_cast<decltype(headerTailPos)>(dataLen);
       KERNEL_NS::LibString dataPart = lineData.GetRaw().substr(headerTailPos + 1, dataEndPos - headerTailPos);

      // parse data through
      KERNEL_NS::LibString errInfo;
      if(!SERVICE_COMMON_NS::DataTypeHelper::Assign(_goal, dataPart, errInfo))
      {
          g_Log->Error(LOGFMT_OBJ_TAG("%s, assign fail field name:_goal, data part:%s, errInfo:%s  line data:%s, pos:%d, headerTailPos:%d, dataEndPos:%d"), KERNEL_NS::RttiUtil::GetByObj(this).c_str(), dataPart.c_str(), errInfo.c_str(), lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), static_cast<Int32>(dataEndPos));
          return false;
      }

      startPos = static_cast<Int32>(dataEndPos);
      ++countFieldNum;
    }// _goal

    {// _roleBuff
        auto pos = lineData.GetRaw().find("column_", startPos);
        if(pos == std::string::npos)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:RoleBuff, data format error: have no column_ prefix, lineData:%s, startPos:%d, countFieldNum:%d"), lineData.c_str(), startPos, countFieldNum);
            return false;
        }

       auto headerTailPos = lineData.GetRaw().find(":", pos);
       if(headerTailPos == std::string::npos)
       {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:RoleBuff, bad line data not find : symbol after column_ line data:%s, startPos:%d, countFieldNum:%d"), lineData.c_str(), startPos, countFieldNum);
            return false;
       }

       // parse data
       const KERNEL_NS::LibString headerInfo = lineData.GetRaw().substr(pos, headerTailPos - pos);
       const auto headParts = headerInfo.Split('_');
       if(headParts.empty())
       {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:RoleBuff, bad line data not find sep symbol:_ in header info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d,"),
            lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
            return false;
       }

       // check column id

       const auto &columnIdString = headParts[1];
       if(columnIdString.length() == 0)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:RoleBuff have no column id, bad line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d,"), lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }
       const UInt64 columnId = KERNEL_NS::StringUtil::StringToUInt64(columnIdString.c_str());
       if(columnId != 6)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:RoleBuff, fail: bad comumn id, columnId:%llu, real column id:6, please check if config data is old version, line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d"), columnId, lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }

       // data len
       const auto &lenInfo = headParts[2];
       if(lenInfo.length() == 0)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:RoleBuff fail: bad line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d"), lineData.c_str(), 
           static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }

       const Int32 dataLen = KERNEL_NS::StringUtil::StringToInt32(lenInfo.c_str());
       const auto dataEndPos = headerTailPos + static_cast<decltype(headerTailPos)>(dataLen);
       KERNEL_NS::LibString dataPart = lineData.GetRaw().substr(headerTailPos + 1, dataEndPos - headerTailPos);

      // parse data through
      KERNEL_NS::LibString errInfo;
      if(!SERVICE_COMMON_NS::DataTypeHelper::Assign(_roleBuff, dataPart, errInfo))
      {
          g_Log->Error(LOGFMT_OBJ_TAG("%s, assign fail field name:_roleBuff, data part:%s, errInfo:%s  line data:%s, pos:%d, headerTailPos:%d, dataEndPos:%d"), KERNEL_NS::RttiUtil::GetByObj(this).c_str(), dataPart.c_str(), errInfo.c_str(), lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), static_cast<Int32>(dataEndPos));
          return false;
      }

      startPos = static_cast<Int32>(dataEndPos);
      ++countFieldNum;
    }// _roleBuff

    {// _isLucky
        auto pos = lineData.GetRaw().find("column_", startPos);
        if(pos == std::string::npos)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:IsLucky, data format error: have no column_ prefix, lineData:%s, startPos:%d, countFieldNum:%d"), lineData.c_str(), startPos, countFieldNum);
            return false;
        }

       auto headerTailPos = lineData.GetRaw().find(":", pos);
       if(headerTailPos == std::string::npos)
       {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:IsLucky, bad line data not find : symbol after column_ line data:%s, startPos:%d, countFieldNum:%d"), lineData.c_str(), startPos, countFieldNum);
            return false;
       }

       // parse data
       const KERNEL_NS::LibString headerInfo = lineData.GetRaw().substr(pos, headerTailPos - pos);
       const auto headParts = headerInfo.Split('_');
       if(headParts.empty())
       {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:IsLucky, bad line data not find sep symbol:_ in header info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d,"),
            lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
            return false;
       }

       // check column id

       const auto &columnIdString = headParts[1];
       if(columnIdString.length() == 0)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:IsLucky have no column id, bad line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d,"), lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }
       const UInt64 columnId = KERNEL_NS::StringUtil::StringToUInt64(columnIdString.c_str());
       if(columnId != 7)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:IsLucky, fail: bad comumn id, columnId:%llu, real column id:7, please check if config data is old version, line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d"), columnId, lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }

       // data len
       const auto &lenInfo = headParts[2];
       if(lenInfo.length() == 0)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:IsLucky fail: bad line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d"), lineData.c_str(), 
           static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }

       const Int32 dataLen = KERNEL_NS::StringUtil::StringToInt32(lenInfo.c_str());
       const auto dataEndPos = headerTailPos + static_cast<decltype(headerTailPos)>(dataLen);
       KERNEL_NS::LibString dataPart = lineData.GetRaw().substr(headerTailPos + 1, dataEndPos - headerTailPos);

      // parse data through
      KERNEL_NS::LibString errInfo;
      if(!SERVICE_COMMON_NS::DataTypeHelper::Assign(_isLucky, dataPart, errInfo))
      {
          g_Log->Error(LOGFMT_OBJ_TAG("%s, assign fail field name:_isLucky, data part:%s, errInfo:%s  line data:%s, pos:%d, headerTailPos:%d, dataEndPos:%d"), KERNEL_NS::RttiUtil::GetByObj(this).c_str(), dataPart.c_str(), errInfo.c_str(), lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), static_cast<Int32>(dataEndPos));
          return false;
      }

      startPos = static_cast<Int32>(dataEndPos);
      ++countFieldNum;
    }// _isLucky

    {// _awards
        auto pos = lineData.GetRaw().find("column_", startPos);
        if(pos == std::string::npos)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:Awards, data format error: have no column_ prefix, lineData:%s, startPos:%d, countFieldNum:%d"), lineData.c_str(), startPos, countFieldNum);
            return false;
        }

       auto headerTailPos = lineData.GetRaw().find(":", pos);
       if(headerTailPos == std::string::npos)
       {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:Awards, bad line data not find : symbol after column_ line data:%s, startPos:%d, countFieldNum:%d"), lineData.c_str(), startPos, countFieldNum);
            return false;
       }

       // parse data
       const KERNEL_NS::LibString headerInfo = lineData.GetRaw().substr(pos, headerTailPos - pos);
       const auto headParts = headerInfo.Split('_');
       if(headParts.empty())
       {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:Awards, bad line data not find sep symbol:_ in header info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d,"),
            lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
            return false;
       }

       // check column id

       const auto &columnIdString = headParts[1];
       if(columnIdString.length() == 0)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:Awards have no column id, bad line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d,"), lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }
       const UInt64 columnId = KERNEL_NS::StringUtil::StringToUInt64(columnIdString.c_str());
       if(columnId != 8)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:Awards, fail: bad comumn id, columnId:%llu, real column id:8, please check if config data is old version, line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d"), columnId, lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }

       // data len
       const auto &lenInfo = headParts[2];
       if(lenInfo.length() == 0)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:Awards fail: bad line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d"), lineData.c_str(), 
           static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }

       const Int32 dataLen = KERNEL_NS::StringUtil::StringToInt32(lenInfo.c_str());
       const auto dataEndPos = headerTailPos + static_cast<decltype(headerTailPos)>(dataLen);
       KERNEL_NS::LibString dataPart = lineData.GetRaw().substr(headerTailPos + 1, dataEndPos - headerTailPos);

      // parse data through
      KERNEL_NS::LibString errInfo;
      if(!SERVICE_COMMON_NS::DataTypeHelper::Assign(_awards, dataPart, errInfo))
      {
          g_Log->Error(LOGFMT_OBJ_TAG("%s, assign fail field name:_awards, data part:%s, errInfo:%s  line data:%s, pos:%d, headerTailPos:%d, dataEndPos:%d"), KERNEL_NS::RttiUtil::GetByObj(this).c_str(), dataPart.c_str(), errInfo.c_str(), lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), static_cast<Int32>(dataEndPos));
          return false;
      }

      startPos = static_cast<Int32>(dataEndPos);
      ++countFieldNum;
    }// _awards

    {// _achieve
        auto pos = lineData.GetRaw().find("column_", startPos);
        if(pos == std::string::npos)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:Achieve, data format error: have no column_ prefix, lineData:%s, startPos:%d, countFieldNum:%d"), lineData.c_str(), startPos, countFieldNum);
            return false;
        }

       auto headerTailPos = lineData.GetRaw().find(":", pos);
       if(headerTailPos == std::string::npos)
       {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:Achieve, bad line data not find : symbol after column_ line data:%s, startPos:%d, countFieldNum:%d"), lineData.c_str(), startPos, countFieldNum);
            return false;
       }

       // parse data
       const KERNEL_NS::LibString headerInfo = lineData.GetRaw().substr(pos, headerTailPos - pos);
       const auto headParts = headerInfo.Split('_');
       if(headParts.empty())
       {
            g_Log->Error(LOGFMT_OBJ_TAG("parse field:Achieve, bad line data not find sep symbol:_ in header info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d,"),
            lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
            return false;
       }

       // check column id

       const auto &columnIdString = headParts[1];
       if(columnIdString.length() == 0)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:Achieve have no column id, bad line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d,"), lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }
       const UInt64 columnId = KERNEL_NS::StringUtil::StringToUInt64(columnIdString.c_str());
       if(columnId != 9)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:Achieve, fail: bad comumn id, columnId:%llu, real column id:9, please check if config data is old version, line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d"), columnId, lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }

       // data len
       const auto &lenInfo = headParts[2];
       if(lenInfo.length() == 0)
       {
           g_Log->Error(LOGFMT_OBJ_TAG("parse field:Achieve fail: bad line data header len info line data:%s, pos:%d, headerTailPos:%d, startPos:%d, countFieldNum:%d"), lineData.c_str(), 
           static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), startPos, countFieldNum);
           return false;
       }

       const Int32 dataLen = KERNEL_NS::StringUtil::StringToInt32(lenInfo.c_str());
       const auto dataEndPos = headerTailPos + static_cast<decltype(headerTailPos)>(dataLen);
       KERNEL_NS::LibString dataPart = lineData.GetRaw().substr(headerTailPos + 1, dataEndPos - headerTailPos);

      // parse data through
      KERNEL_NS::LibString errInfo;
      if(!SERVICE_COMMON_NS::DataTypeHelper::Assign(_achieve, dataPart, errInfo))
      {
          g_Log->Error(LOGFMT_OBJ_TAG("%s, assign fail field name:_achieve, data part:%s, errInfo:%s  line data:%s, pos:%d, headerTailPos:%d, dataEndPos:%d"), KERNEL_NS::RttiUtil::GetByObj(this).c_str(), dataPart.c_str(), errInfo.c_str(), lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), static_cast<Int32>(dataEndPos));
          return false;
      }

      startPos = static_cast<Int32>(dataEndPos);
      ++countFieldNum;
    }// _achieve

    if(countFieldNum != fieldNum)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("field num not enough countFieldNum:%d, need fieldNum:%d lineData:%s"), countFieldNum, fieldNum, lineData.c_str());
        return false;
    }

    return true;
}

void GoalConfig::Serialize(KERNEL_NS::LibString &lineData) const
{
    const Int32 fieldNum = 8;
    Int32 countFieldNum = 0;

    {// _id
        KERNEL_NS::LibString data;
        SERVICE_COMMON_NS::DataTypeHelper::ToString(_id, data);
        lineData.AppendFormat("column_2_%d:%s|", static_cast<Int32>(data.size()), data.c_str());
        ++countFieldNum;
    }// _id

    {// _type
        KERNEL_NS::LibString data;
        SERVICE_COMMON_NS::DataTypeHelper::ToString(_type, data);
        lineData.AppendFormat("column_3_%d:%s|", static_cast<Int32>(data.size()), data.c_str());
        ++countFieldNum;
    }// _type

    {// _title
        KERNEL_NS::LibString data;
        SERVICE_COMMON_NS::DataTypeHelper::ToString(_title, data);
        lineData.AppendFormat("column_4_%d:%s|", static_cast<Int32>(data.size()), data.c_str());
        ++countFieldNum;
    }// _title

    {// _goal
        KERNEL_NS::LibString data;
        SERVICE_COMMON_NS::DataTypeHelper::ToString(_goal, data);
        lineData.AppendFormat("column_5_%d:%s|", static_cast<Int32>(data.size()), data.c_str());
        ++countFieldNum;
    }// _goal

    {// _roleBuff
        KERNEL_NS::LibString data;
        SERVICE_COMMON_NS::DataTypeHelper::ToString(_roleBuff, data);
        lineData.AppendFormat("column_6_%d:%s|", static_cast<Int32>(data.size()), data.c_str());
        ++countFieldNum;
    }// _roleBuff

    {// _isLucky
        KERNEL_NS::LibString data;
        SERVICE_COMMON_NS::DataTypeHelper::ToString(_isLucky, data);
        lineData.AppendFormat("column_7_%d:%s|", static_cast<Int32>(data.size()), data.c_str());
        ++countFieldNum;
    }// _isLucky

    {// _awards
        KERNEL_NS::LibString data;
        SERVICE_COMMON_NS::DataTypeHelper::ToString(_awards, data);
        lineData.AppendFormat("column_8_%d:%s|", static_cast<Int32>(data.size()), data.c_str());
        ++countFieldNum;
    }// _awards

    {// _achieve
        KERNEL_NS::LibString data;
        SERVICE_COMMON_NS::DataTypeHelper::ToString(_achieve, data);
        lineData.AppendFormat("column_9_%d:%s|", static_cast<Int32>(data.size()), data.c_str());
        ++countFieldNum;
    }// _achieve


    if(countFieldNum != fieldNum)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("field num not enough countFieldNum:%d, need fieldNum:%d"), countFieldNum, fieldNum);
    }
}

POOL_CREATE_OBJ_DEFAULT_IMPL(GoalConfigMgr);

const std::vector<GoalConfig *> GoalConfigMgr::s_empty;

GoalConfigMgr::GoalConfigMgr()
:SERVICE_COMMON_NS::IConfigMgr(KERNEL_NS::RttiUtil::GetTypeId<GoalConfigMgr>())
{
}

GoalConfigMgr::~GoalConfigMgr()
{
    _Clear();
}

void GoalConfigMgr::Release()
{
    GoalConfigMgr::DeleteByAdapter_GoalConfigMgr(GoalConfigMgrFactory::_buildType.V, this);
}

void GoalConfigMgr::Clear()
{
    _Clear();
}

KERNEL_NS::LibString GoalConfigMgr::ToString() const
{
    return GetObjName();
}

Int32 GoalConfigMgr::Load()
{
    KERNEL_NS::SmartPtr<std::vector<GoalConfig *>, KERNEL_NS::AutoDelMethods::CustomDelete> configs = new std::vector<GoalConfig *>;
    configs.SetClosureDelegate([](void *p){
        std::vector<GoalConfig *> *ptr = reinterpret_cast<std::vector<GoalConfig *> *>(p);
        KERNEL_NS::ContainerUtil::DelContainer(*ptr, [](GoalConfig *ptr){
            GoalConfig::Delete_GoalConfig(ptr);
        });
        delete ptr;
    });

    const auto basePath = GetLoader()->GetBasePath();
    const auto wholePath = basePath + "/" + "GoalConfig.data";
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
    if(!KERNEL_NS::LibDigest::MakeMd5Init(&ctx))
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
                KERNEL_NS::LibDigest::MakeMd5Clean(&ctx);
                return Status::Failed;
            }
            ++line;
            if(!KERNEL_NS::LibDigest::MakeMd5Continue(&ctx, lineData.data(), static_cast<UInt64>(lineData.size())))
            {
                g_Log->Error(LOGFMT_OBJ_TAG("MakeMd5Continue fail wholePath:%s, line:%d, lineData:%s"), wholePath.c_str(), line, lineData.c_str());
                KERNEL_NS::LibDigest::MakeMd5Clean(&ctx);
                return Status::Failed;
            }

            // first line data is field names
            if(line == 1)
            {
                if(lineData != "Id|Type|title|Goal|RoleBuff|IsLucky|Awards|Achieve|")
                {
                    g_Log->Error(LOGFMT_OBJ_TAG("current data not match this config data wholePath:%s, current data columns:%s, this config columns:Id|Type|title|Goal|RoleBuff|IsLucky|Awards|Achieve|"), wholePath.c_str(), lineData.c_str());
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
            KERNEL_NS::LibDigest::MakeMd5Clean(&ctx);
            return Status::Failed;
        }

        if(readBytes == 0)
            break;

        ++line;

        if(!KERNEL_NS::LibDigest::MakeMd5Continue(&ctx, lineData.data(), static_cast<UInt64>(lineData.size())))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("MakeMd5Continue fail wholePath:%s, line:%d, lineData:%s"), wholePath.c_str(), line, lineData.c_str());
            KERNEL_NS::LibDigest::MakeMd5Clean(&ctx);
            return Status::Failed;
        }

        KERNEL_NS::SmartPtr<GoalConfig, KERNEL_NS::AutoDelMethods::CustomDelete> config = GoalConfig::New_GoalConfig();
        config.SetClosureDelegate([](void *p){
            auto ptr = reinterpret_cast<GoalConfig *>(p);
            GoalConfig::Delete_GoalConfig(ptr);
        });

        if(!config->Parse(lineData))
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("parse GoalConfig fail data path:%s line:%d, lineData:%s"), wholePath.c_str(), line, lineData.c_str());
            return Status::Failed;
        }

        // check unique
        if(unique_ids.find(config->_id) != unique_ids.end())
        {
            g_Log->Warn(LOGFMT_OBJ_TAG("duplicate Id:%s data path:%s line:%d, lineData:%s"), (KERNEL_NS::LibString() << config->_id).c_str(), wholePath.c_str(), line, lineData.c_str());
            return Status::Failed;
        }

        unique_ids.insert(config->_id);

        configs->push_back(config.pop());
    }// while(true)

    KERNEL_NS::LibString dataMd5;
    if(!KERNEL_NS::LibDigest::MakeMd5Final(&ctx, dataMd5))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("MakeMd5Final fail wholePath:%s"), wholePath.c_str());
        KERNEL_NS::LibDigest::MakeMd5Clean(&ctx);
        return Status::Failed;
    }
    KERNEL_NS::LibDigest::MakeMd5Clean(&ctx);

    _dataMd5 = KERNEL_NS::LibBase64::Encode(dataMd5);
    _configs.swap(*configs.AsSelf());

    _idRefConfig.clear();
    _typeRefConfigs.clear();

    for(auto config : _configs)
    {
        // key:Id 
        {
            _idRefConfig.insert(std::make_pair(config->_id, config));
        }

        // key:Type 
        {
            auto iterConfigs = _typeRefConfigs.find(config->_type);
            if(iterConfigs == _typeRefConfigs.end())
                iterConfigs = _typeRefConfigs.insert(std::make_pair(config->_type, std::vector<GoalConfig *>())).first;
            iterConfigs->second.push_back(config);
        }

    }

    return Status::Success;
}

Int32 GoalConfigMgr::Reload()
{
    return Load();
}

const KERNEL_NS::LibString &GoalConfigMgr::GetConfigDataMd5() const
{
    return _dataMd5;
}

void GoalConfigMgr::_OnClose()
{
    _Clear();
}

void GoalConfigMgr::_Clear()
{
    _dataMd5.clear();
    _idRefConfig.clear();

    _typeRefConfigs.clear();


    KERNEL_NS::ContainerUtil::DelContainer(_configs, [](GoalConfig *ptr){
        GoalConfig::Delete_GoalConfig(ptr);
    });
}

Int64 GoalConfigMgr::_ReadConfigData(FILE &fp, KERNEL_NS::LibString &configData, Int32 totalLine, Int32 curLine) const
{
    // read all column data as one line data, it need check if lack. error data format return -1, return zeror if have no data, return > 0 as readBytes

    Int64 readBytes = 0;
    KERNEL_NS::LibString content;
    std::set<Int32> needFieldIds = {2, 3, 4, 5, 6, 7, 8, 9};
    const Int32 fieldNum = 8;
    Int32 count = 0;
    while(true)
    {
        Int64 bytesOnce = static_cast<Int64>(KERNEL_NS::FileUtil::ReadFile(fp, content, 1));
        if(bytesOnce == 0)
            break;

        readBytes += bytesOnce;
        if(content.Contain(":"))
        {
            const auto symbolPos = content.GetRaw().find(":", 0);
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
