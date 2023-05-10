# 配置设计

* 配置代码模版设计

  * cpp 数据文档结构

    * 第一行是列的顺序串，需要对其进行校验避免解析数据时候串了
    * 每一行数据都是文本数据
  
    * 每个单元格的数据前缀：column_id_datalen:xxx|
  
    * column_前缀，列id，数据长度:数据|column_前缀,....
  
    * ```
      column_1_10:1010105555|column_2_10:aaaaaaaaaa|...
      ```
  
      
  
  * Cpp
  
  * ```
    // Generate by ConfigExporter, Dont modify it!!!
    // config:xlsx
    //
    //	file path:
    //
    // sheet name:%s
    // 
    
    #ifndef __CONFIG_EXAMPLE_CONFIG_H__
    #define __CONFIG_EXAMPLE_CONFIG_H__
    
    #pragma once
    
    #include <kernel/kernel.h>
    #include <service_common/config/config.h>
    
    SERVICE_BEGIN

    class ExampleConfig
    {
    	POOL_CREATE_OBJ_DEFAULT(ExampleConfig);
    	
    public:
    	ExampleConfig();
    	~ExampleConfig();
    	
    	bool Parse(const KERNEL_NS::LibString &lineData);
    	void Serialize(KERNEL_NS::LibString &lineData) const;
    	
    	Int32 _Id;	// desc:id
    	Int32 _Type;	// 类型1,2,3
    };
    
    
    class ExampleConfigMgr : public SERVICE_COMMON_NS::IConfigMgr
    {
    	POOL_CREATE_OBJ_DEFAULT_P1(IConfigMgr, ExampleConfigMgr);
    	
    	ExampleConfigMgr();
    	~ExampleConfigMgr();
    	
      virtual void Release();
    	virtual void Clear() override;
    	virtual KERNEL_NS::LibString ToString() const override;
    	virtual Int32 Load() override;
    	virtual Int32 Reload() override;
        virtual const KERNEL_NS::LibString & GetConfigDataMd5() const override;
    	
    	const std::vector<ExampleConfig *> &GetAllConfigs() const;
    	const std::vector<ExampleConfig *> &GetAllIdConfigs() const;
      const ExampleConfig *GetConfigById(Int32 key) const;
    	
    private:
    	virtual void _OnClose() override;
    	void _Clear();
      Int64 _ReadConfigData(FILE &fp, KERNEL_NS::LibString &configData) const;
  	
    private:
    	std::vector<ExampleConfig *> _configs;

      std::unordered_map<Int32, ExampleConfig *> _idRefConfig;
    
    	KERNEL_NS::LibString _dataMd5;		// 数据的md5 加载的时候计算md5
    };
    
    class ExampleConfigMgrFactory : public KERNEL_NS::CompFactory
    {
    public:
    	static constexpr _Build::MT _buildType{};
    	
  	static KERNEL_NS::CompFactory *FactoryCreate()
    	{
    		return KERNEL_NS::ObjPoolWrap<ExampleConfigMgrFactory>::NewByAdapter(_buildType.V);
    	}
    	
        virtual void Release()
        {
        KERNEL_NS::ObjPoolWrap<ExampleConfigMgrFactory>::DeleteByAdapter(_buildType.V, this);
        }
    
        virtual CompObject *Create() const
        {
        	return ExampleConfigMgr::NewByAdapter_ExampleConfigMgr(_buildType.V);
        }
    };
    
    SERVICE_END
    
    #endif
    
    cpp:
    
    #include <pch.h>
    #include "ExampleConfig.h"
    
    POOL_CREATE_OBJ_DEFAULT_IMPL(ExampleConfig);
    
    ExampleConfig::ExampleConfig()
    :_Id(0)
    ,_Type(0)
    {
    
    }
    
    bool ExampleConfig::Parse(const KERNEL_NS::LibString &lineData)
    {
      // 数据使用json序列化的文本
    	// column_前缀，列id，数据长度:数据|column_前缀,....
    	// column_1_10:1010105555|column_2_10:aaaaaaaaaa|...

      const Int32 fieldNum = 5;
      Int32 countFieldNum = 0;
      Int32 startPos = 0;

      {// 第一个字段
          auto pos = lineData.GetRaw().find_first_of("column_", startPos);
          if(pos == std::string::npos)
          {
              g_Log->Error(LOGFMT_OBJ_TAG("%s data not found lineData:%s"), lineData.c_str());
              return false;
          }
            
          auto headerTailPos = lineData.GetRaw().find_first_of(":");
          if(headerTailPos == std::string::npos)
          {
            g_Log->Error(LOGFMT_OBJ_TAG("bad line data not find : symbol after column_ line data:%s"), lineData.c_str());
            return false;
          }
          
          // 解析数据
          const KERNEL_NS::LibString headerInfo = lineData.GetRaw().sub(pos, headerTailPos - pos);
          const auto headParts = headerInfo.Split('_');
          if(headParts.empty())
          {
            g_Log->Error(LOGFMT_OBJ_TAG("bad line data not find sep symbol:_ in header info line data:%s, pos:%d, headerTailPos:%d"), lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos));
            return false;
          }
          
          if(headParts.size() != 3)
          {
            g_Log->Error(LOGFMT_OBJ_TAG("bad line data header parts amount error line data:%s, pos:%d, headerTailPos:%d"), lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos));
            return false;
          }

          // 校验列id,保证数据解析是对的
          const auto &columnIdString = headParts[1];
          if(columnIdString.length() == 0)
          {
            g_Log->Error(LOGFMT_OBJ_TAG("have no column id when parse field:xxx, bad line data header len info line data:%s, pos:%d, headerTailPos:%d"), lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos));
            return false;
          }
          const UInt64 columnId = KERNEL_NS::StringUtil::StringToUInt64(columnIdString.c_str());
          if(columnId != xxx)
          {
            g_Log->Error(LOGFMT_OBJ_TAG("bad comumn id when parse field:xxx, columnId:%llu, real column id:xxx,please check if config data is old version bad line data header len info line data:%s, pos:%d, headerTailPos:%d"), columnId, lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos));
            return false;
          }

          // 数据长度
          const auto &lenInfo = headParts[2];
          if(lenInfo.length() == 0)
          {
            g_Log->Error(LOGFMT_OBJ_TAG("bad line data header len info line data:%s, pos:%d, headerTailPos:%d"), lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos));
            return false;
          }

          const Int32 dataLen = KERNEL_NS::StringUtil::StringToInt32(lenInfo.c_str());
          const auto dataEndPos = headerTailPos + static_cast<decltype(headerTailPos)>(dataLen) - 1;
          KERNEL_NS::LibString dataPart = lineData.GetRaw().sub(headerTailPos + 1, dataEndPos - headerTailPos);

          // 解析数据 通过column_id找到fieldInfo然后解析出_{fieldName} = 
          KERNEL_NS::LibString errInfo;
          if(!DataTypeHelper::Assign(_{xxx}, dataPart, errInfo)
          {
            g_Log->Error(LOGFMT_OBJ_TAG("%s, assign fail field name:%s, data part:%s, errInfo:%s  line data:%s, pos:%d, headerTailPos:%d, dataEndPos:%d"), KERNEL_NS::RttiUtil::GetByObj(this), xxx, dataPart.c_str(), errInfo.c_str(), lineData.c_str(), static_cast<Int32>(pos), static_cast<Int32>(headerTailPos), static_cast<Int32>(dataEndPos));
            return false;
          }

          startPos = dataEndPos;
          ++countFieldNum;
      }
    	
      if(countFieldNum != fieldNum)
      {
        g_Log->Error(LOGFMT_OBJ_TAG("field num not enough countFieldNum:%d, need fieldNum:%d lineData%s"), countFieldNum, fieldNum, lineData.c_str());
        return false;
      }

      return true;
    }
    
    void ExampleConfig::Serialize(KERNEL_NS::LibString &lineData) const
    {
      const Int32 fieldNum = 5;
      Int32 countFieldNum = 0;

       {// Id
        KERNEL_NS::LibString data;
        DataTypeHelper::ToString(_Id, data);
        lineData.AppendFormat("column_%d_%d:%s|", 1, static_cast<Int32>(data.size()), data.c_str());
        ++countFieldNum;
       }

       {// Type
        KERNEL_NS::LibString data;
        DataTypeHelper::ToString(_Type, data);
        lineData.AppendFormat("column_%d_%d:%s|", 2, static_cast<Int32>(data.size()), data.c_str());
        ++countFieldNum;
       }

      if(countFieldNum != fieldNum)
      {
        g_Log->Error(LOGFMT_OBJ_TAG("field num not enough countFieldNum:%d, need fieldNum:%d"), countFieldNum, fieldNum);
      }
    }
    
    POOL_CREATE_OBJ_DEFAULT_IMPL(ExampleConfigMgr);
    
    ExampleConfigMgr::ExampleConfigMgr()
    {
    }
    
    ExampleConfigMgr::~ExampleConfigMgr()
    {
    	_Clear();
    }

    void ExampleConfigMgr::Release()
    {
      ExampleConfigMgr::Delete_ExampleConfigMgr(this);
    }
    
    void ExampleConfigMgr::Clear()
    {
    	_Clear();
    }
    
    KERNEL_NS::LibString ExampleConfigMgr::ToString() const
    {
    	return GetObjName();
    }
    
    Int32 ExampleConfigMgr::Load()
    {
      KERNEL_NS::SmartPtr<std::vector<ExampleConfig *>, KERNEL_NS::AutoDelMethods::CustomDelete> configs = new std::vector<ExampleConfig *>;
      configs.SetClosureDelegate([](void *p){
        std::vector<ExampleConfig *> *ptr = reinterpret_cast<std::vector<ExampleConfig *> *>(p);
        KERNEL_NS::ContainerUtil::DelContainer(*ptr, [](ExampleConfig *ptr){
            ExampleConfig::Delete_ExampleConfig(ptr);
        });
        delete ptr;
      });

      const auto basePath = GetLoader()->GetBasePath();
      const auto wholePath = basePath + "/" + "xxx/xxx.data";
      KERNEL_NS::SmartPtr<FILE, KERNEL_NS::AutoDelMethods::CustomDelete> fp = KERNEL_NS::FileUtil::OpenFile(wholePath.c_str(), false, "rb");
      if(!fp)
      {
        g_Log->Error(LOGFMT_OBJ_TAG("data file not found wholePath:%s"), wholePath.c_str());
        return Status::Fail;
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
        return Status::Fail;
      }

      std::set<Int32> ids;
      while(true)
      {
        KERNEL_NS::LibString lineData;
        Int64 readBytes =_ReadConfigData(*fp, lineData);
        if(readBytes < 0)
        {
          g_Log->Error(LOGFMT_OBJ_TAG("Read a line config data fail wholePath:%s, line:%d, lineData:%s"), wholePath.c_str(), line, lineData.c_str());
          KERNEL_NS::LibDigest::MakeMd5Clean(ctx);
          return Status::Fail;
        }

        if(readBytes == 0)
          break;

          ++line;

          if(!KERNEL_NS::LibDigest::MakeMd5Continue(ctx, lineData.data(), static_cast<UInt64>(lineData.size())))
          {
              g_Log->Error(LOGFMT_OBJ_TAG("MakeMd5Continue fail wholePath:%s, line:%d, lineData:%s"), wholePath.c_str(), line, lineData.c_str());
              KERNEL_NS::LibDigest::MakeMd5Clean(ctx);
            return Status::Fail;
          }

          if(line == 1)
          {// 第一行是校验列字段名
            const KERNEL_NS::LibString columns = KERNEL_NS::LibString("Id") + "|" + "Type";
            if(columns != lineData)
            {
              g_Log->Error(LOGFMT_OBJ_TAG("current data not match this config data wholePath:%s, current data columns:%s, this config columns:%s"), wholePath.c_str(), lineData.c_str(), columns.c_str());
              return Status::Fail;
            }

            continue;
          }

          KERNEL_NS::SmartPtr<ExampleConfig, KERNEL_NS::AutoDelMethods::CustomDelete> config = ExampleConfig::New_ExampleConfig();
          config.SetClosureDelegate([](void *p){
            auto ptr = reinterpret_cast<ExampleConfig *>(p);
            ExampleConfig::Delete_ExampleConfig(ptr);
          });

          if(!config->Parse(lineData))
          {
            g_Log->Warn(LOGFMT_OBJ_TAG("parse ExampleConfig fail data path:%s line:%d, lineData:%s"), wholePath.c_str(), line, lineData.c_str());
            return Status::Fail;
          }

          // unique唯一性校验
          if(ids.find(config->_Id) != ids.end())
          {
            g_Log->Warn(LOGFMT_OBJ_TAG("duplicate id:%d data path:%s line:%d, lineData:%s"), config->_Id, wholePath.c_str(), line, lineData.c_str());
            return Status::Fail;
          }
          ids.insert(config->_Id);

          auto newConfig = config.AsSelf();
          configs->push_back(config.pop());
      }

      KERNEL_NS::LibString dataMd5;
      if(!KERNEL_NS::LibDigest::MakeMd5Final(ctx, dataMd5))
      {
          g_Log->Error(LOGFMT_OBJ_TAG("MakeMd5Final fail wholePath:%s"), wholePath.c_str());
          KERNEL_NS::LibDigest::MakeMd5Clean(ctx);
        return Status::Fail;
      }
      KERNEL_NS::LibDigest::MakeMd5Clean(ctx);

      _dataMd5 = dataMd5;
      _configs.swap(*config.AsSelf());
      _idRefConfig.clear();
      for(auto config : _configs)
      {
        _idRefConfig.insert(std::make_pair(config->_Id, config));
      }
      
      return Status::Success;

    }

    Int32 ExampleConfigMgr::Reload()
    {
      // 发生错误支持回退
      return Load();
    }

    const KERNEL_NS::LibString &ExampleConfigMgr::GetConfigDataMd5() const
    {
      return _dataMd5;
    }
    	
    const std::vector<ExampleConfig *> &ExampleConfigMgr::GetAllConfigs() const
    {
        return _configs;
    }

    void ExampleConfigMgr::_OnClose()
    {
      _Clear();
    }
    
    void ExampleConfigMgr::_Clear()
    {
      _dataMd5.clear();
      _idRefConfig.clear();
      KERNEL_NS::ContainerUtil::DelContainer(_configs, [](ExampleConfig *ptr){
          ExampleConfig::Delete_ExampleConfig(ptr);
      });
    }

    Int64 ExampleConfigMgr::_ReadConfigData(FILE &fp, KERNEL_NS::LibString &configData) const
    {
        // 读够一条配置的列数即可, 完整性校验: 必须把所有需要导出的列导出数据, 数据不完整则返回-1, 0表示没有数据了, >0 表示读取的字节数
        Int64 readBytes = 0;
        // column_1_10:1010105555|column_2_10:aaaaaaaaaa|...
        // 读到:解析头部,继续读数据,统计fieldNum个数, 以及列id,读完整才返回,若完整性有问题则返回-1
        KERNEL_NS::LibString content;
        std::set<Int32> needFieldIds = {xxxxxxxx};
        const Int32 fieldNum = xxx;
        Int32 count = 0;
        while(true)
        {
          auto bytesOnce = KERNEL_NS::FileUtil::ReadFile(fp, content, 1);
          if(bytesOnce == 0)
          {
            break;
          }

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
              g_Log->Warn2(KERNEL_NS::LibString("headerCache:"), headerCache, KERNEL_NS::LibString(", content:"), content);
               return -1;
             }

             const Int32 fieldColumnId = KERNEL_NS::StringUtil::StringToInt32(headerParts[1].c_str());
             const Int64 fieldDataLen = KERNEL_NS::StringUtil::StringToInt64(headerParts[2].c_str());

             configData += content.GetRaw().substr(0, symbolPos + 1);
             content.clear();

             bytesOnce = KERNEL_NS::FileUtil::ReadFile(fp, content, fieldDataLen);
             if(bytesOnce != fieldDataLen)
             {
                g_Log->Warn(LOGFMT_OBJ_TAG("column data error:"));
                g_Log->Warn2(KERNEL_NS::LibString("headerCache:"), headerCache, KERNEL_NS::LibString(", content:"), content, KERNEL_NS::LibString(", fieldDataLen:"), fieldDataLen, KERNEL_NS::LibString(", real len:"), bytesOnce, KERNEL_NS::LibString(", not enough."));
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
                g_Log->Warn2(KERNEL_NS::LibString("configData:"), configData, KERNEL_NS::LibString("need field column ids:"), KERNEL_NS::LibString("xxxx"));
                return -1;
             }

             if((count == fieldNum) && needFieldIds.empty())
             {
               return readBytes;
             }
          }
        }

        g_Log->Warn(LOGFMT_OBJ_TAG("column data error: field maybe changed count:%d, need fieldNum:%d column fieldIds not empty left:%d"), count, fieldNum, static_cast<Int32>(needFieldIds.size()));

        return -1;
    }

    ```
    
  * 