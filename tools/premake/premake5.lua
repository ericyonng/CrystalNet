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
    SCRIPT_PATH = ".\\\\..\\\\..\\\\" .. "scripts\\builds\\"
end

-- debug dir
DEBUG_DIR = OUTPUT_DIR

ENABLE_PERFORMANCE_RECORD = 0

ENABLE_POLLER_PERFORMANCE = 0

ENABLE_TEST_SERVICE = 1

-- 预编译
ENABLE_PRECOMPILE_HEADER = 1

-----------------------------------------------------------------------------------------------------------

-- Common functional functions define
-- Enable multithread compile
function enable_multithread_comp(cppstdver)
    filter { "system:windows" }
        flags { "MultiProcessorCompile", "NoMinimalRebuild", cppstdver }
    filter {}
end


-- 启用预编译头文件机制
function enable_precompileheader(header_file, source_file)
    filter { "system:windows" }
        if ENABLE_PRECOMPILE_HEADER ~= 0 then
            pchsource(source_file or "pch.cpp")
            pchheader(header_file or "pch.h")
            buildoptions { "/Zm1000" }
        end

    filter {}

    filter { "system:not windows" }
        if ENABLE_PRECOMPILE_HEADER ~= 0 then
            pchheader(header_file or "pch.h")
        end

    filter {}
end

-- Set optimize options.
function set_optimize_opts()

end

-- set common options
function set_common_options(optOption, use_dynamic)
    if ISUSE_CLANG then
        toolset("clang")
    end

    -- rdynamic coredump符号 通知连接器所有符号添加到动态符号表中
    filter { "language:c++", "system:not windows" }
        -- buildoptions {
        --     "-std=c++11 -Winvalid-pch -DLINUX -Wall -rdynamic -fPIC -D_FILE_OFFSET_BITS=64 -D_GLIBCXX_USE_CXX11_ABI=1",
        -- }
        -- -Winvalid-pch是禁用pch加速, 需要移除,rdynamic是动态库必须的, 可以获取符号
        buildoptions {
            --"-std=c++11 -DLINUX -Wall -rdynamic -fPIC -D_FILE_OFFSET_BITS=64 -D_GLIBCXX_USE_CXX11_ABI=1",
            "-DLINUX -Wall -fPIC -D_FILE_OFFSET_BITS=64 -D_GLIBCXX_USE_CXX11_ABI=1",
        }
    filter {}
    if use_dynamic then
        filter { "language:c++", "system:not windows" }
        buildoptions {
            "-rdynamic",
        }
        filter {}
    end

	filter { "configurations:debug*", "language:c++", "system:not windows" }
        buildoptions {
            "-ggdb -g",
        }
    filter {}
	filter { "configurations:debug*", "language:c++", "system:windows" }
        runtime "Debug"
        optimize "Debug"
    filter {}

    filter { "configurations:debug*", "language:not c++" }
        optimize (optOption and optOption or "Debug")
    filter {}

    filter { "configurations:release*" }
        optimize (optOption and optOption or "Speed")
    filter {}

    if ENABLE_PERFORMANCE_RECORD ~= 0 then
        defines("CRYSTAL_NET_PORFORMANCE_RECORD")
    end
    if ENABLE_POLLER_PERFORMANCE ~= 0 then
        defines("ENABLE_POLLER_PERFORMANCE")
    end
    if ENABLE_TEST_SERVICE ~= 0 then
        defines("ENABLE_TEST_SERVICE")
    end
    filter { "system:windows" }
	    defines { "CRYSTAL_NET_STATIC_KERNEL_LIB" }
    filter {}

	filter {"language:c++", "system:windows" }
        defines("_WINSOCK_DEPRECATED_NO_WARNINGS")
    filter {}
end

-- lib include实现
function include_libfs(do_post_build, add_protobuflib)	
    -- includedirs
    includedirs {
        ROOT_DIR .. "/kernel/include/",
        -- openssl
		ROOT_DIR .. "/3rd/openssl/include/",
		ROOT_DIR .. "/3rd/protobuf/include/",
		ROOT_DIR .. "/3rd/miniz/include/",
		ROOT_DIR .. "/3rd/uuid/include/",
		ROOT_DIR .. "/3rd/json/include/",
		ROOT_DIR .. "/3rd/curl/include/",
		ROOT_DIR .. "3rd/lua/include/",
    }

	libdirs { 
		ROOT_DIR .. "3rd/openssL/staticlib/$(Configuration)/lib/",
        ROOT_DIR .. "3rd/",
        ROOT_DIR .. "3rd/kernel/",
		ROOT_DIR .. "/3rd/protobuf/lib/",
		ROOT_DIR .. "3rd/miniz/libs/$(Configuration)/",
		ROOT_DIR .. "3rd/uuid/libs",
		ROOT_DIR .. "3rd/curl/lib/$(Configuration)/",
		ROOT_DIR .. "3rd/curl/lib/$(Configuration)/",
		ROOT_DIR .. "3rd/lua/",
	}

    -- 使用curl静态库
	defines { "CURL_STATICLIB" }

    -- files
    -- files {
    --    FS_ROOT_DIR .. "/frame/include/**.h",
    -- }

    -- libdirs(linux) linux 下的库需要指明静态库或者动态库
    filter { "system:linux"}
		includedirs {
        "/usr/include/",
		}
        libdirs {
            ROOT_DIR .. "/usr/lib64/",
        }
		links {
		    "rt",
			"pthread",
            "dl",
        }
    filter {}

    -- links(not windows)
    filter { "system:not windows", "configurations:debug*" }
        links {
            "CrystalKernel_debug",
            "uuid_debug:static",
            "lua:static",
        }
    filter {}
    filter { "system:not windows", "configurations:release*" }
        links {
            "CrystalKernel",
            "uuid:static",
        }
    filter {}

    -- links(windows)
    filter { "system:windows", "configurations:debug*" }
        libdirs{
        }
        links {
            -- "ws2_32",
            "libCrystalKernel_debug",
            "lua5.4.7-static"
        }
    filter {}
    filter { "system:windows", "configurations:release*" }
        libdirs{
        }
        links {
            -- "ws2_32",
            "libCrystalKernel",
            "lua5.4.7-static"
        }
    filter {}

    -- curl 必须在openssl库, libz.so之前 linux连接是严格顺序的
    filter { "system:linux", "configurations:debug*"}
        libdirs { 
		    ROOT_DIR .. "3rd/curl/lib/debug/",

        }
        links {
            "curl:static",
        }
    filter {}
    filter { "system:linux", "configurations:release*"}
        libdirs { 
		    ROOT_DIR .. "3rd/curl/lib/release/",
        }
        links {
            "curl:static",
        }
    filter {}

    -- 需要放在libcurl之后连接
    filter { "system:linux"}
        libdirs { 
            ROOT_DIR .. "/usr/lib64/",
		    ROOT_DIR .. "/3rd/idn2/lib/",
        }
        links {
            "z",
            "idn2:static"
        }
    filter {}

    -- openssl crystalkernel静态库依赖openssl 必须把crystalkernel放在crypto, ssl之前连接
    filter { "system:linux", "configurations:debug*"}
        libdirs { 
            ROOT_DIR .. "/3rd/openssl/linux/lib/debug/",
        }
        links {
            "ssl:static",
            "crypto:static",
        }
    filter {}
    filter { "system:linux", "configurations:release*"}
        libdirs { 
            ROOT_DIR .. "/3rd/openssl/linux/lib/release/",
        }
        links {
            "ssl:static",
            "crypto:static",
        }
    filter {}

    filter { "system:windows"}
        links {
            -- "ws2_32",
            "libcurl"
        }
    filter {}

    if add_protobuflib then
        filter { "system:windows", "configurations:debug*" }
            links {
                "libprotobufd",
                "miniz",
            }
        filter {}
        filter { "system:windows", "configurations:release*" }
            links {
                "libprotobuf",
                "miniz",
            }
        filter {}

        filter { "system:not windows", "configurations:debug*" }
        libdirs { 
            ROOT_DIR .. "3rd/miniz/libs/debug/",
        }
        links {
            "protobufd:static",
            "miniz:static",
        }
        filter {}
        filter { "system:not windows", "configurations:release*" }
            libdirs { 
                ROOT_DIR .. "3rd/miniz/libs/release/",
            }
            links {
                "protobuf:static",
                "miniz:static",
            }
        filter {}
    end
    
end

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
-- FS core library compile setting
project "CrystalKernel"
    -- language, kind
    language "c++"

    -- windows 下静态库
    filter { "system:windows" }
        kind "StaticLib"
    filter{}

    -- linux 下动态库, 为了热更
    filter { "system:linux"}
        kind "SharedLib"
    filter{}

    -- 支持c++20
    -- cppdialect "c++20"

    -- symbols
	debugdir(DEBUG_DIR)
    symbols "On"
    
	enable_precompileheader("pch.h", KERNEL_HEADER_DIR .. "pch.cpp")

	-- 设置通用选项
    set_common_options()
	
	-- includedirs
    includedirs {
		"../../",
		"../../kernel/include/",
		"../../kernel/kernel_pch/",
        ROOT_DIR .. "/3rd/openssl/include/",
        ROOT_DIR .. "/3rd/uuid/include/",
		ROOT_DIR .. "/3rd/miniz/include/",
		ROOT_DIR .. "/3rd/json/include/",
		ROOT_DIR .. "/3rd/curl/include/",
		ROOT_DIR .. "/3rd/idn2/include/",
		ROOT_DIR .. "3rd/lua/include/",
     }
	 
    -- files
    files {
        "../../kernel/**.h",
		"../../kernel/**.c",
		"../../kernel/**.cpp",
		"../../3rd/*.h",
		"../../3rd/**.hpp",
		--"../../3rd/tiny-utf8/lib/*.cpp",
    }

    -- 使用curl静态库 linux 下没有$(Configuration)替换, 只有windows才有
	defines { "CURL_STATICLIB" }
	defines { "CRYSTAL_NET_CPP20"}

    libdirs { 
        ROOT_DIR .. "3rd/openssL/staticlib/$(Configuration)/lib/",
		ROOT_DIR .. "3rd/miniz/libs/$(Configuration)/",
        ROOT_DIR .. "3rd/",
		ROOT_DIR .. "3rd/curl/lib/$(Configuration)/",
		ROOT_DIR .. "3rd/lua/",
    }

    -- openssl
    filter { "system:linux", "configurations:debug*"}
        libdirs { 
            ROOT_DIR .. "/3rd/openssl/linux/lib/debug/",
            ROOT_DIR .. "3rd/uuid/libs/",
		    ROOT_DIR .. "3rd/miniz/libs/debug/",
    		ROOT_DIR .. "3rd/curl/lib/debug/",
    		ROOT_DIR .. "3rd/idn2/lib/",
        }
        links {
            "crypto:static",
            "ssl:static",
            "uuid_debug:static",
            "miniz:static",
            "idn2:static",
            "curl:static",
            "lua:static",
        }
    filter {}
    filter { "system:linux", "configurations:release*"}
        libdirs { 
            ROOT_DIR .. "/3rd/openssl/linux/lib/release/",
            ROOT_DIR .. "3rd/uuid/libs/",
		    ROOT_DIR .. "3rd/miniz/libs/release/",
    		ROOT_DIR .. "3rd/curl/lib/release/",
    		ROOT_DIR .. "3rd/idn2/lib/",
        }
        links {
            "crypto:static",
            "ssl:static",
            "uuid:static",
            "miniz:static",
            "idn2:static",
            "curl:static",
            "lua:static",
        }
    filter {}
	
	-- macos需要额外添加
    filter { "system:macosx" }
    files {
        "../../kernel/**.mm",
    }
    filter {}

    -- target prefix 前缀
    targetprefix "lib"

    -- links
    filter { "system:linux" }
	    includedirs {
        "/usr/include/",
		}
        links {
            "rt",
			"pthread",
			"dl",
        }

    filter { "system:windows" }
        links {
            "ws2_32",
            "Mswsock",
            "DbgHelp",
            "miniz",
            "Crypt32",
            "libcrypto",
            "libssl",
            "shlwapi",
            "Iphlpapi",
            "libcurl",
            "lua5.4.7-static"
        }

    filter { "system:macosx" }
        links {
            "iconv",
        }
    filter {}

    -- flags 移除c++17标志
--    filter { "system:not windows" }
--        buildoptions {
--            "-fvisibility=hidden -std=c++17",
--        }
--    filter {}
    -- hidden是隐藏符号，符号不会导出
    -- filter { "system:not windows" }
    --     buildoptions {
    --         "-fvisibility=hidden",
    --     }
    -- filter {}
	
    -- optimize
    set_optimize_opts()
	
	enable_multithread_comp()

    -- debug target suffix define
    filter { "configurations:debug*" }
        targetsuffix "_debug"
    filter {}

    -- enable multithread compile
    -- enable_multithread_comp("C++14")
	
    -- post prebuild(linux)
    filter { "system:linux"}
	prebuildmessage "Merge partition files ..."
	prebuildcommands(string.format("sh %smerge_files.sh",  ROOT_DIR))
	filter {}

    filter { "system:windows"}
	prebuildmessage "Merge partition files ..."
	prebuildcommands(string.format("start %smerge_files.bat",  WIN_ROOT_DIR))
	filter {}

	-- post build(linux)
	filter { "system:linux", "configurations:debug*"}
	postbuildmessage "Copying dependencies of crystalnet kernel ..."
	postbuildcommands(string.format("sh %sbuilding.sh debug",  SCRIPT_PATH))
	filter {}
	
	-- post build(linux)
	filter { "system:linux", "configurations:release*"}
	postbuildmessage "Copying dependencies of crystalnet kernel ..."
	postbuildcommands(string.format("sh %sbuilding.sh release",  SCRIPT_PATH))
	filter {}

	-- post build(windows)
	filter { "system:windows", "configurations:debug*"}
	postbuildmessage "Copying dependencies of crystalnet kernel ..."
	postbuildcommands(string.format("start %sbuilding.bat debug %s",  SCRIPT_PATH, "output\\" .. _ACTION))
	filter {}
	
	-- post build(windows)
	filter { "system:windows", "configurations:release*"}
	postbuildmessage "Copying dependencies of crystalnet kernel ..."
	postbuildcommands(string.format("start %sbuilding.bat release %s",  SCRIPT_PATH, "output\\" .. _ACTION))
	filter {}

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
            "../../testsuit/**.cppm",
            "../../testsuit/**.cpp",
            "../../testsuit/**.lua",
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
	defines { "CRYSTAL_NET_IMPORT_KERNEL_LIB", "CRYSTAL_NET_STATIC_KERNEL_LIB"}

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
	defines { "CRYSTAL_NET_IMPORT_KERNEL_LIB", "CRYSTAL_NET_STATIC_KERNEL_LIB"}

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
