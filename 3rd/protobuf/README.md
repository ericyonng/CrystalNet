项目protocolbuf简述：
---
protocol buffer用的版本为3.21.9版本

---
#### 目录说明：
> include: 包含3.21.9头文件
> libs: c++ libs
> 原包：3.21.9完整包

#### 编译

debug:

```
mkdir build
cd build

cmake ../cmake -DCMAKE_BUILD_TYPE:STRING=DEBUG -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=/root/pb  -Dprotobuf_BUILD_TESTS=off -Dprotobuf_WITH_ZLIB=OFF

make && make install;
```

release:

```
mkdir build
cd build

cmake ../cmake -DCMAKE_BUILD_TYPE:STRING=RELEASE -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=/root/pb  -Dprotobuf_BUILD_TESTS=off -Dprotobuf_WITH_ZLIB=OFF

make && make install;
```

