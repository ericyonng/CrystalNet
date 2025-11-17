# 消息队列选择

* 选择无锁队列
* 如果是多生产者对单消费者，那么应该分解成多个单生产者单消费者模型, 因为多生产者的即使用上MPMC队列，也会因为竞争记录CAS频繁失败损失性能

# Poller的控制指令

* Poller的控制指令可以: 创建Channel, 销毁Channel， 通过MPMC来发送

# Channel

每个Poller之间的通信通道, 使用SPSC(单生产者单消费者模式)无锁队列通信

* Poller之间创建Channel

  * 通过MPMC（多生产者多消费者）无锁队列向消费者投递创建Channel消息 

    -> 消费者接收到消息创建SPSC队列后将SPSC加入到 vector中 

    -> 返回SPSC给生产者 

    -> 生产者接收到SPSC 后创建 Channel(src => target), 并Attach SPSC(自己不释放SPSC由target自己释放) 

    -> 通道建立完成

* Poller之间建立通道后即可使用Channel的Send接口通信(同时提供Async与无Async版本)

* Channel销毁：通过MPMC发送，并同时标记Channel删除，后续不可发送消息，消费者Poller收到消息后判断是否消费完，消费完立即销毁SPSC，没消费完需要标记delete, 等到SPSC最后一个消息处理完成后销毁

  