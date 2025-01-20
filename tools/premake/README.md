# 编译:

github:https://github.com/premake/premake-core

# windows

直接执行脚本：Bootstrap.bat

# Linux

执行:make -f Bootstrap.mak linux

# 关于支持C++20

* 需要在premake5.lua的workspace作用于添加

  * ```
        -- 支持c++20
        cppdialect "c++20"
        defines { "CRYSTAL_NET_CPP20" }
        
        -- cppm会被当初c++ module进行编译
        filter { "files:**.cppm" }
        compileas "Module"
        
           -- defines
        filter { "configurations:debug*" }
            defines {
                "DEBUG",
    			"_DEBUG",
            }
    
            -- 开启module
            enablemodules("On")
        filter {}
    	
        filter { "configurations:release*" }
            defines {
                "NDEBUG"
            }
            -- 开启module
            enablemodules("On")
        filter {}
    ```

    

* 注意CrystaKernel库用在工具时不需要生成动态链接库，只需要静态库
* 只有需要热更的地方才需要动态链接库