在 MongoDB 中启用认证需要通过配置文件设置身份验证机制（如 SCRAM、x.509 等），以下是详细步骤：

------

**1. 编辑 MongoDB 配置文件**
 配置文件通常位于 `/etc/mongod.conf`，修改以下部分：

yaml

复制

```
# 启用认证
security:
  authorization: enabled  # 关键设置

# 绑定 IP 和端口（可选，按需调整）
net:
  bindIp: 0.0.0.0   # 允许远程连接（生产环境建议限制 IP）
  port: 27017        # 默认端口
```

------

**2. 创建管理员用户**
 a. 启动 MongoDB ​**​无需认证​**​（临时配置）：

bash

复制

```
sudo systemctl stop mongod
sudo mongod --config /etc/mongod.conf --noauth  # 临时禁用认证
```

b. 使用 `mongo` shell 创建用户：

javascript

复制

```
use admin
db.createUser({
  user: "admin",
  pwd: "yourStrongPassword",
  roles: [ 
    { role: "userAdminAnyDatabase", db: "admin" },  // 用户管理权限
    { role: "readWriteAnyDatabase", db: "admin" }   // 数据读写权限
  ]
})
```

c. 重启 MongoDB **启用认证**：

bash

复制

```
sudo systemctl restart mongod
```

------

**3. 验证认证配置**
 a. 尝试无认证连接（应失败）：

bash

复制

```
mongo --host 127.0.0.1 --port 27017
> show dbs  // 输出错误：Unauthorized
```

b. 使用凭据连接（应成功）：

bash

复制

```
mongo -u admin -p yourStrongPassword --authenticationDatabase admin
> show dbs  // 正常显示数据库列表
```

------

**4. 高级认证方式（可选）**
 a. ​**​x.509 证书认证​**​（生产环境推荐）：

yaml

复制

```
security:
  authorization: enabled
  clusterAuthMode: x509  # 集群间通信使用证书

net:
  ssl:
    mode: requireSSL
    PEMKeyFile: /path/to/server.pem
    CAFile: /path/to/ca.pem
```

b. **LDAP 代理认证**（企业版专属）：

yaml

复制

```
security:
  authorization: enabled
  ldap:
    servers: "ldap.example.com"
    transportSecurity: tls
    authz:
      queryTemplate: "ou=users,dc=example,dc=com??sub?(uid={USER})"
```

------

**5. 配置文件权限加固**

bash

复制

```
sudo chmod 600 /etc/mongod.conf   # 限制配置文件仅 root 可读写
sudo chown root:root /etc/mongod.conf
```

------

**常见问题解决**

| 错误场景                           | 解决方案                                                  |
| ---------------------------------- | --------------------------------------------------------- |
| `Authentication failed`            | 检查用户名、密码、认证数据库 (`--authenticationDatabase`) |
| `No permission to execute command` | 为用户分配更高权限角色（如 `root`）                       |
| 配置文件修改后服务无法启动         | 检查 YAML 格式（缩进必须为空格，不能使用 Tab）            |

------

**总结**
 • 核心配置：`security.authorization: enabled`

• 最小权限原则：按需分配角色（如 `readWrite`、`dbAdmin`）

• 生产建议：使用 x.509 证书替代密码认证，并启用 TLS 加密

通过以上步骤，MongoDB 将强制所有客户端连接提供有效凭据，确保数据安全。