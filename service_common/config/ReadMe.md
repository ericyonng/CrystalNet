# 配置设计

* 配置代码模版设计

  * cpp 数据文档结构

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
    	
    	virtual void Clear() override;
    	virtual KERNEL_NS::LibString ToString() const override;
    	virtual Int32 Load() override;
    	virtual Int32 Reload() override;
    	virtual const std::vector<KERNEL_NS::LibString> &GetAllConfigFiles() const override;
        virtual const KERNEL_NS::LibString & GetConfigDataMd5() const override;
    	
    	const std::vector<ExampleConfig *> &GetAllConfigs() const;
    	
    private:
    	virtual void _OnClose() override;
    	void _Clear();
  	
    private:
    	std::vector<ExampleConfig *> _configs;
    	std::vector<KERNEL_NS::LibString> _configFiles;
    
    	KERNEL_NS::LibString _dataFile;		// 数据
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
    
    }
    
    void ExampleConfig::Serialize(KERNEL_NS::LibString &lineData) const
    {
    
    }
    ```
    
  * 