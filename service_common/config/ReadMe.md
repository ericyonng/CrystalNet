# 配置设计

* 配置代码模版设计

  * Cpp

  * ```
    // Generate by ConfigExporter, Dont modify it!!!
    // config:xlsx
    //
    //	file path:
    //
    // sheet name:%s
    // 
    
    #pragma once
    
    #include <kernel/kernel.h>
    #include <config/iconfig.h>
    
    class ExampleConfig : public IConfig
    {
    	POOL_CREATE_OBJ_DEFAULT_P1(IConfig, Example);
    	
    public:
    	Example();
    	~Example();
    	
    	virtual Int32 Load() override;
    	
    	
    };
    
    // 工厂
    class ExampleConfigFactory : public IConfigFactory
    {
    	POOL_CREATE_OBJ_DEFAULT_P1(IConfigFactory, ExampleConfigFactory);
    	
    public:
    	virtual IConfig *Create() override
    	{
    		
    	}
    	
    	virtual void Release(IConfig *config) override
    	{
    		
    	}
    };
    
    
    class ExampleConfigMgr : public IConfigMgr
    {
    	POOL_CREATE_OBJ_DEFAULT_P1(IConfigMgr, ExampleConfigMgr);
    	
    	ExampleConfigMgr();
    	~ExampleConfigMgr();
    	
    	virtual Int32 Load() override;
    	virtual void Destroy() override;
    	
    	const std::vector<ExampleConfig *> &GetAllConfigs() const;
    	
    private:
    	std::vector<ExampleConfig *> _configs;
    };
    
    class ExampleConfigMgrFactory : public IConfigMgrFactory
    {
    
    };
    ```

    