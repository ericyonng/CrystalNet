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
 * Date: 2023-09-30 18:53:42
 * Author: Eric Yonng
 * Description: 
*/
#include <pch.h>
#include <testsuit/testinst/TestCodeAnalyze.h>
#include <OptionComp/CodeAnalyze/CodeAnalyze.h>

void TestCodeAnalyze::Run()
{
    KERNEL_NS::SmartPtr<KERNEL_NS::CodeAnalyzeMgrFactory, KERNEL_NS::AutoDelMethods::Release> factory = 
    KERNEL_NS::KernelCastTo<KERNEL_NS::CodeAnalyzeMgrFactory>(KERNEL_NS::CodeAnalyzeMgrFactory::FactoryCreate());

    KERNEL_NS::SmartPtr<KERNEL_NS::ICodeAnalyzeMgr, KERNEL_NS::AutoDelMethods::Release> codeAnalyzeMgr = 
    factory->Create()->CastTo<KERNEL_NS::ICodeAnalyzeMgr>();

    codeAnalyzeMgr->SetCommentFlag("//");
    codeAnalyzeMgr->SetMultiCommentFlag("/*", "*/");

    auto err = codeAnalyzeMgr->Init();
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(TestCodeAnalyze, "init fail err:%d"), err);
        return;
    }

    err = codeAnalyzeMgr->Start();
    if(err != Status::Success)
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(TestCodeAnalyze, "start fail err:%d"), err);
        return;
    }

    codeAnalyzeMgr->ScanFile("./library.proto", [](KERNEL_NS::LibString &validLineData, Int32 currentLine, const KERNEL_NS::LibString &fullPath, const std::vector<KERNEL_NS::LibString> &lineDatas){
        
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCodeAnalyze, "validLineData:%s, currentLine:%d, fullPath:%s"), validLineData.c_str(), currentLine, fullPath.c_str());

    }, [](){});

    codeAnalyzeMgr->WillClose();
    codeAnalyzeMgr->Close();
    
}
