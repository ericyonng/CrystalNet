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
// Date: 2026-06-06 20:06:38
// Author: Eric Yonng
// Description:

#include <pch.h>
#include <Comps/Plugin/Impl/PluginDelayRemoveInfo.h>
#include <kernel/comp/ShareLibraryLoader/ShareLibraryLoader.h>
#include <TestServicePlugin/TestServicePlugin.h>
#include <kernel/comp/Log/log.h>

SERVICE_BEGIN

PluginDelayRemoveInfo::PluginDelayRemoveInfo()
    :_shareLibraryLoader(NULL)
    ,_pluginGlobal(NULL)

{
    
}

PluginDelayRemoveInfo::~PluginDelayRemoveInfo()
{
    _pluginGlobal = NULL;
    try
    {
        if(_shareLibraryLoader)
        {
            auto willClosePtr = _shareLibraryLoader->LoadSym<WillClosePluginPtr>(KERNEL_NS::LibString("WillClosePlugin"));
            if(willClosePtr)
            {
                CLOG_INFO("will close plugin...");
                (*willClosePtr)();
            }

            auto closePtr = _shareLibraryLoader->LoadSym<ClosePluginPtr>(KERNEL_NS::LibString("ClosePlugin"));
            if(closePtr)
            {
                CLOG_INFO("close plugin...");
                (*closePtr)();
            }

            _shareLibraryLoader->WillClose();
            _shareLibraryLoader->Close();
            _shareLibraryLoader->Release();
        }
    }
    catch (std::exception &e)
    {
        CLOG_ERROR("exception in PluginDelayRemoveInfo::PluginDelayRemoveInfo e:%s", e.what());
    }
    catch (...)
    {
        CLOG_ERROR("unknown exception in PluginDelayRemoveInfo::PluginDelayRemoveInfo");
    }
}

void PluginDelayRemoveInfo::Release()
{
    PluginDelayRemoveInfo::DeleteThreadLocal_PluginDelayRemoveInfo(this);
}

SERVICE_END
