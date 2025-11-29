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
// Date: 2025-01-23 00:01:58
// Author: Eric Yonng
// Description:

#include <pch.h>
#include <TestServicePlugin/PluginEntry.h>
#include <TestServicePlugin/ExternPluginMgr.h>

extern "C"
{
    struct PluginWrap
    {
        Int32 _id = 0;
    };
    
    // 初始化插件集
    Int32 InitPlugin()
    {
        PluginWrap pluginWrap;
        UNUSED(pluginWrap);
        UNUSED(pluginWrap._id);
        
        g_Log->Info(LOGFMT_NON_OBJ_TAG(PluginWrap, "Plugin init success g_PluginMgr:%p %s."), g_PluginMgr, g_PluginMgr->ToString().c_str());
        return Status::Success;
    }

    // 启动插件集
    Int32 StartPlugin()
    {
        g_Log->Info(LOGFMT_NON_OBJ_TAG(PluginWrap, "Plugin start success."));
        return Status::Success;
    }

    // 预关闭插件集
    void WillClosePlugin()
    {
        g_Log->Info(LOGFMT_NON_OBJ_TAG(PluginWrap, "Plugin will close ..."));
    }

    // 释放插件集
    void ClosePlugin()
    {
        g_Log->Info(LOGFMT_NON_OBJ_TAG(PluginWrap, "Plugin close success."));
    }

    void SetPluginMgr(void *pluginMgr)
    {
        g_PluginMgr = reinterpret_cast<SERVICE_NS::IPluginMgr *>(pluginMgr);
    }
}