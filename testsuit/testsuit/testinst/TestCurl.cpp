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
 * Description: 网上的代码繁杂建议参照curl的范例来写,别结合curl_multi相关接口
*/
#include <pch.h>
#include <curl/curl.h>
#include <testsuit/testinst/TestCurl.h>

KERNEL_NS::LibString s_jsonBodyData;
KERNEL_NS::TimeSlice s_genCurlInterval = KERNEL_NS::TimeSlice::FromMilliSeconds(1);
KERNEL_NS::TimeSlice s_batchInterval = KERNEL_NS::TimeSlice::FromMilliSeconds(100);

Int32 s_threadNum = 8;
static std::atomic<UInt64> s_threadLogicId{0};

std::atomic<Int64> s_Qps{0};
std::atomic<Int64> s_waitingComplete{0};
Int32 s_disableWait = 0;
static Int64 s_uplimit = 10000;
std::atomic_bool s_canSend{true};
std::atomic<Int64> s_failQps{0};
std::atomic<Int64> s_curlTask{0};
static std::vector<KERNEL_NS::LibThread *> s_threads;
std::atomic<Int32> s_rrTask{0};
std::atomic<Int64> s_successQps{0};
std::atomic<Int64> s_sendTimeMs{0};
std::atomic<Int64> s_totalCostTimeMs{0};
std::atomic<Int64> s_workingNum{0};

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) 
{  
    ((std::string*)userp)->append((char*)contents, size * nmemb);  
    return size * nmemb;  
}

class SnowflakeInfoWrap : public KERNEL_NS::ITlsObj
{
    virtual void OnDestroy() override
    {

    }

    virtual const char *GetObjTypeName() const override
    {
        return "SnowflakeInfoWrap";
    }

public:
    KERNEL_NS::SnowflakeInfo _snowFlakeInfo = {0};
};

// 同步方式, 性能太低了
static void CurlTask(KERNEL_NS::LibThread *pool)
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

    curl = curl_easy_init();  
    if(curl) 
    {  
        // 构建header
        struct curl_slist *headers = NULL;  
        headers = curl_slist_append(headers, "Content-Type: application/json");  
  
        curl_easy_setopt(curl, CURLOPT_URL, "https://www.baidu.com");  
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
        s_rrTask = ++s_rrTask % s_threads.size();
        auto idx = s_rrTask.load();
        auto thread = s_threads[idx];

        thread->AddTask(&CurlTask);
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

#pragma region // async mode

// http request class
/*
该类是对easy handle的封装，主要做一些初始化操作，设置url 、发送的内容
header以及回调函数
*/
class HttpRequest 
{
    POOL_CREATE_OBJ_DEFAULT(HttpRequest);

public:
  using FinishCallback = std::function<int()>;

  HttpRequest(const std::string& url, const std::string& post_data) :
              url_(url), post_data_(post_data) {
  }
  int Init(const std::vector<std::string> &headers);

  CURL* GetHandle();
  const std::string& Response();

  void SetFinishCallback(const FinishCallback& cb);

  int OnFinish(int response_ret);

  CURLcode Perform();

  void AddMultiHandle(CURLM* multi_handle);

  void Clear(CURLM* multi_handle);

 private:
  void AppendData(char* buffer, size_t size, size_t nmemb);

  static size_t WriteData(char *buffer, size_t size, size_t nmemb, void *userp);

  CURL* handle_ = nullptr;

  std::string url_;

  std::string post_data_;


  struct curl_slist *chunk_ = nullptr;

  FinishCallback cb_;
  int response_ret_;
  std::string response_;
};

POOL_CREATE_OBJ_DEFAULT_IMPL(HttpRequest);


/*http_request.h的实现*/
int HttpRequest::Init(const std::vector<std::string> &headers) 
{
  handle_ = curl_easy_init();
  if (handle_ == nullptr) {
    return Status::Failed;
  }
  CURLcode ret = curl_easy_setopt(handle_, CURLOPT_URL, url_.c_str());
  if (ret != CURLE_OK) {
    return ret;
  }

  ret = curl_easy_setopt(handle_, CURLOPT_WRITEFUNCTION,
          &HttpRequest::WriteData);
  if (ret != CURLE_OK) {
    return ret;
  }

  ret = curl_easy_setopt(handle_, CURLOPT_WRITEDATA, this);
  if (ret != CURLE_OK) {
    return ret;
  }

  ret = curl_easy_setopt(handle_, CURLOPT_NOSIGNAL, 1);
  if (ret != CURLE_OK) {
    return ret;
  }

  ret = curl_easy_setopt(handle_, CURLOPT_PRIVATE, this);
  if (ret != CURLE_OK) {
    return ret;
  }

  ret = curl_easy_setopt(handle_, CURLOPT_POSTFIELDSIZE, post_data_.length());
  if (ret != CURLE_OK) {
    return ret;
  }
  ret = curl_easy_setopt(handle_, CURLOPT_POSTFIELDS, post_data_.c_str());
  if (ret != CURLE_OK) {
    return ret;
  }

    // 设长点不然容易超时
  ret = curl_easy_setopt(handle_, CURLOPT_TIMEOUT_MS, 60000);
  if (ret != CURLE_OK) {
    return ret;
  }

  // 设置短连接(没有设置此项目默认是长连接)
  ret = curl_easy_setopt(handle_, CURLOPT_FRESH_CONNECT, 1L);
  if (ret != CURLE_OK) {
    return ret;
  }
//  ret = curl_easy_setopt(handle_, CURLOPT_CONNECTTIMEOUT_MS, 10);

  ret = curl_easy_setopt(handle_, CURLOPT_DNS_CACHE_TIMEOUT,
          300000);
  if (ret != CURLE_OK) {
    return ret;
  }

  for (auto item : headers) {
    chunk_ = curl_slist_append(chunk_, item.c_str());
  }

  ret = curl_easy_setopt(handle_, CURLOPT_HTTPHEADER, chunk_);

  if (ret != CURLE_OK) {
    return ret;
  }
  //  设置http header
    curl_easy_setopt(handle_, CURLOPT_SSL_VERIFYPEER, 0L);  
    curl_easy_setopt(handle_, CURLOPT_SSL_VERIFYHOST, 0L);  

  return CURLE_OK;
}

//获取easy handle
CURL* HttpRequest::GetHandle() {
  return handle_;
}

// 获取http server端的响应
const std::string& HttpRequest::Response() {
  return response_;
}

//设置回调函数，当server返回完成之后，调用
void HttpRequest::SetFinishCallback(const FinishCallback& cb) {
  cb_ = cb;
}

// libcurl 错误码信息
int HttpRequest::OnFinish(int response_ret) {
  response_ret_ = response_ret;

  if (cb_) {
    return cb_();
  }

  return Status::Success;
}

// 执行http请求
CURLcode HttpRequest::Perform() {
  CURLcode ret = curl_easy_perform(handle_);
  return ret;
}

// 将easy handle 加入到被监控的multi handle
void HttpRequest::AddMultiHandle(CURLM* multi_handle) {
  if (multi_handle != nullptr) {
    curl_multi_add_handle(multi_handle, handle_);
  }
}

// 释放资源
void HttpRequest::Clear(CURLM* multi_handle) {
    if(chunk_)
      curl_slist_free_all(chunk_);
  
  if ((multi_handle != nullptr) && (handle_ != NULL)) 
    curl_multi_remove_handle(multi_handle, handle_);

    if(handle_)
        curl_easy_cleanup(handle_);

    chunk_ = NULL;
    handle_ = NULL;
    multi_handle = NULL;
}

// 获取返回
void HttpRequest::AppendData(char* buffer, size_t size, size_t nmemb) 
{
  response_.append(buffer, size * nmemb);
}
// 回调函数，获取返回内容
size_t HttpRequest::WriteData(
        char* buffer, size_t size,
        size_t nmemb, void* userp) {
  HttpRequest* req = static_cast<HttpRequest*>(userp);
  req->AppendData(buffer, size, nmemb);
  return size * nmemb;
}

KERNEL_NS::SpinLock s_lck;
KERNEL_NS::LibList<HttpRequest *> *s_reqQueue = KERNEL_NS::LibList<HttpRequest *>::New_LibList();

// 单次批量执行请求数量
Int32 batchLimit = 200;

KERNEL_NS::SpinLock s_curlmLck;
std::vector<CURLM *> s_curlms;


static void DoCurlMultiTask(UInt64 threadId)
{
    if(s_disableWait == 0)
    {
        if(!s_canSend)
            return;
    }

    DEF_STATIC_THREAD_LOCAL_DECLEAR SnowflakeInfoWrap *snowFlakeInfo = NULL;
    if(!snowFlakeInfo)
    {
        auto tlsStack = KERNEL_NS::TlsUtil::GetTlsStack();
        snowFlakeInfo = tlsStack->New<SnowflakeInfoWrap>();
        KERNEL_NS::GuidUtil::InitSnowFlake(snowFlakeInfo->_snowFlakeInfo, threadId, KERNEL_NS::LibTime::Now().GetSecTimestamp());
    }
    auto &snowflakeRaw = snowFlakeInfo->_snowFlakeInfo;

    const auto &startTime = KERNEL_NS::LibTime::Now();

    //此处读者来实现,基本功能如下：
    // 1、获取上游的请求内容，从里面获取要发送http的相关信息
    // 2、通过步骤1获取的相关信息，来创建HttpRequest对象
    // 3、将该HttpRequest对象跟multi_handle_对象关联起来
    Int32 loopLimit = batchLimit;
    auto multi_handle_ = curl_multi_init();
    for(; loopLimit > 0; --loopLimit)
    {
        auto newReq = HttpRequest::NewThreadLocal_HttpRequest("https://www.baidu.com", s_jsonBodyData.GetRaw());
        auto ret = newReq->Init({
            "Content-Type: application/json",
        });
        if(ret != CURLE_OK)
        {
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(TestCurl, "HttpRequest init fail ret:%d,%s"), ret, curl_easy_strerror((CURLcode)ret));
            newReq->Clear(NULL);
            HttpRequest::Delete_HttpRequest(newReq);
            continue;
        }

        newReq->AddMultiHandle(multi_handle_);
        ++s_Qps;
        const auto newNum = ++s_waitingComplete;

        if(newNum >= s_uplimit)
            s_canSend = false;
    }

    Int32 still_running = 0;
    curl_multi_perform(multi_handle_, &still_running);

    // 直到发完
    while(still_running)
    {
        struct timeval timeout;
        int rc; /* select() return code */
        CURLMcode mc; /* curl_multi_fdset() return code */

        fd_set fdread;
        fd_set fdwrite;
        fd_set fdexcep;
        int maxfd = -1;

        long curl_timeo = -1;

        FD_ZERO(&fdread);
        FD_ZERO(&fdwrite);
        FD_ZERO(&fdexcep);

        /* set a suitable timeout to play around with */
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        // 设置句柄超时
        curl_multi_timeout(multi_handle_, &curl_timeo);

        if(curl_timeo >= 0) {
        timeout.tv_sec = curl_timeo / 1000;
        if(timeout.tv_sec > 1)
            timeout.tv_sec = 1;
        else
            timeout.tv_usec = (int)(curl_timeo % 1000) * 1000;
        }

        /* get file descriptors from the transfers */
        mc = curl_multi_fdset(multi_handle_, &fdread, &fdwrite, &fdexcep, &maxfd);
        if(mc != CURLM_OK) 
        {
            g_Log->Error(LOGFMT_NON_OBJ_TAG(TestCurl, "curl_multi_fdset fail code:%d"), mc);
            break;
        }

        /* On success the value of maxfd is guaranteed to be >= -1. We call
        select(maxfd + 1, ...); specially in case of (maxfd == -1) there are
        no fds ready yet so we call select(0, ...) --or Sleep() on Windows--
        to sleep 100ms, which is the minimum suggested value in the
        curl_multi_fdset() doc. */

        if(maxfd == -1) 
        {
            #if CRYSTAL_TARGET_PLATFORM_WINDOWS
                KERNEL_NS::SystemUtil::ThreadSleep(100);
                rc = 0;
            #else
                /* Portable sleep for platforms other than Windows. */
                struct timeval wait = { 0, 100 * 1000 }; /* 100ms */
                rc = select(0, NULL, NULL, NULL, &wait);
            #endif
        }
        else 
        {
            /* Note that on some platforms 'timeout' may be modified by select().
                If you need access to the original value save a copy beforehand. */
            rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
        }

        switch(rc) 
        {
        case -1:
        /* select error */
        break;
        case 0: /* timeout */
        default: /* action */
            curl_multi_perform(multi_handle_, &still_running);
        break;
        }
    }

    const auto &sendEnd = KERNEL_NS::LibTime::Now();

    CURLMsg *msg; /* for picking up messages with the transfer status */
    /* See how the transfers went */
    Int32 msgs_left; /* how many messages are left */
    while((msg = curl_multi_info_read(multi_handle_, &msgs_left))) 
    {
        if(msg->msg == CURLMSG_DONE) 
        {

            CURL *easy_handle = msg->easy_handle;  
            char *url;
            curl_easy_getinfo(msg->easy_handle, CURLINFO_EFFECTIVE_URL, &url);

            HttpRequest* req = nullptr;
            // In fact curl_easy_getinfo will
            // always return CURLE_OK if CURLINFO_PRIVATE was set.
            auto curl_ret = curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &req);

            if(msg->data.result == CURLE_OK)
            {
                // 处理完成的请求，例如获取响应数据  
                // ...  
                Int32 response_code;  
                curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &response_code);

                if(response_code != 200)
                {
                    ++s_failQps;
                    g_Log->Warn(LOGFMT_NON_OBJ_TAG(TestCurl, "DoCurlMultiTask fail http request fail code:%d"), response_code);
                }
                else
                {
                    ++s_successQps;
                }

                if (curl_ret != CURLE_OK) 
                {
                    g_Log->Warn(LOGFMT_NON_OBJ_TAG(TestCurl, "curl_easy_getinfo failed, curlf_ret=%d, %s"), curl_ret, curl_easy_strerror(curl_ret));
                }
            }
            else
            {
                ++s_failQps;
                g_Log->Warn(LOGFMT_NON_OBJ_TAG(TestCurl, "DoCurlMultiTask fail Contection fail url:%s, err:%d, %s"), url, msg->data.result, curl_easy_strerror((CURLcode)msg->data.result));
            }

            if(req)
            {
                req->Clear(multi_handle_);
                HttpRequest::DeleteThreadLocal_HttpRequest(req);
            }
            else
            {
                // 从multi_handle中移除这个easy_handle  
                curl_multi_remove_handle(multi_handle_, msg->easy_handle); 

                // 清理  
                curl_easy_cleanup(msg->easy_handle);  
            }
        }
    }

    auto newNum = (s_waitingComplete -= batchLimit);
    if(newNum == 0)
        s_canSend = true;

    curl_multi_cleanup(multi_handle_);

    const auto &finalTime = KERNEL_NS::LibTime::Now();

    const auto &totalCost = finalTime - startTime;
    const auto &sendCost = sendEnd - startTime;
    const auto costSingle = totalCost.GetTotalMilliSeconds() / (Int64)(batchLimit);
    const auto sendCostSingle = sendCost.GetTotalMilliSeconds() / (Int64)(batchLimit);
    s_sendTimeMs.exchange(sendCostSingle);
    s_totalCostTimeMs.exchange(costSingle);
    if(g_Log->IsEnable(KERNEL_NS::LogLevel::Debug))
        g_Log->Debug(LOGFMT_NON_OBJ_TAG(TestCurl, "working threadnum:%d, batch num:%d, total cost:%lld(ms), send cost:%lld(ms), single total cost:%lld(ms), single send cost:%lld(ms)")
        , s_workingNum.load(), batchLimit, totalCost.GetTotalMilliSeconds(), sendCost.GetTotalMilliSeconds(), costSingle, sendCostSingle);
}

// 一次请求大概30ms
const Int64 requestMs = 30;

// 1秒限制10000个令牌
const Int32 tokenNumLimit = 10000;

// 
static void CurlAlwaysTask(KERNEL_NS::LibThreadPool *pool)
{
    const auto threadId = ++s_threadLogicId;
    ++s_workingNum;

    KERNEL_NS::TimerMgr timerMgr;
    timerMgr.Launch(NULL);

    Int32 timeLoopIndex = 0;
    KERNEL_NS::LibTimer *timer = KERNEL_NS::LibTimer::New_LibTimer(&timerMgr);
    timer->SetTimeOutHandler([pool, threadId, &timeLoopIndex](KERNEL_NS::LibTimer *t)
    {
        if(++timeLoopIndex == threadId)
        {
            timeLoopIndex = 0;
            const auto &beginTime = KERNEL_NS::LibTime::Now();
            DoCurlMultiTask(threadId);
            const auto &endTime = KERNEL_NS::LibTime::Now();

            // 更新剩余定时市场
            const auto &diff = (endTime - beginTime);
            const auto &leftSlice = s_batchInterval > diff ? (s_batchInterval - diff) : KERNEL_NS::TimeSlice::FromMicroSeconds(0);
            t->Schedule(leftSlice);
        }
    });
    timer->Schedule(s_batchInterval);
    while(!pool->IsDestroy())
    {
        timerMgr.Drive();
    }

    KERNEL_NS::LibTimer::Delete_LibTimer(timer);
    timerMgr.Close();

    --s_workingNum;
}

static void CurlMultiTask(KERNEL_NS::LibThreadPool *pool)
{
    DEF_STATIC_THREAD_LOCAL_DECLEAR UInt64 threadId = 0;
    if(threadId == 0)
        threadId = ++s_threadLogicId;

    DoCurlMultiTask(threadId);
}


static void HttpRequestGenTask(KERNEL_NS::LibThreadPool *pool)
{
    KERNEL_NS::TimerMgr timerMgr;
    timerMgr.Launch(NULL);

    KERNEL_NS::LibTimer *timer = KERNEL_NS::LibTimer::New_LibTimer(&timerMgr);
    timer->SetTimeOutHandler([pool](KERNEL_NS::LibTimer *t)
    {
        ++s_curlTask;

        pool->AddTask(&CurlMultiTask);
    });
    timer->Schedule(s_genCurlInterval);
    while(!pool->IsDestroy())
    {
        timerMgr.Drive();
    }

    KERNEL_NS::LibTimer::Delete_LibTimer(timer);
    timerMgr.Close();

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCurl, "QUIT HttpRequestGenTask"));
}


#pragma endregion

void TestCurl::Run(int argc, char const *argv[])
{
    Int32 count = 0;
    Int32 threadNum = 1;
    Int64 genTickIntervalMicroSeconds = 0;
    Int64 batchIntervalMicroSeconds = 0;
    Int32 disableWait = 0;
    Int32 upperLimit = 0;
    KERNEL_NS::ParamsHandler::GetStandardParams(argc, argv, [&count, &threadNum, &genTickIntervalMicroSeconds, &batchIntervalMicroSeconds, &disableWait, &upperLimit](const KERNEL_NS::LibString &param, std::vector<KERNEL_NS::LibString> &leftParam){
        if(count == 1)
        {
            threadNum = KERNEL_NS::StringUtil::StringToInt32(param.strip().c_str());
        }
        else if(count == 2)
        {
            batchLimit = KERNEL_NS::StringUtil::StringToInt32(param.strip().c_str());
        }
        else if(count == 3)
        {
            batchIntervalMicroSeconds = KERNEL_NS::StringUtil::StringToInt64(param.strip().c_str());
        }
        else if(count == 4)
        {
            upperLimit = KERNEL_NS::StringUtil::StringToInt32(param.strip().c_str());
        }
        else if(count == 5)
        {
            disableWait = KERNEL_NS::StringUtil::StringToInt32(param.strip().c_str());
        }

        ++count;
        return true;
    });

    if(upperLimit > 0)
        s_uplimit = upperLimit;

    if(batchLimit <= 0)
        batchLimit = 200;

    if(batchIntervalMicroSeconds != 0)
        s_batchInterval = KERNEL_NS::TimeSlice::FromMicroSeconds(batchIntervalMicroSeconds);

    s_threadNum = s_threadNum > threadNum ? s_threadNum : threadNum;
    if(genTickIntervalMicroSeconds > 0)
        s_genCurlInterval = KERNEL_NS::TimeSlice::FromMicroSeconds(genTickIntervalMicroSeconds);

    if(disableWait > 0)
        s_disableWait = disableWait;

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCurl, "thread num:%d, gen micro seconds:%lld, batchLimit:%d, batch interval:%lld(microseconds), s_disableWait:%d,")
    , s_threadNum, s_genCurlInterval.GetTotalMicroSeconds(), batchLimit, s_batchInterval.GetTotalMicroSeconds(), s_disableWait);

    // 构造数据
    nlohmann::json jsonObject;

    KERNEL_NS::LibString ip;
    KERNEL_NS::IPUtil::GetIpByHostName("www.baidu.com", ip, 0, false);

    // 构造数据
    nlohmann::json serviceParams;
    serviceParams["client_ip"] = ip.GetRaw();
    s_jsonBodyData = jsonObject.dump();

    KERNEL_NS::LibThreadPool *pool =  new KERNEL_NS::LibThreadPool();

    pool->Init(0, s_threadNum);
    pool->Start();

    const Int32 taskThreadNum = s_threadNum - 1;
    for(Int32 idx = 0; idx < taskThreadNum; ++idx)
        pool->AddTask(&CurlAlwaysTask, true);

    getchar();

    if(pool->HalfClose())
        pool->FinishClose();

    KERNEL_NS::ContainerUtil::DelContainer(*s_reqQueue, [](HttpRequest *req){
        HttpRequest::Delete_HttpRequest(req);
    });
    KERNEL_NS::LibList<HttpRequest *>::Delete_LibList(s_reqQueue);

    delete pool;

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCurl, "finish curl task..."));
}