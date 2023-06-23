* 需要配置mysql最大连接数为10000保证足够的连接（或者1000）
* mysql可以在程序上指定绑定到某个本地ip进行通讯
* 安装mysql 8.0（使用lnmp选择mysql 8.0安装）
* 安装mysqlclient
* 引入libmysql.dll库(windows下)/libmysqlclient.so（linux下）
* 包含3rd下mysql的相关include路径和库路径
* windows下最好使用mysql官方指定的mysql workbench来管理数据库
* 一张表只能有一个自增id字段

