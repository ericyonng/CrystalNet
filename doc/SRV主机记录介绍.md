The user is asking about the SRV record host name format `_mongodb._tcp.mongoscluster`. Let me explain what this is.
这是 SRV 记录的**标准格式**，不是瞎写的。

---

### 格式拆解

```
_mongodb._tcp.xxx.xxx.com
│         │    │              │
│         │    │              └─ 你的域名
│         │    └─ 服务名（你自定义的，可以叫 cluster1、prod 等）
│         └─ 协议（_tcp / _udp）
└─ 服务前缀（MongoDB 规定必须是 _mongodb）
```

### 为什么是这种下划线前缀

RFC 2782 标准定义 SRV 记录必须用 `_Service._Proto.Name` 格式：

| 服务    | SRV 格式            |
| ------- | ------------------- |
| MongoDB | `_mongodb._tcp.xxx` |
| MySQL   | `_mysql._tcp.xxx`   |
| LDAP    | `_ldap._tcp.xxx`    |
| SIP     | `_sip._udp.xxx`     |

下划线开头是**故意的**，避免和普通主机名（A 记录）冲突。

---

### mongod+srv 怎么用它

你代码里写的是：

```
mongodb+srv://user:pass@xxx.xxx.com
```

MongoDB driver 收到 `mongodb+srv://` 协议时会自动：
1. 在 xxx.xxx.com 前拼接 `_mongodb._tcp.`
2. 组成 `_mongodb._tcp.xxx.xxx.com
3. 发起 SRV 查询
4. 每个返回的 host:port 作为一个 mongos 节点

所以 `mongoscluster` 就是你给这组 mongos 起的名。你可以改，比如：

```
_mongodb._tcp.prod.xxx.xxx.com
→ mongodb+srv://prod.xxx.xxx.com
_mongodb._tcp.staging.xxx.xxx.com → 
mongodb+srv://staging.xxx.xxx.com
```

不同环境用不同名字区分开就行。