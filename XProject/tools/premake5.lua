--  @file   premake5.lua
-- #########################################################################
-- Global compile settings


-- python tool define
IS_WINDOWS = string.match(_ACTION, 'vs') ~= nil
ISUSE_CLANG = _ARGS[1] and (_ARGS[1] == 'clang')
-- 使用动态链接
USE_KERNEL_SO = _ARGS[2] and (_ARGS[2] == 'use_kernel_so')
ISUSE_STORAGE = _ARGS[3] and (_ARGS[3] == 'use_storage')

print('_ARGS:', _ARGS[1], _ARGS[2], _ARGS[3], ', ISUSE_CLANG:', ISUSE_CLANG)

-- root directory
ROOT_DIR = "../../"
WIN_ROOT_DIR = ".\\..\\..\\"
XPROJ_PATH = ROOT_DIR .. "XProject/"

OUTPUT_NAME = "build_x"
-- All libraries output directory
OUTPUT_DIR = XPROJ_PATH .. "output/" .. _ACTION .. "/" .. OUTPUT_NAME .. "/"

-- build directory
BUILD_DIR = XPROJ_PATH .. OUTPUT_NAME .. "/"
-- 脚本路径
SCRIPT_PATH = ROOT_DIR .. "XProject/scripts/builds/"
if IS_WINDOWS then
    SCRIPT_PATH = ".\\\\..\\\\..\\\\" .. "scripts\\\\builds\\\\"
end

-- debug dir
DEBUG_DIR = OUTPUT_DIR

ENABLE_PERFORMANCE_RECORD = 0

ENABLE_POLLER_PERFORMANCE = 0

ENABLE_TEST_SERVICE = 1

-- 预编译
ENABLE_PRECOMPILE_HEADER = 1

-----------------------------------------------------------------------------------------------------------

-- 公共方法
dofile(ROOT_DIR .. "/tools/premake/common.lua")

-- kernel模块
dofile(ROOT_DIR .. "/tools/premake/kernel_premake5.lua")

-----------------------------------------------------------------------------------------------------------

-- zlib library:
-- local ZLIB_LIB = "../../FS/3rd_party/zlib"
-- #########################################################################

workspace ("CrystalNet_" .. _ACTION)
    -- location define
    location (BUILD_DIR .. _ACTION)
    -- target directory define
    targetdir (OUTPUT_DIR)

    filter { "system:windows", "language:c++" }
        defines { "_SCL_SECURE_NO_DEPRECATE" }
        defines { "_CRT_SECURE_NO_DEPRECATE" }
    filter {}

    -- configurations 默认64位 不输出32位
    configurations {"debug", "release"}

    -- architecture 全部配置都生成64位程序
    filter { "configurations:*" }
        architecture "x86_64"
    filter {}

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
	
    -- control symbols
    filter { "system:macosx", "language:c++" }
        symbols("On")
    filter {}

    -- characterset architecture 多字节字符
    filter { "language:c++" }
        characterset "MBCS"
    filter {}
    -- filter { "language:c++" }
    --     characterset "Unicode"
    -- filter {}

    -- disable some warnings
    filter { "system:windows", "language:c++" }
        disablewarnings { "4091", "4819", "4251" }
    filter {}

    -- 支持c++20
    cppdialect "c++20"
    defines { "CRYSTAL_NET_CPP20" }

    -- cppm会被当初c++ module进行编译
    filter { "files:**.cppm" }
    compileas "Module"

    -- ixx 是模块接口, 用于导出接口给外部使用
    -- filter { "files:**.ixx" }
    -- compileas "ModulePartition"

-- ****************************************************************************



-- ****************************************************************************


-- ****************************************************************************
project "LogicPlugin"
    -- language, kind
    language "c++"
    kind "SharedLib"

    -- windows下先生成到tmp
    filter{ "system:windows"}		
        targetdir (OUTPUT_DIR .. "/PluginTmp/")
        libdirs { OUTPUT_DIR .. "/PluginTmp/"}	
    filter{}

    -- 支持c++20
    -- cppdialect "c++20"

    -- symbols
	debugdir(DEBUG_DIR)
    symbols "On"
    
    -- dependents
    dependson {
        "CrystalKernel",
    }

	enable_precompileheader("pch.h", XPROJ_PATH .. "LogicPlugin/LogicPlugin_pch/pch.cpp")

    -- hidden是隐藏符号，符号不会导出
    filter { "system:not windows" }
        buildoptions { "-fvisibility=hidden" }
    filter {}

    if ISUSE_STORAGE then
	    defines {"CRYSTAL_STORAGE_ENABLE"}
    end

    -- 导入内核接口 宏定义
	defines {"CRYSTAL_NET_CPP20", "CRYSTAL_NET_IMPORT_KERNEL_LIB"}

	-- 设置通用选项
    set_common_options(nil, true)
	
	includedirs {
	    "../../",
		ROOT_DIR .. "kernel/include/",
		ROOT_DIR .. "OptionComponent/",
		XPROJ_PATH .. "protocols/cplusplus/",
		XPROJ_PATH .. "LogicPlugin/",
		XPROJ_PATH .. "LogicPlugin/LogicPlugin_pch/",
        XPROJ_PATH .. "Service/",
        ROOT_DIR .. "service_common/",
        XPROJ_PATH .. "Config/code/",
		XPROJ_PATH .. "Service/LogicService/",
    }
    
    -- files
    files {
        XPROJ_PATH .. "LogicPlugin/**.h",
        XPROJ_PATH .. "LogicPlugin/**.c",
        XPROJ_PATH .. "LogicPlugin/**.cpp",
        XPROJ_PATH .. "LogicPlugin/**.lua",
    }

    filter{ "system:windows"}		
        libdirs { 
            ROOT_DIR .. "3rd/"
        }
    filter{}

    filter { "system:windows" }
        links {
            "ws2_32",
            "Mswsock",
            "DbgHelp",
        }
    filter{}

	-- links
    libdirs { OUTPUT_DIR }	
	include_libfs(true, true)

    -- lua
    include_lua()

    -- mongodb driver
    include_mongodb_driver_libs(ROOT_DIR)

    -- debug target suffix define
    filter { "configurations:debug*" }
        local str = "_debug"
        targetsuffix(str)
    filter {}

    -- enable multithread compile
    -- enable_multithread_comp("C++14")
	enable_multithread_comp()
    
    -- target prefix 前缀
    targetprefix "lib"

    -- warnings
    filter { "system:not windows" }
        disablewarnings {
            "invalid-source-encoding",
        }
    filter {}

    -- optimize
    set_optimize_opts()
	
    local suffix = ""
    filter { "configurations:debug*" }
        suffix = "_debug"
    filter {}

	-- set post build commands.
    filter { "system:windows" }
        postbuildcommands(string.format("start %srunfirstly_scripts.bat %s %s", SCRIPT_PATH, _ACTION, suffix))
    filter {}

    	-- post build(linux)
	filter { "configurations:debug*", "system:not windows"}
	    postbuildmessage "Generation hotfix so with timestamp ..."
	    postbuildcommands(string.format("sh %splugin_building.sh %s %s .so",  SCRIPT_PATH, OUTPUT_DIR, "libLogicPlugin_debug"))
	filter {}

    filter { "configurations:release*", "system:not windows"}
        postbuildmessage "Generation hotfix so with timestamp ..."
        postbuildcommands(string.format("sh %splugin_building.sh %s %s .so",  SCRIPT_PATH, OUTPUT_DIR, "libLogicPlugin"))
    filter {}

-- ****************************************************************************

-- core library testsuite compile setting
project "LogicServer"
    -- language, kind
    language "c++"
    kind "ConsoleApp"
	
    -- 支持c++20
    -- cppdialect "c++20"

    -- symbols
	debugdir(DEBUG_DIR)
    symbols "On"

    -- dependents
    dependson {
        "CrystalKernel",
    }

    if ISUSE_STORAGE then
	    defines {"CRYSTAL_STORAGE_ENABLE"}
    end

    -- 导入内核接口 宏定义
	defines {"CRYSTAL_NET_CPP20", "CRYSTAL_NET_IMPORT_KERNEL_LIB"}

	enable_precompileheader("pch.h", ROOT_DIR .. "LogicServer/LogicServer_pch/pch.cpp")

	includedirs {
	    "../../",
		ROOT_DIR .. "kernel/include/",
		XPROJ_PATH .. "LogicServer/",
		XPROJ_PATH .. "LogicServer/LogicServer_pch/",
        XPROJ_PATH .. "Config/code/",
		ROOT_DIR .. "OptionComponent/",
		XPROJ_PATH .. "protocols/cplusplus/",
		XPROJ_PATH .. "Service/LogicService/",
		XPROJ_PATH .. "LogicPlugin/",
    }

    -- lua
    include_lua()

    -- mongodb
    include_mongodb_driver_libs(ROOT_DIR)

	-- 设置通用选项
    set_common_options(nil, true)

    -- files
    files {
        XPROJ_PATH .. "protocols/**.h",
        XPROJ_PATH .. "protocols/**.cc",
        XPROJ_PATH .. "protocols/**.cpp",
        XPROJ_PATH .. "Service/Common/**.h",
        XPROJ_PATH .. "Service/Common/**.cpp",
        XPROJ_PATH .. "Service/LogicService/**.h",
        XPROJ_PATH .. "Service/LogicService/**.cpp",
        XPROJ_PATH .. "Service/LogicService/**.lua",
        XPROJ_PATH .. "Service/LogicService/**.cppm",
        XPROJ_PATH .. "Config/code/**.h",
        XPROJ_PATH .. "Config/code/**.cpp",
        ROOT_DIR .. "service_common/**.h",
        ROOT_DIR .. "service_common/**.cpp",
        XPROJ_PATH .. "LogicServer/**.h",
        XPROJ_PATH .. "LogicServer/**.cpp",
        XPROJ_PATH .. "LogicServer/**.lua",
        XPROJ_PATH .. "LogicServer/**.cppm",
        ROOT_DIR .. "OptionComponent/OptionComp/**.h",
        ROOT_DIR .. "OptionComponent/OptionComp/**.cpp",
        XPROJ_PATH .. "Yaml/**.yaml",
    }

    filter{ "system:windows"}		
        libdirs { 
            ROOT_DIR .. "3rd/"
        }
    filter{}

    filter { "system:windows" }
        links {
            "ws2_32",
            "Mswsock",
            "DbgHelp",
        }
    filter{}

	-- links
    libdirs { OUTPUT_DIR }	
	include_libfs(true, true)

    -- debug target suffix define
    filter { "configurations:debug*" }
        targetsuffix "_debug"
    filter {}

    -- enable multithread compile
    -- enable_multithread_comp("C++14")
	enable_multithread_comp()

    -- warnings
    filter { "system:not windows" }
        disablewarnings {
            "invalid-source-encoding",
        }
    filter {}

    -- optimize
    set_optimize_opts()
	
    -- if not IS_WINDOWS then
    --     build_cpp_modules2("../../testsuit", "module_interface", include_paths, false)
    -- end

	-- set post build commands.
    filter { "system:windows" }
        postbuildcommands(string.format("start %srunfirstly_scripts.bat %s", SCRIPT_PATH, _ACTION))
    filter {}
	
-- ****************************************************************************
