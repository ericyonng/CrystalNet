# Protobuf版本:v3.21.9.0

简称21.9版本

## Proto规则

2. 注释解析需要使用/// 来用于某条协议的解析参数, /// 后是要解析的内容 /

   /// 必须前面没有任何的有效字符 ，特殊字符会被自动trip掉：“\t\v\r\n\f”，包括空格

   要解析的参数格式：Opcode:xxx, 每个参数逗号隔开（参数也可以换行写，如果觉得一行过长，但前缀必须是///）， 例如：

   ```protobuf
   /// Opcode:
   message TestProtoReq
   {
   	string content = 0;	// content
   }
   ```

   

3. 格式参数列表

   ```protobuf
   Opcode:xxx opcode用来指定协议号，如果:后面是空的，即缺省指定，那么会自动生成协议号
   NoLog:true/false 用来指定是不是不需要打印协议的网络日志
   XorEncrypt:true 开启xor加密
   KeyBase64:true 生成的key需要base64编码
   ```
   
4. 关于pbcache文件

   该文件是用来缓存生成的协议信息的，请提交代码仓库，用来保证原有生产的协议信息不会被覆盖
   
5. 执行脚本生成协议：

   在代码工程根目录执行protoc.bat/protoc.sh即可生成

6. cmake构建solution的时候把INSTALL工程加上，会自动生成include目录

7. linux下：

   ```shell
   cmake ../cmake -DCMAKE_BUILD_TYPE:STRING=RELEASE -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=/root/pb  -Dprotobuf_BUILD_TESTS=off
   cmake ../cmake -DCMAKE_BUILD_TYPE:STRING=DEBUG -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=/root/pb  -Dprotobuf_BUILD_TESTS=off
   make install;
   ```

   

8. windows下 根据protobuf官方的方法生成静态库

9. protobuf 下bytes转成json的时候会使用base64编码

   

### proto文件结构/格式约束

#### 文件结构约束

> com.proto：项目中所有系统都会使用的基础数据类型定义。
> com_xxxx.proto：各系统通用数据结构定义，可以被其它系统的proto文件import，如非网络协议的message定义可以放在此处。
> xxxx.proto：各系统网络协议message定义，自己内部使用的message定义，此文件不允许被其它proto文件包含。
>
> 

#### 格式约束

要求每个message都有注释，message中的关键字段都必须有注释。

### 语法

#### protocol buffer基础语法

*遵循protocol buffer基础语法，不在此赘述。*

# protobuf arenas使用

[protobuf](https://cloud.tencent.com/developer/tools/blog-entry?target=https%3A%2F%2Fgithub.com%2Fprotocolbuffers%2Fprotobuf) 从3.0版本开始对C++增加了Arena接口，可以用于使用连续的内存块分配内部对象，并且可以更容易精确地控制对象地生命周期，最终达到减少内存碎片地目的。最近我给我们项目的部分接口流程进行相关地改造，在大多数使用 [protobuf](https://cloud.tencent.com/developer/tools/blog-entry?target=https%3A%2F%2Fgithub.com%2Fprotocolbuffers%2Fprotobuf) 的地方都增加了对Arena地支持，但是在接入过程中也碰到了一些问题和坑。

#### Arena实现地基本原理

Arena的原理十分简单，就是预先分配一个内存块。创建Message和内部对象的时候全部在分配好的内存块上 [placement new](https://cloud.tencent.com/developer/tools/blog-entry?target=https%3A%2F%2Fen.cppreference.com%2Fw%2Fcpp%2Flanguage%2Fnew) 出来，所有的Message对象也会内部记录所属的Arena以便创建字对象和某些情况下需要检查Arena时使用。如果创建的对象不支持Arena的，在 [placement new](https://cloud.tencent.com/developer/tools/blog-entry?target=https%3A%2F%2Fen.cppreference.com%2Fw%2Fcpp%2Flanguage%2Fnew) 完成后要在Arena上设置一个析构回调，以便在释放的时候调用析构流程。如果Arena内部的内存块剩余内存不足则会自动创建下一个(可能是更大的)内存块。

Arena可以在创建Arena的时候通过指定自定义的ArenaOptions来设置一些系数，包括最大内存块大小（如果超出了会直接用）、初始内存块大小、分配/回收内存块的实现、事件接口等。

每次Arena内存块剩余内存不足时，会尝试分配 ***最后一个内存块size\*2*** 和 ***ArenaOptions设置里的最大内存块*** 中的最小值（即: `min(2 * last_block.size, ArenaOptions.max_block_size)` ）。如果要分配的内存大小本身就是大于 ***ArenaOptions设置里的最大内存块*** 的，则会直接分配需要的内存块的大小+Header的大小（当前版本Header的大小是三个指针长度对齐到8，64位系统下就是24字节）。

#### 对长期存在对象的生命周期

Arena有一个特点是它维护的所有对象都是在Arena析构的时候统一释放的。这中间它内部维护的内存块只会不断地append，并不会删除。所以这也决定了由Arena维护的对象要么只能是临时对象，要么是不可变的。否则它的内存会无限增长下去。比如，我们是有状态[服务器](https://cloud.tencent.com/act/pro/promotion-cvm?from_column=20065&from=20065)，如果我们把一个用户的数据块长期缓存在内存里，然后Arena和用户对象的生命绑定。那么中间很多操作会不断地变更内部的对象结构，这就会导致用户下线前Arena无限增长。

所以，我们主要对Arena的集成最终集中在各个Task的入口处，然后一个Task里的子Task和RPC请求中需要创建的局部变量数据都复用这个Arena。当一个Task及其子Task全部结束以后，Arena就释放了。而除非少量的一些对全服数据操作的Task以外，大多数Task生命周期也就几秒中，内存的回收时间就相对可控。

#### 初始化分配的大小和最大分配的大小

在 ***ArenaOptions设置*** 里，默认的初始分配大小是 256B ，最大分配大小是 8KB 。前面也提到，我们的集成主要在各个Task的入口处，在Task里光是链路跟踪和RPC header相关的数据就占了100-200字节，而实际使用中一个Task的请求包、应答包就2倍曹处256B了。所以我们把初始值提升到了512B。同时我们项目中战斗记录的包都偏大，然后一些玩家数据拉取的包体也比较大，所以最大值也提高到了64KB。当然这些值后面有待观察，我们后面出了更详细的统计之后可能也再会调整。

#### 直接迁移 `set_allocated_XXX/release_XXX` 可能导致内存泄漏

在 [protobuf](https://cloud.tencent.com/developer/tools/blog-entry?target=https%3A%2F%2Fgithub.com%2Fprotocolbuffers%2Fprotobuf) 里，经常会碰上一些类似消息转发或者复用某些Message的操作，如果这些Message比较大，Copy的话显然是比较浪费的。所以有些地方会使用 `set_allocated_XXX` 和 `release_XXX` 接口来复用某些Message。 比如在我们的项目里，保存数据到DB的时候经常会有这种操作：

```javascript
// 参数传入 user_basic_profile;
table_message_type container;
container.set_id(user_id);
container.set_allocated_basic_profile(&user_basic_profile); // 直接复用已有的数据结构，用于后续打包
// ... 其他类似赋值代码
int result = pack_and_send(TcaplusService::TCAPLUS_API_UPDATE_REQ, container); // 打包和RPC
container.release_basic_profile(&user_basic_profile);       // 释放生命周期管理
// ... 其他类似释放代码
```

复制

但是加入了Arena之后就不一样了。我们不能简单地把代码改成这样:

```javascript
// 参数传入 arena, user_basic_profile;
table_message_type* container = ::google::protobuf::Arena::CreateMessage<table_message_type>(arena);
container->set_id(user_id);
container->set_allocated_basic_profile(&user_basic_profile); // 直接复用已有的数据结构，用于后续打包
// ... 其他类似赋值代码
int result = pack_and_send(TcaplusService::TCAPLUS_API_UPDATE_REQ, container); // 打包和RPC
container->release_basic_profile(&user_basic_profile);       // 释放生命周期管理
// ... 其他类似释放代码
// ...
```

复制

为什么呢？我们来看看生成的 `set_allocated_basic_profile` 和 `release_basic_profile` 。

```javascript
// ============ set_allocated_basic_profile ============
void table_message_type::set_allocated_basic_profile(user_basic_profile_t* basic_profile) {
  ::PROTOBUF_NAMESPACE_ID::Arena* message_arena = GetArena();
  if (message_arena == nullptr) {
    delete reinterpret_cast< ::PROTOBUF_NAMESPACE_ID::MessageLite*>(basic_profile_);
  }
  if (basic_profile) {
    ::PROTOBUF_NAMESPACE_ID::Arena* submessage_arena =
      reinterpret_cast<::PROTOBUF_NAMESPACE_ID::MessageLite*>(basic_profile)->GetArena();
    if (message_arena != submessage_arena) {
      basic_profile = ::PROTOBUF_NAMESPACE_ID::internal::GetOwnedMessage(
          message_arena, basic_profile, submessage_arena);
    }
    
  } else {
    
  }
  basic_profile_ = basic_profile;
  // @@protoc_insertion_point(field_set_allocated:tdr2pb.TABLE_USER_DEF.basic_profile)
}

// ------------ set_allocated_basic_profile 相关接口实现: GetOwnedMessage  ------------
template <typename T>
T* GetOwnedMessage(Arena* message_arena, T* submessage,
                   Arena* submessage_arena) {
  // The casts must be reinterpret_cast<> because T might be a forward-declared
  // type that the compiler doesn't know is related to MessageLite.
  return reinterpret_cast<T*>(GetOwnedMessageInternal(
      message_arena, reinterpret_cast<MessageLite*>(submessage),
      submessage_arena));
}

// ------------ set_allocated_basic_profile 相关接口实现: GetOwnedMessage  ------------
MessageLite* GetOwnedMessageInternal(Arena* message_arena,
                                     MessageLite* submessage,
                                     Arena* submessage_arena) {
  GOOGLE_DCHECK(submessage->GetArena() == submessage_arena);
  GOOGLE_DCHECK(message_arena != submessage_arena);
  if (message_arena != NULL && submessage_arena == NULL) {
    message_arena->Own(submessage);                    // 堆上的message直接转移进arena
    return submessage;
  } else {
    MessageLite* ret = submessage->New(message_arena); // 如果message_arena非空，则是复制了一个对象并放在message_arena上，否则堆上复制了一个对象。并不影响原message的生命周期
    ret->CheckTypeAndMergeFrom(*submessage);
    return ret;
  }
}

// ============ release_basic_profile ============
inline user_basic_profile_t* table_message_type::release_basic_profile() {
  user_basic_profile_t* temp = basic_profile_;
  basic_profile_ = nullptr;
  if (GetArena() != nullptr) {
    temp = ::PROTOBUF_NAMESPACE_ID::internal::DuplicateIfNonNull(temp);
  }
  return temp;
}
```

复制

重要的注解都在上面标注了。回到我们之前的例子，对于底层接口而言，我们不能假设传入的 `user_basic_profile` 是在哪里分配的。实际上对于上面例子里的[数据库](https://cloud.tencent.com/solution/database?from_column=20065&from=20065)保存操作，大多数情况下 `user_basic_profile` 来自于user对象的内存缓存。前面也提及了，这部分数据是在堆上的，那么对于这种情况，前面改Arena的例子里实际的流程就会变成这样:

```javascript
// 参数传入 arena, user_basic_profile;
table_message_type* container = ::google::protobuf::Arena::CreateMessage<table_message_type>(arena);
container->set_id(user_id);
container->set_allocated_basic_profile(&user_basic_profile); // 只要user_basic_profile不在arena上，那么这里就复制了一份数据
// ... 其他类似赋值代码
int result = pack_and_send(TcaplusService::TCAPLUS_API_UPDATE_REQ, container); // 打包和RPC
container->release_basic_profile(&user_basic_profile);       // 这里则是复制了一份user_basic_profile，因为返回值被忽略了，这里就内存泄露了。
// ... 其他类似释放代码
// ...
```

复制

开启Arena之后，实际上增加了两个函数 `unsafe_arena_set_allocated_XXX` 和 `unsafe_arena_release_XXX` 。我们能不能直接用这个代替掉 `set_allocated_XXX` 和 `release_XXX` 呢？我们继续来看它生成的代码:

```javascript
// ============ unsafe_arena_set_allocated_basic_profile ============
inline void TABLE_USER_DEF::unsafe_arena_set_allocated_basic_profile(
    ::mvp::table_user_basic_profile* basic_profile) {
  if (GetArena() == nullptr) {
    delete reinterpret_cast<::PROTOBUF_NAMESPACE_ID::MessageLite*>(basic_profile_); // 注意这里父Message如果Arena是空直接调用了delete子成员，这里没有判断子成员是否是在某个Arena里的。
  }
  basic_profile_ = basic_profile;
  if (basic_profile) {
    
  } else {
    
  }
  // @@protoc_insertion_point(field_unsafe_arena_set_allocated:tdr2pb.TABLE_USER_DEF.basic_profile)
}

// ============ unsafe_arena_release_basic_profile ============
inline user_basic_profile_t* TABLE_USER_DEF::unsafe_arena_release_basic_profile() {
  // @@protoc_insertion_point(field_release:tdr2pb.TABLE_USER_DEF.basic_profile)
  
  user_basic_profile_t* temp = basic_profile_;
  basic_profile_ = nullptr;
  return temp;
}
```

复制

只要使用者能保证如果父级Message如果是Arena分配的，调用 `unsafe_arena_set_allocated_XXX` 时成员为空，那么这里是可以直接代替的。但是在实际调用流程复杂了以后仍然怕这部分不小心误用，一旦误用带来的后果也很严重并且很难排查。所以我们项目中是仅仅代码生成器会使用这个接口，人工调用是禁止的。最终的变更形式如下:

```javascript
// 参数传入 arena, user_basic_profile;
table_message_type* container = ::google::protobuf::Arena::CreateMessage<table_message_type>(arena); // 这里能保证刚创建出来的一定为空
container->set_id(user_id);
if (container->GetArena() == user_basic_profile.GetArena()) {
    if (nullptr == user_basic_profile.GetArena()) {
        container->set_allocated_basic_profile(&user_basic_profile);
    } else {
        container->unsafe_arena_set_allocated_basic_profile(&user_basic_profile);
    }
} else {
    protobuf_copy_message(*container->mutable_basic_profile(), user_basic_profile); // 退化到复制message，下面会贴protobuf_copy_message的实现
}
// ... 其他类似赋值代码
int result = pack_and_send(TcaplusService::TCAPLUS_API_UPDATE_REQ, container); // 打包和RPC
if (container->GetArena() == user_basic_profile.GetArena()) {
    if (nullptr == user_basic_profile.GetArena()) {
        container->release_basic_profile();
    } else {
        container->unsafe_arena_release_basic_profile();
    }
}
// ... 其他类似释放代码
// ...
```

复制

#### Swap退化成Copy

最后一个问题是和 `Swap` 接口有关。有些接口流程里，我们会用Swap来减少不必要的复制。常见的地方比如在dispatcher层（有些框架叫 executor）来解包后处理一些前置信息，完了之后透传部分数据到业务层。这时候就经常用 `Swap` 接口来减少不必要的复制。但是开启了Arena之后，生成的代码就变成了如下形式:

```javascript
// .pb.h
inline void Swap(ConstSettingsType* other) {
  if (other == this) return;
  if (GetArena() == other->GetArena()) {
    InternalSwap(other);
  } else {
    ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
  }
}
// 增加了一个 UnsafeArenaSwap 函数
void UnsafeArenaSwap(ConstSettingsType* other) {
  if (other == this) return;
  GOOGLE_DCHECK(GetArena() == other->GetArena());
  InternalSwap(other);
}

// .pb.cpp
void ConstSettingsType::InternalSwap(ConstSettingsType* other) {
  using std::swap;
  _internal_metadata_.Swap<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(&other->_internal_metadata_);                        // 内部数据swap
  rpc_version_.Swap(&other->rpc_version_, &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena()); // 字段的swap
}

// reflection_ops.cc , MessageLite的版本和这个差不多，就不列举了
void GenericSwap(Message* m1, Message* m2) {
  Arena* m2_arena = m2->GetArena();
  GOOGLE_DCHECK(m1->GetArena() != m2_arena);

  // Copy semantics in this case. We try to improve efficiency by placing the
  // temporary on |m2|'s arena so that messages are copied twice rather than
  // three times.
  Message* tmp = m2->New(m2_arena);
  std::unique_ptr<Message> tmp_deleter(m2_arena == nullptr ? tmp : nullptr);
  tmp->CheckTypeAndMergeFrom(*m1);
  m1->Clear();
  m1->CheckTypeAndMergeFrom(*m2);
  m2->GetReflection()->Swap(tmp, m2);
}
```

复制

可以看到，在所属的Arena不同的时候 `Swap` 函数实际执行了两次Copy和一次创建Message。所以这种时候还不如直接Copy。基于此，我们原来为了编译期检查一下Copy的protobuf message的类型提供了 `protobuf_copy_message` 函数来代替直接 `CopyFrom` ， 现在又额外提供了 `protobuf_move_message` 来处理这种转移数据的 `Swap` 调用。

```javascript
// protobuf_copy_message
template <class TMsg>
inline void protobuf_copy_message(TMsg &dst, const TMsg &src) {
    dst.CopyFrom(src);
}

template <class TField>
inline void protobuf_copy_message(::google::protobuf::RepeatedField<TField> &dst, const ::google::protobuf::RepeatedField<TField> &src) {
    dst.Reserve(src.size());
    dst.CopyFrom(src);
}

template <class TField>
inline void protobuf_copy_message(::google::protobuf::RepeatedPtrField<TField> &dst, const ::google::protobuf::RepeatedPtrField<TField> &src) {
    dst.Reserve(src.size());
    dst.CopyFrom(src);
}

// protobuf_move_message
template <class TMsg>
inline void protobuf_move_message(TMsg &dst, TMsg &&src) {
    if (dst.GetArena() == src.GetArena()) {
        dst.Swap(&src);
    } else {
        protobuf_copy_message(dst, src);
    }

    src.Clear();
}

template <class TField>
inline void protobuf_move_message(::google::protobuf::RepeatedField<TField> &dst, ::google::protobuf::RepeatedField<TField> &&src) {
    if (dst.GetArena() == src.GetArena()) {
        dst.Swap(&src);
    } else {
        protobuf_copy_message(dst, src);
    }

    src.Clear();
}

template <class TField>
inline void protobuf_move_message(::google::protobuf::RepeatedPtrField<TField> &dst, ::google::protobuf::RepeatedPtrField<TField> &&src) {
    if (dst.GetArena() == src.GetArena()) {
        dst.Swap(&src);
    } else {
        protobuf_copy_message(dst, src);
    }

    src.Clear();
}
```

复制

#### 写在最后

目前的碰到的问题基本就这么多了，近期的 [protobuf](https://cloud.tencent.com/developer/tools/blog-entry?target=https%3A%2F%2Fgithub.com%2Fprotocolbuffers%2Fprotobuf) 大版本更新对Arena还有一些改进，其中包含对 `std::string` 类型的特殊处理和在Arena上分配Map时的一处 **use-after-destroy bug** ，避开使用就好了。

> 以上代码使用 [protobuf](https://cloud.tencent.com/developer/tools/blog-entry?target=https%3A%2F%2Fgithub.com%2Fprotocolbuffers%2Fprotobuf) 3.13.0 版本。在 [protobuf](https://cloud.tencent.com/developer/tools/blog-entry?target=https%3A%2F%2Fgithub.com%2Fprotocolbuffers%2Fprotobuf) 3.14.0 版本之前，要开启C++ Arena接口要在proto的文件级选项里加上 `option cc_enable_arenas = true;` 。

# 其他

若需要支持pb与json互转:

```
#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>
#include <google/protobuf/text_format.h>

::google::protobuf::util::MessageToJsonString(_data, &jsonStr)
```



## TODO ：

1. protobuf DebugString有坑，它内部是使用指针做偏移那么此时若是多继承的话,就可能导致内存错乱了，正确做法是，将新增的基类放到继承链的最后，尽量少做一些操作，这样会导致内存布局发生变化
2. 小程序端服务器的下行包一般不加密（小程序端的rsa不支持私钥加密公钥解密）