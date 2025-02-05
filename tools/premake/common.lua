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

function get_directory(path)
    -- 匹配最后一个路径分隔符（/ 或 \）之前的内容
    local dir = path:match("(.*[/\\])") 
    return dir or "."  -- 如果无分隔符，默认返回当前目录（.）
end

-- 构造包含路径
function build_include_paths(include_path, new_include_path)
    return include_path .. "-I" .. new_include_path .. " "
end

-- 构建pcm/ifc等缓存文件
-- @param(module_impl_file_extension):.cppm/.ixx 
-- @param(module_middle_file_extension):.pcm/.ifc
function build_cpp_modules(base_path, include_path,  is_windows)

    local compile_exe = ISUSE_CLANG and "clang" or "g++"

    local module_impl_file_extension = ".cppm"
    local module_middle_file_extension = ""
    if is_windows then
        module_middle_file_extension = ".ifc"
    else
        module_middle_file_extension = ".pcm"
    end

    -- 动态获取模块名称
    local module_files = os.matchfiles(base_path .. "/**" .. module_impl_file_extension)  -- 获取所有 .cppm 文件
    local modules = {}
    for _, file in ipairs(module_files) do
        local module_name = path.getbasename(file)  -- 提取模块名称（去掉路径和扩展名）
        table.insert(modules, module_name)
    end

    -- 自定义规则：预编译模块接口文件（生成 .pcm 文件）
    -- 注意：此规则需要根据编译器调整参数（示例为 GCC/Clang）
    for _, file in ipairs(module_files) do
        local module_name = path.getbasename(file)  -- 提取模块名称（去掉路径和扩展名）
        local path = get_directory(file)
        local module_file = path .. module_name .. module_impl_file_extension  -- 模块接口文件路径
        local pcm_file = module_name .. module_middle_file_extension  -- 生成的 .pcm/.ifc等中间文件 文件路径

        -- 定义预编译命令
        if is_windows then
            -- 定义预编译命令
            prebuildcommands {
                -- 预编译模块接口文件（生成 .ifc）
                "%{cfg.toolset.cxx} /std:c++latest /interface /c " .. module_file .. " /Fo " .. pcm_file
            }
        else
            prebuildcommands {
                -- 预编译模块接口文件（生成 .pcm）
                compile_exe .. " -std=c++20 -fmodules-ts " .. include_path .. " --precompile " .. module_file .. " -o " .. pcm_file
            }
        end

        -- 将 .pcm 文件添加到构建输出
        buildoutputs { pcm_file }
    end

    if is_windows then
        local options = {}
        for _, module in ipairs(modules) do
            table.insert(options, "/reference " .. module .. "=" .. module .. module_middle_file_extension)
        end
        -- 主程序编译时引用预编译模块
        -- 动态添加 /reference <module>=<module>.ifc
        filter { "files:**.cpp" }
            buildoptions { options }
    else
        local options = {}
        for _, module in ipairs(modules) do
            table.insert(options, "-fmodule-file=" .. module .. "=" .. module .. module_middle_file_extension)
        end
        -- 主程序编译时引用预编译模块
        -- 动态添加 -fmodule-file=<module>=<module>.pcm
        filter { "files:**.cpp" }
            buildoptions { options }
    end

    -- 清理时删除生成的 .pcm 文件
    -- postbuildcommands {
    --     "{DELETE} *" .. module_middle_file_extension
    -- }
end

function build_cpp_modules2(base_path, module_rule, include_path,  is_windows)
    local compile_exe = ISUSE_CLANG and "clang" or "g++"

    local module_impl_file_extension = ".cppm"
    local module_middle_file_extension = ""
    if is_windows then
        module_middle_file_extension = ".ifc"
    else
        module_middle_file_extension = ".pcm"
    end

    -- 动态获取模块名称
    local module_files = os.matchfiles(base_path .. "/**" .. module_impl_file_extension)  -- 获取所有 .cppm 文件
    local modules = {}
    for _, file in ipairs(module_files) do
        local module_name = path.getbasename(file)  -- 提取模块名称（去掉路径和扩展名）
        table.insert(modules, module_name)
    end

    -- 应用规则到 .cppm 文件
    filter { "files:**.cppm" }
        rules { module_rule }

    if is_windows then
        local options = {}
        for _, module in ipairs(modules) do
            table.insert(options, "/reference " .. module .. "=" .. module .. module_middle_file_extension)
        end
        -- 主程序编译时引用预编译模块
        -- 动态添加 /reference <module>=<module>.ifc
        filter { "files:**.cpp" }
            buildoptions { options }
    else
        local options = {}
        for _, module in ipairs(modules) do
            table.insert(options, "-fmodule-file=" .. module .. "=" .. module .. module_middle_file_extension)
        end
        -- 主程序编译时引用预编译模块
        -- 动态添加 -fmodule-file=<module>=<module>.pcm
        filter { "files:**.cpp" }
            buildoptions { options }
    end
end
