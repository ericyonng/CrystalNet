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
 * Date: 2022-12-25 17:38:29
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestPipeline.h>

void TestPipeline::Run()
{
    KERNEL_NS::LibString testStr = "hello pipeline.";
    // {
    //     auto memoryPipe = KERNEL_NS::MemoryPipe<KERNEL_NS::_Build::TL>::NewThreadLocal_MemoryPipe();
    //     memoryPipe->SetRelease([memoryPipe](){
    //         KERNEL_NS::MemoryPipe<KERNEL_NS::_Build::TL>::DeleteThreadLocal_MemoryPipe(memoryPipe);
    //     });

    //     memoryPipe->Init(16);

    //     memoryPipe->Open();
    //     Int64 len = static_cast<Int64>(testStr.size());
    //     memoryPipe->Write(testStr.c_str(), len);

    //     BUFFER128 cache = {0};
    //     Int64 cacheLen = 5;
    //     memoryPipe->Read(cache, cacheLen);

    //     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestPipeline, "cache :%s"), cache);

    //     // memoryPipe->Flush();

    //     memoryPipe->Close();
        
    //     memoryPipe->Release();
    // }

    {
        auto pipe = KERNEL_NS::PipeFactory::Create<KERNEL_NS::_Build::TL>(KERNEL_NS::PipeType::MEMORY, [](KERNEL_NS::IPipe *pipe)->bool
        {
            auto memPipe = KERNEL_NS::KernelCastTo<KERNEL_NS::MemoryPipe<KERNEL_NS::_Build::TL>>(pipe);
            memPipe->SetRelease([memPipe]()
            {
                KERNEL_NS::MemoryPipe<KERNEL_NS::_Build::TL>::DeleteThreadLocal_MemoryPipe(memPipe);
            });

            memPipe->Init(16);

            return true;
        });

        pipe->Open();
        Int64 len = static_cast<Int64>(testStr.size());
        pipe->Write(testStr.c_str(), len);

        BUFFER128 cache = {0};
        Int64 cacheLen = 5;
        pipe->Read(cache, cacheLen);

        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestPipeline, "cache :%s"), cache);

        // memoryPipe->Flush();

        pipe->Close();
        
        pipe->Release();
    }

    // {
    //     auto filePipe = KERNEL_NS::FilePipe::NewThreadLocal_FilePipe();
    //     filePipe->SetRelease([filePipe](){
    //         KERNEL_NS::FilePipe::DeleteThreadLocal_FilePipe(filePipe);
    //     });

    //     filePipe->Init("./testpipe.log");

    //     filePipe->Open();

    //     Int64 len = static_cast<Int64>(testStr.size());
    //     filePipe->Write(testStr.c_str(), len);

    //     BUFFER128 cache = {0};
    //     Int64 cacheLen = 5;
    //     filePipe->Read(cache, cacheLen);

    //     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestPipeline, "cache :%s"), cache);

    //     filePipe->Flush();

    //     filePipe->Close();

    //     filePipe->Release();
    // }

    {
        auto pipe = KERNEL_NS::PipeFactory::Create<KERNEL_NS::_Build::TL>(KERNEL_NS::PipeType::FILE, [](KERNEL_NS::IPipe *pipe)->bool
        {
            auto filePipe = KERNEL_NS::KernelCastTo<KERNEL_NS::FilePipe>(pipe);
            filePipe->SetRelease([filePipe]()
            {
                KERNEL_NS::FilePipe::DeleteThreadLocal_FilePipe(filePipe);
            });

            return true;
        });

        pipe->SetPipeName("./testpipe.log");
        pipe->Open();
        Int64 len = static_cast<Int64>(testStr.size());
        pipe->Write(testStr.c_str(), len);

        BUFFER128 cache = {0};
        Int64 cacheLen = 5;
        pipe->Read(cache, cacheLen);

        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestPipeline, "cache :%s"), cache);

        // memoryPipe->Flush();

        pipe->Close();
        
        pipe->Release();
    }
}
