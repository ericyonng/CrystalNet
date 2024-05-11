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
 * Date: 2024-05-11 14:51:29
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <toolbox/scan_reason/ScanReason.h>
#include <fstream>
#include <iostream>

static UInt64 ReadUtf8OneLine(std::ifstream &fp, KERNEL_NS::LibString &outBuffer, UInt64 *utf8CharCount)
{
    // 读取单字节字符时候判断是否\n
    char get_c = 0;
    UInt64 cnt = 0;
    while(!fp.eof())
    {
        get_c = 0;
        fp.read(&get_c, 1);
        
        if(fp.gcount() == 1)
        {
            // 该utf8字符总字节数
            auto totalBytes = KERNEL_NS::StringUtil::CalcUtf8CharBytes(get_c);
            UInt64 leftBytes = totalBytes - 1;
            if(!leftBytes)
            {// 单字节字符

                #if CRYSTAL_TARGET_PLATFORM_WINDOWS
                            
                    if(get_c == '\r')
                        continue;

                    if(get_c != '\n')
                    {
                        outBuffer.AppendData(reinterpret_cast<const char *>(&get_c), 1);
                        ++cnt;

                        if(UNLIKELY(utf8CharCount))
                            ++(*utf8CharCount);
                    }
                    else
                    {
                        break;
                    }
                #else
                    if(get_c != '\n')
                    {
                        outBuffer.AppendData(reinterpret_cast<const char *>(&get_c), 1);
                        ++cnt;
                        
                        if(UNLIKELY(utf8CharCount))
                            ++(*utf8CharCount);
                    }
                    else
                    {
                        //fread(&get_c, sizeof(get_c), 1, fpOutCache);
                        break;
                    }
                #endif
            }
            else
            {// 多字节字符

                outBuffer.AppendData(reinterpret_cast<const char *>(&get_c), 1);
                ++cnt;
                
                if(UNLIKELY(utf8CharCount))
                    ++(*utf8CharCount);

                // 读取剩下字节
                do
                {
                    fp.read(&get_c, 1);

                    if(fp.gcount() == 1)
                    {
                        outBuffer.AppendData(reinterpret_cast<const char *>(&get_c), 1);
                        ++cnt;
                    }
                    else
                    {
                        break;
                    }
                } while (--leftBytes);

                // 错误字符
                if(UNLIKELY(leftBytes))
                {
                    CRYSTAL_TRACE("ERROR UTF8 Char totalBytes[%llu], leftBytes[%llu], fread fail", totalBytes, leftBytes);
                    break;
                }
            }
        }
        else
        {
            // 文件结束
            break;
        }
    }

    return cnt;
}


std::atomic<Int64> g_fileSize{0};
std::atomic<Int64> g_CurrentReadBytes{0};

FILE *g_fp = NULL;

std::map<UInt64, std::map<KERNEL_NS::LibString, Int32>> g_ThreadCache;
KERNEL_NS::Locker g_lck;

KERNEL_NS::LibString keyContent;
std::atomic<Int32> g_workingThread{0};

// mid:4204kqps, average:4213kqps
static void ReadReason(KERNEL_NS::LibThreadPool *t)
{
    KERNEL_NS::LibCpuCounter swapCounterStart;
    KERNEL_NS::LibCpuCounter swapCounterEnd;

    auto threadId = KERNEL_NS::SystemUtil::GetCurrentThreadId();
    g_lck.Lock();
    auto iter = g_ThreadCache.find(threadId);
    if(iter == g_ThreadCache.end())
        iter = g_ThreadCache.insert(std::make_pair(threadId, std::map<KERNEL_NS::LibString, Int32>())).first;

    g_lck.Unlock();

    std::map<KERNEL_NS::LibString, Int32> &reasonRefNum = iter->second;
    auto key = keyContent;

    g_Log->Info(LOGFMT_NON_OBJ_TAG(ScanReason, "thread in %llu"), threadId);

    ++g_workingThread;

    do
    {
        // 逐行扫描文件
        KERNEL_NS::LibString patten = KERNEL_NS::LibString().AppendFormat(".*%s.*", key.c_str());
        
        while(true)
        {
            KERNEL_NS::LibString lineStr;
            g_lck.Lock();
            g_CurrentReadBytes += static_cast<Int64>(KERNEL_NS::FileUtil::ReadUtf8OneLine(*g_fp, lineStr, NULL));
            if(lineStr.empty() && !KERNEL_NS::FileUtil::IsEnd(*g_fp))
            {
                g_Log->Info(LOGFMT_NON_OBJ_TAG(ScanReason, "thread %llu file eof"), threadId);
                g_lck.Unlock();
                break;
            }
            g_lck.Unlock();

            KERNEL_NS::LibString buffer = lineStr;
            if(buffer.empty())
                continue;

            if(KERNEL_NS::StringUtil::IsMatch(buffer, patten))
            {
                auto pos = buffer.GetRaw().find(key.GetRaw());
                if(pos != std::string::npos)
                {
                    auto &&reasonStr = buffer.lsub(key);
                    auto endPos = reasonStr.GetRaw().find(" ");
                    if(endPos != std::string::npos)
                        reasonStr = reasonStr.GetRaw().substr(0, endPos);

                    auto iter = reasonRefNum.find(reasonStr);
                    if(iter == reasonRefNum.end())
                        iter = reasonRefNum.insert(std::make_pair(reasonStr, 0)).first;

                    ++iter->second;
                }
            }
        } 
    }while (!t->IsDestroy());

    --g_workingThread;
    g_Log->Info(LOGFMT_NON_OBJ_TAG(ScanReason, "thread out %llu"), threadId);
}

void ScanReason::Run(int argc, char const *argv[])
{
    std::vector<KERNEL_NS::LibString> params;
    KERNEL_NS::LibString file;
    Int32 threadNum = 1;
    Int32 count = 0;
    KERNEL_NS::LibString key;
    KERNEL_NS::ParamsHandler::GetStandardParams(argc, argv, [&file, &count, &threadNum, &key](const KERNEL_NS::LibString &param, std::vector<KERNEL_NS::LibString> &leftParam){
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
        else if(count == 3)
        {
            key = param.strip();
        }

        ++count;

        return true;
    });

    if(file.empty())
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(ScanReason, "have no file name"));
        return;
    }

    keyContent = key;
    g_Log->Info(LOGFMT_NON_OBJ_TAG(ScanReason, "scan file:%s, keyContent:%s"), file.c_str(), keyContent.c_str());

    KERNEL_NS::SmartPtr<FILE, KERNEL_NS::AutoDelMethods::CustomDelete> fp = KERNEL_NS::FileUtil::OpenFile(file.c_str());
    if(!fp)
    {
        g_Log->Error(LOGFMT_NON_OBJ_TAG(ScanReason, "open file fail file name:%s"), file.c_str());
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
                auto intervalTime = KERNEL_NS::TimeSlice::FromSeconds(1);
                Int64 lastProgress = 0;
                auto fileSize = g_fileSize.load();
                const auto &fileSizeStr = KERNEL_NS::MathUtil::ToFmtDataSize(fileSize);

                ptr->FinishClose([&tickTime, &intervalTime, &lastProgress, &fileSizeStr, fileSize, &ptr](){
                    auto nowTime = KERNEL_NS::LibTime::Now();

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

                    g_Log->Info(LOGFMT_NON_OBJ_TAG(ScanReason, "scan process: working thread num:%d, file size:%s, processing:%lf%%, handling bytes:%s, speed:%s/s")
                    , ptr->GetWorkThreadNum(), fileSizeStr.c_str(), (double)(newHandle)/fileSize * 100, newHandleStr.c_str(), speedStr.c_str());
                });
            }

        });

        threadNum = threadNum > 1 ? threadNum : 1;
        pool->Init(1, threadNum);

        for(Int32 idx = 0; idx < threadNum; ++idx)
                pool->AddTask(&ReadReason);

        pool->Start(true, threadNum + 1);
    }
    auto endTime = KERNEL_NS::LibTime::Now();


    std::map<KERNEL_NS::LibString, Int32> reasonNumCount;
    KERNEL_NS::LibString reasonRefNumStr;
    for(auto iter : g_ThreadCache)
    {
        for(auto itt : iter.second)
        {
            auto iterrr = reasonNumCount.find(itt.first);
            if(iterrr == reasonNumCount.end())
                iterrr = reasonNumCount.insert(std::make_pair(itt.first, 0)).first;

            iterrr->second += itt.second;
        }
    }

    for(auto iter : reasonNumCount)
        reasonRefNumStr.AppendFormat("reason:%s-%d, ", iter.first.c_str(), iter.second);

    g_Log->Info(LOGFMT_NON_OBJ_TAG(ScanReason, "%s, cost time:%d(seconds)")
    , reasonRefNumStr.c_str(), (endTime - startTime).GetTotalSeconds());
}
