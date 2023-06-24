* 需要配置mysql最大连接数为10000保证足够的连接（或者1000）
* mysql可以在程序上指定绑定到某个本地ip进行通讯
* 安装mysql 8.0（使用lnmp选择mysql 8.0安装）
* 安装mysqlclient
* 引入libmysql.dll库(windows下)/libmysqlclient.so（linux下）
* 包含3rd下mysql的相关include路径和库路径
* windows下最好使用mysql官方指定的mysql workbench来管理数据库
* 一张表只能有一个自增id字段
* mysql 错误问题解析
  * MySQL server has gone away: 一个mysql_real_query过长, 超过了max_allowed_packet限制大小
  * 在一次性执行多条sql后，无论是insert/update等这些没有结果的还是select等有结果的sql, 必须要store_result/user_result来移除result，才能继续执行sql
  * 执行完mysql_real_query后必须再执行store_result/user_result才算完整的执行完sql，因为mysql_real_query只是把sql发过去, mysql还没真正执行完成
* 配置
  * my.ini中配置最大连接数5000(my.ini)
  * my.ini中配置最大包大小: max_allowed_packet=2048M
  * my.ini中配置最大连接数:max_connections
  * mysql连接是有上限的所以为了保证mysql可以被访问，需要对mysqlclient的连接控制,这时候就需要连接池，限制client-side的连接数量
    * 在使用连接池时必须设置不可自动重连，因为自动重连时就会阻塞mysql_ping导致性能下降, 应该设置不重连, 手动重新连接
  * MYSQL_OPT_MAX_ALLOWED_PACKET 设置mysql连接一次接收的最大包大小 最大建议设置2GB, 不超过3GB
    * UseResult：
      * 优点: 不会把所有的数据一次性全部取回本地缓存， 对于全表查询数据量比较大，但又不需要缓存所有数据来说是比较友好的
      * 缺点，当接收mysql数据比较慢的情况下会导致接收数据不全
    * StoreResult
      * 一次性把所有的数据结果拿回本地再FetchRow本地的缓存，好处是不用担心网络不好的问题，
      * 缺点全表查询数据量比较大的情况下会导致内存被吃光, 这种情况下应该限制全表行数的上限，避免内存耗尽，有这种需求一般需要控制单表大小
      * 当即将返回的数据 >  MYSQL_OPT_MAX_ALLOWED_PACKET 时候 执行的sql可能会失败，建议 MYSQL_OPT_MAX_ALLOWED_PACKET = 1024M
    * 一般情况下采用StoreResult, 如果数据较大, 需要进行分页查询, 分多次将数据查完select *from tbl where id > last_max_id order by id asc limit 1000
    * 对于单台机器内存128G来说一半用来用户自己的内存, 且单点负载5000人情况下， 一个用户数据占用内存建议不超过16MB, 也就是说可能一次查询需要的内存是最大约16GB，但是一般玩家通常内存占用不会这么大一般4MB已经很大了, 那么可以设置MYSQL_OPT_MAX_ALLOWED_PACKET  4GB
* mysql_options 需要在connect之前调用
* 事务
  * 概念：一个程序执行单元，要么都执行要么都不执行
  * 操作
    * START TRANSACTION:开启事务
    * set autocommit = 0:不自动提交事务（一个sql是一个事务，如果设置自动提交那么就回滚不了）
    * COMMIT:手动提交事务
    * ROLLBACK:中间出错会回滚
    * set autocommit = 1恢复自动提交
  * 存储引擎: myisam:查询快, 表级锁,不支持行锁 ROLLBACK 不支持， InnoDB: 支持回滚，所以要支持事务必须选用InnoDB， 行级锁
  * 事务不是原子操作，原子操作由锁来保证
  * 因为MyISAM是表级锁，会导致其他人读不了数据

