/*!
 * MIT License
 *  
 * Copyright (c) 2020 Eric Yonng<120453674@qq.com>
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *  
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *  
 * 
 * Date: 2023-06-30 15:03:00
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <RsaGen/RsaGen.h>
#include <service_common/ServiceCommon.h>
#include <RsaGen/RsaGenIni.h>

Int32 RsaGen::Run(int argc, char const *argv[])
{
    {// 同时生成pkc1/pkc8
        KERNEL_NS::LibRsa rsa;
        rsa.GenKey(KERNEL_NS::LibRsaDefs::RSA_1024, KERNEL_NS::LibRsa::PUB_PKC8_FLAG | KERNEL_NS::LibRsa::PUB_PKC1_FLAG);
        const auto &pubKeyPkc1Encode = KERNEL_NS::LibBase64::Encode(rsa.GetPubPkc1Key());
        const auto &pubKeyPkc8Encode = KERNEL_NS::LibBase64::Encode(rsa.GetPubPkc8Key());
        const auto &privateKeyEncode = KERNEL_NS::LibBase64::Encode(rsa.GetPrivateKey());
        
        g_Log->Custom("[PUBLIC KEY 1024 PKC1]:\n%s\n", rsa.GetPubPkc1Key().c_str());
        g_Log->Custom("[PUBLIC KEY 1024 PKC8]:\n%s\n", rsa.GetPubPkc8Key().c_str());
        g_Log->Custom("[PUBLIC KEY 1024 PKC1 TO BASE64]:\n%s\n", pubKeyPkc1Encode.c_str());
        g_Log->Custom("[PUBLIC KEY 1024 PKC8 TO BASE64]:\n%s\n", pubKeyPkc8Encode.c_str());

        g_Log->Custom("[PRIVATE KEY 1024]:\n%s\n", rsa.GetPrivateKey().c_str());
        g_Log->Custom("[PRIVATE KEY TO BASE64]:\n%s\n", privateKeyEncode.c_str());
    }

    getchar();
    return Status::Success;
}
