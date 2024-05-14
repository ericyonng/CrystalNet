* 源码：https://github.com/curl/curl
* windows
  * 先阅读winbuild/README.md, 需要在curl下创建deps, 并创建include, lib两个目录
  * 并将openssl的对应的include相关代码拷贝到include下，以及相关lib拷贝到lib下
    * somedirectory\
       |_curl-src
       | |_winbuild
       |
       |_deps
         |_ lib
         |_ include
         |_ bin
  * 再执行buildconf.bat
  * 随后使用cmkae打开curl，并创建_winbuild目录
  * 在配置cmake时勾选BUILD_STATIC_CURL, BUILD_STATIC_LIBS, CURL_ENABLE_SSL,CURL_USE_OPENSSL, LIB_EAY_DEBUG选择libcrypto.lib对应的路径，LIB_EAY_RELEASE选择librypto.lib对应的路径，
  * 添加OPENSSL_CRYPTO_LIBRARY路径该路径需要指向具体的libcrypto.lib所在目录
  * 添加OPENSSL_CRYPTO_DIR 路径执行librypto.lib所在路径
  * SSL_EAY_DEBUG/SSL_EAY_RELEASE 指向libssl.lib路径
  * 去掉idn的勾选
  * 点击Configure，Generate
  * lib生成在_winbuild/lib下
  * 使用时将include目录内拷贝走
  * 将生成的libcurl拷贝走，
  * 在使用静态库的时候必须#define CURL_STATICLIB(在#include<curl/curl.h>之前，也可以在premake中统一定义这个宏)
* linux
  * 需要在/root/下创建openssl目录
  * 并且在openssl中创建lib和include目录
  * 再把openssl的库.a文件复制到lib目录中，将openssl的头文件（openssl整个目录拷贝到include中）
  * 再执行./configure --with-openssl=/root/openssl/ --enable-static --disable-shared --prefix=/root/curlli
  * 再执行make && make install
  * 再到curllib目录中，其中会生成：bin, include, lib, share几个目录