### 1. `result::bulk_write` 可访问的附加方法

通过 `result.result()` 获取后，可调用：

| 方法               | 返回类型       | 说明                                        |
| ------------------ | -------------- | ------------------------------------------- |
| `inserted_count()` | `std::int32_t` | 插入文档数（replace_one 场景通常为 0 或 1） |
| `matched_count()`  | `std::int32_t` | 同 `replace_one::matched_count()`           |
| `modified_count()` | `std::int32_t` | 同 `replace_one::modified_count()`          |
| `deleted_count()`  | `std::int32_t` | 删除文档数（replace_one 场景始终为 0）      |
| `upserted_count()` | `std::int32_t` | upsert 文档数（0 或 1）                     |
| `upserted_ids()`   | `id_map`       | 所有 upsert 文档的 `_id` 映射               |

> ⚠️ **注意**：`result::replace_one` 自身**没有** `upserted_count()` 方法（与 `result::update` 不同），判断是否发生 upsert 可通过 `upserted_id().has_value()` 来判断。

```
mongocxx::options::replace opts{};
opts.upsert(true);  // 无匹配时插入新文档

auto result = coll.replace_one(filter, replacement, opts);
if (result) {
    if (result->upserted_id()) {
        // upsert 发生：原文档不存在，新文档被插入
        std::cout << "文档不存在，已insert，_id: " 
                  << result->upserted_id()->get_oid().value.to_string() << std::endl;
    } else if (result->matched_count() > 0) {
        // 正常替换：原文档存在，被替换
        std::cout << "文档已替换，modified: " << result->modified_count() << std::endl;
    }
}

```

### 2. `matched_count` vs `modified_count` 的关键区别

| 场景                                 | `matched_count()` | `modified_count()`           |
| ------------------------------------ | ----------------- | ---------------------------- |
| 找到文档，内容不同 → 替换            | 1                 | 1                            |
| 找到文档，内容完全相同 → 无实际修改  | 1                 | **0**                        |
| 没找到文档，无 upsert                | 0                 | 0                            |
| 没找到文档，upsert 触发 → 插入新文档 | 0                 | 0（但 `upserted_id()` 有值） |

这一点非常重要：**不要用 `matched_count() == 0` 来判断操作是否成功**。如果新文档和旧文档内容一致，`matched_count()` 为 1 但 `modified_count()` 为 0，在业务上需要正确区分这两种情况。



# options::replace的write_concern(wc), 我不设置我设置在collection.write_concern(concern)可以不

The user is asking about write_concern in mongocxx: if they set the write_concern on the collection level (via `collection.write_concern(concern)`), does it apply to `replace_one` operations, or do they need to set it specifically in `options::replace`?

This is a knowledge question about mongocxx behavior. Let me check the collection.hpp to confirm how collection-level write_concern works and its relationship with per-operation options.

I already have the collection.hpp partially read. Let me search for the write_concern method on the collection class.
好问题，让我确认一下 collection 级别的 `write_concern` 与 per-operation 的 `options::replace.write_concern` 之间的关系。
Now I can see that the collection class has a `write_concern()` setter. The question is: does the collection-level write_concern cascade down to `replace_one` operations? 

Based on the mongocxx driver design, the answer is yes: collection-level write_concern serves as the default for all write operations on that collection. If you set it on the collection, it will be inherited by `replace_one`. If you also set it in `options::replace`, the per-operation setting overrides the collection-level setting.

This is a standard behavior in mongocxx:
- Collection-level `write_concern` = default for all operations
- Per-operation `options::replace.write_concern()` = override for specific operation

So the user can absolutely just set `coll.write_concern(wc)` and not set it in `options::replace`. The per-operation option only needs to be set if you want a DIFFERENT write concern for that specific operation compared to the collection default.

This is a straightforward knowledge question, no plan needed.
**可以，完全没问题。**

`collection.write_concern(wc)` 设置的是**集合级别的默认写关注**，所有该集合上的写操作（包括 `replace_one`、`insert_one`、`update_one`、`delete_one` 等）都会自动继承这个设置。

### 优先级规则

```
per-operation 设置 > collection 默认设置 > client/database 默认设置 > 服务器默认
```

具体来说：

| 设置方式                               | 作用范围             | 优先级   |
| -------------------------------------- | -------------------- | -------- |
| `options::replace{}.write_concern(wc)` | 仅本次 `replace_one` | **最高** |
| `collection.write_concern(wc)`         | 该集合所有写操作     | 中间     |
| `uri` 中的 `w` 参数                    | 整个连接             | 最低     |

### 实际写法

```cpp
// 方式一：集合级别设置（推荐，保持一致）
coll.write_concern(mongocxx::write_concern{}.acknowledge_level(
    mongocxx::write_concern::level::k_majority));

// replace_one 自动继承上述 write_concern，无需重复设置
auto result = coll.replace_one(filter, replacement);
```

只用在你需要**某个特定操作使用不同的写关注**时，才在 `options::replace` 中单独指定：

```cpp
// 集合默认 majority，但这次操作想用 w:1 加快速度
auto opts = mongocxx::options::replace{}.write_concern(
    mongocxx::write_concern{}.acknowledge_level(
        mongocxx::write_concern::level::k_default)
);
auto result = coll.replace_one(filter, replacement, opts);
```

---

另外注意：`options::replace` 里的 `write_concern` 类型是 `optional<write_concern>`，**不设置它就是空的 optional**，驱动会 fallback 到 collection 级别。所以两者不冲突，是完全正交的设计。