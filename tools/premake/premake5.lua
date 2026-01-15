--  @file   premake5.lua
-- #########################################################################
-- Global compile settings


-- python tool define
IS_WINDOWS = string.match(_ACTION, 'vs') ~= nil
ISUSE_CLANG = _ARGS[1] and (_ARGS[1] == 'clang')
-- 使用动态链接
USE_KERNEL_SO = _ARGS[2] and (_ARGS[2] == 'use_kernel_so')

print('_ARGS:', _ARGS[1], _ARGS[2], _ARGS[3], ', ISUSE_CLANG:', ISUSE_CLANG)

-- header directory
KERNEL_HEADER_DIR = "../../kernel/kernel_pch/"
-- All libraries output directory
OUTPUT_DIR = "../../output/" .. _ACTION
-- root directory
ROOT_DIR = "../../"
WIN_ROOT_DIR = ".\\..\\..\\"
-- build directory
BUILD_DIR = "../../build/"
-- 脚本路径
SCRIPT_PATH = ROOT_DIR .. "scripts/builds/"
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
dofile("./common.lua")

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
        disablewarnings { "4091", "4819" }
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

-- kernel模块
dofile("./kernel_premake5.lua")

-- ****************************************************************************


-- ****************************************************************************
-- FS core library compile setting
project "TestServicePlugin"
    -- language, kind
    language "c++"
    if IS_WINDOWS then
        kind "StaticLib"
    else
        kind "SharedLib"
    end

    -- 支持c++20
    -- cppdialect "c++20"

    -- symbols
	debugdir(DEBUG_DIR)
    symbols "On"
    
    -- dependents
    dependson {
        "CrystalKernel",
    }

	enable_precompileheader("pch.h", ROOT_DIR .. "TestServicePlugin/TestServicePlugin_pch/pch.cpp")

    -- windows下是静态库, linux下是动态库
    if IS_WINDOWS then
        defines{"TEST_PLUGIN_STATIC_LIB"}
    end

    -- 导入内核接口 宏定义
	defines {"CRYSTAL_NET_CPP20", "CRYSTAL_NET_IMPORT_KERNEL_LIB", "SIMPLE_API_IMPORT_KERNEL_LIB", "CRYSTAL_STORAGE_ENABLE"}

	-- 设置通用选项
    set_common_options(nil, true)
	
	includedirs {
	    "../../",
		"../../kernel/include/",
        "../../3rd/mysql/win/include/",
		"../../OptionComponent/",
		"../../protocols/cplusplus/",
		"../../TestServicePlugin/",
		"../../TestServicePlugin/TestServicePlugin_pch/",
        "../../service/",
        "../../service_common/",
        "../../service/TestService/config/code/",
		"../../service/TestService/",
    }
    
    -- files
    files {
        -- "../../3rd/protobuf/include/**.h",
        -- "../../3rd/protobuf/include/**.cc",
        "../../TestServicePlugin/**.h",
		"../../TestServicePlugin/**.c",
		"../../TestServicePlugin/**.cpp",
		"../../TestServicePlugin/**.lua",
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
        targetsuffix "_debug"
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
	
	-- set post build commands.
    filter { "system:windows" }
        postbuildcommands(string.format("start %srunfirstly_scripts.bat %s", SCRIPT_PATH, _ACTION))
    filter {}

-- ****************************************************************************

-- 构造包含路径
local include_paths = build_include_paths("", "../../")
include_paths = build_include_paths(include_paths, "../../kernel/include/")
include_paths = build_include_paths(include_paths, "../../testsuit/")
include_paths = build_include_paths(include_paths, "../../testsuit/testsuit_pch/")
include_paths = build_include_paths(include_paths, "../../service/TestService/config/code/")
include_paths = build_include_paths(include_paths, "../../3rd/mysql/win/include/")
include_paths = build_include_paths(include_paths, "../../OptionComponent/")
include_paths = build_include_paths(include_paths, "../../protocols/cplusplus/")
include_paths = build_include_paths(include_paths, "../../service/TestService/")
include_paths = build_include_paths(include_paths, "../../TestServicePlugin/")
include_paths = build_include_paths(include_paths, ROOT_DIR .. "/3rd/openssl/include/")
include_paths = build_include_paths(include_paths, ROOT_DIR .. "/3rd/protobuf/include/")
include_paths = build_include_paths(include_paths, ROOT_DIR .. "/3rd/miniz/include/")
include_paths = build_include_paths(include_paths, ROOT_DIR .. "/3rd/uuid/include/")
include_paths = build_include_paths(include_paths, ROOT_DIR .. "/3rd/json/include/")

-- 模块预编译规则（GCC/Clang）
rule "module_interface"
    display "Precompiling %{file.name}"
    local compile_exe = ISUSE_CLANG and "clang" or "g++"
    buildoutputs { "%{file.basename}.pcm" }
    buildcommands {
        compile_exe .. " -std=c++20 -fmodules-ts" .. include_paths .. " --precompile %{file.relpath} -o %{file.basename}.pcm"
    }

-- core library testsuite compile setting
project "testsuit"
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

    filter {"system:windows"}
        dependson {
            "TestServicePlugin",
        }
    filter{}

    -- 导入内核接口 宏定义
	defines {"CRYSTAL_NET_CPP20", "CRYSTAL_NET_IMPORT_KERNEL_LIB", "SIMPLE_API_IMPORT_KERNEL_LIB", "CRYSTAL_STORAGE_ENABLE"}

	enable_precompileheader("pch.h", ROOT_DIR .. "testsuit/testsuit_pch/pch.cpp")

	includedirs {
	    "../../",
		"../../kernel/include/",
		"../../testsuit/",
		"../../testsuit/testsuit_pch/",
        "../../service/TestService/config/code/",
		"../../3rd/mysql/win/include/",
		"../../OptionComponent/",
		"../../protocols/cplusplus/",
		"../../service/TestService/",
		"../../TestServicePlugin/",
    }

    -- mysql
    filter { "system:windows"}
        includedirs {
            "../../3rd/mysql/win/include/"
        }

        libdirs { 
            ROOT_DIR .. "3rd/mysql/win/lib/",
        }

        links {
            "libmysql",
        }
    filter {}
    -- mysql
    filter { "system:not windows"}
        includedirs {
            "../../3rd/mysql/linux/include/"
        }

        libdirs { 
            ROOT_DIR .. "3rd/mysql/linux/lib/",
        }

        links {
            "mysqlclient",
        }
    filter {}

    -- lua
    include_lua()

    -- mongodb
    include_mongodb_driver_libs(ROOT_DIR)

    -- windows 下连接plugin静态库
    filter { "configurations:debug*", "language:c++", "system:windows" }
        links {
            "libTestServicePlugin_debug",
        }
    filter {}
    filter { "configurations:release*", "language:c++", "system:windows" }
        links {
            "libTestServicePlugin",
        }
    filter {}

	-- 设置通用选项
    set_common_options(nil, true)

    if ENABLE_TEST_SERVICE ~= 0 then
        -- files
        files {
            -- "../../3rd/protobuf/include/**.h",
            -- "../../3rd/protobuf/include/**.cc",
            "../../protocols/**.h",
            "../../protocols/**.cc",
            "../../protocols/**.cpp",
            "../../service/common/**.h",
            "../../service/common/**.cpp",
            "../../service/TestService/**.h",
            "../../service/TestService/**.cpp",
            "../../service/TestService/**.lua",
            "../../service_common/**.h",
            "../../service_common/**.cpp",
            "../../testsuit/**.h",
            -- "../../testsuit/**.ixx",
            "../../testsuit/**.cpp",
            "../../testsuit/**.lua",
            "../../testsuit/**.cppm",
            "../../service/TestService/**.cppm",
            "../../service/TestService/config/code/**.h",
            "../../service/TestService/config/code/**.cpp",
            "../../OptionComponent/OptionComp/**.h",
            "../../OptionComponent/OptionComp/**.cpp",
        }
    else
        -- files
        files {
            -- "../../3rd/protobuf/include/**.h",
            -- "../../3rd/protobuf/include/**.cc",
            "../../protocols/**.h",
            "../../protocols/**.cc",
            "../../protocols/**.cpp",
            "../../service_common/**.h",
            "../../service_common/**.cpp",
            "../../testsuit/**.h",
            "../../testsuit/**.cpp",
            "../../OptionComponent/OptionComp/**.h",
            "../../OptionComponent/OptionComp/**.cpp",
        }       
    end

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
	

-- core library testsuite compile setting
project "client"
    -- language, kind
    language "c++"
    kind "ConsoleApp"
	
    -- symbols
	debugdir(DEBUG_DIR)
    symbols "On"

    -- dependents
    dependson {
        "CrystalKernel",
    }

    -- 导入内核接口
	defines { "CRYSTAL_NET_IMPORT_KERNEL_LIB"}

	enable_precompileheader("pch.h", ROOT_DIR .. "client/client_pch/pch.cpp")

	includedirs {
	    "../../",
		"../../kernel/include/",
		"../../client/",
		"../../client/client_pch/",
		"../../service/Client/",
		"../../protocols/cplusplus/",
    }
	
	-- 设置通用选项
    set_common_options(nil, true)
	
    -- files
    files {
        -- "../../3rd/protobuf/include/**.h",
		-- "../../3rd/protobuf/include/**.cc",
		"../../protocols/**.h",
		"../../protocols/**.cc",
		"../../protocols/**.cpp",
		"../../service/common/**.h",
		"../../service/common/**.cpp",
        "../../service/Client/**.h",
        "../../service/Client/**.cpp",
		"../../service_common/**.h",
        "../../service_common/**.cpp",
        "../../client/**.h",
        "../../client/**.cpp",
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
	
	
	-- set post build commands.
    filter { "system:windows" }
        postbuildcommands(string.format("start %srunfirstly_scripts.bat %s", SCRIPT_PATH, _ACTION))
    filter {}


-- core library testsuite compile setting
project "Gateway"
    -- language, kind
    language "c++"
    kind "ConsoleApp"
	
    -- symbols
	debugdir(DEBUG_DIR)
    symbols "On"

    -- dependents
    dependson {
        "CrystalKernel",
    }

    -- 导入内核接口 宏定义
	defines { "CRYSTAL_NET_IMPORT_KERNEL_LIB"}

	enable_precompileheader("pch.h", ROOT_DIR .. "Gateway/Gateway_pch/pch.cpp")

	includedirs {
	    "../../",
		"../../kernel/include/",
		"../../Gateway/",
		"../../Gateway/Gateway_pch/",
        "../../service/GateService/config/code/",
		"../../service/GateService/",
		"../../protocols/cplusplus/",
    }

	-- 设置通用选项
    set_common_options(nil, true)
	
    -- files
    files {
		-- "../../3rd/protobuf/include/**.h",
		-- "../../3rd/protobuf/include/**.cc",
		"../../protocols/**.h",
		"../../protocols/**.cc",
		"../../protocols/**.cpp",
		"../../service/common/**.h",
		"../../service/common/**.cpp",
        "../../service/GateService/**.h",
        "../../service/GateService/**.cpp",
		"../../service_common/**.h",
        "../../service_common/**.cpp",
        "../../Gateway/**.h",
        "../../Gateway/**.cpp",
        "../../service/GateService/config/code/**.h",
        "../../service/GateService/config/code/**.cpp",
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
	
	-- set post build commands.
    filter { "system:windows" }
        postbuildcommands(string.format("start %srunfirstly_scripts.bat %s", SCRIPT_PATH, _ACTION))
    filter {}
	
-- ****************************************************************************



-- core library testsuite compile setting
project "CenterServer"
    -- language, kind
    language "c++"
    kind "ConsoleApp"
	
    -- symbols
	debugdir(DEBUG_DIR)
    symbols "On"

    -- dependents
    dependson {
        "CrystalKernel",
    }

    -- 导入内核接口 宏定义
	defines { "CRYSTAL_NET_IMPORT_KERNEL_LIB"}

	enable_precompileheader("pch.h", ROOT_DIR .. "CenterServer/CenterServer_pch/pch.cpp")

	includedirs {
	    "../../",
		"../../kernel/include/",
		"../../CenterServer/",
		"../../CenterServer/CenterServer_pch/",
        "../../service/CenterService/config/code/",
		"../../service/CenterService/",
		"../../protocols/cplusplus/",
    }

	-- 设置通用选项
    set_common_options(nil, true)
	
    -- files
    files {
		-- "../../3rd/protobuf/include/**.h",
		-- "../../3rd/protobuf/include/**.cc",
		"../../protocols/**.h",
		"../../protocols/**.cc",
		"../../protocols/**.cpp",
		"../../service/common/**.h",
		"../../service/common/**.cpp",
        "../../service/CenterService/**.h",
        "../../service/CenterService/**.cpp",
		"../../service_common/**.h",
        "../../service_common/**.cpp",
        "../../CenterServer/**.h",
        "../../CenterServer/**.cpp",
        "../../service/CenterService/config/code/**.h",
        "../../service/CenterService/config/code/**.cpp",
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
	
	-- set post build commands.
    filter { "system:windows" }
        postbuildcommands(string.format("start %srunfirstly_scripts.bat %s", SCRIPT_PATH, _ACTION))
    filter {}
	
-- ****************************************************************************
