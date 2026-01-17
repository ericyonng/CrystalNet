--  @file   premake5.lua
-- #########################################################################
-- Global compile settings

-- python tool define
IS_WINDOWS = string.match(_ACTION, 'vs') ~= nil
ISUSE_CLANG = _ARGS[1] and (_ARGS[1] == 'clang')
print('_ARGS:', _ARGS[1], _ARGS[2], _ARGS[3], ', ISUSE_CLANG:', ISUSE_CLANG)

-- header directory
KERNEL_HEADER_DIR = "../../kernel/kernel_pch/"
-- All libraries output directory
OUTPUT_NAME = "build_tools"
OUTPUT_DIR = "../../output/" .. _ACTION .. "/" .. OUTPUT_NAME .. "/"
-- root directory
ROOT_DIR = "../../"
WIN_ROOT_DIR = ".\\..\\..\\"
-- build directory
BUILD_DIR = "../../" .. OUTPUT_NAME .. "/"

-- script path
SCRIPT_PATH = ROOT_DIR .. "scripts/" .. OUTPUT_NAME .. "/"
if IS_WINDOWS then
    SCRIPT_PATH = ".\\\\..\\\\..\\\\" .. "scripts\\" .. OUTPUT_NAME .. "\\"
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


-- core library testsuite compile setting
project "protogentool"
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

	enable_precompileheader("pch.h", ROOT_DIR .. "ProtoGen/protogen_pch/pch.cpp")

	includedirs {
	    "../../",
		"../../kernel/include/",
		"../../ProtoGen/",
		"../../ProtoGen/protogen_pch/",
		"../../OptionComponent/",
    }
	
	-- 设置通用选项
    set_common_options("Size")
	
    defines("DISABLE_OPCODES")
    
    -- files
    files {
        "../../ProtoGen/**.h",
        "../../ProtoGen/**.cpp",
		"../../OptionComponent/OptionComp/CodeAnalyze/**.h",
		"../../OptionComponent/OptionComp/CodeAnalyze/**.cpp",
    }

    -- 工具不需要动态库连接
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
	include_libfs(true)

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
project "filetool"
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

    enable_precompileheader("pch.h", ROOT_DIR .. "file_tool/file_tool_pch/pch.cpp")

    includedirs {
        "../../",
        "../../kernel/include/",
        "../../file_tool/",
        "../../file_tool/file_tool_pch/",
    }

    -- 设置通用选项
    set_common_options("Size")

    defines("DISABLE_OPCODES")

    -- files
    files {
        "../../file_tool/**.h",
        "../../file_tool/**.cpp",
    }

    -- 工具不需要动态库连接
    defines { "CRYSTAL_NET_STATIC_KERNEL_LIB" }

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
    include_libfs(true)

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

if IS_WINDOWS == false then
	print("builddir = " .. BUILD_DIR)
end



-- core library testsuite compile setting
project "ConfigExporter"
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
	defines { "CRYSTAL_NET_IMPORT_KERNEL_LIB", "CRYSTAL_NET_STATIC_KERNEL_LIB", "DISABLE_OPCODES"}

	enable_precompileheader("pch.h", ROOT_DIR .. "ConfigExporter/ConfigExporter_pch/pch.cpp")

	includedirs {
	    "../../",
		"../../kernel/include/",
		"../../ConfigExporter/",
		"../../ConfigExporter/ConfigExporter_pch/",
		"../../protocols/cplusplus/",
		ROOT_DIR .. "/3rd/miniz/include/",
    }
	
	-- 设置通用选项
    set_common_options("Size")
	
    -- files
    files {
		"../../service_common/common/**.h",
		"../../service_common/common/**.cpp",
		"../../service_common/params/**.h",
		"../../service_common/params/**.cpp",
        "../../service_common/config/DataTypeHelper.h",
        "../../service_common/config/DataTypeHelper.cpp",
        "../../ConfigExporter/**.h",
        "../../ConfigExporter/**.cpp",
    }

    -- 工具不需要动态库连接
	defines { "CRYSTAL_NET_STATIC_KERNEL_LIB" }

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

    libdirs { 
		ROOT_DIR .. "3rd/miniz/libs/$(Configuration)/",
    }

    filter { "system:linux", "configurations:debug*"}
        links {
            "miniz:static",
        }
    filter {}
    filter { "system:linux", "configurations:release*"}
        links {
            "miniz:static",
        }
    filter {}

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
project "md5tool"
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
    defines { "CRYSTAL_NET_IMPORT_KERNEL_LIB", "CRYSTAL_NET_STATIC_KERNEL_LIB", "DISABLE_OPCODES" }

    enable_precompileheader("pch.h", ROOT_DIR .. "md5tool/md5tool_pch/pch.cpp")

    includedirs {
        "../../",
        "../../kernel/include/",
        "../../md5tool/",
        "../../md5tool/md5tool_pch/",
    }

    -- 设置通用选项
    set_common_options("Size")

    -- files
    files {
		"../../service_common/common/**.h",
		"../../service_common/common/**.cpp",
		"../../service_common/params/**.h",
		"../../service_common/params/**.cpp",
        "../../md5tool/**.h",
        "../../md5tool/**.cpp",
    }

    -- 工具不需要动态库连接
    defines { "CRYSTAL_NET_STATIC_KERNEL_LIB" }

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
project "CloseProcess"
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

	enable_precompileheader("pch.h", ROOT_DIR .. "CloseProcess/CloseProcess_pch/pch.cpp")

	includedirs {
	    "../../",
		"../../kernel/include/",
		"../../CloseProcess/",
		"../../CloseProcess/CloseProcess_pch/",
    }
	
	-- 设置通用选项
    set_common_options("Size")
	
    -- files
    files {
		"../../service_common/common/**.h",
		"../../service_common/common/**.cpp",
		"../../service_common/params/**.h",
		"../../service_common/params/**.cpp",
        "../../CloseProcess/**.h",
        "../../CloseProcess/**.cpp",
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


-- ****************************************************************************

-- RsaGen
project "RsaGen"
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

	enable_precompileheader("pch.h", ROOT_DIR .. "RsaGen/RsaGen_pch/pch.cpp")

	includedirs {
	    "../../",
		"../../kernel/include/",
		"../../RsaGen/",
		"../../RsaGen/RsaGen_pch/",
    }
	
	-- 设置通用选项
    set_common_options("Size")
	
    -- files
    files {
        "../../RsaGen/**.h",
        "../../RsaGen/**.cpp",
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


-- ****************************************************************************


-- core library testsuite compile setting
project "toolbox"
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

    -- 导入内核接口 宏定义
	defines {"CRYSTAL_NET_CPP20", "CRYSTAL_NET_IMPORT_KERNEL_LIB", "CRYSTAL_NET_STATIC_KERNEL_LIB", "SIMPLE_API_IMPORT_KERNEL_LIB"}

	enable_precompileheader("pch.h", ROOT_DIR .. "toolbox/toolbox_pch/pch.cpp")

	includedirs {
	    "../../",
		"../../kernel/include/",
		"../../toolbox/",
		"../../toolbox/toolbox_pch/",
		"../../OptionComponent/",
		"../../protocols/cplusplus/",
    }

	-- 设置通用选项
    set_common_options()
	
    -- files
    files {
		-- "../../3rd/protobuf/include/**.h",
		-- "../../3rd/protobuf/include/**.cc",
		"../../protocols/**.h",
		"../../protocols/**.cc",
		"../../protocols/**.cpp",
        "../../toolbox/**.h",
        "../../toolbox/**.cpp",
        "../../service_common/**.h",
        "../../service_common/**.cpp",
        "../../OptionComponent/OptionComp/BehaviorTree/**.h",
        "../../OptionComponent/OptionComp/CodeAnalyze/**.cpp",
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
