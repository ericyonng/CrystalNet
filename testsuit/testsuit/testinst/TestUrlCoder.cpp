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
 * Date: 2023-11-05 00:31:27
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestUrlCoder.h>

void TestUrlCoder::Run()
{
    KERNEL_NS::SmartPtr<FILE, KERNEL_NS::AutoDelMethods::CustomDelete> fp = KERNEL_NS::FileUtil::OpenFile("./testurlcoder.txt");
    if(!fp)
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(TestUrlCoder, "testurlcoder.txt not exist."));
        return;
    }
    fp.SetClosureDelegate([](void *p){
        auto ptr = KERNEL_NS::KernelCastTo<FILE>(p);
        KERNEL_NS::FileUtil::CloseFile(*ptr);
    });

    KERNEL_NS::LibString content;
    KERNEL_NS::FileUtil::ReadFile(*fp.AsSelf(), content);
    if(content.empty())
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(TestUrlCoder, "testurlcoder.txt have no content."));
        return;
    }

    {
        // url 编码
        KERNEL_NS::LibString encoded;
        KERNEL_NS::UrlCoder::Encode(content, encoded);

        // url解码
        KERNEL_NS::LibString decoded;
        KERNEL_NS::UrlCoder::Decode(encoded, decoded);

        auto hex = content.ToHexString();
        KERNEL_NS::LibString fromHex;
        fromHex.FromHexString(hex);
    }

    {
        // url 编码
        KERNEL_NS::LibString encoded;
        KERNEL_NS::UrlCoder::Encode(content, encoded, true);

        // url解码
        KERNEL_NS::LibString decoded;
        KERNEL_NS::UrlCoder::Decode(encoded, decoded, true);

        auto hex = content.ToHexString();
        KERNEL_NS::LibString fromHex;
        fromHex.FromHexString(hex); 
    }

}