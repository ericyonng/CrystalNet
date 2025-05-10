在启用认证的 MongoDB 复制集中，必须配置 `security.keyFile` 以实现节点间安全通信。以下是具体解决方案：

------

**步骤 1：生成密钥文件（keyFile）**
 生成一个符合要求的密钥文件：

bash

复制

```
# 使用 OpenSSL 生成随机密钥（必须为 Base64 编码，长度 ≤ 1024 字节）
openssl rand -base64 756 > /root/mongodb/keyfile

# 设置文件权限（仅允许所有者读取）
chmod 600 /root/mongodb/keyfile
```

------

**步骤 2：修改 MongoDB 配置文件**
 编辑 `/root/mongodb/db1/mongod.conf`，添加以下配置：

yaml

复制

```
security:
  authorization: enabled   # 启用认证
  keyFile: /root/mongodb/keyfile  # 指向密钥文件路径

replication:
  replSetName: rs0  # 复制集名称（所有节点需一致）
```

------

**步骤 3：将密钥文件复制到所有节点**
 将生成的密钥文件复制到复制集的 所有节点 相同路径下，并确保权限一致：

bash

复制

```
scp /root/mongodb/keyfile user@other_node_ip:/root/mongodb/
ssh user@other_node_ip "chmod 600 /root/mongodb/keyfile"
```

------

**步骤 4：重启所有 MongoDB 实例**
 在每个节点上重启服务（使用相同配置文件）：

bash

复制

```
mongod -f /root/mongodb/db1/mongod.conf --shutdown  # 先关闭
mongod -f /root/mongodb/db1/mongod.conf            # 重新启动
```

------

**步骤 5：初始化复制集**

1. 连接到其中一个节点的 

   ```
   mongo
   ```

    shell：

   bash

   复制

   ```
   mongo --host 节点IP --port 端口 -u admin -p 密码 --authenticationDatabase admin
   ```

2. 执行复制集初始化：

   javascript

   复制

   ```
   rs.initiate({
     _id: "rs0",
     members: [
       { _id: 0, host: "节点1IP:端口" },
       { _id: 1, host: "节点2IP:端口" },
       { _id: 2, host: "节点3IP:端口", arbiterOnly: true }  // 可选仲裁节点
     ]
   })
   ```

------

**验证配置**

1. 检查复制集状态：

   javascript

   复制

   ```
   rs.status()
   ```

   • 输出应显示 `PRIMARY` 和 `SECONDARY` 节点状态。

2. 测试数据同步：

   javascript

   复制

   ```
   use test
   db.demo.insert({msg: "Replica Set Test"})
   ```

   在其他节点查询数据是否同步。

------

**常见问题解决**

| 错误场景                              | 解决方案                                                  |
| ------------------------------------- | --------------------------------------------------------- |
| `Permissions on keyFile are too open` | 确保密钥文件权限为 `600`：`chmod 600 /path/to/keyfile`    |
| `replSetInitiate quorum check failed` | 检查所有节点配置文件中的 `replSetName` 是否一致           |
| `Unauthorized to execute command`     | 使用管理员用户连接并添加 `--authenticationDatabase admin` |

------

**关键配置项总结**

| 配置项                    | 作用           | 要求                 |
| ------------------------- | -------------- | -------------------- |
| `security.keyFile`        | 节点间认证密钥 | 所有节点使用相同文件 |
| `replication.replSetName` | 复制集名称     | 所有节点必须一致     |
| `authorization: enabled`  | 启用客户端认证 | 需配合用户权限使用   |

通过以上步骤，您可以在启用认证的 MongoDB 复制集中正确配置 `keyFile`，确保节点间安全通信和数据同步。