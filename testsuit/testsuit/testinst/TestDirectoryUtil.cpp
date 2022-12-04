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
 * Date: 2022-10-18 23:18:11
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestDirectoryUtil.h>

void TestDirectoryUtil::Run()
{
    auto callback = [](const KERNEL_NS::FindFileInfo &fileAttr, bool &isParantDirContinue)->bool{
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestDirectoryUtil, "file name:%s, is dir:%d, root dir:%s"), fileAttr._fileName.c_str(), KERNEL_NS::FileUtil::IsDir(fileAttr), fileAttr._rootPath.c_str());
        isParantDirContinue = true;
        return true;
    };

    auto delg = KERNEL_CREATE_CLOSURE_DELEGATE(callback, bool, const KERNEL_NS::FindFileInfo &, bool &);
    KERNEL_NS::DirectoryUtil::TraverseDirRecursively("./ini", delg);
}
