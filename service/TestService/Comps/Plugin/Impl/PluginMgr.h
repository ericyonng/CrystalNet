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
// Date: 2025-02-02 22:02:11
// Author: Eric Yonng
// Description:

#include <Comps/Plugin/Interface/IPluginMgr.h>

SERVICE_BEGIN

class PluginMgr : public IPluginMgr
{
    POOL_CREATE_OBJ_DEFAULT_P1(IPluginMgr, PluginMgr);

public:
    PluginMgr();
    ~PluginMgr() override;
    void Release() override;
    void OnRegisterComps() override;

    KERNEL_NS::LibString ToString() const override;

private:
    virtual Int32 _OnGlobalSysCompsCreated() override;
    virtual Int32 _OnHostStart() override;
    void _OnHostBeforeCompsWillClose() override;
    virtual void _OnHostBeforeCompsClose() override;

    Int32 _OnGlobalSysInit() override;
    void _OnGlobalSysClose() override;
    
    void _OnHotfixPlubin(KERNEL_NS::PollerEvent *ev);
    void _OnHotfixPlubinComplete(KERNEL_NS::PollerEvent *ev);

    void _InitPluginModule();
    void _WillClosePlugin();
    void _ClosePlugin();


private:
    KERNEL_NS::LibString _hotfixKey;
};

SERVICE_END