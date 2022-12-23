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
 * Date: 2022-12-22 13:08:00
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <service_common/config/xlsx/impl/Archive.h>

SERVICE_COMMON_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(Archive);

Archive::Archive(const KERNEL_NS::LibString &filePath)
{
    cache_flag = false;
    KERNEL_NS::SmartPtr<FILE, KERNEL_NS::AutoDelMethods::CustomDelete> fp = KERNEL_NS::FileUtil::OpenFile(filePath.c_str());
    if(!fp)
    {
        g_Log->Error(LOGFMT_OBJ_TAG("open archive file fail :%s"), filePath.c_str());
        return;
    }

    fp.SetClosureDelegate([](void *ptr)
    {
        auto fpPtr = reinterpret_cast<FILE *>(ptr);
        KERNEL_NS::FileUtil::CloseFile(*fpPtr);        
    });

    KERNEL_NS::LibString content;
    KERNEL_NS::FileUtil::ReadFile(*fp, content);
    
    mz_zip_archive cur_archive;
    ::memset(&cur_archive, 0, sizeof(cur_archive));

    int status = mz_zip_reader_init_mem(&cur_archive, static_cast<const void*>(content.c_str()), content.size(), 0);
    if (!status)
    {
        std::cerr << "invalid zip file " << file_path << " status " << cur_archive.m_last_error << std::endl;
        valid_flag = false;
        clear_resource();
        return;
    }

    for (int i = 0; i < (int)mz_zip_reader_get_num_files(&cur_archive); i++)
    {
        mz_zip_archive_file_stat cur_file_stat;
        if (!mz_zip_reader_file_stat(&cur_archive, i, &cur_file_stat))
        {
            std::cerr << "mz_zip_reader_file_stat failed" << std::endl;
            valid_flag = false;
            clear_resource();
            return;
        }
        //std::cout << " Filename " << cur_file_stat.m_filename << "comment " << cur_file_stat.m_comment <<
        //	" uncompressed size " << cur_file_stat.m_uncomp_size << " compressed size " << cur_file_stat.m_comp_size
        //	<< " is dir " << mz_zip_reader_is_file_a_directory(&cur_archive, i) << std::endl;

        size_t cur_uncom_size = 0;
        void* p = mz_zip_reader_extract_file_to_heap(&cur_archive, cur_file_stat.m_filename, &cur_uncom_size, 0);
        if (!p)
        {
            std::cerr << "memory allocation fail for file " << cur_file_stat.m_filename << std::endl;
            valid_flag = false;
            clear_resource();
            return;
        }
        valid_flag = true;
        //std::cout << "success extracted file " << cur_file_stat.m_filename << std::endl;
        char* new_p = (char*)p;
        archive_file_buffers.push_back(p);
        archive_content[string(cur_file_stat.m_filename)] = string_view(static_cast<char*>(p), cur_file_stat.m_uncomp_size);
    }
    mz_zip_reader_end(&cur_archive);
}
SERVICE_COMMON_END
