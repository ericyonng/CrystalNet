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
OUTPUT_DIR = "../../output/" .. _ACTION
-- root directory
ROOT_DIR = "../../"
WIN_ROOT_DIR = ".\\..\\..\\"
-- build directory
BUILD_DIR = "../../build_tools/"

-- script path
SCRIPT_PATH = ROOT_DIR .. "scripts/build_tools/"
if IS_WINDOWS then
    SCRIPT_PATH = ".\\\\..\\\\..\\\\" .. "scripts\\build_tools\\"
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
function set_common_options(optOption)
    if ISUSE_CLANG then
        toolset("clang")
    end

    -- rdynamic coredump符号 通知连接器所有符号添加到动态符号表中
    filter { "language:c++", "system:not windows" }
        -- buildoptions {
        --     "-std=c++11 -Winvalid-pch -DLINUX -Wall -rdynamic -fPIC -D_FILE_OFFSET_BITS=64 -D_GLIBCXX_USE_CXX11_ABI=1",
        -- }
        -- -Winvalid-pch是禁用pch加速, 需要移除
        buildoptions {
            --"-std=c++11 -DLINUX -Wall -rdynamic -fPIC -D_FILE_OFFSET_BITS=64 -D_GLIBCXX_USE_CXX11_ABI=1",
            "-DLINUX -Wall -fPIC -D_FILE_OFFSET_BITS=64 -D_GLIBCXX_USE_CXX11_ABI=1",
        }
    filter {}
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

    -- 工具的kernel使用静态库
	defines { "CRYSTAL_NET_STATIC_KERNEL_LIB" }

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
	defines { "CURL_STATICLIB"}

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
            "CrystalKernel_debug:static",
            "uuid_debug:static",
            "lua:static",
        }
    filter {}
    filter { "system:not windows", "configurations:release*" }
        links {
            "CrystalKernel:static",
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

    -- 工具的kernel使用静态库
    kind "StaticLib"

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
            "curl:static",
            "ssl:static",
            "crypto:static",
            "uuid_debug:static",
            "miniz:static",
            "idn2:static",
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
            "curl:static",
            "ssl:static",
            "crypto:static",
            "uuid:static",
            "miniz:static",
            "idn2:static",
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
