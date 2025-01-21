--  @file   kernel_premake5.lua
-- #########################################################################
-- Global compile settings

-- ****************************************************************************
-- FS core library compile setting
project "CrystalKernel"
    -- language, kind
    language "c++"

    -- windows 下静态库比较合适(动态库windows下每个模块的堆空间都是独立的)
    if USE_KERNEL_SO then
        kind "SharedLib"
    else
        kind "StaticLib"
    end

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
            "libssl",
            "libcrypto",
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

