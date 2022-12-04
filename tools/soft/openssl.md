openssl 1.1.1q

linux debug:

```shell
./config --prefix=/root/openssl111q --openssldir=/root/openssl111qdir --debug no-shared

make && make install
```

linux release:

```
./config --prefix=/root/openssl111q --openssldir=/root/openssl111qdir --release no-shared

make && make install
```



windows下安装

```
1.安装perl
2.安装nasm
3.按照INSTALL中windows部分生成openssl的库
默认安装的是release版本
安装debug版本：
VC-WIN64A 换成：debug-VC-WIN64A
```

debug:

```
perl Configure --debug VC-WIN64A no-shared --prefix=install_dir_xxx --openssldir=xxx 

cd C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\amd64
# 安装必要的环境变量
vcvars64.bat 
nmake
nmake install
```

release:

```
perl Configure --release VC-WIN64A no-shared --prefix=install_dir_xxx --openssldir=xxx 

cd C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\amd64
# 安装必要的环境变量
vcvars64.bat 
nmake
nmake install
```

