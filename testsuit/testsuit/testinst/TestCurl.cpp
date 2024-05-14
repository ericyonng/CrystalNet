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
 * Date: 2024-05-14 19:45:14
 * Author: Eric Yonng
 * Description: 
*/
#include <pch.h>
#include <curl/curl.h>
#include <testsuit/testinst/TestCurl.h>

KERNEL_NS::LibString s_jsonBodyData;
// 10000 qps
KERNEL_NS::TimeSlice s_genCurlInterval = KERNEL_NS::TimeSlice::FromMilliSeconds(5);

const Int32 s_threadNum = 50;

std::atomic<Int64> s_Qps{0};
std::atomic<Int64> s_failQps{0};
std::atomic<Int64> s_curlTask{0};

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) 
{  
    ((std::string*)userp)->append((char*)contents, size * nmemb);  
    return size * nmemb;  
}  


static void Monitor(KERNEL_NS::LibThreadPool *pool)
{
    KERNEL_NS::TimerMgr timerMgr;
    timerMgr.Launch(NULL);

    KERNEL_NS::LibTimer *timer = KERNEL_NS::LibTimer::New_LibTimer(&timerMgr);
    timer->SetTimeOutHandler([pool](KERNEL_NS::LibTimer *t)
    {
        auto qps = s_Qps.exchange(0); 
        auto failNum = s_failQps.exchange(0); 
        auto curlTask = s_curlTask.exchange(0);
        g_Log->Custom("[Ace Qps]:gen curl task:%llu, http request:%lld, fail:%lld, working thread:%d"
        , curlTask, qps, failNum, pool->GetWorkThreadNum());
    });
    timer->Schedule(KERNEL_NS::TimeSlice::FromSeconds(1));

    while(!pool->IsDestroy())
    {
        timerMgr.Drive();
    }

    KERNEL_NS::LibTimer::Delete_LibTimer(timer);
    timerMgr.Close();
}

class SnowflakeInfoWrap : public KERNEL_NS::ITlsObj
{
    virtual void Destoy() override
    {

    }

    virtual const char *GetObjTypeName()
    {
        return "SnowflakeInfoWrap";
    }

public:
    KERNEL_NS::SnowflakeInfo _snowFlakeInfo = {0};
};

static void CurlTask(KERNEL_NS::LibThreadPool *pool)
{
    DEF_STATIC_THREAD_LOCAL_DECLEAR SnowflakeInfoWrap *snowFlakeInfo = NULL;
    if(!snowFlakeInfo)
    {
        auto tlsStack = KERNEL_NS::TlsUtil::GetTlsStack();
        snowFlakeInfo = tlsStack->New<SnowflakeInfoWrap>();
        const auto threadId = KERNEL_NS::SystemUtil::GetCurrentThreadId();
        KERNEL_NS::GuidUtil::InitSnowFlake(snowFlakeInfo->_snowFlakeInfo, threadId, KERNEL_NS::LibTime::Now().GetSecTimestamp());
    }

    auto &snowflakeRaw = snowFlakeInfo->_snowFlakeInfo;

    CURL *curl;  
    CURLcode res;  
    std::string readBuffer;  

    // 时间
    const auto &nowTime = KERNEL_NS::LibTime::Now();
    const auto &timeStr = nowTime.FormatAsUtc("%Y-%m-%dT%H:%M:%SZ");

    auto id = KERNEL_NS::GuidUtil::Snowflake(snowflakeRaw);

    const auto url = KERNEL_NS::LibString().AppendFormat("https://www.baidu.com");
  
    curl = curl_easy_init();  
    if(curl) 
    {  
        // 构建header
        struct curl_slist *headers = NULL;  
        headers = curl_slist_append(headers, "Content-Type: application/json");  
  
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());  
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);  
        curl_easy_setopt(curl, CURLOPT_POST, 1L);  
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, s_jsonBodyData.c_str());  
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);  
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);  
  
        // 如果需要验证SSL证书，确保设置了CURLOPT_CAINFO或CURLOPT_CAPATH  
        // 例如: curl_easy_setopt(curl, CURLOPT_CAINFO, "/path/to/cert.pem");  
        
        // 忽略SSL证书验证（仅用于测试）  
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);  
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);  

        // 执行GET请求  
        ++s_Qps; 
        res = curl_easy_perform(curl); 
        if(res != CURLE_OK) {  
            ++s_failQps;
            KERNEL_NS::LibString errStr;
            errStr = curl_easy_strerror(res);
            g_Log->Error(LOGFMT_NON_OBJ_TAG(TestCurl, "curl_easy_perform() failed: %s"), errStr.c_str());
        } else {  
            
            // std::cout << "Received: " << readBuffer << std::endl;  
        }  
  
        // 清理  
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);  
    }    
    else
    {
        ++s_failQps;
    }
}

static void GenerateTask(KERNEL_NS::LibThreadPool *pool)
{
    KERNEL_NS::TimerMgr timerMgr;
    timerMgr.Launch(NULL);

    KERNEL_NS::LibTimer *timer = KERNEL_NS::LibTimer::New_LibTimer(&timerMgr);
    timer->SetTimeOutHandler([pool](KERNEL_NS::LibTimer *t)
    {
        pool->AddTask(&CurlTask);
        ++s_curlTask;
    });
    timer->Schedule(s_genCurlInterval);
    while(!pool->IsDestroy())
    {
        timerMgr.Drive();
    }

    KERNEL_NS::LibTimer::Delete_LibTimer(timer);
    timerMgr.Close();
}

void TestCurl::Run()
{
    // 构造数据
    nlohmann::json jsonObject;

    KERNEL_NS::LibString ip;
    KERNEL_NS::IPUtil::GetIpByHostName("www.baidu.com", ip, 0, false);

    KERNEL_NS::SnowflakeInfo snow;
    const auto threadId = KERNEL_NS::SystemUtil::GetCurrentThreadId();
    KERNEL_NS::GuidUtil::InitSnowFlake(snow, threadId, KERNEL_NS::LibTime::Now().GetSecTimestamp());
    auto id = KERNEL_NS::GuidUtil::Snowflake(snow);

    // 构造数据
    nlohmann::json serviceParams;
    serviceParams["id"] = id;
    serviceParams["ip"] = ip.GetRaw();
    jsonObject["service_params"] = serviceParams;
    s_jsonBodyData = jsonObject.dump();

    KERNEL_NS::LibThreadPool *pool =  new KERNEL_NS::LibThreadPool();

    pool->Init(s_threadNum, s_threadNum);
    pool->Start();

    pool->AddTask(&GenerateTask);
    pool->AddTask(&Monitor);

    getchar();

    if(pool->HalfClose())
        pool->FinishClose();

    delete pool;

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCurl, "finish curl task..."));
}