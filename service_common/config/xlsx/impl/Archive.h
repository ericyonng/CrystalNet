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
#ifndef __CRYSTAL_NET_SERVICE_COMMON_CONFIG_XLSX_IMPL_ARCHIVE_H__
#define __CRYSTAL_NET_SERVICE_COMMON_CONFIG_XLSX_IMPL_ARCHIVE_H__

#pragma once

#include <kernel/kernel.h>
#include <service_common/common/common.h>
#include <service_common/config/xml/xml.h>

SERVICE_COMMON_BEGIN

class Archive
{
    POOL_CREATE_OBJ_DEFAULT(Archive);
    
public:
    Archive(const KERNEL_NS::LibString &filePath);
    Archive &operator=(const Archive &rhs) = delete;
    Archive(const Archive &rhs) = delete;
    ~Archive();

    SERVICE_COMMON_NS::XMLDocument *get_xml_document(const std::string& doc_path);

    std::vector<sheet_desc> get_all_sheet_relation();
    
    std::vector<std::string> get_shared_string(); 
    void get_shared_string_view(spiritsaway::memory::arena& string_arena, std::vector<std::string_view>& view_vec);
    void clear_xml_document_cache();
    bool is_valid() const;
    bool get_cache_mode() const;
    void set_cache_mode(bool enable_cache);

private:
    void clear_resource();

private:
    // std::vector<void*> archive_file_buffers;
    
    bool _validFlag = false;
    bool _cacheFlag = false;
    std::unordered_map<KERNEL_NS::LibString, KERNEL_NS::LibString> _archiveContent;
    std::unordered_map<KERNEL_NS::LibString, SERVICE_COMMON_NS::XMLDocument> _xmlContent;
};

SERVICE_COMMON_END

#endif
