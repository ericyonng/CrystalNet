

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

## Languages（剔除第三方代码和生成的代码后）
| language | files | code | comment | blank | total |
| :--- | ---: | ---: | ---: | ---: | ---: |
| C++ | 1,078 | 105,580 | 46,550 | 28,171 | 180,301 |
| Markdown | 25 | 2,284 | 0 | 806 | 3,090 |
| Lua | 2 | 910 | 252 | 267 | 1,429 |
| TypeScript | 3 | 320 | 155 | 104 | 579 |
| JavaScript | 3 | 279 | 111 | 3 | 393 |
| Batch | 16 | 261 | 5 | 132 | 398 |
| Shell Script | 19 | 246 | 266 | 135 | 647 |
| YAML | 3 | 65 | 19 | 16 | 100 |


# 要求

要求具备C++11以上基础，以及一些C++20特性（module/c++20无栈协程/concept/requires, 了解c++20协程的一些基本关键字）

不为任何抽象付出不可接受的多余运行时性能损耗

# 构建状态

**Linux**: [![Build](https://app.travis-ci.com/ericyonng/CrystalNet.svg?branch=main)](http://travis-ci.com)

**Windows**: [![Build status](https://ci.appveyor.com/api/projects/status/5ikkrcubaax0vq84?svg=true)](https://ci.appveyor.com/project/ericyonng/crystalnet)

--------

# 快速构建

* 编译器要求：GCC11以上(推荐GCC13以上)/Clang15以上/vs2022以上

* Linux(scripts/builds/)

  * 支持Clang构建，且linux下默认使用clang构建，clang具有编译快生成的体积小且代码检查严格等特点

  * 构建: onekeybuild_linux.sh debug clean // 生成debug版本，在生成前clean表示清理原先生成的结果，默认不填写debug表示release版本或者显示指定release

  * 运行

    start.sh/stop.sh 运行或者关闭

    或者：linux_run.sh testsuit

* Windows(scripts/builds/)

  * 构建：winsolution_build.bat （选择vs2022生成solution），请使用指定版本vs编译
  * 编译可以使用自动化脚本编译:winms_onekey_build.bat 默认选择vs2022构建, 如果没安装vs2022需要安装
  * start.bat/stop.bat 运行或者关闭
  * export LD_LIBRARY_PATH=/usr/lib:$LD_LIBRARY_PATH 若提示mysqlclient找不到
  * **一键build**(脚本默认使用**vs2022,** 并**自动查询**vs2022安装路径的MSBuild.exe), :scripts/builds/winms_onekey_vs2022_build.bat
  * 关于其他版本vs的一键build，可以模仿当前一键build很小的改动即可(不同的vs有不同的版本号来识别)
  
* 导表工具

  * update_configs.bat/update_configs.sh

* 协议导出工具

  * protoc.bat/protoc.sh

----

# 简介

* 晶石框架是一个简易轻量的服务端框架，基于C++11及以上
* 旨在开发出一套跨平台，代码简洁可读性高，执行高效的框架
* 框架正在持续开发中，接受各种建议吐槽
* **Attention**:region 注释会尽量使用简洁的英文，CI可能会报错哈QVQ
* 多个重要组件移植于[LLBC](https://github.com/lailongwei/llbc "Source from lailongwei:llbc")、 [nlohmann/Json](https://github.com/nlohmann/json "json")、 [OpenSSL](https://github.com/openssl/openssl)， 再次感谢开源作者的贡献，开发过程中给予了我很大的帮助

--------

# 特性

- 采用iocp/epoll网络模型，跨windows/linux平台

- 支持protobuf 3.21.9， 定制了protobuf协议生成器，自动生成c++/c#的协议代码， 支持基于protobuf的ORM，对protobuf生成的ormdata的数据修改会被自动标脏

- 支持ipv4/ipv6 

- 支持session级别的packet限速

- 支持openssl md5/sha1/aes等加解密，签名验签

- 支持ECS设计，轻松设计大型复杂系统

- 支持任意对象对象池，内存池，可以很轻松的进行内存管理

- 支持tinyxml

- 支持对zip文档解压

- 支持对xlsx解析

- 强大的日志系统

- 丰富的组件支持

- 支持Mysql 8.0存储, 不需要自行建表建库,支持自动建库建表， 支持标脏持久化, 自动存库，数据类型与大小自适应

- 支持C++20 协程

- 集成Lua-5.4.7 静态库

- linux下支持so级别的热更(原理是在运行时使用dlopen重新加载so, 见PlugingMgr模块, 并采用数据和逻辑分离，逻辑写在so中, 数据定义在可执行程序中, so例子见TestServicePlugin模块)，见热更原理介绍

  



# 热更原理

* #### 组件构成

  * ShareLibraryLoader： 动态库加载组件, 作为组件嵌入到PluginMgr中, 用于加载指定的动态库

    LibraryHotfixMonitor: 动态库热更监控组件, 作为组件嵌入到Application中，用于监控是否有动态库需要热更, 

    PluginMgr: 用于管理插件集, 作为组件嵌入到MyTestService中

* 流程图

   ![image-20250205162034744](.\resource\img\hotfix.png)

* 插件集热更脚本

  ```
  # **.sh 可执行程序名 可执行程序编译版本 插件集HotfixKey 要热更的插件集so文件名
  sh ./scripts/hotfix/hotfix.sh testsuit debug TestPlugin libTestPlugin2.so
  ```

  

# 性能(见doc/压测/)

```
1. 16C32G机器，ini配置1个service, 网络层2个datatransfer poller, qps:26w/s, 客户端延迟:最大<200ms, 平均:150ms
2. 32c64G机器，ini配置6个service，网络层每个service1个datatransfer poller， 总的qps:52w/s, 客户端延迟:最大<200ms, 平均:150ms
```



# 注意

* windows下需要开启开发者模式(设置=>更新与安全=>开发者选项)，避免bat相关cmd失败
* 所有静态库都需要添加-fPIC编译, 避免无法识别到符号的编译问题
* 由于协程在初始化的时候会将参数，局部变量保存到协程上下文中, 参数如果是引用的也会以引用的形式保存，所以请务必注意参数和局部变量的生命周期问题,  建议使用智能指针
* 尽量避免提交二进制文件到git上，会导致仓库膨胀，二进制文件不是存储文件差异而是整个文件存下来，每次提交都会整个文件存下来会导致仓库体积膨胀
* windows下kernel不生成dll，而是把代码插入到exe工程中
* 使用ECS时千万警惕循环依赖：A <= B <= A <= ... 循环往复把系统资源耗光
* 注意如果基类的命名空间与派生类的命名空间不同，则不可以使用Comp<Ixxx>(), 不可以使用基类类型来获取对象，因为注册的时候是根据派生类全名来推导基类名并注册进去的
* 在模版函数中的lambda表达式中调用inline函数且该函数带有局部的static变量有可能导致变量多次初始化
* linux 内核版本3.9.0以上才有reuseport特性
* windows 下dll, exe拥有各自的堆栈空间，各自的全局变量都会各自初始化，所以注意全局变量以及static变量可能出现的重复初始化以及释放问题，建议为了只使用一个堆栈空间，windows下一个程序只有一个exe不依赖dll或者说依赖dll，但是不依赖dll中的全局变量
* thread_local 关键字 gcc 8.0以下有bug，相关连接:https://gcc.gnu.org/bugzilla/show_bug.cgi?id=60702
* 对于core，请不要使用try catch去处理异常，让系统输出core_dump的因为linux 的core_dump可以输出比较全面的崩溃时的数据，而如果仅仅在try catch去使用backtrace打印堆栈信息，是有缺陷的无法还原崩溃时的信息，导致解决错误的困难
* gcc 需要支持8.3及以上，建议切换到tencent os 3 因为默认支持gcc 8.3
* CXX11 ABI兼容性 请设置编译选项:D_GLIBCXX_USE_CXX11_ABI=1 放弃兼容gcc 5.x以下版本
* 由于github单文件100MB 限制的原因，对3rd/kernel 3rd/protobuf/lib 两个目录库超过100MB文件进行了partition拆分，编译时会自动合并
* 框架层不可以抛异常，只能使用错误码或者bool, 需不需要异常要上层决定，一般不建议抛异常即使是上层
* 数据库应该采用参数化查询来放置SQL注入攻击
* 关于关闭基于Application的引用, 可以在应用所在目录放入和应用同名后缀加上 .kill_进程id 的后缀其中进程id是当前允许程序的进程id,当使用工具执行关服时, 会创建这样的一个文件，app会监控到有这个文件就会执行关闭操作，也可以使用Ctrl + C进行常规的关闭，windows下不支持点击关闭窗口来关服，因为会导致数据丢失
* 多使用分支预测，可以提前加载预测为真的代码，提升性能
* DisableTruncateDB 线上的时候必须设置成1, 避免手残更改版本号导致清库
* Mysql windows下是大小写不敏感(表名和数据库名), 所以即使表名在建表时有大小写之分也会在建表后变成小写，为了兼容linux/windows的统一，数据库和表名都使用小写，字段名是区分大小写
* 日志开启控制台打印会比较大的影响到性能, 酌情开启，linux下应该禁止开启控制台打印
* windows下编码等请设置系统编码格式为UTF8, 以便在代码编辑时可以使用utf8对字符串正常编码
* 对模板, 内联需要保持敬畏之心，因为会导致代码膨胀, 编译时间无法容忍, 除非过度到c++20采用module, 否则避免过度设计，应该运用巧妙的方式弥补性能上的担忧, 以及在没必要关心性能的部分代码审慎使用模板与内联, 鼓励使用代码生成工具来替代
* 项目头文件中不可直接include kernel.h等比较大的库头文件，应该在实现文件中include
* 使用协程一定注意对象的生命周期
* 如果需要在so中使用库, 则需要在编译so的时候打开PROG_USE_SO的宏定义， 因为对象池或者内存池需要追踪这些对象以便定位内存泄漏问题

# 使用Lua

只需要include以下头文件即可

```
#include <lua/lua.hpp>
```

lua脚本的相对路径是相对于C++可执行程序的位置

# 简单使用

* 见testinst.cpp (测试用例)

* 简单使用内核

  * ```c++
    class LogFactory : public KERNEL_NS::ILogFactory
    {
    public:
        virtual KERNEL_NS::ILog *Create()
        {
            return new KERNEL_NS::LibLog();
        }
    };    
    
    LogFactory logFactory;
    KERNEL_NS::LibString programPath = KERNEL_NS::SystemUtil::GetCurProgRootPath();
    KERNEL_NS::LibString logIniPath;
    logIniPath = programPath + "/ini/";
    KERNEL_NS::SystemUtil::GetProgramPath(true, programPath);
    
    // 内核初始化
    Int32 err = KERNEL_NS::KernelUtil::Init(&logFactory, "LogCfg.ini", logIniPath.c_str());
    if(err != Status::Success)
    {
        CRYSTAL_TRACE("kernel init fail err:%d", err);
        return;
    }
    
    // 内核启动
    KERNEL_NS::KernelUtil::Start();
    
    // TODO:执行用户代码
    
    // 内核销毁
    KERNEL_NS::KernelUtil::Destroy();
    ```

* 复杂的app启动

  * ```c++
    class LogFactory : public KERNEL_NS::ILogFactory
    {
    public:
        virtual KERNEL_NS::ILog *Create()
        {
            return new KERNEL_NS::LibLog();
        }
    };    
    
    void TestInst::Run(int argc, char const *argv[])
    {
        LogFactory logFactory;
        KERNEL_NS::LibString programPath = KERNEL_NS::SystemUtil::GetCurProgRootPath();
        KERNEL_NS::LibString logIniPath;
        logIniPath = programPath + "/ini/";
        KERNEL_NS::SystemUtil::GetProgramPath(true, programPath);
    
        // 内核初始化
        Int32 err = KERNEL_NS::KernelUtil::Init(&logFactory, "LogCfg.ini", logIniPath.c_str());
        if(err != Status::Success)
        {
            CRYSTAL_TRACE("kernel init fail err:%d", err);
            return;
        }
    
        // 内核启动
        KERNEL_NS::KernelUtil::Start();
    
        // app 启动代码
        KERNEL_NS::SmartPtr<SERVICE_COMMON_NS::Application, KERNEL_NS::AutoDelMethods::CustomDelete> app = SERVICE_COMMON_NS::Application::New_Application();
        app.SetClosureDelegate([](void *ptr)
        {
            auto p = KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::Application>(ptr);
            SERVICE_COMMON_NS::Application::Delete_Application(p);
            ptr = NULL;
        });
    
        SERVICE_COMMON_NS::ApplicationHelper::Start(app.AsSelf(), SERVICE_NS::ServiceFactory::New_ServiceFactory(), argc, argv, "./ini/service.ini");
        
        // 内核销毁
        KERNEL_NS::KernelUtil::Destroy();
    }
    
    ```

    

## 可执行程序目录结构

* 可执行程序
* ini目录
* configs目录（放配置文件的数据目录）
* 其他dll，pdb, so等

## 配置辅助

* 线程数量 目前总数9个线程
  * 日志: 必须，3个线程（业务逻辑日志线程, 系统日志线程, 网络日志线程）
  * 业务逻辑: 必须，1个
  * 网络：必须，在必要情况下可以将监听的预数据传输的合并成1个poller，每个poller2个线程，包括一个监听poller，一个数据传输poller，目前已优化到只使用一个poller（监听和数据传输共用一个poller）
  * 主线程: 必须，1个（性能监控（收集网络，业务逻辑的性能指标）, 内存管理日志）
  * 中央收集器: 1个, 用于收集跨线程释放的内存，如果是tls创建的分配器会协助中转内存块到创建分配器的线程，由改线程释放block, 如果是MT模式下创建的分配器，则收集器直接释放该内存
  * 垃圾回收：必须，目前暂时1个，用于释放空闲的内存不至于因为内存池对象池内存占用的暴涨
* 机器基本配置要求
  * 4核8G, 或者8核16G，4Mbps带宽

* 

# 依赖

* 依赖libuuid1.0.3(默认自带，集成了静态库，在gcc 8.3以上编译)
* 依赖openssl 1.1.1q (默认自带 在gcc 8.3以上编译)
* 依赖protobuf 3.21.9版本（官方:v21.9版本）(默认自带 在gcc 8.3以上编译)
* 依赖premake5 (默认自带)
* windows下依赖DbgHelp(默认自带)
* Mysql8.0可选组件（需要环境安装mysql8.0服务端, 需要把libmysqlclient.so拷贝到公共路径或者和程序运行目录同级目录即可）

* * 

# 环境构建

* 安装cmake不低于v 3.25.0-rc4 （不是必须, 用于编译支持cmake的openssl，protobuf等库的时候用到）

* 安装gcc 8.3以上 必须，注意升级gcc同时需要升级glibc(c运行库)到更高版本（建议升级到glibc 2.28(Tencent os 3 默认运行库版本)，默认使用的话glibc 2.17）, 可以使用ldd --version来查看glibc版本,升级glibc需谨慎, 因为其他软件依赖它

* 安装jenkins（doc中安装步骤， ci等功能）（不是必须，用于持续集成）

  

# 编码规范
* 业务一般不推荐使用智能指针，使用智能指针等于把资源的释放职责交给了智能指针，导致生命周期不可控，使用RAII，来控制资源的生命周期，避免资源的生命周期无法控制，原则就是谁分配的资源应该由谁负责释放

* 设计上：不能没有设计也不能过度设计

* 非必要情况下尽量避免使用内存拷贝，可以通过设计达到写时复制效果,惰性拷贝,

* 多使用指针链表交换,避免使用节点全拷贝,提升并发效率

* 为了兼容lua所有的64位id的最高位建议置0

* 接口类规则:I+具体的类名， 例如：一个User类它的接口类命名：IUser

* 派生类重写的虚函数需要加上override约束,保证该接口的基类是有该虚函数

* 在不想让派生类重写的虚函数加上final约束，以避免派生类重写该方法

* 除了必要之外不要引入过多的语言特性，因为语言特性越多那么学习成本越高，对入门的门槛越高，这违背了我们要提供一个易上手框架的初衷

* 对象的分配与释放建议使用：CRYSTAL_MA_NEW/CRYSTAL_MA_DELETE (基于对象池的内存管理)，泛型：(CRYSTAL_MA_TEMPLATE_NEW_P1/CRYSTAL_MA_TEMPLATE_DELETE_P1,...)一方面基于对象池，一方面有利于内存泄漏的追踪

* 一块缓存的分配与释放建议使用：KernelAllocMemory/KernelFreeMemory 基于内存池

* __thread 用在gcc，clang等编译器上，初始化时初始化pod值，非pod类型，使用指针，并动态初始化,thread_local,  thead 不同：https://blog.csdn.net/weixin_43705457/article/details/106624781

* __thread关键字

  ```
  __thread是GCC内置的线程局部存储设施，存取效率可以和全局变量相比。__thread变量每一个线程有一份独立实体，各个线程的值互不干扰。可以用来修饰那些带有全局性且值可能变，但是又不值得用全局变量保护的变量。
  
         __thread使用规则：只能修饰POD类型(类似整型指针的标量，不带自定义的构造、拷贝、赋值、析构的类型，二进制内容可以任意复制memset,memcpy,且内容可以复原)，不能修饰class类型，因为无法自动调用构造函数和析构函数，可以用于修饰全局变量，函数内的静态变量，不能修饰函数的局部变量或者class的普通成员变量，且__thread变量值只能初始化为编译器常量(值在编译器就可以确定const int i=5,运行期常量是运行初始化后不再改变const int i=rand()).
  
   
  
  #include<iostream>
  #include<pthread.h>
  #include<unistd.h>
  using namespace std;
  const int i=5;
  __thread int var=i;//两种方式效果一样
  //__thread int var=5;//
  void* worker1(void* arg);
  void* worker2(void* arg);
  int main(){
      pthread_t pid1,pid2;
      //__thread int temp=5;
      static __thread  int temp=10;//修饰函数内的static变量
      pthread_create(&pid1,NULL,worker1,NULL);
      pthread_create(&pid2,NULL,worker2,NULL);
      pthread_join(pid1,NULL);
      pthread_join(pid2,NULL);
      cout<<temp<<endl;//输出10
      return 0;
  }
  void* worker1(void* arg){
      cout<<++var<<endl;//输出 6
  }
  void* worker2(void* arg){
      sleep(1);//等待线程1改变var值，验证是否影响线程2
      cout<<++var<<endl;//输出6
  }
  程序输出：
  6
  
  6         //可见__thread值线程间互不干扰
  
  10
  ————————————————
  版权声明：本文为CSDN博主「liuxuejiang158」的原创文章，遵循CC 4.0 BY-SA版权协议，转载请附上原文出处链接及本声明。
  原文链接：https://blog.csdn.net/liuxuejiang158blog/article/details/14100897
  
  ```

  

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

* 简化版本的Comp/Host:

  ```c++
  
  // 简化版 组件C
  class CompC : public KERNEL_NS::CompObject
  {
      POOL_CREATE_OBJ_DEFAULT_P1(CompObject, CompC);
  
  public:
      CompC()
      :KERNEL_NS::CompObject(KERNEL_NS::RttiUtil::GetTypeId<CompC>())
      {
          g_Log->Info(LOGFMT_OBJ_TAG("CompC constructor"));
      }
  
      ~CompC()
      {
          g_Log->Info(LOGFMT_OBJ_TAG("CompC destructor"));
          _Clear();
      }
  
      void Release()
      {
      	// 业务层可以使用性能更好的tls版本：
      	CompC::DeleteThreadLocal_CompC(this);
          // CompC::Delete_CompC(this);
      }
  
  protected:
      virtual Int32 _OnInit() override
      {
          g_Log->Info(LOGFMT_OBJ_TAG("%s on init"), ToString().c_str());
          return Status::Success;
      }
  
  private:
      void _Clear()
      {
          g_Log->Info(LOGFMT_OBJ_TAG("%s _Clear"), ToString().c_str());
      }
  
  private:
      KERNEL_NS::LibString _name = "CompC name field";
      
  };
  
  // 宿主3 简化版的Host
  class HostC : public KERNEL_NS::CompHostObject
  {
      POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, HostC);
  
  public:
      HostC()
      {
  
      }
  
      ~HostC()
      {
          _Clear();
      }
  
      void Release()
      {
      	// 业务层可以使用tls版本的对象池：
      	HostC::DeleteThreadLocal_HostC(this);
          // HostC::Delete_HostC(this);
      }
      
      virtual void OnRegisterComps();
  
      // 组件接口资源
  protected:
  
      // 在组件初始化前 必须重写
      virtual Int32 _OnHostInit() override
      {
          g_Log->Info(LOGFMT_OBJ_TAG("%s on host init."), ToString().c_str());
          return Status::Success;
      }
  
      // 组件启动之后 此时可以启动线程 必须重写
      virtual Int32 _OnHostStart() override
      {
          g_Log->Info(LOGFMT_OBJ_TAG("%s on host start."), ToString().c_str());
          return Status::Success;
      }
  
      // 在组件Close之后
      virtual void _OnHostClose()
      {
          g_Log->Info(LOGFMT_OBJ_TAG("%s on host close."), ToString().c_str());
      }
  
  private:
      void _Clear()
      {
          g_Log->Info(LOGFMT_OBJ_TAG("%s _Clear"), ToString().c_str());
      }
  
  private:
      KERNEL_NS::LibString _name = "HostC name field";
  };
  
  
  void HostC::OnRegisterComps()
  {
      // 注意循环依赖,若HostC中有HostA, HostA中也有HostC那么将导致死循环
      RegisterComp<CompCFactory>();
  }
  
  class CompCFactory : public KERNEL_NS::CompFactory
  {
  public:
  	// TLS版本对象池构建：
      static constexpr KERNEL_NS::_Build::TL _buildType{};
  	// static constexpr KERNEL_NS::_Build::MT _buildType{};

    virtual void Release()
    {
        OBJ_POOL_WRAP_DELETE(CompCFactory, KERNEL_NS::_Build::TL, this);
    }
      
      virtual KERNEL_NS::CompObject *Create() const
      {
           CREATE_CRYSTAL_COMP(comp, CompC);
           return comp;
      }
  };
  
  
  class HostCFactory : public KERNEL_NS::CompFactory
  {
  public:
  	// TLS版本对象池构建：
      static constexpr KERNEL_NS::_Build::TL _buildType{};
      // static constexpr KERNEL_NS::_Build::MT _buildType{};
     virtual void Release()
     {
        OBJ_POOL_WRAP_DELETE(HostCFactory, KERNEL_NS::_Build::TL, this);
     }
     
      virtual KERNEL_NS::CompObject *Create() const
      {
           CREATE_CRYSTAL_COMP(comp, HostC);
           return comp;
      }
  };
  
  ```

  

* 所有global系统可继承于IGlobalSys，简化开发，且需要对外提供接口类Ixxx, 所有的GlobalSys都需要注册到Service中称为Service的组件

  例子：

  接口类：

  ```c++
  SERVICE_BEGIN
  class ServiceSession;
  
  class ISessionMgr : public IGlobalSys
  {
      POOL_CREATE_OBJ_DEFAULT_P1(IGlobalSys, ISessionMgr);
  public:
      virtual ServiceSession *GetSession(UInt64 sessionId) = 0;
      virtual const ServiceSession *GetSession(UInt64 sessionId) const = 0;
  };
  
  SERVICE_END
  ```

  具体功能模块：

  ```c++
  
  class SessionMgr : public ISessionMgr
  {
      POOL_CREATE_OBJ_DEFAULT_P1(ISessionMgr, SessionMgr);
  
  public:
      SessionMgr();
      ~SessionMgr();
  	
  	// 用于释放
      void Release() override;
  
  public:
      virtual KERNEL_NS::LibString ToString() const override;
      virtual ServiceSession *GetSession(UInt64 sessionId) override;
      virtual const ServiceSession *GetSession(UInt64 sessionId) const override;
  	
  	// 初始化和关闭
  protected:
      Int32 _OnGlobalSysInit() override;
      void _OnGlobalSysClose() override;
  
  	// 其他接口
  private:
      void _OnSessionWillCreated(KERNEL_NS::LibEvent *ev);
      void _OnSessionDestroy(KERNEL_NS::LibEvent *ev);
  
      void _Clear();
  
      void _RegisterEvents();
      void _UnRegisterEvents();
  
      /*
      * 会话
      */
     ServiceSession *_CreateSession(const ServiceSessionInfo &sessionInfo);
     void _DestroySession(ServiceSession *session);
     void _MakeSessionDict(ServiceSession *session);
  
  private:
  
      /* 会话 */
      std::map<UInt64, ServiceSession *> _sessionIdRefSession;
  
      /* 事件 */
      KERNEL_NS::ListenerStub _sessionWillCreatedStub;
  };
  ```

* 消息订阅只能在Global系统上，IPlayerSys禁止进行消息订阅，避免一个玩家来消息所有玩家都收到这种情况