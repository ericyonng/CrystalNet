// MIT License
// 
// Copyright (c) 2020 ericyonng<120453674@qq.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// Date: 2025-01-22 13:10:36
// Author: Eric Yonng
// Description:

#include <pch.h>
#include <kernel/comp/ShareLibraryLoader/ShareLibraryLoader.h>
#include <kernel/comp/ShareLibraryLoader/ShareLibraryLoaderFactory.h>

#include "kernel/comp/Log/ILog.h"
#include "kernel/comp/Utils/FileUtil.h"

#if CRYSTAL_TARGET_PLATFORM_LINUX
 #include <dlfcn.h>
#endif

KERNEL_BEGIN

ShareLibraryLoader::ShareLibraryLoader()
:CompObject(RttiUtil::GetTypeId<ShareLibraryLoader>())
,_library(NULL)
{
    
}

ShareLibraryLoader::~ShareLibraryLoader()
{
    _ClearRes();
}

void ShareLibraryLoader::Release()
{
    ShareLibraryLoader::DeleteByAdapter_ShareLibraryLoader(ShareLibraryLoaderFactory::_buildType.V, this);
}

Int32 ShareLibraryLoader::Load(const LibString &libraryPath)
{
    // 文件是否存在
    if(!KERNEL_NS::FileUtil::IsFileExist(libraryPath.c_str()))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("library path:%s, not exists."), libraryPath.c_str());
        return Status::NotFound;
    }

    void *newLibrary = NULL;
#if CRYSTAL_TARGET_PLATFORM_LINUX
    newLibrary = dlopen(libraryPath.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if(!newLibrary)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("dlopen fail err:%s, library path:%s."), dlerror(), libraryPath.c_str());
        return Status::LoadShareLibraryFail;
    }
#endif

    g_Log->Info(LOGFMT_OBJ_TAG("load library success, old:%s,%p => new:%s,%p"), _libPath.c_str(), _library, libraryPath.c_str(), newLibrary);

    // 关闭旧的库
    _CloseLib();

    _library = newLibrary;
    _libPath = libraryPath;

    return Status::Success;
}

void ShareLibraryLoader::SetLibraryPath(const LibString &libraryPath)
{
    g_Log->Info(LOGFMT_OBJ_TAG("Old Library:%s,%p => New Library:%s")
        , _libPath.c_str(), _library, libraryPath.c_str());

    _libPath = libraryPath;
}

Int32 ShareLibraryLoader::_OnInit()
{
    return Status::Success;
}

Int32 ShareLibraryLoader::_OnStart()
{
    if(!_library && !_libPath.empty())
    {
        const auto err = Load(_libPath);
        if(err != Status::Success)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("load library fail:%d"), err);
            return err;
        }
    }

    return Status::Success;
}

void ShareLibraryLoader::_OnClose()
{
    _ClearRes();
}

void ShareLibraryLoader::_ClearRes()
{
    _CloseLib();

    _libPath.clear();
}

void ShareLibraryLoader::_CloseLib()
{
    if(!_library)
        return;

    g_Log->Info(LOGFMT_OBJ_TAG("close lib:%s,%p"), _libPath.c_str(), _library);
    
#if CRYSTAL_TARGET_PLATFORM_LINUX
    auto err = dlclose(_library);
    if(err != 0)
    {
        auto errStr = dlerror();
        g_Log->Error(LOGFMT_OBJ_TAG("dlclose library:%s,%p, fail:%d(%s)."), _libPath.c_str(), _library, err, errStr ? errStr : "unknown fail");
    }
#endif

    _library = NULL;
}

void *ShareLibraryLoader::_LoadSym(const LibString &symName)
{
    if(UNLIKELY(!_library))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("share library not loaded before, lib path:%s, symName:%s")
            , _libPath.c_str(), symName.c_str());
        
        return NULL;
    }

#if CRYSTAL_TARGET_PLATFORM_LINUX
    auto symPtr = dlsym(_library, symName.c_str());
    if(UNLIKELY(!symPtr))
    {
        auto errStr = dlerror();
        g_Log->Error(LOGFMT_OBJ_TAG("dlsym fail err:%s, library:%s,%p, symName:%s")
            ,errStr, _libPath.c_str(), _library, symName.c_str());
        return NULL;
    }

    g_Log->Info(LOGFMT_OBJ_TAG("share library:%s,%p, load sym :%s, success."), _libPath.c_str(), _library, symName.c_str());
    return symPtr;
#else

    g_Log->Info(LOGFMT_OBJ_TAG("share library:%s,%p, windows cant load sym :%s"), _libPath.c_str(), _library, symName.c_str());

    return NULL;
#endif
}

LibString ShareLibraryLoader::ToString() const
{
    return LibString().AppendFormat("lib path:%s, library address:%p", _libPath.c_str(), _library);
}

KERNEL_END