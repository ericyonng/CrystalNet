#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# mongos配置


# 创建conf文件
# 目标db目录
TARGET_DB_PATH=$1
# 端口号
TARGET_PORT=$2
# 复制集名
REPL_SET_NAME=$3
# 配置文件名
MONGOS_CONF_FILE_NAME=$4
# configserver地址
CONFIG_SVR_ADDRS=$5

if [ -z "${TARGET_PORT}" ]; then
    echo "please specify a port!!!"
    exit 1
fi

if [ -z "${REPL_SET_NAME}" ]; then
    echo "please specify a REPL_SET_NAME!!!"
    exit 1
fi

# 创建目录
if [ -z "${TARGET_DB_PATH}" ]; then
    echo "please specify a target db path!!!"
    exit 1
fi

# 配置名
if [ -z "${MONGOS_CONF_FILE_NAME}" ]; then
    echo "mongod conf name is empty!!!"
    exit 1
fi

# 配置服务器地址
if [ -z "${CONFIG_SVR_ADDRS}" ]; then
    echo "mongos config server addr is empty!!!"
    exit 1  
fi

MONGOS_CONF=${TARGET_DB_PATH}/${MONGOS_CONF_FILE_NAME}

echo 'systemLog:' > ${MONGOS_CONF}
echo '    destination: file' >> ${MONGOS_CONF}
echo "    path: ${TARGET_DB_PATH}/mongos.log" >> ${MONGOS_CONF}
echo "    logAppend: true" >> ${MONGOS_CONF}
echo -e "\n" >> ${MONGOS_CONF}
echo "net:" >> ${MONGOS_CONF}
echo "    bindIp: 0.0.0.0" >> ${MONGOS_CONF}
echo "    port: ${TARGET_PORT}" >> ${MONGOS_CONF}
echo -e "\n" >> ${MONGOS_CONF}
echo "processManagement:" >> ${MONGOS_CONF}
echo "    fork: true" >> ${MONGOS_CONF}
echo -e "\n" >> ${MONGOS_CONF}
echo "sharding:" >> ${MONGOS_CONF}
echo "    configDB: ${REPL_SET_NAME}/${CONFIG_SVR_ADDRS}" >> ${MONGOS_CONF}

echo "mongos conf:${MONGOS_CONF} create success..."