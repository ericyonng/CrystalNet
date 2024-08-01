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
 * Date: 2024-02-28 16:45:21
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include "TestCoroutine.h"
#include <unordered_set>
#include <iostream>
// #include <format>
#include <functional>

#pragma region co

#include <coroutine>

// 协程打印堆栈
#include <source_location>

#include <exception>
#include <variant>
#include <optional>

#pragma endregion

// namespace std {

// // hash surport
// template <class _Tp>
// struct hash<coroutine_handle<_Tp>> {
//     using argument_type = coroutine_handle<_Tp>;
//     using result_type = size_t;
//     using __arg_type = coroutine_handle<_Tp>;
//     size_t operator()(__arg_type const& __v) const noexcept
//     {
//         return hash<void*>()(__v.address());
//     }
// };
// }

// class CoroutineScheduler
// {
// public:
//     template<typename T>
//     void AddCoroutineHandle(std::coroutine_handle <>h, T &&cb)
//     {
//         auto delg = KERNEL_CREATE_CLOSURE_DELEGATE(cb, void);
//         AddCoroutineHandle(h, delg);
//     }

//     void AddCoroutineHandle(std::coroutine_handle <>h, KERNEL_NS::IDelegate<void> *cb)
//     {
//         _handlerRefResumeCallback.insert(std::make_pair(h, cb));
//     }

//     void RemoveCoroutine(std:: coroutine_handle <>h)
//     {
//         auto iter = _handlerRefResumeCallback.find(h);
//         if(iter == _handlerRefResumeCallback.end())
//             return;
        
//         iter->second->Release();
//         _handlerRefResumeCallback.erase(h);
//     }

//     void update()
//     {
//         std::unordered_map<std:: coroutine_handle <>, KERNEL_NS::IDelegate<void> *> tmp;
//         tmp.swap(_handlerRefResumeCallback);
//         for(auto iter : tmp)
//         {
//             iter.second->Invoke();
//             iter.second->Release();
//         }
//     }

// private:
//     std::unordered_map<std:: coroutine_handle <>, KERNEL_NS::IDelegate<void> *> _handlerRefResumeCallback;
// };

// CoroutineScheduler s_scheduler;

// class CoroutineAwaitable
// {
// public:
//     CoroutineAwaitable() {
//         _scheduler = &s_scheduler;
//     }
//     virtual ~CoroutineAwaitable() {}
    
//     virtual bool await_ready() const
//     {
//         return true;
//     }

//     virtual void await_suspend(std:: coroutine_handle <>h)
//     {

//     }

//     void SetScheduler(CoroutineScheduler *scheduler)
//     {
//         _scheduler = scheduler;
//     }

//     CoroutineScheduler *_scheduler;
// };

// template<typename T>
// struct WrapValue
// {
//     T _value;
// };

// template<typename T>
// class AwaitableType;

// template<typename T>
// class AwaitableTypePromise;

// template<typename T>
// struct std::coroutine_traits<AwaitableType<T>>
// {
//     using promise_type = AwaitableTypePromise<T>;
// };

// template<typename PromiseType>
// struct AwaitableTypeRaw
// {
//     AwaitableTypeRaw(std::coroutine_handle<PromiseType> h) noexcept
//     :_co(h)
//     {

//     }

//     bool await_ready() const noexcept
//     {
//         return !_co || _co.done();
//     }

//     bool await_suspend(std::coroutine_handle<> awaitingCo) noexcept
//     {
//         _co.resume();
//         return _co.promise().try_set_continuation(awaitingCo);
//     }

//     decltype(auto) await_resume()
//     {
//         return this->_co.promise().result();
//     }

//     std::coroutine_handle<PromiseType> _co;
// };

// template<typename T>
// class AwaitableType
// {
// public:
//     using value_type = T;
//     AwaitableType()
//     :_coroutine(nullptr)
//     {

//     }

//     explicit AwaitableType(std::coroutine_handle<typename std::coroutine_traits<AwaitableType>::promise_type> h)
//     :_coroutine(h)
//     {

//     }

//     ~AwaitableType()
//     {
//         if(_coroutine)
//             _coroutine.destroy();
//     }

//     auto operator co_await() const& noexcept{
//         return AwaitableTypeRaw{_coroutine};
//     }

//     // 协程是否准备好了
//     // bool await_ready() const override
//     // {
//     //     return false;
//     // }

//     // // 协程挂起/并添加到schedule
//     // void await_suspend(std:: coroutine_handle <>h) override
//     // {
//     //     _scheduler->AddCoroutineHandle(h, [this, h](){
//     //         this->_value = 100;
//     //         h();
//     //     });
//     // }

//     // T await_resume()
//     // {
//     //     return this->_value;   
//     // }

//     // T get()
//     // {
//     //     return this->_value;
//     // }

// private:
//     std::coroutine_handle<typename std::coroutine_traits<AwaitableType>::promise_type> _coroutine;
// };

// template<typename T>
// class AwaitableTypePromise
// {
// public:
//     AwaitableTypePromise()
//     :_state{false}
//     {

//     }

//     auto initial_suspend() noexcept
//     {
//         return std::suspend_never{};
//     }

//     T result()
//     {
//         return std::move(_value);
//     }

//     friend struct final_awaitable;
//     struct final_awaitable
//     {
//         bool await_ready() const noexcept
//         {
//             return false;
//         }

//         template<typename PromiseType>
//         void await_suspend(std::coroutine_handle<PromiseType> co) noexcept
//         {
//             AwaitableTypePromise &p = co.promise();
//             if(p._state.exchange(true, std::memory_order_acq_rel))
//             {
//                 p._h.resume();
//             }
//         }

//         void await_resume() noexcept {}
//     };

//     auto final_suspend() noexcept
//     {
//         return final_awaitable{};
//     }

//     bool try_set_continuation(std::coroutine_handle<> co)
//     {
//         _h = co;
//         return _state.exchange(true, std::memory_order_acq_rel);
//     }

//     AwaitableType<T> get_return_object() noexcept
//     {
//         return AwaitableType<T>{std::coroutine_handle<AwaitableTypePromise>::from_promise(*this)};
//     }

//     void unhandled_exception() noexcept{

//     }

//     template<typename ValueT>
//     void return_value(ValueT&& value)
//     {
//         _value = value;
//     }

// private:
//     std::coroutine_handle<> _h;
//     T _value;
//     std::atomic_bool _state;
// };



// static AwaitableType<Int32> GetCoInt()
// {
//     std::cout << "GetCoInt\n";
//     co_return 500;
// }

// static AwaitableType<Int32> TestCoInt()
// {
//     std::cout << "TestCoInt:\n";

//     auto v = co_await GetCoInt();
//     std::cout << "after TestCoInt:\n";
//     co_return v;
// }

// template<typename T>
// struct co_promise_type;

// template<typename T>
// struct co_generator{

// using Handle = std::coroutine_handle<co_promise_type<T>>;

// void next()
// {
//     return _co_handle.resume();
// }

// bool done()
// {
//     return _co_handle.done();
// }

// T current_value()
// {
//     return std::move(_co_handle.promise()._value);
// }

// co_generator(co_generator<T> &&other) noexcept
// :_co_handle(std::exchange(other._co_handle, {}))
// {

// }

// ~co_generator()
// {
//     if(_co_handle)
//     {
//         _co_handle.destroy();
//     }
// }

// private:
//     co_generator(Handle h):_co_handle(h){

//     }

//     Handle _co_handle;
// };

// template <typename T>
// struct co_promise_type
// {
//     co_generator<T> get_return_object()
//     {
//         return {co_generator<T>::Handle::from_promise(*this)};
//     }

//     auto  initial_suspend() noexcept {return std::suspend_never{};}

//     // 这个地方需要挂起，因为让协程由程序员手动销毁, 不自动销毁
//     auto final_suspend() noexcept
//     {
//         return std::suspend_always{};
//     }

//     void unhandled_exception()
//     {
//         // TODO:这个地方是因为协程出现异常了, 需要保存异常
//         _error = std::current_exception();
//     }

//     void return_void()
//     {

//     }

//     // co_yield 返回一个value值
//     template<typename ValueType>
//     auto yield_value(ValueType&& value)
//     {
//         _value = std::forward<ValueType>(value);

//         // co_yield总是挂起
//         return std::suspend_always{};
//     }

//     // co_return 返回一个value
//     template<typename ValueType>
//     void return_value(ValueType && value)
//     {
//         _value = std::forward<ValueType>(value);
//     }

//     T _value;
//     std::exception_ptr _error;
// };

// 特化coroutine_traits
// template<typename T>
// struct Task;

// template<typename T>
// struct TaskPromise;

// template<typename T>
// struct std::coroutine_traits<Task<T>>
// {
//     using promise_type = TaskPromise<T>;
// };

// // Promise

// template<typename ResultValue>
// struct TaskResult
// {
//     TaskResult(ResultValue&& v)
//     {

//     }
// };

// // future
// template<typename T>
// struct Task
// {
//     Task()
//     :_coroutine(nullptr)
//     {

//     }

//     explicit Task(std::coroutine_handle<typename std::coroutine_traits<Task<T>>::promise_type> h)
//     :_coroutine(h)
//     {
        
//     }

//     ~Task()
//     {
//         // 脱离调度
//         if(_coroutine)
//             _coroutine.destroy();
//     }


//     bool await_ready() const noexcept
//     {
//         return !_coroutine || _coroutine.done();
//     }

// 	template<class _PromiseT/*, typename = std::enable_if_t<traits::is_promise_v<_PromiseT>>*/>
//     void await_suspend(std::coroutine_handle<_PromiseT> awaitingCo) noexcept
//     {
//         if(!_coroutine)
//         {
//             _coroutine  = awaitingCo;
//         }
//     }

//     decltype(auto) await_resume()
//     {
//         return this->_coroutine.promise().result();
//     }

// std::coroutine_handle<typename std::coroutine_traits<Task<T>>::promise_type> _coroutine;
// };

// template<typename T>
// struct TaskPromise
// {
//     // 生成future对象
//     Task<T> get_return_object()
//     {
//         return Task<T>(::std::coroutine_handle<TaskPromise<T>>::from_promise(*this));
//     }

//     // initial 永不挂起
//     auto  initial_suspend() noexcept 
//     {
//         return std::suspend_never{};
//     }

//     // 这个地方需要挂起，因为让协程由程序员手动销毁, 不自动销毁
//     auto final_suspend() noexcept
//     {
//         return std::suspend_always{};
//     }

//     void unhandled_exception()
//     {
//         // TODO:这个地方是因为协程出现异常了, 需要保存异常
//         _error = std::current_exception();
//     }

//     // co_return;
//     // void return_void()
//     // {

//     // }

//     // co_yield 返回一个value值
//     template<typename ValueType>
//     auto yield_value(ValueType&& value)
//     {
//         _value = std::forward<ValueType>(value);

//         // co_yield总是挂起
//         return std::suspend_always{};
//     }

//     // co_return 返回一个value
//     template<typename ValueType>
//     void return_value(ValueType && value)
//     {
//         _value = std::forward<ValueType>(value);
//     }

//     T result()
//     {
//         return _value;
//     }

//     T _value;
//     std::exception_ptr _error;
// };


// Task<Int32> GetFutureValue()
// {
//     co_return 100;
// }



// template<typename T>
// using promise_cb_t = std::function<void(std::function<void(T&& v)>&& resolve_cb)>;

// template<typename T>
// auto promise(promise_cb_t<T>&& cb) {
// struct awaitable {
//     bool await_ready() { return false; }
//     void await_suspend(std::coroutine_handle<> resolve) {
//     cb([this, resolve](T&& v) {
//         ::new (&value) T(std::forward<T>(v));
//         value_inited = true;
//         resolve.resume();
//         });
//     }
//     T&& await_resume() {
//     return std::move(*(reinterpret_cast<T*>(&value)));
//     }
//     awaitable(promise_cb_t<T>&& cb) noexcept : cb(std::move(cb)), value_inited(false) {}
//     ~awaitable() noexcept {
//     if (std::exchange(value_inited, false)) {
//         reinterpret_cast<T*>(&value)->~T();
//     }
//     }
//     awaitable(const awaitable&) = delete;
//     awaitable& operator=(const awaitable&) = delete;
// private:
//     alignas(T) std::byte value[sizeof(T)];
//     promise_cb_t<T> cb;
//     bool value_inited;
// };
// return awaitable(std::move(cb));
// }

// template<typename T>
// struct async {
// struct awaitable_final;

// struct promise_type {
//     async<T>* a;
//     std::coroutine_handle<> prev_handle;
//     std::coroutine_handle<> handle;
//     bool done;
//     bool final_ready;

//     enum class value_type { empty, value, exception };
//     value_type type = value_type::empty;
//     union {
//     T value;
//     std::exception_ptr exception;
//     };
//     async get_return_object() { 
//     handle = std::coroutine_handle<promise_type>::from_promise(*this);
//     return async(this); 
//     }
//     std::suspend_never initial_suspend() { return {}; }
//     awaitable_final final_suspend() noexcept { return awaitable_final(*this); }
//     template<std::convertible_to<T> From>
//     void return_value(From&& from) {
//     ::new (static_cast<void*>(std::addressof(value)))
//         T(std::forward<From>(from));
//     type = value_type::value;
//     }
//     void unhandled_exception() noexcept {
//     ::new (static_cast<void*>(std::addressof(exception)))
//         std::exception_ptr(std::current_exception());
//     type = value_type::exception;
//     }
//     void destroy() {
//     if (handle) {
//         handle.destroy();
//         handle = nullptr;
//     }
//     }
//     promise_type() noexcept : a(nullptr), done(false), final_ready(false) {}
//     ~promise_type() noexcept {
//     if (type == value_type::value) {
//         value.~T();
//     }
//     else if (type == value_type::exception) {
//         exception.~exception_ptr();
//     }
//     if (a) a->promise = nullptr;
//     }

//     promise_type(const promise_type&) = delete;
//     promise_type& operator=(const promise_type&) = delete;
// };

// promise_type* promise;

// struct awaitable_value {
//     async<T>* a;
//     bool await_ready() { return false; }
//     void await_suspend(std::coroutine_handle<> prev_handle) {
//     if (!a->promise->done) {
//         a->promise->prev_handle = prev_handle;
//     }
//     else {
//         prev_handle.resume();
//     }
//     }
//     T await_resume() {
//     auto r = std::move(a->result());
//     a->destroy();
//     return r;
//     }
//     explicit awaitable_value(async<T>* a) noexcept : a(a) {}
//     awaitable_value(const awaitable_value&) = delete;
//     awaitable_value& operator=(const awaitable_value&) = delete;
// };

// struct awaitable_final {
//     promise_type& promise;
//     bool await_ready() const noexcept {
//     return promise.final_ready; 
//     }
//     void await_suspend(std::coroutine_handle<> h) noexcept {
//     promise.done = true;
//     if (promise.prev_handle) {
//         promise.prev_handle.resume();
//     }
//     }
//     void await_resume() noexcept {}
//     explicit awaitable_final(promise_type& promise) noexcept : promise(promise) {}
//     awaitable_final(const awaitable_final&) = delete;
//     awaitable_final& operator=(const awaitable_final&) = delete;
// };

// auto operator co_await() {
//     return awaitable_value(this);
// }

// T await(std::function<bool()> update) {
//     while (!promise->done && update());
//     auto r = std::move(result());
//     destroy();
//     return r;
// }

// explicit async(promise_type* promise) noexcept : promise(promise) {
//     promise->a = this;
// }
// ~async() {
//     if (promise) {
//     promise->final_ready = true;
//     }
// }
// async(const async&) = delete;
// async& operator=(const async&) = delete;

// T get_result()
// {
//     return result();
// }

// void destroy() {
//     if (promise) {
//     promise->destroy();
//     }
// }

// private:
// T& result() {
//     if (promise->type == promise_type::value_type::value) {
//     return promise->value;
//     }
//     else if (promise->type == promise_type::value_type::exception) {
//     auto e = std::move(promise->exception);
//     destroy();
//     std::rethrow_exception(e);
//     }
//     else {
//     destroy();
//     throw std::exception("co::async return value is empty!");
//     }
// }

// };

// std::function<void(int)> resolve_cb;

// async<int> get_id() {
//   // 等待一个承若协程
//   int r = co_await promise<int>([](auto resolve_cb) {
//     ::resolve_cb = std::move(resolve_cb);
//     });
//   co_return r;
// }

// async<std::string> fun() {
//   int id = co_await get_id();
//   co_return std::to_string(id);
// }

// struct NonCopyable {
// protected:
//     NonCopyable() = default;
//     ~NonCopyable() = default;
//     NonCopyable(NonCopyable&&) = default;
//     NonCopyable& operator=(NonCopyable&&) = default;
//     NonCopyable(const NonCopyable&) = delete;
//     NonCopyable& operator=(const NonCopyable&) = delete;
// };

// /// Task设计
// struct NoWaitAtInitialSuspend {};
// inline constexpr NoWaitAtInitialSuspend no_wait_at_initial_suspend;

// template<typename R = void>
// struct Task : private NonCopyable
// {
//     struct promise_type;
//     using coro_handle = std::coroutine_handle<promise_type>;

// };

// Task<int> GetIdFrom()
// {
//     co_return 100;
// }

// Task<int> GetGlobalId()
// {
//     co_return co_await GetIdFrom();
// }

// class ResultIsUnsetException : public std::exception
// {
// public:
//     virtual char const* what() const noexcept override
//     {
//         return "result is unset";
//     }
// };

// class SelfCoroInvalidException : public std::exception
// {
// public:
//     virtual char const *what() const noexcept override
//     {
//         return "self coro invalid";
//     }
// };


// // 结果
// template<typename T>
// struct Result 
// {
//     constexpr bool HasValue() const noexcept 
//     {
//         return std::get_if<std::monostate>(&_result) == nullptr;
//     }

//     template<typename R>
//     constexpr void SetValue(R&& value) noexcept {
//         _result.template emplace<T>(std::forward<R>(value));
//     }

//     template<typename R> // for promise_type
//     constexpr void return_value(R&& value) noexcept 
//     {
//         return SetValue(std::forward<R>(value));
//     }

//     // co_yield 返回一个value值
//     template<typename R>
//     std::suspend_always yield_value(R&& value)
//     {
//         SetValue(std::forward<R>(value));

//         // co_yield总是挂起
//         return {};
//     }

//     constexpr T GetResult() & 
//     {
//         if (auto exception = std::get_if<std::exception_ptr>(&_result)) 
//         {
//             std::rethrow_exception(*exception);
//         }
//         if (auto res = std::get_if<T>(&_result)) 
//         {
//             return *res;
//         }

//         throw ResultIsUnsetException();
//     }
//     constexpr T GetResult() && 
//     {
//         if (auto exception = std::get_if<std::exception_ptr>(&_result)) {
//             std::rethrow_exception(*exception);
//         }
//         if (auto res = std::get_if<T>(&_result)) {
//             return std::move(*res);
//         }

//         throw ResultIsUnsetException();
//     }

//     // 必须的接口处理异常
//     void set_exception(std::exception_ptr exception) noexcept { _result = exception; }
//     void unhandled_exception() noexcept { _result = std::current_exception(); }

// private:
//     std::variant<std::monostate, T, std::exception_ptr> _result;
// };

// template<>
// struct Result<void> 
// {
//     constexpr bool HasValue() const noexcept {
//         return _result.has_value();
//     }
//     void return_void() noexcept 
//     {
//         _result.emplace(nullptr);
//     }
//     void GetResult() 
//     {
//         if (_result.has_value() && *_result != nullptr) { std::rethrow_exception(*_result); }
//     }

//     void set_exception(std::exception_ptr exception) noexcept { _result = exception; }
//     void unhandled_exception() noexcept { _result = std::current_exception(); }

// private:
//     std::optional<std::exception_ptr> _result;
// };

// // for cancelled
// using HandleId = UInt64;

// struct Handle 
// {
//     enum State: U8 
//     {
//         UNSCHEDULED,
//         SUSPEND,
//         SCHEDULED,
//     };

//     Handle() noexcept
//     : _handleId(++MaxHandleId) 
//     {

//     }
    
//     virtual ~Handle()
//     {
        
//     }

//     virtual void Run() = 0;

//     // 设置状态
//     void SetState(State state) 
//     { 
//         _state = state; 
//     }

//     // 获取id
//     HandleId GetHandleId() const
//     { 
//         return _handleId; 
//     }

// private:
//     HandleId _handleId;

//     static std::atomic<HandleId> MaxHandleId;

// protected:
//     State _state {Handle::UNSCHEDULED};
// };

// std::atomic<HandleId> Handle::MaxHandleId{0};

// // handle maybe destroyed, using the increasing id to track the lifetime of handle.
// // don't directly using a raw pointer to track coroutine lifetime,
// // because a destroyed coroutine may has the same address as a new ready coroutine has created.
// struct HandleInfo 
// {
//     HandleId _id { 0 };
//     Handle* _handle {NULL};
// };

// struct CoroHandle: Handle 
// {
//     // 获取堆栈信息
//     KERNEL_NS::LibString FrameName() const 
//     {
//         const auto& frameInfo = GetFrameInfo();
//         return KERNEL_NS::LibString().AppendFormat("%s at %s:%d", frameInfo.function_name(), frameInfo.file_name(), frameInfo.line());
//     }

//     virtual void DumpBacktrace(UInt64 depth = 0) const 
//     {

//     };

//     void Schedule();
//     void Cancel();

// private:
//     virtual const std::source_location &GetFrameInfo() const;
// };

// const std::source_location& CoroHandle::GetFrameInfo() const 
// {
//     static const std::source_location frameInfo = std::source_location::current();
//     return frameInfo;
// }

// void CoroHandle::Schedule() 
// {
//     // TODO没在EventLoop中就放入EventLoop队列中调度
//     if (_state == Handle::UNSCHEDULED)
//     {
//         // get_event_loop().call_soon(*this);
//     }
// }

// void CoroHandle::Cancel()
// {
//     // 需求eventloop设置成非调度，并放入到cancel队列
//     if (_state != Handle::UNSCHEDULED)
//     {
//         // get_event_loop().cancel_handle(*this);
//     }
// }

// struct NoWaitAtInitialSuspend {};
// inline constexpr NoWaitAtInitialSuspend no_wait_at_initial_suspend;
// // 必须设计成既是Awatable/又得是Future
// template<typename R = void>
// struct LibTask
// {
//     // 编译要求
//     struct promise_type;
//     using coro_handle = std::coroutine_handle<promise_type>;

//     explicit LibTask(coro_handle h) noexcept
//     :_handle(h)
//     {

//     }

//     LibTask(LibTask &&t)
//     :_handle(std::exchange(t._handle, {}))
//     {

//     }

//     ~LibTask()
//     {
//         _Destroy();
//     }

//     static LibTask<> Delay(const KERNEL_NS::TimeSlice &slice)
//     {
//         auto timerMgr = KERNEL_NS::TlsUtil::GetDefTls()->_pollerTimerMgr;
//         if(UNLIKELY(!timerMgr))
//         {
//             co_return;
//         }

//         auto timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer(timerMgr);
//         std::coroutine_handle<> handle;
//         timer->SetTimeOutHandler([handle](KERNEL_NS::LibTimer *t)
//         {
//             handle();
//             KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(t);
//         });
//         timer->Schedule(slice);
//         g_Log->Info(LOGFMT_NON_OBJ_TAG(LibTask<>, "will Delay slice:%s"), slice.ToString().c_str());
//         co_await LibTask(handle);
//         g_Log->Info(LOGFMT_NON_OBJ_TAG(LibTask<>, "Delay slice:%s"), slice.ToString().c_str());
//     }

//     // 获取结果 promise是获取promise_type对象
//     decltype(auto) GetResult() & 
//     {
//         return _handle.promise().GetResult();
//     }
//     decltype(auto) GetResult() && 
//     {
//         return std::move(_handle.promise()).GetResult();
//     }

//     // 可等待体基类, 需要有await_ready/await_suspend/await_resume, await_resume是返回结果
//     struct AwaiterBase {
//         constexpr bool await_ready() 
//         {
//             if (LIKELY(_selfCoro))
//                 return _selfCoro.done();

//             return true;
//         }

//         template<typename Promise>
//         void await_suspend(std::coroutine_handle<Promise> resumer) const noexcept 
//         {
//             assert(! _selfCoro.promise()._continuation);

//             resumer.promise().SetState(Handle::SUSPEND);
//             _selfCoro.promise()._continuation = &resumer.promise();

//             // 设置调度 => 放入Poller
//             _selfCoro.promise().Schedule();
//         }

//         coro_handle _selfCoro {};
//     };

//     // 让Task成为可等待体
//     auto operator co_await() const & noexcept 
//     {
//         struct Awaiter: AwaiterBase 
//         {
//             // 返回结果, 成为Awaiter必须的一个方法
//             decltype(auto) await_resume() const 
//             {
//                 if (UNLIKELY(!AwaiterBase::_selfCoro))
//                 { 
//                     throw SelfCoroInvalidException();
//                 }

//                 return AwaiterBase::_selfCoro.promise().GetResult();
//             }
//         };
//         return Awaiter {_handle};
//     }

//     // Promise, 改变获取结果以及调度执行流
//     struct promise_type : CoroHandle, Result<R>
//     {
//         promise_type() = default;

//         template<typename... Args> // from free function
//         promise_type(NoWaitAtInitialSuspend, Args&&...): _wait_at_initial_suspend{false} { }
//         template<typename Obj, typename... Args> // from member function
//         promise_type(Obj&&, NoWaitAtInitialSuspend, Args&&...): _wait_at_initial_suspend{false} { }

//         // 协程创建Promise用户自定义协程执行第一行代码前是否需要挂起
//         auto initial_suspend() noexcept 
//         {
//             struct InitialSuspendAwaiter 
//             {
//                 // true:不挂起, false:await_suspend(h)
//                 constexpr bool await_ready() const noexcept { return !_wait_at_initial_suspend; }

//                 // 无条件挂起
//                 constexpr void await_suspend(std::coroutine_handle<>) const noexcept {}

//                 constexpr void await_resume() const noexcept {}
//                 const bool _wait_at_initial_suspend{true};
//             };
//             return InitialSuspendAwaiter{_wait_at_initial_suspend};
//         }

//         // final_suspend的返回, 
//         struct FinalAwaiter {
//             constexpr bool await_ready() const noexcept { return false; }

//             // 强制在final_suspend挂起, 并后续手动销毁协程
//             template<typename Promise>
//             constexpr void await_suspend(std::coroutine_handle<Promise> h) const noexcept 
//             {
//                 // TODO:
//                 if (auto cont = h.promise()._continuation) 
//                 {
//                     // get_event_loop().call_soon(*cont);
//                 }
//             }
//             constexpr void await_resume() const noexcept {}
//         };

//         // 异常或者co_return的时候都会经过final_suspend, final_suspend挂起时，后续可手动销毁协程，否则会自动销毁协程, 后续再销毁协程就是二次销毁
//         auto final_suspend() noexcept 
//         {
//             return FinalAwaiter {};
//         }

//         // 创建协程返回对象Future,Promise构造完成后会通过用户提供的Promise::get_return_object构造Future对象, from_promise会返回协程句柄,Future作用是为用户提供获取值并管理协程生命周期
//         LibTask get_return_object() noexcept 
//         {
//             return LibTask{coro_handle::from_promise(*this)};
//         }

//         void Run() final 
//         {
//             // 恢复协程
//             coro_handle::from_promise(*this).resume();
//         }

//         // 获取调用栈信息
//         const std::source_location& GetFrameInfo() const final 
//         { 
//             return _frameInfo; 
//         }

//         // 打印堆栈信息
//         void DumpBacktrace(UInt64 depth = 0) const final 
//         {
//             g_Log->Warn(LOGFMT_OBJ_TAG("[%llu], frame name:%s"), depth, FrameName().c_str());
//             if (_continuation) 
//             { 
//                 _continuation->DumpBacktrace(depth + 1);
//             }
//         }

//         const bool _wait_at_initial_suspend {true};
//         CoroHandle* _continuation {NULL};
//         std::source_location _frameInfo{};
//     };
    

//     bool Valid() const 
//     { 
//         return _handle != NULL; 
//     }

//     bool Done() const 
//     { 
//         return _handle.done(); 
//     }

//     void _Destroy() 
//     {
//         if (auto handle = std::exchange(_handle, nullptr)) {
//             handle.promise().Cancel();
//             handle.destroy();
//         }
//     }

//     coro_handle _handle;
// };

// template<typename Task>
// struct ScheduledTask {
//     template<typename Fut>
//     explicit ScheduledTask(Fut&& fut): _task(std::forward<Fut>(fut)) {
//         if (_task.Valid() && ! _task.Done()) {
//             _task._handle.promise().Schedule();
//         }
//     }

//     void Cancel() { _task._Destroy(); }

//     decltype(auto) operator co_await() const & noexcept {
//         return _task.operator co_await();
//     }

//     auto operator co_await() const && noexcept {
//         return _task.operator co_await();
//     }

//     decltype(auto) GetResult() & {
//         return _task.GetResult();
//     }

//     decltype(auto) GetResult() && {
//         return std::move(_task).GetResult();
//     }

//     bool Valid() const { return _task.Valid(); }
//     bool Done() const { return _task.Done(); }

// private:
//     Task _task;
// };

// template<typename Fut>
// ScheduledTask(Fut&&) -> ScheduledTask<Fut>;

// template<typename Fut>
// ScheduledTask<Fut> schedule_task(Fut&& fut) {
//     return ScheduledTask { std::forward<Fut>(fut) };
// }

// template<typename Fut>
// decltype(auto) TaskRun(Fut&& main) {
//     auto t = schedule_task(std::forward<Fut>(main));
//     // get_event_loop().run_until_complete();
//     if constexpr (std::is_lvalue_reference_v<Fut&&>) {
//         return t.GetResult();
//     } else {
//         return std::move(t).GetResult();
//     }
// }

// Int32 g_id{0};

// LibTask<Int32> BuildId()
// {
//     for(Int32 idx = 0; idx < 10; ++idx)
//         co_yield ++g_id;

//     co_return ++g_id;
// }

// LibTask<Int32> RemoteGetId()
// {
//     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCoroutine, "test delay"));
//     co_await LibTask<>::Delay(KERNEL_NS::TimeSlice::FromSeconds(10));
//     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCoroutine, "test delay 10seconds elapse"));

//     co_return 100;
// }


void TestCoroutine::Run()
{
    // // 跑任务,任务挂起要返回调度器执行或者
    // TaskRun([]()->LibTask<> 
    // {
    //     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCoroutine, "get remote id begin"));
    //     auto id = co_await RemoteGetId();
    //     g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCoroutine, "get remote id:%d"), id);
    // }());

    // g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCoroutine, "after run task."));

//     std::cout<< "Before TestCoroutine pass TestCoInt" << std::endl;

// //   auto result = fun().await([]() {

// //     // 当所有协程被挂起后，会执行该update函数，该函数在主线程中执行
// //     // 当该update返回false，则会强制结束await等待，有可能抛出异常
// //     // 在这里可以做恢复协程操作

// //     resolve_cb(1); // 恢复协程, id值为1

// //     return true;

// //     });

//     auto result = fun();
    
//     resolve_cb(1);

//     while (!result.promise->done);

//     auto &&r = result.get_result();
//     result.destroy();
//     std::cout << r << std::endl;
//     // TestCoInt();

//     // auto r = GetFutureValue();

//     // auto flag = r._coroutine.done();
//     // r._coroutine();
    
//     // s_scheduler.update();

//     std::cout<< "After TestCoroutine schedule finish" << std::endl;
}
