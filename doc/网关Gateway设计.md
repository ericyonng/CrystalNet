* 接收服务器节点协议的注册，帮助协议路由
* 承载面向客户端的网络连接管理
* 削峰限流
* 后面所有服务节点的流量入口，可以无感的做流量转向（A服挂掉，将A服的流量打到备份服），实现高可用
* 高可用实现一套一致性hash