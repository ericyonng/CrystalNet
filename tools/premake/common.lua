--  @file   common.lua
-- #########################################################################
-- Global compile settings

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
    -- rdynamic是链接器选项
    if use_dynamic then
        filter { "language:c++", "system:not windows" }
        linkoptions {
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

    filter {}

    -- links(not windows)
    if USE_KERNEL_SO then
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
    else
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
    end

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

    -- 需要放在ssl, crypto, libcurl之后连接
    filter { "system:linux"}
        libdirs { 
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
