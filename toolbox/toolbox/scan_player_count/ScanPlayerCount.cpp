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
 * Date: 2024-06-02 12:07:29
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <toolbox/scan_player_count/ScanPlayerCount.h>


static std::atomic<Int64> g_fileSize{0};
static std::atomic<Int64> g_CurrentReadBytes{0};

static FILE *g_fp = NULL;

static std::map<UInt64, std::set<Int64>> g_ThreadCache;
static KERNEL_NS::Locker g_lck;

static KERNEL_NS::LibString keyContent;
static std::atomic<Int32> g_workingThread{0};

// mid:4204kqps, average:4213kqps
static void ReadReason(KERNEL_NS::LibThreadPool *t)
{
    KERNEL_NS::LibCpuCounter swapCounterStart;
    KERNEL_NS::LibCpuCounter swapCounterEnd;

    auto threadId = KERNEL_NS::SystemUtil::GetCurrentThreadId();
    g_lck.Lock();
    auto iter = g_ThreadCache.find(threadId);
    if(iter == g_ThreadCache.end())
        iter = g_ThreadCache.insert(std::make_pair(threadId, std::set<Int64>())).first;

    g_lck.Unlock();

    auto &playerIds = iter->second;
    auto key = keyContent;

    g_Log->Info(LOGFMT_NON_OBJ_TAG(ScanPlayerCount, "thread in %llu"), threadId);

    ++g_workingThread;

    do
    {
        // 逐行扫描文件
        KERNEL_NS::LibString patten = KERNEL_NS::LibString().AppendFormat(".*Random,.*");
        
        while(true)
        {
            KERNEL_NS::LibString lineStr;
            g_lck.Lock();
            g_CurrentReadBytes += static_cast<Int64>(KERNEL_NS::FileUtil::ReadUtf8OneLine(*g_fp, lineStr, NULL));
            if(lineStr.empty() && KERNEL_NS::FileUtil::IsEnd(*g_fp))
            {
                g_Log->Info(LOGFMT_NON_OBJ_TAG(ScanPlayerCount, "thread %llu file eof"), threadId);
                g_lck.Unlock();
                break;
            }
            g_lck.Unlock();

            KERNEL_NS::LibString buffer = lineStr;
            if(buffer.empty())
                continue;

            if(KERNEL_NS::StringUtil::IsMatch(buffer, patten))
            {
                auto &&parts = buffer.Split("Random,");
                if(parts.empty())
                    continue;

                auto &&pidStrLine = parts[0].strip();
                auto rPos = pidStrLine.GetRaw().rfind("]");
                auto leftPos = pidStrLine.GetRaw().rfind("[");
                if(rPos == std::string::npos || leftPos == std::string::npos)
                    continue;

                KERNEL_NS::LibString pidStr = pidStrLine.GetRaw().substr(leftPos + 1, rPos - leftPos - 1);
                pidStr.strip();
                if(pidStr.empty())
                    continue;

                auto playerId = KERNEL_NS::StringUtil::StringToInt64(pidStr.c_str());
                playerIds.insert(playerId);
            }
        } 
    }while (!t->IsDestroy());

    --g_workingThread;
    g_Log->Info(LOGFMT_NON_OBJ_TAG(ScanPlayerCount, "thread out %llu"), threadId);
}

void ScanPlayerCount::Run(int argc, char const *argv[])
{
    std::vector<KERNEL_NS::LibString> params;
    KERNEL_NS::LibString file;
    Int32 threadNum = 1;
    Int32 count = 0;
    KERNEL_NS::ParamsHandler::GetStandardParams(argc, argv, [&file, &count, &threadNum](const KERNEL_NS::LibString &param, std::vector<KERNEL_NS::LibString> &leftParam){
        if(count == 0)
        {
            ++count;
            return false;
        }

        if(count == 1)
        {
            file = param.strip();
        }
        else if(count == 2)
        {
            auto value = param.strip();
            if(value.isdigit())
            {
                threadNum = KERNEL_NS::StringUtil::StringToInt32(value.c_str());
            }
        }

        ++count;

        return true;
    });

    if(file.empty())
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(ScanPlayerCount, "have no file name"));
        return;
    }

    keyContent = "SPlayerId:";
    g_Log->Info(LOGFMT_NON_OBJ_TAG(ScanPlayerCount, "scan file:%s, keyContent:%s"), file.c_str(), keyContent.c_str());

    KERNEL_NS::SmartPtr<FILE, KERNEL_NS::AutoDelMethods::CustomDelete> fp = KERNEL_NS::FileUtil::OpenFile(file.c_str());
    if(!fp)
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(ScanPlayerCount, "open file fail file name:%s"), file.c_str());
        return;
    }
    fp.SetClosureDelegate([](void *p){
        auto ptr = KERNEL_NS::KernelCastTo<FILE>(p);
        KERNEL_NS::FileUtil::CloseFile(*ptr);
    });
    g_fp = fp.AsSelf();

    g_fileSize = KERNEL_NS::FileUtil::GetFileSize(*fp);

    auto startTime = KERNEL_NS::LibTime::Now();
    {
        KERNEL_NS::SmartPtr<KERNEL_NS::LibThreadPool, KERNEL_NS::AutoDelMethods::CustomDelete> pool = new KERNEL_NS::LibThreadPool();
        pool.SetClosureDelegate([](void *p){
            auto ptr = KERNEL_NS::KernelCastTo<KERNEL_NS::LibThreadPool>(p);
            if(ptr->HalfClose())
            {
                auto tickTime = KERNEL_NS::LibTime::Now();
                auto startTime2 = tickTime;
                auto intervalTime = KERNEL_NS::TimeSlice::FromSeconds(1);
                Int64 lastProgress = 0;
                auto fileSize = g_fileSize.load();
                const auto &fileSizeStr = KERNEL_NS::MathUtil::ToFmtDataSize(fileSize);

                ptr->FinishClose([&tickTime, &intervalTime, &lastProgress, &fileSizeStr, fileSize, &ptr, &startTime2](){
                    auto nowTime = KERNEL_NS::LibTime::Now();
                    const auto &costTime = nowTime - startTime2;

                    auto diff = nowTime - tickTime;
                    if(diff < intervalTime)
                    {
                        return;
                    }
                    tickTime = nowTime;

                    auto newHandle = g_CurrentReadBytes.load();
                    if(lastProgress == 0)
                        lastProgress = newHandle;
                    auto diffBytes = newHandle - lastProgress;
                    lastProgress = newHandle;

                    const auto &newHandleStr = KERNEL_NS::MathUtil::ToFmtDataSize(newHandle);
                    const auto &speedStr = KERNEL_NS::MathUtil::ToFmtDataSize(diffBytes);

                    g_Log->Info(LOGFMT_NON_OBJ_TAG(ScanPlayerCount, "scan process: working thread num:%d, file size:%s, processing:%lf%%, handling bytes:%s, speed:%s/s, costTime:%lld(seconds)")
                    , ptr->GetWorkThreadNum(), fileSizeStr.c_str(), (double)(newHandle)/fileSize * 100, newHandleStr.c_str(), speedStr.c_str(), costTime.GetTotalSeconds());
                });
            }

        });

        threadNum = 10;
        pool->Init(1, threadNum);

        for(Int32 idx = 0; idx < threadNum; ++idx)
                pool->AddTask(&ReadReason);

        pool->Start(true, threadNum + 1);
    }
    auto endTime = KERNEL_NS::LibTime::Now();

    std::set<Int64> allPlayerIds;
    for(auto iter : g_ThreadCache)
    {
        for(auto playerId : iter.second)
            allPlayerIds.insert(playerId);
    }

    for(auto playerId : allPlayerIds)
        g_Log->Custom("[PlayerId]:%lld", playerId);

    g_Log->Info(LOGFMT_NON_OBJ_TAG(ScanPlayerCount, "key:%s, cost time:%lld(seconds) all player count:%lld")
    , keyContent.c_str(), (endTime - startTime).GetTotalSeconds(), static_cast<Int64>(allPlayerIds.size()));
}
