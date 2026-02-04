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
// Date: 2026-01-29 02:01:59
// Author: Eric Yonng
// Description:


#include <pch.h>
#include <kernel/comp/FileMonitor/FileChangeDefine.h>
#include <kernel/comp/Utils/ContainerUtil.h>

KERNEL_BEGIN


FileChangeHandle::FileChangeHandle()
    :_notListen{false}
,_data{NULL}
,_deserialize(NULL)
,_release(NULL)
{
        
}

FileChangeHandle::~FileChangeHandle()
{
    auto data = _data.exchange(NULL);
    if(data && _release)
    {
        _release->Invoke(data);
    }

    CRYSTAL_RELEASE_SAFE(_deserialize);
    CRYSTAL_RELEASE_SAFE(_release);
}

void FileChangeHandle::Release()
{
    delete this;
}

FileMonitorInfo::FileMonitorInfo()
    :_checkChange(NULL)
,_releaseObj(NULL)
,_loadNewObj(NULL)
,_sourceObj(NULL)
,_releaseFromMemory(NULL)
,_fromMemory(NULL)
{
    
}

FileMonitorInfo::~FileMonitorInfo()
{
    KERNEL_NS::ContainerUtil::DelContainer2(_keyRefFileChangeHandle);

    if(_sourceObj)
        _releaseObj->Invoke(_sourceObj);
    _sourceObj = NULL;
    CRYSTAL_RELEASE_SAFE(_releaseObj);

    if (_releaseFromMemory && _fromMemory)
        _releaseFromMemory->Invoke(_fromMemory);
    CRYSTAL_RELEASE_SAFE(_releaseFromMemory);
    _fromMemory = NULL;
    
    CRYSTAL_RELEASE_SAFE(_checkChange);
    CRYSTAL_RELEASE_SAFE(_loadNewObj);
}

void FileMonitorInfo::Release()
{
    delete this;
}


KERNEL_END