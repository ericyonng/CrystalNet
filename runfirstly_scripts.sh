# @file linuxmakefile_build.sh
###
 #  MIT License
 #  
 #  Copyright (c) 2020 ericyonng<120453674@qq.com>
 #  
 #  Permission is hereby granted, free of charge, to any person obtaining a copy
 #  of this software and associated documentation files (the "Software"), to deal
 #  in the Software without restriction, including without limitation the rights
 #  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 #  copies of the Software, and to permit persons to whom the Software is
 #  furnished to do so, subject to the following conditions:
 #  
 #  The above copyright notice and this permission notice shall be included in all
 #  copies or substantial portions of the Software.
 #  
 #  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 #  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 #  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 #  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 #  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 #  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 #  SOFTWARE.
 # 
 # @Date: 2022-09-03 04:44:16
 # @Author: Eric Yonng
 # @Description: 
###

# @author EricYonng<120453674@qq.com>
# @brief 3rd scripts
#!/usr/bin/env bash

# 版本
# VER="debug"
# if [ -n "$1" ]
# then
# VER="$1"
#    if [ $VER != "debug" ]
#    then
#	    $VER="release"
#	if
# if

# 路径
SCRIPT_PATH="$(cd $(dirname $0); pwd)"

# install szrz
# yum -y install lrzsz
# 依赖uuid
# sudo yum -y install libuuid-devel

# install uuid soft package
# yum -y install openssl

# sudo rm -rf /usr/local/openssl
# sudo mkdir /usr/local/openssl
# sudo rm -rf /usr/include/openssl
# sudo rm -rf /usr/lib64/libcrypto.so
# sudo rm -rf /usr/lib64/libssl.so
# sudo rm -rf /usr/lib64/libcrypto.so.1.0.0
# sudo rm -rf /usr/lib64/libssl.so.1.0.0
# 
# sudo cp -fR $SCRIPT_PATH/3rd/openssl/linux/* /usr/local/openssl/
# sudo ln -sv /usr/local/openssl/include/openssl /usr/include/openssl
# sudo ln -sv /usr/local/openssl/lib/libcrypto.so /usr/lib64/libcrypto.so
# sudo ln -sv /usr/local/openssl/lib/libssl.so /usr/lib64/libssl.so
# sudo ln -sv /usr/local/openssl/lib/libcrypto.so.1.0.0 /usr/lib64/libcrypto.so.1.0.0
# sudo ln -sv /usr/local/openssl/lib/libssl.so.1.0.0 /usr/lib64/libssl.so.1.0.0

