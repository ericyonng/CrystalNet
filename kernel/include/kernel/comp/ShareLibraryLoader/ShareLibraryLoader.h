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
// Date: 2025-01-22 01:01:36
// Author: Eric Yonng
// Description: 动态库加载器

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_SHARE_LIBRARY_LOADER_SHARE_LIBRARY_LOADER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_SHARE_LIBRARY_LOADER_SHARE_LIBRARY_LOADER_H__

#pragma once

#include <kernel/comp/CompObject/CompObject.h>

KERNEL_BEGIN

class KERNEL_EXPORT ShareLibraryLoader : public CompObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, ShareLibraryLoader);
    
public:
    ShareLibraryLoader();
    ~ShareLibraryLoader() override;
    void Release() override;

    Int32 Load(const LibString &libraryPath);
    void SetLibraryPath(const LibString &libraryPath);
    const LibString &GetLibraryPath() const;
    bool IsLoaded() const;

    // 加载符号
    template<typename SymType>
    SymType LoadSym(const LibString &symName);
    
private:
    Int32 _OnInit() override;
    Int32 _OnStart() override;
    void _OnClose() override;
    void _ClearRes();
    void _CloseLib();
    void *_LoadSym(const LibString &symName);
    
private:
    LibString _libPath;
    void *_library;
};

template<typename SymType>
ALWAYS_INLINE SymType ShareLibraryLoader::LoadSym(const LibString &symName)
{
    auto ptr = _LoadSym(symName);
    return SymType(ptr);
}

ALWAYS_INLINE const LibString &ShareLibraryLoader::GetLibraryPath() const
{
    return _libPath;
}

ALWAYS_INLINE bool ShareLibraryLoader::IsLoaded() const
{
    return !_library;
}

KERNEL_END

#endif
