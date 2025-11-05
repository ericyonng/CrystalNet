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
 * Date: 2020-10-11 23:13:25
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <service/TestService/service.h>
#include <testsuit/testinst.h>

#include <testsuit/testinst/TestMemoryAlloctor.h>
#include <testsuit/testinst/TestMemoryPool.h>
#include <testsuit/testinst/TestObjAlloctor.h>
#include <testsuit/testinst/TestThread.h>
#include <testsuit/testinst/TestGarbageCollector.h>
#include <testsuit/testinst/TestLibString.h>
#include <testsuit/testinst/TestString.h>
#include <testsuit/testinst/TestTime.h>
#include <testsuit/testinst/TestFileUtil.h>
#include <testsuit/testinst/TestGuid.h>
#include <testsuit/testinst/TestTemplateObjPool.h>
#include <testsuit/testinst/TestBinaryArray.h>
#include <testsuit/testinst/TestTls.h>
#include <testsuit/testinst/TestRandom.h>
#include <testsuit/testinst/TestCypher.h>
#include <testsuit/testinst/TestStatic.h>
#include <testsuit/testinst/TestCountUtil.h>
#include <testsuit/testinst/TestRttiUtil.h>
#include <testsuit/testinst/TestContainerUtil.h>
#include <testsuit/testinst/TestSystemUtil.h>
#include <testsuit/testinst/TestEncrypt.h>
#include <testsuit/testinst/TestFile.h>
#include <testsuit/testinst/TestLog.h>
#include <testsuit/testinst/TestStream.h>
#include <testsuit/testinst/TestLocker.h>
#include <testsuit/testinst/TestMessageQueue.h>
#include <testsuit/testinst/TestVariant.h>
#include <testsuit/testinst/TestTimer.h>
#include <testsuit/testinst/TestCpuInfo.h>
#include <testsuit/testinst/TestBackTrace.h>
#include <testsuit/testinst/TestDaemon.h>
#include <testsuit/testinst/TestEvent.h>
#include <testsuit/testinst/TestConcurrentPriorityQueue.h>
#include <testsuit/testinst/TestBlackWhiteList.h>
#include <testsuit/testinst/TestECS.h>
#include <testsuit/testinst/TestCpu.h>
#include <testsuit/testinst/TestDelegate.h>
#include <testsuit/testinst/TestPoller.h>
#include <testsuit/testinst/TestMemoryAssist.h>
#include <testsuit/testinst/TestService.h>
#include <testsuit/testinst/TestInlineStatic.h>
#include <testsuit/testinst/TestList.h>
#include <testsuit/testinst/TestDirectoryUtil.h>
#include <testsuit/testinst/TestProtobuf.h>
#include <testsuit/testinst/TestPopen.h>
#include <testsuit/testinst/TestRegex.h>
#include <testsuit/testinst/TestDestructor.h>
#include <testsuit/testinst/TestAlloc.h>
#include <testsuit/testinst/TestPipeline.h>
#include <testsuit/testinst/TestArchive.h>
#include <testsuit/testinst/TestXlsx.h>
#include <testsuit/testinst/TestSimpleApi.h>
#include <testsuit/testinst/TestConfig.h>
#include <testsuit/testinst/TestMysql.h>
#include <testsuit/testinst/TestCharset.h>
#include <testsuit/testinst/TestSql.h>
#include <testsuit/testinst/TestCenterMemoryCollector.h>
#include <testsuit/testinst/TestBt.h>
#include <testsuit/testinst/TestCodeAnalyze.h>
#include <testsuit/testinst/TestUrlCoder.h>
#include <testsuit/testinst/TestOrm.h>
#include <testsuit/testinst/TestTimeWheel.h>
#include <testsuit/testinst/TestCheckAdapter.h>
#include <testsuit/testinst/TestCoroutine.h>
#include <testsuit/testinst/TestLargeFile.h>
#include <testsuit/testinst/TestCurl.h>
#include <testsuit/testinst/TestModule/TestModule.h>
#include <testsuit/testinst/TestConcepts/TestConcepts.h>
#include <testsuit/testinst/TestConceptModules/TestConceptModules.h>
#include <testsuit/testinst/TestBigNum/TestBigNum.h>
#include <testsuit/testinst/TestJson.h>
#include "testinst/TestLua/TestLua.h"
#include "testsuit/testinst/TestIdGenerator.h"
#include <testsuit/testinst/TestLoadShareLibrary.h>
#include <testsuit/testinst/TestMongo.h>
#include <testsuit/testinst/TestSort.h>

// void *operator new(size_t bytes)
// {
//     g_TotalBytes.fetch_add(bytes);

//     printf("\noperator new bytes:%llu, \n", bytes);
    
//     return ::malloc(bytes);
// }

// void *operator new[](size_t bytes)
// {
//     g_TotalBytes.fetch_add(bytes);

//     printf("\noperator new[] bytes:%llu, \n", bytes);

//     return ::malloc(bytes);
// }

// static inline void DebugLogHookBefore(KERNEL_NS::LogData *logData)
// {
//     logData->_logInfo.AppendFormat("\nDebugLogHookBefore .");
// }

class LibTestLog : public KERNEL_NS::LibLog
{
public:
    LibTestLog() {}
    ~LibTestLog() {}
};

class LogFactory : public KERNEL_NS::ILogFactory
{
public:
    virtual KERNEL_NS::ILog *Create()
    {
        return new LibTestLog();
    }
};

// static void TestPool(KERNEL_NS::LibThreadPool *pool)
// {
//     while(!pool->IsDestroy())
//     {
//         KERNEL_NS::SystemUtil::ThreadSleep(1000);
//         g_Log->Info(LOGFMT_NON_OBJ_TAG(TestInst, "hello thread:%llu"), KERNEL_NS::SystemUtil::GetCurrentThreadId());
//     }
// }

// static void TestBreak(KERNEL_NS::LibThreadPool *pool)
// {
//     while(!pool->IsDestroy())
//     {
//         KERNEL_NS::SystemUtil::ThreadSleep(1000);
//         g_Log->Info(LOGFMT_NON_OBJ_TAG(TestInst, "hello TestBreak thread:%llu"), KERNEL_NS::SystemUtil::GetCurrentThreadId());
//     }
// }

void TestInst::Run(int argc, char const *argv[])
{
    printf("/*********************************************/!\n");
    printf("/*           Hello Crystal Net!              */\n");
    printf("/*********************************************/!\n\n");

    {
        KERNEL_NS::ParamsInfo params;
        KERNEL_NS::LibString errParamsInfo;
        KERNEL_NS::LibString sucParamsInfo;
        Int32 paramNum = KERNEL_NS::ParamsHandler::GetParams(argc, argv, params, sucParamsInfo, errParamsInfo);

        SERVICE_NS::LibServiceLogFactory serviceLogFactory;
        KERNEL_NS::LibString programPath = KERNEL_NS::SystemUtil::GetCurProgRootPath();
        KERNEL_NS::LibString logIniPath;
        logIniPath = programPath + "/ini/";
        KERNEL_NS::SystemUtil::GetProgramPath(true, programPath);
        Int32 err = KERNEL_NS::KernelUtil::Init(&serviceLogFactory, "LogCfg.ini", logIniPath.c_str(), KERNEL_NS::LibString(), KERNEL_NS::LibString(), true, params._fileSoftLimit, params._fileHardLimit);
        if(err != Status::Success)
        {
            CRYSTAL_TRACE("kernel init fail err:%d", err);
            return;
        }

        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestInst, "system test init paramNum:%d.\nsucParamsInfo:\n%s, errParamsInfo:\n%s."), paramNum, sucParamsInfo.c_str(), errParamsInfo.c_str());
    }

    // 安装loghook
    // g_Log->InstallBeforeLogHookFunc(KERNEL_NS::LogLevel::Debug, &DebugLogHookBefore);

    KERNEL_NS::KernelUtil::Start();

    // // TODO:analyze argv参数 使用lua分析
    KERNEL_NS::LibString testInstInfo;
    testInstInfo.AppendFormat("test started. nihao:你好.shabi %s", " 🌍 olleH");
    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestInst, testInstInfo.c_str()));

    #else
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestInst, "test started. nihao:你好.shabi %s"), " 🌍 olleH");
    #endif

// 
//     KERNEL_NS::AllocUtil::GetStaticTemplateObjNoFree<Int32>([]()->Int32 * {
//         return new Int32;
//     });
// 
//     KERNEL_NS::AllocUtil::GetStaticTemplateObjNoFree<Int32>([]()->Int32 * {
//         return new Int32;
//     });
// 
//     auto pone = KERNEL_NS::AllocUtil::GetStaticThreadLocalTemplateObjNoFree<Int32>([]()->void * {
//         return new Int32;
//     });
// 
//     auto ptow = KERNEL_NS::AllocUtil::GetStaticThreadLocalTemplateObjNoFree<Int32>([]()->void * {
//         return new Int32;
//     });
// 
//     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestInst, "pone:%p, ptow:%p"), pone, ptow);

    // TestMemoryAlloctor::Run();
    // TestMemoryPool::Run();
    // TestObjAlloctor::Run();
    // TestThread::Run();

    // //TestGarbageCollector::Run();
    // TestLibString::Run();
    // TestString::Run();
    // TestTime::Run();
    // //TestFileUtil::Run();
    // TestGuid::Run();
    // // TestTemplateObjPool::Run();
    // // TestBinaryArray::Run();
    // TestTls::Run();
    // TestRandom::Run();
    // // TestCypher::Run();
    // //TestStatic::Run();
    // //TestCountUtil::Run();
    // // TestTls::Run();
    // TestRttiUtil::Run();
    // //TestContainerUtil::Run();
    // // TestSystemUtil::Run();
    // TestEncrypt::Run(argc, argv);
    // TestFile::Run();
    // TestLog::Run();
    // TestStream::Run();
    // // TestLocker::Run();
    // TestMessageQueue::Run();
    // TestVariant::Run();
    // TestTimer::Run();
    // // TestCpuInfo::Run();
    // TestBackTrace::Run();
    // //TestDaemon::Run();
    // TestEvent::Run();
    // TestConcurrentPriorityQueue::Run();
    // CRYSTAL_TRACE("test inst finish.");
    // TestBlackWhiteList::Run();
    // TestECS::Run();
    // TestCpu::Run();
    // TestDelegate::Run();
    TestPoller::Run();
    // TestMemoryAssist::Run();
    // TestService::Run(argc, argv);
    // TestInlineStatic::Run();
    // TestList::Run();
    // TestDirectoryUtil::Run();
    // TestProtobuf::Run();
    // TestPopen::Run();
    // TestRegex::Run(argc, argv);
    // TestDestructor::Run();
    // TestAlloc::Run();
    // TestPipeline::Run();
    // TestArchive::Run();
    // TestXlsx::Run();
    // TestSimpleApi::Run();
    // TestConfig::Run();
    // TestMysql::Run();
    // TestCharset::Run();
    // TestSql::Run();
    // TestCenterMemoryCollector::Run();
    // TestBt::Run();
    // TestCodeAnalyze::Run();
    // TestUrlCoder::Run();
    // TestOrm::Run();
    // TestTimeWheel::Run();
    // TestCheckAdapter::Run();
    // TestCoroutine::Run();
    // TestCurl::Run(argc, argv);
    // TestModule::Run();
    // TestConcepts::Run();
    // TestConceptModules::Run();
    // TestBigNum::Run();
    // TestLua::Run();
    // TestJson::Run();
    // TestIdGenerator::Run();
    // TestLoadShareLibrary::Run();
    // TestMongo::Run();
    // TestSort::Run();
    
    // write a large file
    // do
    // {
    //     {
    //         KERNEL_NS::SmartPtr<FILE, KERNEL_NS::AutoDelMethods::CustomDelete> fp = KERNEL_NS::FileUtil::OpenFile("./largetfile.txt", true, "wb+");
    //         if(!fp)
    //         {
    //             break;
    //         }

    //         fp.SetClosureDelegate([](void *p){
    //             auto ptr = KERNEL_NS::KernelCastTo<FILE>(p);
    //             KERNEL_NS::FileUtil::CloseFile(*ptr);
    //         });

    //         const Int64 writeBytes = 4LL * 1024LL * 1024LL * 1024LL;
    //         KERNEL_NS::LibString lineData = "kldajfskdjfasdfj-x\n";
    //         for(Int64 idx = 0; idx < writeBytes; idx += static_cast<Int64>(lineData.size()))
    //             KERNEL_NS::FileUtil::WriteFile(*fp, lineData);

    //         // 最后一条
    //         KERNEL_NS::FileUtil::WriteFile(*fp, "dalfkajsdf------------/adskfjaslk------------\n");
    //         KERNEL_NS::FileUtil::FlushFile(*fp);
    //     }

    //     TestLargeFile::Run(argc, argv);

    // }while(false);

    // KERNEL_NS::SmartPtr<KERNEL_NS::LibThreadPool> pool = new KERNEL_NS::LibThreadPool();
    // pool->Init(0, 4);
    // pool->AddTask(&TestPool, false);
    // pool->AddTask(&TestBreak, false);

    // pool->Start(true, 4);

    // getchar();

    KERNEL_NS::KernelUtil::Destroy();
    
    printf("\ntest case finish.\n");
    KERNEL_NS::SystemUtil::ThreadSleep(5000);
    getchar();
    // while(true)
    // {
    //     auto v = getchar();
    //     if(v == 'q')
    //         break;
    // }
}
