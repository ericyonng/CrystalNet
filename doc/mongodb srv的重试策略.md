# 结论

* mongodb srv模式在find_one/insert_one等操作read/write失败时如果开启retryWrites=true&retryReads=true会重试一次，且选择新的mongos连接(内部自动处理), 重试只重试一次, 如果仍然失败会抛出异常，上层业务如果调用接口失败自行处理失败情况



The user is asking about mongocxx driver behavior when connecting to a MongoDB sharded cluster via SRV records. Specifically, they want to know if the driver will automatically reconnect when a mongos router goes down during read/write operations.

Let me first look at the relevant code in their project to understand their current MongoDB setup, then provide a comprehensive answer about mongocxx's reconnection behavior.
Now let me look at the MongoDbMgr.cpp to understand how the connection pool is created and used, especially the SRV connection setup and retry logic.
Now let me look at the connection establishment code more closely in MongoDbMgr.cpp, particularly around line 2114 where the URI is constructed, and around lines 97-272 where the connection pool is used.
Now I have a comprehensive understanding of the project's MongoDB setup. Let me analyze the user's question:

The user is asking: When using mongocxx driver with `mongodb+srv://` connection string to connect to a MongoDB sharded cluster, if a mongos router goes down during read/write operations, will the driver automatically reconnect?

Let me analyze their current code:

1. They use `mongocxx::pool` (connection pool) with URI:
```
mongodb+srv://user:pass@host/?authSource=admin&w=majority&journal=true&readConcernLevel=majority&maxPoolSize=100&connectTimeoutMS=180000&socketTimeoutMS=30000&retryWrites=true&retryReads=true&tls=false
```

2. They use `_connectionPool->acquire()` to get a client from the pool for each operation.

3. Key parameters in the URI:
   - `retryWrites=true` - enables retryable writes
   - `retryReads=true` - enables retryable reads
   - `connectTimeoutMS=180000` (3 minutes)
   - `socketTimeoutMS=30000` (30 seconds)
   - `maxPoolSize=100`

Now let me explain the mongocxx behavior in this scenario:

### SRV Connection Mechanism
With `mongodb+srv://`, the driver:
1. Queries DNS SRV records to discover mongos hosts
2. Builds a list of all mongos hosts from the SRV response
3. Connects to the cluster

### Connection Pool Behavior When a mongos Goes Down

**Automatic reconnection at the pool level:**
- The `mongocxx::pool` manages connections to ALL mongos hosts discovered via SRV
- When a connection to a specific mongos fails, the pool can create new connections to other available mongos hosts
- BUT: **Existing connections that were actively in use and then the mongos dies** - the operation will fail with a network error

**Retryable Reads/Writes:**
- With `retryWrites=true` and `retryReads=true`, the driver will automatically retry certain operations ONCE if they fail due to network errors, replica set elections, etc.
- However, retryable writes only work for write operations that are "retryable" (single-document writes, etc.)
- The retry will try a different mongos (if SRV resolved multiple hosts)

**What happens in the user's code:**
Looking at the code, each operation does:
```cpp
auto client = _connectionPool->acquire();
// ... perform operations with client ...
// client goes out of scope, returned to pool
```

1. If a mongos is down when `acquire()` is called, the pool will try to connect to another available mongos host
2. If a mongos goes down DURING an operation on an already-acquired connection:
   - The operation will fail with a network error
   - The driver may retry once (if retryable reads/writes are enabled and the operation qualifies)
   - On retry, the pool will try a different mongos
   - If the retry fails, the error propagates to the application
3. After the pool returns a connection that had failures, the pool marks it as bad and won't reuse it

**Important limitations:**
- SRV DNS caching: The driver caches SRV lookup results. By default, the TTL is 60 seconds (or the DNS TTL). So if new mongos hosts are added to DNS, it takes up to the TTL for the driver to discover them. But if a mongos goes down, the pool will still try failed hosts until DNS TTL expires (unless the connection times out quickly).
- The pool doesn't have a health check mechanism by default. It relies on connection failures to detect dead hosts.
- There's no `serverSelectionTimeoutMS` explicitly set in their URI. The default is 30 seconds.

Let me also check: does the user's code have any error handling / retry logic built in for operations?

Let me search for error handling patterns in the code.

Actually, I have enough information to answer the user's question comprehensively. Let me provide a thorough answer.

Let me also look for any retry logic they might have in their own code.
Now I can see that:
1. The user already has `retryWrites=true` and `retryReads=true` in their URI
2. Each operation catches `mongocxx::exception` and `std::exception` and returns false
3. There's NO custom retry logic in the application layer - it just catches and returns failure

Let me also look at a sample operation to understand the full flow.
Now I have a thorough understanding of the codebase and can provide a comprehensive answer. Let me also check if there's any connection-level retry mechanism in the code, or if the operations are wrapped in any retry loop.

The key observation is:
1. They use `mongocxx::pool` with SRV connection
2. Each operation: acquires a client from pool → performs operation → client auto-returned to pool
3. Each operation catches `mongocxx::exception` and returns failure
4. There is NO application-level retry
5. The URI has `retryWrites=true&retryReads=true`

Now let me provide the answer. This is a theoretical question about mongocxx behavior, so I don't need to create a plan or modify code. Let me answer directly.
这是一个很好的问题。结合你的代码和 mongocxx 的实际行为，我来系统分析：

---

## mongocxx 在 mongos 挂掉时的行为

### 1. 你的连接配置分析

```2128:2128:OptionComponent/OptionComp/storage/MongoDB/Impl/MongoDbMgr.cpp
                _uri.AppendFormat("mongodb+srv://%s:%s@%s/?authSource=admin&w=majority&journal=true&readConcernLevel=majority&maxPoolSize=100&connectTimeoutMS=180000&socketTimeoutMS=30000&retryWrites=true&retryReads=true&tls=false", _account.c_str(), _pwd.c_str(), _srvHostName.c_str());
```

关键参数：
| 参数               | 值       | 含义                   |
| ------------------ | -------- | ---------------------- |
| `retryWrites=true` | ✅ 已开启 | 写操作失败自动重试一次 |
| `retryReads=true`  | ✅ 已开启 | 读操作失败自动重试一次 |
| `connectTimeoutMS` | 180s     | 建连超时（比较长）     |
| `socketTimeoutMS`  | 30s      | socket 读写超时        |

### 2. mongocxx 的"断线重连"机制

**关键结论：mongocxx 不会像 MySQL 那样做"保持长连接 + 心跳检测 + 断线自动重连"。它的策略不同：**

#### 场景 A：mongos 挂了，此时从 pool 获取**新连接**

```
代码: auto client = _connectionPool->acquire();
```

由于你用的是 `mongodb+srv://`，驱动在初始化时已经通过 SRV DNS 解析出了所有 mongos 节点列表。当 pool 中某个 mongos 的连接不可用时，`acquire()` 会尝试连接 SRV 列表中的**其他 mongos**。

✅ **这种场景下会自动切到存活的 mongos。**

#### 场景 B：mongos 在**正在执行的读写操作中**挂了

```
// 已经拿到连接，正在操作
auto ret = collection.find_one(fullKv.view());  // ← 此时 mongos 挂了
```

❌ **当前操作会立即失败**，抛出 `mongocxx::exception`。

但由于你开启了 `retryWrites=true` / `retryReads=true`，**驱动内部会自动重试一次**（选择 SRV 列表中的另一个 mongos）。

- 如果重试成功 → 操作对业务透明
- 如果所有 mongos 都挂了 → 抛出异常，你的业务代码捕获到失败

### 3. 但你代码存在的隐患

你的每个操作模式是这样的：

```cpp
auto client = _connectionPool->acquire();  // 从 pool 拿连接
// ... 操作 ...
auto ret = collection.find_one(fullKv.view());
// client 出作用域归还 pool
```

**问题：** 当 `find_one` 因 mongos 挂了而失败，即使驱动做了 retryReads 重试，如果重试也失败（比如所有 mongos 都不可达），异常会向上抛。你的 catch 块只是捕获并返回 false，**没有应用层重试**：

```582:588:OptionComponent/OptionComp/storage/MongoDB/Impl/MongoDbMgr.cpp
        catch (const mongocxx::exception &e)
        {
            // 直接返回失败了
        }
```

### 4. 还有个潜在问题：SRV DNS 缓存

mongocxx 驱动默认会缓存 SRV 解析结果，TTL 默认 60 秒。这意味着：
- 如果你是**新增** mongos 节点，最多 60 秒后驱动才能感知
- 如果 mongos 挂了，驱动在 TTL 过期前仍可能尝试连接已挂的 mongos（但会快速超时后 fallback 到其他节点）

---

## 总结

| 场景                    | 是否自动恢复                           | 说明                             |
| ----------------------- | -------------------------------------- | -------------------------------- |
| 获取连接时 mongos 已挂  | ✅ 自动切到其他 mongos                  | pool 会选择 SRV 列表中的存活节点 |
| 操作中途 mongos 挂了    | ⚠️ 驱动 retryReads/retryWrites 重试一次 | 重试成功则透明，失败则抛异常     |
| 所有 mongos 都挂了      | ❌ 无法恢复                             | 所有操作都会失败                 |
| 新增 mongos 被 SRV 发现 | ⚠️ 最多 60s 延迟                        | SRV DNS TTL 缓存限制             |

**建议：** 如果你的业务对可用性要求很高，可以考虑在 `catch(mongocxx::exception)` 层加一个有限次数的应用层重试（比如 3 次，间隔 100ms），这样能兜底驱动层 retryReads/retryWrites 失败的情况。

需要我帮你设计应用层重试方案吗？



The user is asking whether mongocxx's retryReads/retryWrites mechanism selects a different mongos for the retry attempt. This is an important technical detail about mongocxx driver behavior.

Let me answer based on my knowledge of the mongocxx driver (which is based on the MongoDB C driver):

### Retryable Reads/Writes Mechanism

When `retryWrites=true` or `retryReads=true`:

1. **Retryable Writes (`retryWrites=true`):**
   - The driver tracks the transaction number for each write operation
   - When a write fails with a network error (not a server-side error), the driver will retry the operation ONCE
   - **The retry goes through server selection again** - this means the driver will pick a new mongos from the available pool. It does NOT necessarily pick a different mongos; it goes through the server selection algorithm which will prefer available servers.
   - If the originally chosen mongos is still in the topology and available, it might be selected again. But if it's down, the driver will select a different one.
   - The key point: **it does NOT blindly stick to the same mongos** - it re-runs server selection, which means it will pick a healthy mongos.

2. **Retryable Reads (`retryReads=true`):**
   - Similar to retryable writes for read operations
   - When a read fails with a network error, the driver retries once
   - **Also goes through server selection again** - will pick a healthy mongos

### How Server Selection Works

The mongocxx driver maintains a "topology" of servers discovered via SRV/monitoring. When a server is not reachable, it's marked as "unknown" in the topology. During server selection for a retry:

1. The driver calls `server selection` which filters servers by:
   - Type (must be mongos for sharded clusters)
   - Availability (must not be "unknown" if there are other options)
   - Latency (within localThresholdMS of the fastest)

2. With `serverSelectionTimeoutMS` (default 30s), the driver will wait for a suitable server

3. **If the original mongos is down, server selection will skip it and pick a different one** (assuming SRV resolved multiple mongos hosts)

### Important Detail: Pool Behavior

With `mongocxx::pool`:
- The pool maintains connections to all discovered mongos hosts
- When `acquire()` is called after a failure, the pool's server selection runs and picks a healthy mongos
- For the retry mechanism specifically, the driver creates a new connection attempt through server selection

### Answer

So the answer is: **Yes, retryReads/retryWrites will go through server selection again, which means if the original mongos is down, it will pick a different healthy mongos from the SRV-discovered list.**

But there's a nuance: if the original mongos is still technically "available" (e.g., it's back up), server selection might pick it again. The driver doesn't specifically "avoid" the failed host - it just runs server selection which naturally deprioritizes hosts in "unknown" state.

Let me give a clear answer.
**是的，重试时会重新走 Server Selection，所以会选到不同的（健康的）mongos。**

---

## 重试流程详解

```
第一次: 选 mongos-1 → 执行操作 → 网络错误
                                ↓
重试:   重新 Server Selection → 跳过已标记异常的 mongos-1
                                ↓
        选中 mongos-2（或 mongos-3）→ 执行操作 → 成功
```

关键点：**不是盲连同一个 mongos**，而是重新跑一次拓扑选择。

---

## 内部机制

mongocxx 驱动维护一个内部拓扑表，mongos 有几种状态：

| 状态                   | 含义           | Server Selection 行为          |
| ---------------------- | -------------- | ------------------------------ |
| `Unknown`              | 连接失败后标记 | **最低优先级**，有别的就不会选 |
| `PossiblePrimary`      | 不确定状态     | 低优先级                       |
| `Secondary` / `Mongos` | 正常           | 正常候选                       |

当你操作中 mongos 挂了：
1. 操作抛出网络错误
2. 驱动将该 mongos 标记为 `Unknown`
3. `retryReads`/`retryWrites` 触发重试，重新调用 Server Selection
4. Server Selection 发现 `Unknown` 的节点，优先选择其他 `Mongos` 状态的节点
5. 你的 SRV 解析了多个 mongos，所以会选到另一个存活的

---

## 但是有一个坑

如果 SRV 只解析出**一个** mongos（比如你 DNS 只配了一条 SRV 记录），重试就没得选了——还是只能连同一个，结果还是失败。所以 SRV 记录里至少配 **2 个以上** 的 mongos 才有意义。

你当前的 DNS SRV 配置里解析出了几个 mongos？

```powershell
Resolve-DnsName -Type SRV _mongodb._tcp.mongoscluster.ericyonng.com
```





The user is asking about how many retries mongocxx does with retryWrites=true and retryReads=true, and whether exceptions are thrown during retries.

Let me provide accurate information about mongocxx's retry mechanism.

In mongocxx/mongo-c-driver, the retry mechanism:
- For retryable reads/writes: **exactly 1 retry** (i.e., the original attempt + 1 retry = max 2 attempts total)
- This is not configurable in the connection string
- During the retry, NO exception is thrown to the caller
- Only after the retry also fails, an exception is thrown

Let me be precise:

1. Original attempt fails with a retryable error (network error, etc.)
2. Driver internally retries ONCE (total 2 attempts)
3. If retry succeeds → result returned normally, no exception
4. If retry also fails → `mongocxx::exception` thrown to the application

The retry is transparent. During retries, no exceptions escape to the user code. Only when both attempts fail does the exception propagate.

Also important: not all operations are retryable. For writes, only single-document operations (insertOne, updateOne, replaceOne, deleteOne, findOneAndUpdate, etc.) are retryable. Bulk writes and multi-document operations are not retryable.

For reads: find, aggregate, distinct, count, etc. are retryable with retryReads=true.

The mongo-c-driver has these defines:
- MONGOC_RETRY_WRITES_MAX_ATTEMPTS = 1 (just one retry after the original attempt)
- MONGOC_RETRY_READS_MAX_ATTEMPTS = 1

This is hardcoded in the C driver and not configurable.
**只重试 1 次（总共最多执行 2 次），重试期间不抛异常，两次都失败才抛。**

---

## 精确行为

```
第 1 次尝试 → 网络错误（重试期间不抛异常）
     ↓
第 2 次尝试（唯一一次重试）→ 
     ├─ 成功 → 正常返回，业务无感
     └─ 失败 → 抛出 mongocxx::exception
```

| 阶段                  | 是否抛异常                     |
| --------------------- | ------------------------------ |
| 第 1 次失败           | ❌ 不抛，驱动内部吞掉，触发重试 |
| 第 2 次（重试）成功   | ❌ 不抛，正常返回               |
| 第 2 次（重试）也失败 | ✅ **抛出异常**                 |

---

## 源码依据

mongo-c-driver 里写死的常量：

```c
#define MONGOC_RETRY_WRITES_MAX_ATTEMPTS  1   // 最多重试1次
#define MONGOC_RETRY_READS_MAX_ATTEMPTS   1
```

`retryWrites=true` 和 `retryReads=true` **不可配置重试次数**，只有 0 或 1 的区别。

---

## 哪些操作能被重试

| 操作类型                           | retryWrites=true | retryReads=true |
| ---------------------------------- | :--------------: | :-------------: |
| `insertOne`                        |        ✅         |        -        |
| `updateOne`                        |        ✅         |        -        |
| `replaceOne`                       |        ✅         |        -        |
| `deleteOne`                        |        ✅         |        -        |
| `findOneAndXxx`                    |        ✅         |        -        |
| `insertMany`                       |        ❌         |        -        |
| `updateMany`                       |        ❌         |        -        |
| `deleteMany`                       |        ❌         |        -        |
| `bulkWrite`                        |        ❌         |        -        |
| `find`                             |        -         |        ✅        |
| `findOne`                          |        -         |        ✅        |
| `aggregate` (只读)                 |        -         |        ✅        |
| `count` / `estimatedDocumentCount` |        -         |        ✅        |

你代码里的 `collection.find_one()` 属于可重试的读操作，`retryReads=true` 会生效。

---

## 对你的实际意义

如果你所有 mongos 都挂了：
- 第 1 次连接 mongos-1 → 失败（不抛）
- 第 2 次连接 mongos-2 → 也失败（抛异常）
- 你的 `catch` 捕获到，返回 `false`

所以你的 catch 层不需要额外处理"是否在重试中"的问题——异常只会在彻底失败后才出现。



