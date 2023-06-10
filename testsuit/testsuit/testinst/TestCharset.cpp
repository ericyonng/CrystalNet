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
 * Date: 2023-06-10 19:38:00
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestCharset.h>

void TestCharset::Run()
{
    KERNEL_NS::SmartPtr<FILE, KERNEL_NS::AutoDelMethods::CustomDelete> fp = KERNEL_NS::FileUtil::OpenFile("gbk.txt");
    if(!fp)
    {
        return;
    }
    fp.SetClosureDelegate([](void *p){
        FILE *ptr = reinterpret_cast<FILE *>(p);
        KERNEL_NS::FileUtil::CloseFile(*ptr);
    });

    KERNEL_NS::LibString gbk;
    KERNEL_NS::FileUtil::ReadFile(*fp, gbk);

    KERNEL_NS::LibString utf8Text;
    if(!gbk.IsUtf8())
        KERNEL_NS::TranscoderUtil::MultiByteToMultiByte("GBK", gbk, "UTF8", utf8Text);

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCharset, "gbk is utf8:%d, utf8: %s, utf8Text is utf8:%d"), gbk.IsUtf8(), utf8Text.c_str(), utf8Text.IsUtf8());
}