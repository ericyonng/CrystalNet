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
// Date: 2025-02-02 22:02:52
// Author: Eric Yonng
// Description:

#include <pch.h>
#include <kernel/kernel.h>
#include <Comps/Plugin/Impl/PluginMgrFactory.h>
#include <Comps/Plugin/Impl/PluginMgr.h>

SERVICE_BEGIN

KERNEL_NS::CompFactory *PluginMgrFactory::FactoryCreate()
{
    return KERNEL_NS::ObjPoolWrap<PluginMgrFactory>::NewByAdapter(_buildType.V);
}

void PluginMgrFactory::Release()
{
    KERNEL_NS::ObjPoolWrap<PluginMgrFactory>::DeleteByAdapter(_buildType.V, this);
}

KERNEL_NS::CompObject *PluginMgrFactory::Create() const
{
    CREATE_CRYSTAL_COMP(comp, PluginMgr);
    return comp;
}

SERVICE_END
