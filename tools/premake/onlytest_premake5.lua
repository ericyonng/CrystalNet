--  @file   premake5.lua
-- #########################################################################
-- Global compile settings

-- python tool define
IS_WINDOWS = string.match(_ACTION, 'vs') ~= nil
-- header directory
KERNEL_HEADER_DIR = "../../kernel/kernel_pch/"
-- All libraries output directory
OUTPUT_DIR = "../../output/" .. _ACTION
-- root directory
ROOT_DIR = "../../"
WIN_ROOT_DIR = ".\\..\\..\\"
-- build directory
BUILD_DIR = "../../build/"

-- debug dir
DEBUG_DIR = OUTPUT_DIR

ENABLE_PERFORMANCE_RECORD = 1

ENABLE_POLLER_PERFORMANCE = 0

ENABLE_TEST_SERVICE = 0

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
        -- pchsource(source_file or "pch.cpp")
        -- pchheader(header_file or "pch.h")
        buildoptions { "/Zm1000" }
    filter {}

    filter { "system:not windows" }
        -- pchheader(header_file or "pch.h")
    filter {}
end

-- Set optimize options.
function set_optimize_opts()

end

-- set common options
function set_common_options()
    -- rdynamic coredump符号
    filter { "language:c++", "system:not windows" }
        -- buildoptions {
        --     "-std=c++11 -Winvalid-pch -DLINUX -Wall -rdynamic -fPIC -D_FILE_OFFSET_BITS=64 -D_GLIBCXX_USE_CXX11_ABI=1",
        -- }
        -- -Winvalid-pch是禁用pch加速, 需要移除
        buildoptions {
            "-std=c++11 -DLINUX -Wall -rdynamic -fPIC -D_FILE_OFFSET_BITS=64 -D_GLIBCXX_USE_CXX11_ABI=1",
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
        optimize "Debug"
    filter {}

    filter { "configurations:release*" }
        optimize "Speed"
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
    }

	libdirs { 
		ROOT_DIR .. "3rd/openssL/staticlib/$(Configuration)/lib/",
        ROOT_DIR .. "3rd/",
        ROOT_DIR .. "3rd/kernel/",
		ROOT_DIR .. "/3rd/protobuf/lib/",
		ROOT_DIR .. "3rd/miniz/libs/$(Configuration)/",
		ROOT_DIR .. "3rd/uuid/libs",
	}

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
            "libCrystalKernel_debug"
        }
    filter {}
    filter { "system:windows", "configurations:release*" }
        libdirs{
        }
        links {
            -- "ws2_32",
            "libCrystalKernel"
        }
    filter {}

    -- openssl crystalkernel静态库依赖openssl 必须把crystalkernel放在crypto, ssl之前连接
    filter { "system:linux", "configurations:debug*"}
        libdirs { 
            ROOT_DIR .. "/3rd/openssl/linux/lib/debug/",
        }
        links {
            "crypto:static",
            "ssl:static",
        }
    filter {}
    filter { "system:linux", "configurations:release*"}
        libdirs { 
            ROOT_DIR .. "/3rd/openssl/linux/lib/release/",
        }
        links {
            "crypto:static",
            "ssl:static",
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
        links {
            "protobufd:static",
            "miniz:static",
        }
        filter {}
        filter { "system:not windows", "configurations:release*" }
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

workspace ("CrystalNetOnlyTest_" .. _ACTION)
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
    filter {}
	
    filter { "configurations:release*" }
        defines {
            "NDEBUG"
        }
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

-- ****************************************************************************
-- FS core library compile setting
project "CrystalKernel"
    -- language, kind
    language "c++"
    kind "StaticLib"
	
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

	defines { "CRYSTAL_NET_STATIC_KERNEL_LIB" }
    libdirs { 
        ROOT_DIR .. "3rd/openssL/staticlib/$(Configuration)/lib/",
		ROOT_DIR .. "3rd/miniz/libs/$(Configuration)/",
        ROOT_DIR .. "3rd/",
    }

    -- openssl
    filter { "system:linux", "configurations:debug*"}
        libdirs { 
            ROOT_DIR .. "/3rd/openssl/linux/lib/debug/",
            ROOT_DIR .. "3rd/uuid/libs/",
        }
        links {
            "crypto:static",
            "ssl:static",
            "uuid_debug:static",
            "miniz:static",
        }
    filter {}
    filter { "system:linux", "configurations:release*"}
        libdirs { 
            ROOT_DIR .. "/3rd/openssl/linux/lib/release/",
            ROOT_DIR .. "3rd/uuid/libs/",
        }
        links {
            "crypto:static",
            "ssl:static",
            "uuid:static",
            "miniz:static",
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
	postbuildcommands(string.format("sh %sbuilding.sh debug",  ROOT_DIR))
	filter {}
	
	-- post build(linux)
	filter { "system:linux", "configurations:release*"}
	postbuildmessage "Copying dependencies of crystalnet kernel ..."
	postbuildcommands(string.format("sh %sbuilding.sh release",  ROOT_DIR))
	filter {}

	-- post build(windows)
	filter { "system:windows", "configurations:debug*"}
	postbuildmessage "Copying dependencies of crystalnet kernel ..."
	postbuildcommands(string.format("start %sbuilding.bat debug %s",  WIN_ROOT_DIR, "output\\" .. _ACTION))
	filter {}
	
	-- post build(windows)
	filter { "system:windows", "configurations:release*"}
	postbuildmessage "Copying dependencies of crystalnet kernel ..."
	postbuildcommands(string.format("start %sbuilding.bat release %s",  WIN_ROOT_DIR, "output\\" .. _ACTION))
	filter {}

-- ****************************************************************************


-- core library testsuite compile setting
project "testsuit_onlytest"
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
	defines { "CRYSTAL_NET_IMPORT_KERNEL_LIB", "CRYSTAL_NET_STATIC_KERNEL_LIB", "SIMPLE_API_IMPORT_KERNEL_LIB", "CRYSTAL_STORAGE_ENABLE"}

	enable_precompileheader("pch.h", ROOT_DIR .. "testsuit/testsuit_pch/pch.cpp")

	includedirs {
	    "../../",
		"../../kernel/include/",
		"../../testsuit/",
		"../../testsuit/testsuit_pch/",
		"../../3rd/mysql/win/include/",
		"../../OptionComponent/",
		"../../protocols/cplusplus/",
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
    set_common_options()
	
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
        postbuildcommands(string.format("start %srunfirstly_scripts_onlytest.bat %s", WIN_ROOT_DIR, _ACTION))
    filter {}
	