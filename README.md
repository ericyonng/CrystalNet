

![CrystalNet](./resource/img/CnLogo.png)

----



# 晶石框架

[![License](https://img.shields.io/github/license/ericyonng/FrightenStone.svg?style=popout "https://shields.io/category/license")](https://opensource.org/licenses/MIT) 

**作者**: ericyonng

**时间**: 2020-10-11

**联系方式**: <120453674@qq.com>

**FrameName**: CrystalNet

**Author**: Eric Yonng

**CreateTime**: 10/11/2020

# 构建状态

**Linux**: [![Build](https://app.travis-ci.com/ericyonng/CrystalNet.svg?branch=main)](http://travis-ci.com)

**Windows**: [![Build status](https://ci.appveyor.com/api/projects/status/0jarbeono1kve7dx?svg=true)](https://ci.appveyor.com/project/ericyonng/crystalnet)

--------

* 晶石框架是一个简易轻量的服务端框架，是在FrightenStone的基础上大改，完善的，基于C++11及以上
* 旨在开发出一套跨平台，代码简洁可读性高，执行高效的框架
* 框架正在持续开发中，接受各种建议吐槽
* **Attention**:region 注释会尽量使用简洁的英文，CI可能会报错哈QVQ
* 多个重要组件移植于[LLBC](https://github.com/lailongwei/llbc "Source from lailongwei:llbc")、 [nlohmann/Json](https://github.com/nlohmann/json "json")、 [OpenSSL](https://github.com/openssl/openssl)， 再次感谢开源作者的贡献，开发过程中给予了我很大的帮助

--------

# 注意

* linux 内核版本3.9.0以上才有reuseport特性

# 编码规范
* 非必要情况下尽量避免使用内存拷贝，可以通过设计达到写时复制效果,惰性拷贝,

* 多使用指针链表交换,避免使用节点全拷贝,提升并发效率

* 为了兼容lua所有的64位id的最高位都必须为0

* 接口类规则:I+具体的类名， 例如：一个User类它的接口类命名：IUser

* 派生类重写的虚函数需要加上override约束,保证该接口的基类是有该虚函数

* 在不想让派生类重写的虚函数加上final约束，以避免派生类重写该方法

* 除了必要之外不要引入过多的语言特性，因为语言特性越多那么学习成本越高，对入门的门槛越高，这违背了我们要提供一个易上手框架的初衷

* 对象的分配与释放建议使用：CRYSTAL_MA_NEW/CRYSTAL_MA_DELETE (基于对象池的内存管理)，泛型：(CRYSTAL_MA_TEMPLATE_NEW_P1/CRYSTAL_MA_TEMPLATE_DELETE_P1,...)一方面基于对象池，一方面有利于内存泄漏的追踪

* 一块缓存的分配与释放建议使用：KernelAllocMemory/KernelFreeMemory 基于内存池

* 简单数据类型（定义在type.h）：

  ```
  typedef char Byte8;
  typedef unsigned char U8;
  typedef short Int16;
  typedef unsigned short UInt16;
  typedef int Int32;
  typedef unsigned int UInt32;
  typedef long Long;
  typedef unsigned long ULong;
  typedef long long Int64;
  typedef unsigned long long UInt64;
  typedef float Float;
  typedef double Double;
  ```

* 尽量避免使用多继承

* 系统设计采用ECS设计，基础资源包括：IObject, CompObject, CompHostObject

* 回调多使用Delegate(可以支持：全局函数，类成员函数，lambda， std::function等， 见LibDelegate)

* 文档编码使用utf8，所以尽量使用vscode编辑，因为vscode默认utf8编码，vs则是ascii编码，vs只用于编译

* 容器的删除可以使用ContainerUtil::DelContainer, 该util可以支持AutoDelMethods中定义的各种删除方式，也可以支持传入lambda回调方式的自定义删除方式，例如：

  ```c++
  // dict 的value使用Release接口删除方式：
  KERNEL_NS::ContainerUtil::DelContainer(_dict, KERNEL_NS::AutoDelMethods::Release);
  ```

  

* 每个组件系统代码目录结构：

  /CompDir/Impl/xxxMgrFactory.h

  /CompDir/Impl/xxxMgrFactory.cpp

  /CompDir/Impl/xxxGlobalFactory.h

  /CompDir/Impl/xxxGlobalFactory.cpp

  /CompDir/Impl/xxxMgr.h

  /CompDir/Impl/xxxMgr.cpp

  /CompDir/Impl/xxxGlobal.h

  /CompDir/Impl/xxxGlobal.cpp

  /CompDir/Interface/IXXXMgr.h

  /CompDir/Interface/IXXXGlobal.h

  /CompDir/XXX.h(此文件适用于包含给外部使用的相关文件，例如：interface下的接口文件，Impl下的factory文件等)

* 文件格式

  1. 普通的组件格式：

  ```c++
  /*
  * author: xxx
  * create time: xxx
  * Mail: xxx.com
  * Description: xxx
  */
  
  #pagma once
  
  #include <xxx.h>
  
  class XXX
  {
    
  };
  
  ```

  2. 公共组件格式：

  ```c++
  /*
  * author: xxx
  * create time: xxx
  * Mail: xxx.com
  * Description: xxx
  */
  
  #ifndef __CRYSTAL_NET_SERVICE_COMMON_DB_DB_H__
  #define __CRYSTAL_NET_SERVICE_COMMON_DB_DB_H__
  
  #pagma once
  
  #include <xxx.h>
  
  class XXX
  {
    
  };
  
  #endif
  
  ```

  公共需要加文件宏，文件宏的格式是从项目的根目录开始，到该文件名，前后缀都加__

* 关于内存池使用：不管是TL/MT的内存池 其最大能分配的内存是2MB，超过的会自动调用系统内存分配，所以如果有超过2MB内存的分配需求，可以使用MemoryAlloctor，来指定分配

* 注意TlsStack的大小默认大小是1MB，使用TlsStack是面向小对象的线程本地，且是线程生命周期内不会释放的，若需要分配内存请使用TlsMemoryPool来分配内存

