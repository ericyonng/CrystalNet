#pragma once

// 当前模块在导入接口
#ifdef SIMPLE_API_IMPORT_KERNEL_LIB
    // 动态库的导入
    #undef SIMPLE_API_EXPORT
    #ifdef _WIN32
        #define SIMPLE_API_EXPORT _declspec(dllimport)
    #else
        #define SIMPLE_API_EXPORT  __attribute__((__visibility__("default")))  // default 是默认导出符号（linux下）
    #endif
#else
    #undef SIMPLE_API_EXPORT
    #ifdef _WIN32
        #define SIMPLE_API_EXPORT _declspec(dllexport)
    #else
        #define SIMPLE_API_EXPORT __attribute__((__visibility__("default")))  // default 是默认导出符号（linux下）
    #endif
#endif

// #pragma warning(disable:4251) // 模版类造成的警告
// #if CRYSTAL_TARGET_PLATFORM_WINDOWS
//     #pragma warning(disable:4819) // 屏蔽文件中存在汉字时的编码警告
// #endif

#define D_SCL_SECURE_NO_WARNINGS // disable warning C4996