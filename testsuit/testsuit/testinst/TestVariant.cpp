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
 * Date: 2021-03-16 23:13:58
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestVariant.h>

void TestVariant::Run() 
{
    KERNEL_NS::Variant var;
//    KERNEL_NS::Variant *var3 = KERNEL_NS::Variant::New_Variant();

    var["id"] = 1000;
    var["10"] = 30;
    KERNEL_NS::LibStream<KernelBuildMT> stream;
    stream.Init(1);
    var.Serialize(stream);
    g_Log->Custom("var [\"10\"] = %d", var["10"].AsInt32());
    g_Log->Custom("var [\"id\"] = %d", var["id"].AsInt32());
    g_Log->Custom("var string = %s", var.ToString().c_str());

    KERNEL_NS::Variant var2;
    var2.DeSerialize(stream);

    g_Log->Custom("var2 [\"10\"] = %d", var2["10"].AsInt32());
    g_Log->Custom("var2 [\"id\"] = %d", var2["id"].AsInt32());
    g_Log->Custom("var2 string = %s", var2.ToString().c_str());
    getchar();
    
}
