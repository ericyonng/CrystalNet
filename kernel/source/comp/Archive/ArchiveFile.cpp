/*!
 *  MIT License
 *  
 *  Copyright (c) 2020 ericyonng<120453674@qq.com>
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 * 
 * Date: 2023-01-09 22:21:23
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Archive/ArchiveFile.h>
#include <kernel/comp/SmartPtr.h>
#include <kernel/comp/Utils/FileUtil.h>
#include <kernel/comp/Utils/ContainerUtil.h>
#include <kernel/comp/Log/log.h>
#include <3rd/3rdForKernel.h>


static inline void *miniz_alloc_func(void *opaque, size_t items, size_t size)
{
    (void)opaque, (void)items, (void)size;
    return g_MemoryPool->Alloc(static_cast<UInt64>(items * size));
}

static inline void miniz_free_func(void *opaque, void *address)
{
    (void)opaque, (void)address;
    return g_MemoryPool->Free(address);
}

static inline void *miniz_realloc_func(void *opaque, void *address, size_t items, size_t size)
{
    (void)opaque, (void)address, (void)items, (void)size;
    return g_MemoryPool->Realloc(address, static_cast<UInt64>(items * size));
}

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(ArchiveFile);

ArchiveFile::ArchiveFile()
:_init(false)
{

}

ArchiveFile::~ArchiveFile()
{
    Clear();
}

ArchiveFile::ArchiveFile(ArchiveFile &&other)
{
    _init = other._init;
    other._init = false;

    _path.Swap(other._path);
    _contents.swap(other._contents);
    _fileRefContent.swap(other._fileRefContent);
}


bool ArchiveFile::ExtractToMem(const LibString &archivePathName)
{
    if(UNLIKELY(_init))
        return true;

    SmartPtr<FILE, AutoDelMethods::CustomDelete> fp = FileUtil::OpenFile(archivePathName.c_str(), false, "rb");
    if(!fp)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("open archive file fail :%s"), archivePathName.c_str());
        return false;
    }

    fp.SetClosureDelegate([](void *ptr)
    {
        auto fpPtr = reinterpret_cast<FILE *>(ptr);
        FileUtil::CloseFile(*fpPtr);        
    });

    LibString content;
    FileUtil::ReadFile(*fp, content);

    mz_zip_archive curArchive;
    ::memset(&curArchive, 0, sizeof(curArchive));
    curArchive.m_pAlloc = &miniz_alloc_func;
    curArchive.m_pFree = &miniz_free_func;
    curArchive.m_pRealloc = &miniz_realloc_func;

    Int32 st = mz_zip_reader_init_mem(&curArchive, static_cast<const void*>(content.c_str()), content.size(), 0);
    if (st == 0)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("invalid zip file:%s, archive last error:%d"), archivePathName.c_str(), curArchive.m_last_error);
        return false;
    }

   SmartPtr<std::vector<Byte8 *>, AutoDelMethods::CustomDelete> contents = CRYSTAL_NEW(std::vector<Byte8 *>);
   contents.SetClosureDelegate([](void *ptr){
       auto vec = reinterpret_cast<std::vector<Byte8 *> *>(ptr);
       for (auto c : *vec)
           miniz_free_func(NULL, c);
   });
   std::unordered_map<LibString, std::pair<Byte8 *, UInt64>> fileRefContents;
   
    const Int32 count = static_cast<Int32>(mz_zip_reader_get_num_files(&curArchive));
    if(count > 0)
    {
        for (int i = 0; i < count; ++i)
        {
            mz_zip_archive_file_stat curFileStat;
            if (!mz_zip_reader_file_stat(&curArchive, i, &curFileStat))
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("mz_zip_reader_file_stat fail."));
                return false;
            }

            size_t curUncomSize = 0;
            void* p = mz_zip_reader_extract_file_to_heap(&curArchive, curFileStat.m_filename, &curUncomSize, 0);
            if (!p)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("memory allocation fail for file:%s"), curFileStat.m_filename);
                return false;
            }

            Byte8* new_p = reinterpret_cast<Byte8 *>(p);
            contents->push_back(new_p);
            fileRefContents[LibString(curFileStat.m_filename)] = std::make_pair(new_p, static_cast<UInt64>(curFileStat.m_uncomp_size));
        }
    }
    
    mz_zip_reader_end(&curArchive);

    // 有内容
    if(!contents->empty())
    {
        _contents.swap(*contents);
        _fileRefContent.swap(fileRefContents);
    }

    _init = true;
    _path = archivePathName;
    return true;
}

void ArchiveFile::Clear()
{
    if(UNLIKELY(!_init))
        return;

    _init = false;

    ContainerUtil::DelContainer(_contents, [](Byte8 *ptr){
        miniz_free_func(NULL, ptr);
    });

    _fileRefContent.clear();
    _path.clear();
}

KERNEL_END
