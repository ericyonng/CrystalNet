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
// Date: 2026-02-02 23:01:08
// Author: Eric Yonng
// Description:

#include "pch.h"
#include <kernel/comp/FileMonitor/YamlMemory.h>
#include <kernel/comp/Log/log.h>

#include "kernel/comp/Coder/base64.h"
#include "kernel/comp/Encrypt/LibDigest.h"

KERNEL_BEGIN
    
void YamlMemoryData::Release()
{
    YamlMemoryData::Delete_YamlMemoryData(this);
}


YamlMemory::YamlMemory()
    :_source{NULL}
{
    
}

YamlMemory::~YamlMemory()
{
    if (auto src = _source.exchange(NULL))
    {
        src->Release();
    }
}

void YamlMemory::Release()
{
    YamlMemory::Delete_YamlMemory(this);
}

YamlMemoryData* YamlMemory::CheckAndChange()
{
    auto src = _source.load(std::memory_order_acquire);
    while (!_source.compare_exchange_weak(src, NULL, std::memory_order_acq_rel))
    {
        
    }

    if (src)
    {
        if (src->_version != _currentVersion)
        {
            if (g_Log && g_Log->IsEnable(LogLevel::Info))
                g_Log->Info(LOGFMT_OBJ_TAG("yaml memory changed version:%s, old version:%s"), src->_version.c_str(), _currentVersion.c_str());

            _currentVersion = src->_version;
            return src;
        }

        src->Release();
    }

    return NULL;
}

void YamlMemory::SetNewData(YamlMemoryData *data)
{
    if (UNLIKELY(data->_version.empty()))
    {
        data->_version = KERNEL_NS::LibDigest::MakeSha256(data->_data);
        if (g_Log && g_Log->IsEnable(LogLevel::Warn))
            g_Log->Warn(LOGFMT_OBJ_TAG("version should not be empty data:%p, version:%s(after base64)"), data, KERNEL_NS::LibBase64::Encode(data->_version).c_str());
    }
    
    auto oldData = _source.exchange(data, std::memory_order_acq_rel);
    if (UNLIKELY(oldData == data))
    {
        if (g_Log && g_Log->IsEnable(LogLevel::Error))
            g_Log->Error(LOGFMT_OBJ_TAG("set same memory data:%p"), data);
        return;
    }

    if (g_Log && g_Log->IsEnable(LogLevel::Info))
        g_Log->Info(LOGFMT_OBJ_TAG("set new memory data:%p, oldData:%p"), data, oldData);

    if (oldData)
        oldData->Release();
}

YamlMemory *YamlMemory::From(const LibString &content)
{
    auto memory = YamlMemory::New_YamlMemory();
    auto data = YamlMemoryData::New_YamlMemoryData();
    data->_data = content;
    data->_version = KERNEL_NS::LibDigest::MakeSha256(data->_data);
    memory->_source.exchange(data, std::memory_order_acq_rel);
    return memory;
}



KERNEL_END