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
 * Date: 2022-11-20 01:14:17
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestPopen.h>

void TestPopen::Run()
{
    KERNEL_NS::LibString cmd;
    cmd.AppendFormat("sudo /var/lib/jenkins/workspace/CrystalNet/tools/protogen/../../tools/protobuf/bin/protoc --cpp_out=/var/lib/jenkins/workspace/CrystalNet/tools/protogen/../../protocols/cplusplus --proto_path=/var/lib/jenkins/workspace/CrystalNet/tools/protogen/../../protocols/proto com.proto");
    Int32 err = 0;
    KERNEL_NS::LibString outInfo;

    ::system("ls -l");
    err = ::system(cmd.c_str());
    auto perr = KERNEL_NS::SystemUtil::GetErrNo();
    g_Log->Custom("err:%d, perr:%d", err, perr);
    KERNEL_NS::SystemUtil::Exec("sudo ls", err, outInfo);
    KERNEL_NS::SystemUtil::Exec("sudo chmod a+x /var/lib/jenkins/workspace/CrystalNet/tools/protogen/../../tools/protobuf/bin/protoc", err, outInfo);
    KERNEL_NS::SystemUtil::Exec("sh ./test.sh", err, outInfo);
    KERNEL_NS::SystemUtil::Exec("ls -l /var/lib/jenkins/workspace/CrystalNet/tools/protogen/../../tools/protobuf/bin/protoc", err, outInfo);
    KERNEL_NS::SystemUtil::Exec("/var/lib/jenkins/workspace/CrystalNet/tools/protogen/../../tools/protobuf/bin/protoc", err, outInfo);

    KERNEL_NS::SystemUtil::Exec(cmd.c_str(), err, outInfo);

    g_Log->Custom("err:%d, out:%s", err, outInfo.c_str());
}