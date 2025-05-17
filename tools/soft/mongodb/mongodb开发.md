# 官方文档

https://www.mongodb.com/zh-cn/docs/drivers/

* 要使用mongodb driver需要先初始化driver实例：mongocxx::instance instance;

```
要使用C++驾驶员，必须首先创建mongocxx::instance类的实例。 此实例在您的应用程序中执行以下功能：

初始化和关闭C++驾驶员

初始化驾驶员依赖项

确保驾驶员正常运行

管理在mongocxx::client对象之间共享的资源的生命周期，例如连接池和BSON库

本指南将向您介绍如何创建mongocxx::instance对象。

创建一个mongocxx::instance
要创建mongocxx::instance对象，请在应用程序中包含mongocxx/instance.hpp头文件。 然后，在应用程序启动时构造mongocxx::instance的实例，如以下示例所示：

#include <mongocxx/instance.hpp>
int main()
{
    mongocxx::instance instance;
}

重要
在使用C++驾驶员之前，您必须创建一个mongocxx::instance对象，并且只要任何其他MongoDB对象在作用域内，该对象就必须保持活动状态。


```

