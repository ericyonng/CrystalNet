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
BUILD_DIR = "../../build_cs/"
-- 脚本路径
SCRIPT_PATH = ROOT_DIR .. "scripts/build_cs/"
if IS_WINDOWS then
    SCRIPT_PATH = ".\\\\..\\\\..\\\\" .. "scripts\\\\build_cs\\\\"
end

-- debug dir
DEBUG_DIR = OUTPUT_DIR

ENABLE_PERFORMANCE_RECORD = 0

ENABLE_POLLER_PERFORMANCE = 0

ENABLE_TEST_SERVICE = 1

-- 预编译
ENABLE_PRECOMPILE_HEADER = 1

-- 不使用CPP20
NOT_USE_CPP20 = true

-----------------------------------------------------------------------------------------------------------

-- 公共方法
dofile("./common_cs.lua")

-----------------------------------------------------------------------------------------------------------

-- zlib library:
-- local ZLIB_LIB = "../../FS/3rd_party/zlib"
-- #########################################################################

workspace ("CrystalNet_" .. _ACTION)
    -- location define
    location (BUILD_DIR .. _ACTION)
    -- target directory define
    targetdir (OUTPUT_DIR)

	defines { "CRYSTAL_NET_STATIC_KERNEL_LIB" }
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

    -- 支持c++17
    cppdialect "C++17"
    -- defines { "CRYSTAL_NET_CPP20" }

    -- cppm会被当初c++ module进行编译
    -- filter { "files:**.cppm" }
    -- compileas "Module"

    -- ixx 是模块接口, 用于导出接口给外部使用
    -- filter { "files:**.ixx" }
    -- compileas "ModulePartition"

-- ****************************************************************************

-- kernel模块
dofile("./kernel_premake5.lua")

-- ****************************************************************************


-- ****************************************************************************

-- core library testsuite compile setting
project "client_lib"
    -- language, kind
    language "c++"
    kind "SharedLib"
	
    -- symbols
	debugdir(DEBUG_DIR)
    symbols "On"

    -- target prefix 前缀
    targetprefix "lib"

    -- dependents
    dependson {
        "CrystalKernel",
    }

    -- 导入内核接口
	defines { "CRYSTAL_NET_IMPORT_KERNEL_LIB"}

	enable_precompileheader("pch.h", ROOT_DIR .. "client_lib/client_pch/pch.cpp")

	includedirs {
	    "../../",
		"../../kernel/include/",
		"../../client_lib/",
		"../../client_lib/client_pch/",
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
        "../../client_lib/**.h",
        "../../client_lib/**.cpp",
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



-- close windows process
project "UseClientLib"
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
	defines { "CRYSTAL_NET_IMPORT_KERNEL_LIB", "CRYSTAL_NET_STATIC_KERNEL_LIB" }

	enable_precompileheader("pch.h", ROOT_DIR .. "Useclient_lib/Useclient_lib_pch/pch.cpp")

	includedirs {
	    "../../",
		"../../kernel/include/",
		"../../client_lib/",
		"../../Useclient_lib/",
		"../../Useclient_lib/Useclient_lib_pch/",
    }
	
	-- 设置通用选项
    set_common_options("Size")
	
    -- files
    files {
		"../../service_common/common/**.h",
		"../../service_common/common/**.cpp",
		"../../service_common/params/**.h",
		"../../service_common/params/**.cpp",
        "../../Useclient_lib/**.h",
        "../../Useclient_lib/**.cpp",
    }

    -- 工具不需要动态库连接
	defines { "CRYSTAL_NET_STATIC_KERNEL_LIB" }
    defines("DISABLE_OPCODES")

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
	include_libfs(true, false)

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

