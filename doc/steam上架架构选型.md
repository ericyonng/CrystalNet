# 节点

* LoginServer/Gateway 同节点 1台 4C8G
* LogicServer 1台 4C8G
* MongoDB 3节点复制集部署, 3台 4C8G
* 所有有线程的需求都使用线程池管理(服务线程除外, 服务线程独立一个Thread)
* 线程池: heavy:1个线程, easytask:2个线程, Service 1 个线程



# 域名

* 给LoginServer/Gateway配公网域名, 解析到公网ip(AAAA记录解析成ipv4，AAAAAA解析成ipv6)
* 给LogicServer/Mongodb配置内网域名解析, 解析到内网ip，腾讯云可以配置Private DNS给内网服务使用



# mongodb

* 查数据尽量用跳板机, 上去查，怕Compass查数据会缓存导致合规问题(数据出境)
* 初期采用复制集承载
* 如果Mongodb出现瓶颈, 才考虑要不要分片集群
* 