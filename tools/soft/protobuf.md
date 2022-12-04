debug:

```
mkdir build
cd build

cmake ../cmake -DCMAKE_BUILD_TYPE:STRING=DEBUG -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=/root/pb  -Dprotobuf_BUILD_TESTS=off

make && make install;
```

release:

```
mkdir build
cd build

cmake ../cmake -DCMAKE_BUILD_TYPE:STRING=RELEASE -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=/root/pb  -Dprotobuf_BUILD_TESTS=off

make && make install;
```

