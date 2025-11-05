# 构成

* mongodbmgr

  * 提供mongodbdb的操作

* 线程池：提供mongodb操作数据库的异步操作接口处理, 全局唯一, 通过消息队列通信

* 为了方便需要Poller支持协程(等待响应包)

  * 对外接口例如：

  * ```
    auto res = co_await PostEvent(TargetPoller, ev);
    // TODO: res 逻辑（Post的逻辑是投递到TargetPoller之后，TargetPoller处理完之后再把消息回调当前Poller， 当前Poller唤醒协程处理剩下逻辑）
    ```

    