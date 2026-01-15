--  @file   premake5.lua
-- #########################################################################
-- Global compile settings

-- python tool define
IS_WINDOWS = string.match(_ACTION, 'vs') ~= nil
ISUSE_CLANG = _ARGS[1] and (_ARGS[1] == 'clang')

print('_ARGS:', _ARGS[1], _ARGS[2], _ARGS[3], ', ISUSE_CLANG:', ISUSE_CLANG)

-- header directory
-- All libraries output directory
OUTPUT_DIR = "../../output/" .. _ACTION
-- root directory
ROOT_DIR = "../../"
WIN_ROOT_DIR = ".\\..\\..\\"
-- build directory
BUILD_DIR = "../../build/"

-- debug dir
DEBUG_DIR = OUTPUT_DIR

ENABLE_PERFORMANCE_RECORD = 0

ENABLE_POLLER_PERFORMANCE = 0

ENABLE_TEST_SERVICE = 1

-----------------------------------------------------------------------------------------------------------

-- set common options
function set_common_options(optOption)

    -- rdynamic coredump符号 通知连接器所有符号添加到动态符号表中
    filter { "language:c++", "system:not windows" }
        -- buildoptions {
        --     "-std=c++11 -Winvalid-pch -DLINUX -Wall -rdynamic -fPIC -D_FILE_OFFSET_BITS=64 -D_GLIBCXX_USE_CXX11_ABI=1",
        -- }
        -- -Winvalid-pch是禁用pch加速, 需要移除
        buildoptions {
            --"-std=c++11 -DLINUX -Wall -rdynamic -fPIC -D_FILE_OFFSET_BITS=64 -D_GLIBCXX_USE_CXX11_ABI=1",
            "-std=c++11",
        }
    filter {}
end

-- lib include实现
function include_libfs(do_post_build, add_protobuflib)	
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
    
end

-- zlib library:
-- local ZLIB_LIB = "../../FS/3rd_party/zlib"
-- #########################################################################

workspace ("CrystalNet_" .. _ACTION)
    -- location define
    location (BUILD_DIR .. _ACTION)
    -- target directory define
    targetdir (OUTPUT_DIR)

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

-- ****************************************************************************
-- FS core library compile setting
project "test_static"
    -- language, kind
    language "c++"
    kind "StaticLib"
	
    -- symbols
	debugdir(DEBUG_DIR)
    symbols "On"
    
	-- 设置通用选项
    set_common_options()
	
	-- includedirs
    includedirs {
		"../../",
     }
	 
    -- files
    files {
        "../../test_static/**.h",
        "../../test_static/**.cpp",
    }

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
	
    -- debug target suffix define
    filter { "configurations:debug*" }
        targetsuffix "_debug"
    filter {}


-- ****************************************************************************


-- ****************************************************************************
-- FS core library compile setting
project "test_so"
    -- language, kind
    language "c++"
    kind "SharedLib"
	
    -- symbols
	debugdir(DEBUG_DIR)
    symbols "On"
    
	-- 设置通用选项
    set_common_options()
	
	-- includedirs
    includedirs {
		"../../",
     }
	 
    -- files
    files {
        "../../test_so/**.h",
        "../../test_so/**.cpp",
    }

    -- target prefix 前缀
    targetprefix "lib"

    libdirs { 
		ROOT_DIR .. "test_static/",
	}

    -- links
    filter { "system:linux" }
	    includedirs {
        "/usr/include/",
        "/usr/include/",
		}
        links {
            "rt",
			"pthread",
			"dl",
            "test_static:static",
        }
    filter {}
	
    -- debug target suffix define
    filter { "configurations:debug*" }
        targetsuffix "_debug"
    filter {}


-- ****************************************************************************
