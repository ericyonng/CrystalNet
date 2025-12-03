# 引入协程背景
* 有大量的异步业务逻辑, 传统的回调代码割裂, 可读性差, 不可避免的回调地狱
* 简化编码复杂度，希望底层能够支持协程, 简化跨线程或者实现rpc的能力

# 项目
**Github**: [CrystalNet](https://github.com/ericyonng/CrystalNet "CrystalNet")
* 协程框架作为CrystalNet的一个底层支持存在
* 协程源码路径:kernel/include/kernel/comp/Coroutines
* 测试case: TestCoroutine.h/TestCoroutine.cpp TestPoller.h/TestPoller.cpp

# 细节介绍
* 封装一个通用的协程类template<typename T> class CoTask<T>
* 封装一个阻塞等待类CoWaiter,并提供阻塞等待接口:CoTask<> Waiting(), 用于等待条件满足时唤醒协程
* 设计封装了一个Poller，主要用于处理事件循环, 协程的suspend时候会向Poller抛异步任务调度协程, 直到在CoWaiting时永久阻塞
* CoTask提供GetParam让用户在协程阻塞时获取到协程句柄，方便用户在条件满足时通过协程句柄唤醒协程
* Poller提供SendAsync接口实现跨线程的通信(协程方式提供)
  ``` C++
  // 跨线程协程消息(otherPoller也可以是自己)
    // req暂时只能传指针，而且会在otherChannel（可能不同线程）释放
    // req/res 必须实现Release, ToString接口
    template<typename ResType, typename ReqType>
    requires requires(ReqType req, ResType res)
    {
        // req/res必须有Release接口
        req.Release();
        res.Release();
    
        // req/res必须有ToString接口
        req.ToString();
        res.ToString();
    }
    CoTask<KERNEL_NS::SmartPtr<ResType, AutoDelMethods::Release>> SendToAsync(Poller &otherPoller, ReqType *req)
    {
        // 1.ptr用来回传ResType
        KERNEL_NS::SmartPtr<ResType *,  KERNEL_NS::AutoDelMethods::CustomDelete> ptr(KERNEL_NS::KernelCastTo<ResType *>(
            kernel::KernelAllocMemory<KERNEL_NS::_Build::TL>(sizeof(ResType **))));
        ptr.SetClosureDelegate([](void *p)
        {
            // 释放packet
            auto castP = KERNEL_NS::KernelCastTo<ResType*>(p);
            if(*castP)
                (*castP)->Release();

            KERNEL_NS::KernelFreeMemory<KERNEL_NS::_Build::TL>(castP);
        });
        *ptr = NULL;

        // 设置stub => ResType的事件回调
        UInt64 stub = ++_maxStub;
        KERNEL_NS::SmartPtr<KERNEL_NS::TaskParamRefWrapper, KERNEL_NS::AutoDelMethods::Release> params = KERNEL_NS::TaskParamRefWrapper::NewThreadLocal_TaskParamRefWrapper();
        SubscribeStubEvent(stub, [ptr, params](KERNEL_NS::StubPollerEvent *ev) mutable 
        {
            KERNEL_NS::ObjectPollerEvent<ResType> *finalEv = KernelCastTo<KERNEL_NS::ObjectPollerEvent<ResType>>(ev);
            // 将结果带出去
            *ptr = finalEv->_obj;
            finalEv->_obj = NULL;
            
            // 唤醒Waiter
            auto &coParam = params->_params;
            if(coParam && coParam->_handle)
                coParam->_handle->ForceAwake();
        });
        
        // 发送对象事件 ObjectPollerEvent到 other
        auto iterChannel = _targetPollerRefChannel.find(&otherPoller);
        if(LIKELY(iterChannel != _targetPollerRefChannel.end()))
        {
            auto objEvent = ObjectPollerEvent<ReqType>::New_ObjectPollerEvent(stub, false, this, iterChannel->second);
            objEvent->_obj = req;
            iterChannel->second->Send(objEvent);
        }
        else
        {
            auto objEvent = ObjectPollerEvent<ReqType>::New_ObjectPollerEvent(stub, false, this, nullptr);
            objEvent->_obj = req;
            otherPoller.Push(objEvent);
        }
        
        // 等待 ObjectPollerEvent 的返回消息唤醒
        auto poller = this;
        // 外部如果协程销毁兜底销毁资源
        auto releaseFun = [stub, poller]()
        {
            poller->UnSubscribeStubEvent(stub);
        };
        auto delg = KERNEL_CREATE_CLOSURE_DELEGATE(releaseFun, void);
        co_await KERNEL_NS::Waiting().SetDisableSuspend().GetParam(params).SetRelease(delg);
        if(LIKELY(params->_params))
        {
            auto &pa = params->_params; 
            if(pa->_errCode != Status::Success)
            {
                g_Log->Warn(LOGFMT_OBJ_TAG("waiting err:%d, stub:%llu, req:%p")
                    , pa->_errCode, stub, req);
                
                UnSubscribeStubEvent(stub);
            }

            // 销毁waiting协程
            if(pa->_handle)
                pa->_handle->DestroyHandle(pa->_errCode);
        }
        
        // 3.将消息回调中的ResType引用设置成空
        auto res = *ptr;
        *ptr = NULL;
        co_return KERNEL_NS::SmartPtr<ResType,  KERNEL_NS::AutoDelMethods::Release>(res);
    }
  ```
* 提供异步化工具函数: PostCaller
  
  
# 异步编码举例
代码在测试用例:TestPoller, 示例中实现了co_await 请求一个req，并返回一个res
``` C++

class TestTimeoutStartup : public KERNEL_NS::IThreadStartUp
{
    POOL_CREATE_OBJ_DEFAULT_P1(IThreadStartUp, TestTimeoutStartup);

public:
    TestTimeoutStartup(KERNEL_NS::LibEventLoopThread * target)
    : _target(target)
    {
        
    }

    virtual void Run() override
    {
        KERNEL_NS::PostCaller([this]() mutable  -> KERNEL_NS::CoTask<>
        {
            auto targetPoller = co_await _target->GetPoller();

            auto req = HelloWorldReq::New_HelloWorldReq();
            auto res = co_await targetPoller->template SendAsync<HelloWorldRes, HelloWorldReq>(req).SetTimeout(KERNEL_NS::TimeSlice::FromSeconds(5));

            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestTimeoutStartup, "res return"));
        });
    }
    
    virtual void Release() override
    {
        TestTimeoutStartup::Delete_TestTimeoutStartup(this);
    }

    KERNEL_NS::LibEventLoopThread * _target;
};
```