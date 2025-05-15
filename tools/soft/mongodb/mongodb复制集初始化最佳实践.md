以下是一个 **三节点 MongoDB 复制集的最佳初始化配置**，兼顾高可用性、安全性及性能优化。该配置假设节点角色为 **1 主 + 2 从**，无仲裁节点（Arbiter），适用于生产环境。

------

### **一、配置文件示例（`mongod.conf`）**

所有节点的配置文件中需包含以下核心参数：

```
storage:
  dbPath: /data/db
  engine: wiredTiger
  wiredTiger:
    engineConfig:
      cacheSizeGB: 2  # 根据服务器内存调整（建议不超过物理内存的 60%）

replication:
  replSetName: rs0    # 复制集名称（所有节点一致）
  oplogSizeMB: 2048   # Oplog 大小（根据写入负载调整，建议至少容纳 72 小时操作）

net:
  bindIp: 0.0.0.0     # 允许外部访问
  port: 27017         # 主节点默认端口

security:
  authorization: enabled
  keyFile: /etc/mongo/keyfile  # 所有节点使用相同的密钥文件
  clusterAuthMode: keyFile     # 启用密钥文件认证
```

------

### **二、初始化命令（连接到主节点候选节点）**

```
mongosh mongodb://admin:password@node1:27017/admin --eval '
rs.initiate({
  _id: "rs0",
  members: [
    { 
      _id: 0, 
      host: "node1:27017", 
      priority: 2          // 主节点，高优先级
    },
    { 
      _id: 1, 
      host: "node2:27017", 
      priority: 1,         // 常规从节点
      votes: 1             // 默认有投票权（可省略）
    },
    { 
      _id: 2, 
      host: "node3:27017", 
      priority: 1,         // 常规从节点
      hidden: false,        // 非隐藏节点（默认值）
      secondaryDelaySecs: 0 // 非延迟节点（默认值）
    }
  ],
  settings: {
    heartbeatIntervalMillis: 2000,  // 心跳间隔 2 秒
    electionTimeoutMillis: 10000    // 选举超时 10 秒
  }
})'
```

------

### **三、关键参数解析**

#### **1. 节点角色配置**

| 参数                 | 说明                                                         |
| -------------------- | ------------------------------------------------------------ |
| `priority`           | 主节点优先级设为 `2`，确保其优先成为主节点；从节点设为 `1`。 |
| `votes`              | 默认 `1`（所有节点参与投票），三节点天然满足奇数投票机制。   |
| `hidden`             | 设为 `false`（默认），所有从节点对客户端可见。               |
| `secondaryDelaySecs` | 设为 `0`（默认），从节点实时同步数据。                       |

#### **2. 全局配置（`settings`）**

| 参数                      | 说明                                                         |
| ------------------------- | ------------------------------------------------------------ |
| `heartbeatIntervalMillis` | 节点间心跳检测间隔（默认 `2000` 毫秒）。                     |
| `electionTimeoutMillis`   | 节点等待心跳响应的超时时间（默认 `10000` 毫秒）。超时后触发选举。 |
| `chainingAllowed`         | 默认 `true`，允许从节点从其他从节点同步数据（优化网络拓扑）。 |

------

### **四、安全配置**

#### **1. 密钥文件生成**

```
openssl rand -base64 756 > /etc/mongo/keyfile
chmod 600 /etc/mongo/keyfile
chown mongod:mongod /etc/mongo/keyfile  # 根据系统用户调整
```

#### **2. 用户权限配置**

确保管理员用户拥有 `clusterAdmin` 角色：

```
db.grantRolesToUser("admin", [{ role: "clusterAdmin", db: "admin" }])
```

------

### **五、验证配置**

#### **1. 检查复制集状态**

```
rs.status()
```

- 

  关键指标

  ：

  - 主节点 `stateStr: "PRIMARY"`。
  - 从节点 `stateStr: "SECONDARY"` 且 `health: 1`。
  - Oplog 时间差（`optimeDate`）在秒级范围内。

#### **2. 测试故障转移**

```
# 在主节点执行，触发主节点降级
rs.stepDown()
# 观察新主节点选举（应在 10 秒内完成）
```

------

### **六、性能优化建议**

#### **1. Oplog 监控**

```
rs.printReplicationInfo()
```

- 

  输出示例

  ：

  ```
  configured oplog size: 2048MB
  log length start to end: 86400 secs (24 hrs)
  ```

- 

  调整 Oplog

  （若不足）：

  ```
  db.adminCommand({ replSetResizeOplog: 1, size: 4096 })  # 动态调整为 4096MB
  ```

#### **2. 读写分离**

客户端连接字符串中使用 `readPreference=secondary`：

```
mongodb://node1:27017,node2:27017,node3:27017/dbname?replicaSet=rs0&readPreference=secondary
```

------

### **七、维护命令**

#### **1. 强制重新配置**

```
cfg = rs.conf()
cfg.members[0].priority = 3  // 提升主节点优先级
rs.reconfig(cfg)
```

#### **2. 移除故障节点**

```
rs.remove("node4:27017")
```

------

### **总结**

此配置通过 **优先级控制选举**、**合理心跳参数** 和 **密钥文件认证**，实现了三节点复制集的高可用与安全。建议定期监控 `rs.status()` 和 Oplog 状态，并根据业务负载动态调整参数（如 `oplogSizeMB`）。