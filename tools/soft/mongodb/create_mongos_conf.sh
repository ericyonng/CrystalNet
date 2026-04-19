#!/usr/bin/env bash
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
 # @Date: 2025-07-27 22:43:47
 # @Author: Eric Yonng
 # @Description: 
###

# @author EricYonng<120453674@qq.com>
# mongos配置


# 创建conf文件
# 目标db目录
LOCAL_TARGET_DB_PATH=$1
# 端口号
LOCAL_TARGET_PORT=$2
# 复制集名
LOCAL_REPL_SET_NAME=$3
# 配置文件名
LOCAL_MONGOS_CONF_FILE_NAME=$4
# configserver地址
LOCAL_CONFIG_SVR_ADDRS=$5

if [ -z "${LOCAL_TARGET_PORT}" ]; then
    echo "please specify a port!!!"
    exit 1
fi

if [ -z "${LOCAL_REPL_SET_NAME}" ]; then
    echo "please specify a LOCAL_REPL_SET_NAME!!!"
    exit 1
fi

# 创建目录
if [ -z "${LOCAL_TARGET_DB_PATH}" ]; then
    echo "please specify a target db path!!!"
    exit 1
fi

# 配置名
if [ -z "${LOCAL_MONGOS_CONF_FILE_NAME}" ]; then
    echo "mongod conf name is empty!!!"
    exit 1
fi

# 配置服务器地址
if [ -z "${LOCAL_CONFIG_SVR_ADDRS}" ]; then
    echo "mongos config server addr is empty!!!"
    exit 1  
fi

MONGOS_CONF=${LOCAL_TARGET_DB_PATH}/${LOCAL_MONGOS_CONF_FILE_NAME}

echo 'systemLog:' > ${MONGOS_CONF}
echo '    destination: file' >> ${MONGOS_CONF}
echo "    path: ${LOCAL_TARGET_DB_PATH}/mongos.log" >> ${MONGOS_CONF}
echo "    logAppend: true" >> ${MONGOS_CONF}
echo -e "\n" >> ${MONGOS_CONF}
echo "net:" >> ${MONGOS_CONF}
echo "    bindIp: 0.0.0.0" >> ${MONGOS_CONF}
echo "    port: ${LOCAL_TARGET_PORT}" >> ${MONGOS_CONF}
echo -e "\n" >> ${MONGOS_CONF}
echo "processManagement:" >> ${MONGOS_CONF}
echo "    fork: true" >> ${MONGOS_CONF}
echo -e "\n" >> ${MONGOS_CONF}
echo "sharding:" >> ${MONGOS_CONF}
echo "    configDB: ${LOCAL_REPL_SET_NAME}/${LOCAL_CONFIG_SVR_ADDRS}" >> ${MONGOS_CONF}

echo "mongos conf:${MONGOS_CONF} create success..."