* TODO:需要放到OptionComp

* 系统设计

  * start的时候扫描所有的带有storageInfo的系统， 将存储系统注册到MysqlDbMgr的依赖中
  * 拉取当前mysql数据库的所有表字段信息
  * 与当前所有存储系统进行表信息比对，做出需要创建新表和需要修改字段的系统
  * 创建新表, 修改表字段
  * 通知mysql存储系统ready事件
  * mysqlmgr收到存储系统ready事件时更新其存储信息(mysqlmgr会记录所有表的相关信息, 包括自增id的使用，版本号等)
  * service收到mysql存储系统ready事件做后续事情

* 限制

  * 每个表必须至少一个字段最多1017个字段

  * mysql列名长度64字节限制

  * ### 9.2.1 Identifier Length Limits

    The following table describes the maximum length for each type of identifier.

    | Identifier Type          | Maximum Length (characters)                |
    | :----------------------- | :----------------------------------------- |
    | Database                 | 64 (includes NDB Cluster 8.0.18 and later) |
    | Table                    | 64 (includes NDB Cluster 8.0.18 and later) |
    | Column                   | 64                                         |
    | Index                    | 64                                         |
    | Constraint               | 64                                         |
    | Stored Program           | 64                                         |
    | View                     | 64                                         |
    | Tablespace               | 64                                         |
    | Server                   | 64                                         |
    | Log File Group           | 64                                         |
    | Alias                    | 256 (see exception following table)        |
    | Compound Statement Label | 16                                         |
    | User-Defined Variable    | 64                                         |
    | Resource Group           | 64                                         |

* 逻辑

  * 1.先收集所有mysql中旧表及其结构, 
  * 

* 数据库表构成

  * 系统表:
    * 1.记录所有表结构信息的表, 命名:tbl_MysqlMgr
      * 主键: 每个业务模块表的表名
      * 字段: value大小（用于选择到底是TINY_BLOB, BLOB, MEDIUMBLOB, LONGBLOB）
  * 业务模块表



---

# Mongodb

#### 接口需求

* 定义数据持久化格式
  * 持久化模式: 二进制持久化, json持久化, 直接持久化(数据是啥就是啥)
  * 统一使用Variant接受数据
* 标脏接口
* 持久化粒度: 
  * 对于n个子系统的Player/User以子系统为单位UpdateData
  * 对于全局表数据，应该以单条或者多条数据为单位
  * 标脏之后对于User系统，可以在帧末或者定时触发时, 将多个操作合并成一条执行
  * 对于全局表, 只能一条一条的执行
  * 对于业务主动调用db持久化user表的, 可以提供接口让该User的数据持久化并清空该User的脏标记
* 对于Save
  * User: 子系统有二进制模式，json模式，当是二进制模式时候使用LibStreamTL, 当是json模式时候应制定序列化的类型, 或者Variant添加JsonString类型
    * 护具存储模式应该在创建子系统的factory时就应该定义了,然后再OnSave/OnLoad的时候自动被指定
  * Global系统: 一条数据是被整体的ReplaceData， 必须要加锁, 或者必须要验证版本号，做到从获取数据到更新数据这段是原子的,使用行锁
* 对于OnLoad, 
  * User: query指定的若干系统的数据(遍历User组件)，根据指定的模式来反序列化一个数据
  * 