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