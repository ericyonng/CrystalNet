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



# 注意

* public key 有pkc#1 pkc#等填充格式

*  项目需要，对c++代码中的几个用poenssl库实现的rsa加解密函数进行了整合。
    rsa加密的public key格式有多种，常见的有两种，一种密钥头为‘-----BEGIN RSA PUBLIC KEY-----’,一种开头为‘-----BEGIN PUBLIC KEY-----’，二者分别对应rsa的PKCS#1和PKCS#8格式。
    使用openssl库加载rsa的公钥时，使用的函数也不同。以字符串公钥为例，对PKCS#1格式的密钥加载使用PEM_read_bio_RSAPublicKey()函数，对PKCS#8格式公钥的加载使用PEM_read_bio_RSA_PUBKEY()函数。
    对于不那么熟悉rsa的同学，使用时注意区分者，可以减少不必要的麻烦。

  string Base64Encode_std(const string& encodeStr){
  //ignore
  }
  string rsaEncode(const unsigned char * in, int len, const char *pubKey){
  	BIO *bio=NULL;
  	RSA *rsa = NULL;
  	char szOut[512] = {0};
  	string pkcs1_header = "-----BEGIN RSA PUBLIC KEY-----";
  	string pkcs8_header = "-----BEGIN PUNLIC KEY-----";
  	bio = BIO_bew(BIO_s_mem());
  	if (NULL == bio) return "";
  	BIO_puts(bio,pubKey);
  	if( 0 == strncmp(pubKey,pkcs8_header.c_str(),pkcs8.size())){
  		rsa = PEM_read_bio_RSA_PUBKEY(bio,NULL,NULL,NULL);
  	}
  	else if(0 == strncmp(pubKey,pkcs1_header.c_str(),pkcs1.size())){
  		rsa = PEM_read_bio_RSAPublicKey(bio,NULL,NULL,NULL);
  	}
  	if(NULL == rsa) return "";
  	int ret = RSA_public_encrypt(len,in,(unsigned char*)szOut,rsa,RSA_PKCS1_PADDING);
  	if (ret < 0 ) return "";
  	string encrypt_data = string(szOut,ret);
  	string resultStr = Base64Encode_std(encrypt_data);
  	if(bio) BIO_free(bio);
  	if(rsa) RSA_free(rsa);
  	return resultStr;
  }

  上面代码中加了一点儿判断，区分了一下pkcs#8和pkcs#1的格式，完全的情况还有一些其他的格式。StackOverflow上有个关于这个的回答，写的比较详细，有兴趣的可以参考。
  https://stackoverflow.com/questions/18039401/how-can-i-transform-between-the-two-styles-of-public-key-format-one-begin-rsa/29707204#29707204
  要了解rsa加密的算法原理，可以参考这个知乎的文章
  https://zhuanlan.zhihu.com/p/45317622
  ————————————————
  版权声明：本文为CSDN博主「freesonWANG」的原创文章，遵循CC 4.0 BY-SA版权协议，转载请附上原文出处链接及本声明。
  原文链接：https://blog.csdn.net/freesonWANG/article/details/87717361