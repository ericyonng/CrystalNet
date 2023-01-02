# 服务

* 每个服务遵循ECS设计，服务包含若干个组件，组件提供Factory注册到服务

* 每个服务独占一个线程，若一个app有若干服务，那么便是在各自的线程运行

* 不同服务之间通过消息队列通信

* 不同服务有相同点与不同点，公共组件抽象到service/common中，包括公共事件定义，GlobalSys抽象，SessionMgr组件等

* 对于事件，有公共事件与服务自己的事件，服务自己的事件从EVENT_COMMON_END + 1开始

* 服务与其他外部通过ServiceProxy提供通信支持

* 全局组件继承于IGlobalSys有利于快速开发，也可根据需要继承于ILogicSys或者CompHostObject, 或者CompObject

  

