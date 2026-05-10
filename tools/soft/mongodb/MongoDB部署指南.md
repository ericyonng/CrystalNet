# MongoDB 分片集群部署与新增分片指南

## 目录

- [1. 架构概览](#1-架构概览)
- [2. 前置条件](#2-前置条件)
- [3. iplist.txt 配置说明](#3-iplisttxt-配置说明)
- [4. 部署分片集群](#4-部署分片集群)
  - [4.1 一键部署](#41-一键部署)
  - [4.2 部署内部流程详解](#42-部署内部流程详解)
- [5. 新增分片](#5-新增分片)
  - [5.1 准备 iplist_add_shard.txt](#51-准备-iplist_add_shardtxt)
  - [5.2 执行新增分片](#52-执行新增分片)
  - [5.3 新增分片内部流程详解](#53-新增分片内部流程详解)
- [6. 安全关闭集群](#6-安全关闭集群)
- [7. 常用运维命令](#7-常用运维命令)
- [8. 注意事项](#8-注意事项)

---

## 1. 架构概览

MongoDB 分片集群由三类组件组成：

```
┌─────────────────────────────────────────────────┐
│                  客户端应用                       │
└────────────────────┬────────────────────────────┘
                     │
          ┌──────────▼──────────┐
          │    mongos 路由节点    │  (至少1个, 建议2+)
          │  接收查询, 路由分发   │
          └──────────┬──────────┘
                     │
      ┌──────────────┼──────────────┐
      │              │              │
┌─────▼─────┐  ┌─────▼─────┐  ┌─────▼─────┐
│  Shard 1  │  │  Shard 2  │  │  Shard N  │  (数据分片)
│ 复制集(3节点)│  │ 复制集(3节点)│  │ 复制集(3节点)│
│ Primary    │  │ Primary    │  │ Primary    │
│ Secondary  │  │ Secondary  │  │ Secondary  │
│ Secondary  │  │ Secondary  │  │ Secondary  │
└───────────┘  └───────────┘  └───────────┘
      ▲              ▲              ▲
      └──────────────┼──────────────┘
                     │
          ┌──────────▼──────────┐
          │  Config Server 复制集 │  (必须3节点)
          │  存储集群元数据       │
          └─────────────────────┘
```

- **Config Server**：存储集群配置和分片元数据，3节点复制集，`clusterRole: configsvr`
- **Shard**：存储实际数据，每个分片是独立的复制集，`clusterRole: shardsvr`
- **mongos**：查询路由，将客户端请求转发到对应分片，`clusterRole: 无`

**认证方式**：keyfile 内部认证 + 用户名密码认证

**复制集配置**：
- 主节点 `priority: 2`，从节点 `priority: 1`
- `heartbeatIntervalMillis: 2000`
- `electionTimeoutMillis: 10000`

---

## 2. 前置条件

| 条件 | 说明 |
|------|------|
| 操作系统 | Linux (CentOS/RHEL 推荐) |
| MongoDB | 8.0.6 (脚本内置版本) |
| SSH | 各节点间 root 免密 SSH 互通 |
| 权限 | root 用户执行 |
| 依赖 | openssl (脚本自动检测并安装) |
| 网络 | 各节点间端口互通 |

**目录规划**：

| 目录 | 说明 | 示例 |
|------|------|------|
| 软件包安装路径 | MongoDB 程序目录 | `/root/mongo_install` |
| 数据库数据路径 | 各节点数据目录的父路径 | `/root/mongo_data` |
| 脚本临时路径 | 自动创建 | `/root/mongodb_script` |

数据子目录自动按 `{DB_NAME}_{类型}_{序号}` 命名，如：
```
/root/mongo_data/
├── admin_config_1/     # config 主节点
├── admin_config_2/     # config 从节点
├── admin_config_3/     # config 从节点
├── admin_shard1_1/     # shard1 主节点
├── admin_shard1_2/     # shard1 从节点
├── admin_shard1_3/     # shard1 从节点
├── admin_shard2_1/     # shard2 主节点
├── admin_shard2_2/     # shard2 从节点
├── admin_shard2_3/     # shard2 从节点
├── admin_mongos_1/     # mongos 节点
└── admin_mongos_2/     # mongos 节点
```

---

## 3. iplist.txt 配置说明

**格式**：`类型 复制集前缀 IP地址/域名 端口`

| 字段 | 说明 | 示例 |
|------|------|------|
| 类型 | `config` / `shardN` / `mongos` | `config`, `shard1`, `mongos` |
| 复制集前缀 | 最终复制集名 = `前缀_类型` | `testsuit_rs` → `testsuit_rs_config` |
| IP地址 | IPv4 / IPv6 / 域名 | `192.168.1.1`, `2001:db8::1`, `mongo1.example.com` |
| 端口 | 节点端口号 | `27010`, `27011`, `27017` |

**规则**：
- 同类型多行 = 同一复制集的多个节点（建议3节点）
- 同类型组内复制集前缀必须一致
- `#` 开头的行和空行会被跳过
- IPv6 地址不需要加方括号，脚本自动处理

**示例**（2分片 + 3config + 2mongos）：

```bash
# config 配置服务器 (3节点复制集, 复制集名: testsuit_rs_config)
config testsuit_rs 192.168.1.1 27010
config testsuit_rs 192.168.1.2 27010
config testsuit_rs 192.168.1.3 27010

# shard1 分片1 (3节点复制集, 复制集名: testsuit_rs_shard1)
shard1 testsuit_rs 192.168.1.1 27011
shard1 testsuit_rs 192.168.1.2 27011
shard1 testsuit_rs 192.168.1.3 27011

# shard2 分片2 (3节点复制集, 复制集名: testsuit_rs_shard2)
shard2 testsuit_rs 192.168.1.4 27011
shard2 testsuit_rs 192.168.1.5 27011
shard2 testsuit_rs 192.168.1.6 27011

# mongos 路由 (2节点)
mongos testsuit_rs 192.168.1.1 27017
mongos testsuit_rs 192.168.1.2 27017
```

> 完整示例参考 `iplist.txt.example`

---

## 4. 部署分片集群

### 4.1 一键部署

```bash
sh install_mongo_shard_cluster.sh <iplist.txt> <用户名> <密码> <软件包安装路径> <数据库数据路径> [数据库名]
```

| 参数 | 必填 | 说明 | 示例 |
|------|------|------|------|
| iplist.txt | 是 | 节点列表文件 | `iplist.txt` |
| 用户名 | 是 | 管理员用户名 | `admin` |
| 密码 | 是 | 管理员密码 | `eric123` |
| 软件包安装路径 | 是 | MongoDB 程序目录 | `/root/mongo_install` |
| 数据库数据路径 | 是 | 数据目录父路径 | `/root/mongo_data` |
| 数据库名 | 否 | 数据库名, 默认 `admin` | `mydb` |

**执行示例**：

```bash
sh install_mongo_shard_cluster.sh iplist.txt admin eric123 /root/mongo_install /root/mongo_data mydb
```

### 4.2 部署内部流程详解

脚本自动执行以下 5 个步骤：

#### 步骤1：初始化 Config 复制集

```
1.1 初始化 config 主节点
    → no_auth 模式启动 mongod (clusterRole: configsvr)
    → rs.initiate() 初始化复制集
      rs.initiate({
        _id: "testsuit_rs_config",
        members: [{_id: 0, host: "192.168.1.1:27010", priority: 2}],
        settings: {heartbeatIntervalMillis: 2000, electionTimeoutMillis: 10000}
      })
    → db.createUser() 创建管理员用户
    → 关闭 mongod

1.2 带认证启动 config 主节点
    → mongod 启动 (keyFile + authorization)

1.3 启动 config 从节点并加入复制集
    → 依次启动每个从节点 (keyFile + authorization)
    → 从主节点执行 rs.add() 添加从节点:
      rs.add({_id: 1, host: "192.168.1.2:27010", priority: 1, votes: 1})
      rs.add({_id: 2, host: "192.168.1.3:27010", priority: 1, votes: 1})
```

#### 步骤2：初始化每个 Shard 分片复制集

对 iplist.txt 中每个 shardN 重复以下流程：

```
2.1 初始化 shard 主节点
    → no_auth 模式启动 mongod (clusterRole: shardsvr)
    → rs.initiate() 初始化复制集
      rs.initiate({
        _id: "testsuit_rs_shard1",
        members: [{_id: 0, host: "192.168.1.1:27011", priority: 2}],
        settings: {heartbeatIntervalMillis: 2000, electionTimeoutMillis: 10000}
      })
    → db.createUser() 创建管理员用户
    → 关闭 mongod

2.2 带认证启动 shard 主节点
    → mongod 启动 (keyFile + authorization)

2.3 启动 shard 从节点并加入复制集
    → 依次启动每个从节点
    → rs.add() 添加从节点:
      rs.add({_id: 1, host: "192.168.1.2:27011", priority: 1, votes: 1})
      rs.add({_id: 2, host: "192.168.1.3:27011", priority: 1, votes: 1})
```

#### 步骤3：初始化 mongos 路由节点

```
3.1 初始化 mongos 节点
    → 带 keyFile 启动 mongos (mongos 不能 no_auth, 必须通过 keyFile 内部认证连接 config server)
    → mongos 不需要单独创建用户(用户存储在 config server 上, 步骤1已创建)
    → 关闭 mongos

3.2 带认证启动 mongos 节点
    → mongos 启动 (keyFile, 无 authorization)
    → configDB 指向 config 复制集地址:
      testsuit_rs_config/192.168.1.1:27010,192.168.1.2:27010,192.168.1.3:27010
```

> **注意**：mongos 配置只设 keyFile，不设 authorization。mongos 的认证依赖于 config server。

#### 步骤4：通过 mongos 添加分片

```
→ 连接 mongos 执行:
  sh.addShard("testsuit_rs_shard1/192.168.1.1:27011,192.168.1.2:27011,192.168.1.3:27011")
  sh.addShard("testsuit_rs_shard2/192.168.1.4:27011,192.168.1.5:27011,192.168.1.6:27011")
```

#### 步骤5：启用数据库分片

```
→ 连接 mongos 执行:
  sh.enableSharding("mydb")
```

> 分片键 (`sh.shardCollection`) 属于业务逻辑，需手动执行。

---

## 5. 新增分片

当集群已有分片需要扩容时，使用 `addShard.sh` 添加新分片。

### 5.1 准备 iplist_add_shard.txt

**只需填写新增的 shard 节点**，不需要 config 和 mongos：

```bash
# 新增 shard2 分片 (3节点复制集, 复制集名: testsuit_rs_shard2)
shard2 testsuit_rs 192.168.1.4 27011
shard2 testsuit_rs 192.168.1.5 27011
shard2 testsuit_rs 192.168.1.6 27011

# 也可以同时添加多个分片
# shard3 testsuit_rs 192.168.1.7 27011
# shard3 testsuit_rs 192.168.1.8 27011
# shard3 testsuit_rs 192.168.1.9 27011
```

> 完整示例参考 `iplist_add_shard.txt.example`

**关键约束**：
- shard 编号不能与已有分片重复（如已有 shard1，则新分片从 shard2 开始）
- 复制集前缀必须与已有集群一致
- keyfile 必须使用集群原有的 keyfile

### 5.2 执行新增分片

```bash
sh addShard.sh <iplist.txt> <mongos_ip> <mongos_port> <用户名> <密码> <软件包安装路径> <数据库数据路径> <keyfile绝对路径> [数据库名]
```

| 参数 | 必填 | 说明 | 示例 |
|------|------|------|------|
| iplist.txt | 是 | 新增分片节点列表 | `iplist_add_shard.txt` |
| mongos_ip | 是 | 已有集群的 mongos 节点 IP | `192.168.1.1` |
| mongos_port | 是 | 已有集群的 mongos 节点端口 | `27017` |
| 用户名 | 是 | 集群管理员用户名 | `admin` |
| 密码 | 是 | 集群管理员密码 | `eric123` |
| 软件包安装路径 | 是 | MongoDB 程序目录 | `/root/mongo_install` |
| 数据库数据路径 | 是 | 数据目录父路径 | `/root/mongo_data` |
| keyfile绝对路径 | 是 | 集群原有 keyfile 路径 | `/root/mongodb_script/keyfile` |
| 数据库名 | 否 | 数据库名, 默认 `admin` | `mydb` |

**执行示例**：

```bash
sh addShard.sh iplist_add_shard.txt 192.168.1.1 27017 admin eric123 /root/mongo_install /root/mongo_data /root/mongodb_script/keyfile mydb
```

> **重要**：keyfile 必须与已有集群的 keyfile 一致！通常位于 `/root/mongodb_script/keyfile`。

### 5.3 新增分片内部流程详解

#### 步骤1：初始化新 Shard 分片复制集

```
1.1 初始化 shard 主节点
    → no_auth 启动 mongod (clusterRole: shardsvr)
    → rs.initiate() 初始化复制集
    → db.createUser() 创建管理员用户
    → 关闭 mongod

1.2 带认证启动 shard 主节点

1.3 启动 shard 从节点并加入复制集
    → rs.add() 添加从节点
```

#### 步骤2：通过 mongos 添加新分片

```
→ 连接已有 mongos 执行:
  sh.addShard("testsuit_rs_shard2/192.168.1.4:27011,192.168.1.5:27011,192.168.1.6:27011")
```

**addShard.sh 额外功能**：
- 自动检测节点是否已安装 MongoDB（已安装则跳过安装）
- 自动验证 mongos 连接是否可用
- 自动检查已有分片列表，防止重复添加
- 自动分发 keyfile 到新节点

---

## 6. 安全关闭集群

```bash
sh stop_mongo_shard_cluster.sh <iplist.txt> <用户名> <密码>
```

**执行示例**：

```bash
sh stop_mongo_shard_cluster.sh iplist.txt admin eric123
```

**关闭顺序**（官方推荐）：

```
1. 先关闭 mongos 路由节点
   → 发送 SIGINT(2) 信号, 等待30秒, 超时则 SIGKILL(9)

2. 再关闭 shard 从节点(secondary)
   → mongod -f xxx.conf --shutdown

3. 再关闭 shard 主节点(primary)
   → rs.stepDown(120, 30, {force: true})  降级为主节点
   → mongod -f xxx.conf --shutdown

4. 最后关闭 config 从节点, 再关闭 config 主节点
   → rs.stepDown(120, 30, {force: true})
   → mongod -f xxx.conf --shutdown
```

> **注意**：关闭顺序很重要。先关 mongos 防止新请求，再关数据节点，最后关元数据节点。

---

## 7. 常用运维命令

### 连接集群

```bash
mongosh --host 192.168.1.1:27017 -u admin -p eric123 --authenticationDatabase admin
```

### 查看集群状态

```bash
# 分片集群总状态
sh.status()

# 查看分片列表
db.adminCommand({listShards: 1})

# 查看数据库分片状态
db.getSiblingDB("config").databases.find()
```

### 查看复制集状态

```bash
# 复制集配置
rs.conf()

# 复制集状态
rs.status()

# 查看主节点
rs.isMaster()
```

### 集合分片

```bash
# 先启用数据库分片
sh.enableSharding("mydb")

# 对集合按分片键分片 (hashed 分片)
sh.shardCollection("mydb.users", {userId: "hashed"})

# 对集合按分片键分片 (range 分片)
sh.shardCollection("mydb.orders", {orderId: 1})
```

### 数据均衡管理

```bash
# 查看均衡器状态
sh.getBalancerState()

# 查看当前迁移操作
sh.isBalancerRunning()

# 暂停均衡器 (大流量期间)
sh.stopBalancer()

# 恢复均衡器
sh.startBalancer()
```

### 手动添加分片 (不使用脚本)

```bash
# 1. 在新分片节点上初始化复制集 (参考步骤2流程)

# 2. 连接 mongos 添加分片
mongosh --host 192.168.1.1:27017 -u admin -p eric123 --authenticationDatabase admin

# 3. 执行 addShard
sh.addShard("testsuit_rs_shard3/192.168.1.7:27011,192.168.1.8:27011,192.168.1.9:27011")

# 4. 验证
sh.status()
```

---

## 8. 注意事项

### keyfile

- 脚本首次部署时自动通过 `openssl rand -base64 756` 生成 keyfile
- 新增分片时**必须**使用与已有集群相同的 keyfile
- keyfile 权限必须为 600，否则 MongoDB 启动失败
- keyfile 路径示例: `/root/mongodb_script/keyfile`

### 认证机制

- **mongod (config/shard)**: `keyFile` + `authorization = true`
- **mongos**: 只配 `keyFile`，不配 `authorization`（认证依赖 config server）
- 首次初始化主节点时以 no_auth 模式启动，创建用户后关闭，再以带认证模式重启

### IP 地址

- 脚本自动处理 IPv4 / IPv6 / 域名格式
- IPv6 在 iplist.txt 中直接填写原始地址，无需加方括号
- `rs.initiate()` 和 `rs.add()` 中 host 使用 iplist.txt 中的原始地址，保证注册地址一致

### 数据目录

- 数据子目录按 `{DB_NAME}_{类型}_{序号}` 自动命名
- 每个节点有独立的子目录和数据文件
- 目录由脚本自动创建，无需手动创建

### 分片键

- `sh.enableSharding()` 由脚本自动执行
- `sh.shardCollection()` 需根据业务逻辑**手动执行**
- 分片键一旦设定不可修改，请谨慎选择

### 生产环境建议

- Config Server: 3节点，使用独立机器
- 每个 Shard: 3节点复制集（1 Primary + 2 Secondary）
- mongos: 至少2个，避免单点故障
- 不同分片部署在不同机器上
- 确保 NTP 时间同步
