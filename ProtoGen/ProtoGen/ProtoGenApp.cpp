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
 * Date: 2023-12-23 23:45:32
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <ProtoGen/ProtoGenApp.h>
#include <ProtoGen/Exporter/Exporter.h>
#include <OptionComponent/OptionComp/CodeAnalyze/CodeAnalyze.h>

POOL_CREATE_OBJ_DEFAULT_IMPL(ProtoGenApp);

Int32 ProtoGenApp::_OnCompsCreated()
{
    _appName = KERNEL_NS::SystemUtil::GetCurProgramNameWithoutExt();

    Int32 errCode = KERNEL_NS::SystemUtil::GetProgramPath(true, _path);
    if(errCode != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("GetProgramPath fail"));
        return errCode;
    }

    auto comp = GetComp<KERNEL_NS::ICodeAnalyzeMgr>();
    comp->SetCommentFlag("//");
    comp->SetMultiCommentFlag("/*", "*/");

    return Status::Success;
}

Int32 ProtoGenApp::_OnHostInit() 
{ 
    return Status::Success; 
};

Int32 ProtoGenApp::_OnHostStart()
{
    return Status::Success; 
}

void ProtoGenApp::Release()
{
    ProtoGenApp::DeleteThreadLocal_ProtoGenApp(this);
}

void ProtoGenApp::OnRegisterComps() 
{
    RegisterComp<KERNEL_NS::CodeAnalyzeMgrFactory>();
    RegisterComp<ExporterMgrFactory>();
};  

const KERNEL_NS::LibString &ProtoGenApp::GetAppName() const
{
    return _appName;
}

const KERNEL_NS::LibString &ProtoGenApp::GetAppPath() const
{
    return _path;
}
