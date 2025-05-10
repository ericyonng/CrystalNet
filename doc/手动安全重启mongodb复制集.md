在不使用系统服务（如 `systemd`）的情况下，手动通过 `mongod` 命令重启 MongoDB 复制集（Replica Set）需要按顺序操作，以下是详细步骤：

------

**步骤 1：安全关闭复制集节点**
 a. ​**​逐个关闭从节点（Secondary）​**​

1. 连接到从节点的 

   ```
   mongo --host secondary_ip:port
   ```

    shell：

   bash

   复制

   ```
   mongo --host primary_ip:port
   ```

2. 执行关闭命令：

   javascript

   复制

   ```
   use admin
   db.shutdownServer()
   ```

b. **最后关闭主节点（Primary）**

1. 连接到主节点的 

   ```
   
   ```

    shell：

   bash

   复制

   ```
   mongo --host primary_ip:port
   ```

2. 执行关闭命令：

   javascript

   复制

   ```
   use admin
   db.shutdownServer()
   ```

------

**步骤 2：重启所有节点**
 a. ​**​启动主节点​**​

bash

复制

```
mongod --replSet "replicaSetName" \
       --dbpath /path/to/primary/data \
       --port 27017 \
       --logpath /path/to/primary/mongod.log \
       --fork
```

b. **启动从节点**

bash

复制

```
mongod --replSet "replicaSetName" \
       --dbpath /path/to/secondary/data \
       --port 27018 \
       --logpath /path/to/secondary/mongod.log \
       --fork
```

------

**步骤 3：验证复制集状态**

1. 连接到主节点的 `mongo` shell：

   bash

   复制

   ```
   mongo --host primary_ip:27017
   ```

2. 检查复制集状态：

   javascript

   复制

   ```
   rs.status()
   ```

   • 确认所有节点处于 `PRIMARY` 或 `SECONDARY` 状态。

   • 观察 `optime` 是否同步。

------

**关键参数说明**

| 参数        | 作用                           |
| ----------- | ------------------------------ |
| `--replSet` | 指定复制集名称（必须一致）     |
| `--dbpath`  | 数据存储目录（需与关闭前一致） |
| `--logpath` | 日志文件路径（建议指定）       |
| `--fork`    | 后台运行进程                   |

------

**注意事项**

1. 启动顺序：
    • 先启动旧主节点（避免选举冲突）。

   • 若主节点无法启动，强制重新配置复制集：

   javascript

   复制

   ```
   rs.reconfig(newConfig, {force: true})
   ```

2. 日志监控：

   bash

   复制

   ```
   tail -f /path/to/mongod.log
   ```

   关注以下关键词：
    • `Replication initialized`（复制初始化成功）

   • `transition to PRIMARY/SECONDARY`（角色切换）

3. 数据一致性：
    • 确保所有节点的数据目录未被手动修改。

   • 使用 `--dbpath` 指向原始数据目录。

------

**故障排查**
 ​**​场景 1：节点无法加入复制集​**​
 • 检查网络连通性：

bash

复制

```
telnet primary_ip 27017
```

• 验证复制集配置：

javascript

复制

```
rs.conf()  // 确保所有节点IP和端口正确
```

**场景 2：主节点选举失败**
 • 强制重新初始化复制集：

javascript

复制

```
rs.initiate({
  _id: "replicaSetName",
  members: [
    { _id: 0, host: "primary_ip:27017" },
    { _id: 1, host: "secondary_ip:27018" }
  ]
})
```

------

**总结**

| 操作         | 命令                                       |
| ------------ | ------------------------------------------ |
| 安全关闭节点 | `db.shutdownServer()`                      |
| 启动节点     | `mongod --replSet ... --dbpath ... --fork` |
| 验证状态     | `rs.status()`                              |

通过以上步骤，可在不使用系统服务的情况下，手动重启 MongoDB 复制集并确保数据一致性。