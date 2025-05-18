#!/usr/bin/env bash

# 创建conf文件
# 目标db目录
TARGET_DB_PATH=$1
# 端口号
TARGET_PORT=$2
# 复制集名
REPL_SET_NAME=$3
# 配置文件名
MONGOD_CONF_FILE_NAME=$4

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
if [ -z "${MONGOD_CONF_FILE_NAME}" ]; then
    echo "mongod conf name is empty!!!"
    exit 1
fi

MONGOD_CONF=${TARGET_DB_PATH}/${MONGOD_CONF_FILE_NAME}

echo 'systemLog:' > ${MONGOD_CONF}
echo '    destination: file' >> ${MONGOD_CONF}
echo "    path: ${TARGET_DB_PATH}/mongod.log" >> ${MONGOD_CONF}
echo "    logAppend: true" >> ${MONGOD_CONF}
echo -e "\n" >> ${MONGOD_CONF}
echo -e "storage:" >> ${MONGOD_CONF}
echo "    dbPath: ${TARGET_DB_PATH}" >> ${MONGOD_CONF}
echo "    journal:" >> ${MONGOD_CONF}
echo "        enabled: true" >> ${MONGOD_CONF}
echo -e "\n" >> ${MONGOD_CONF}
echo "net:" >> ${MONGOD_CONF}
echo "    bindIp: 0.0.0.0" >> ${MONGOD_CONF}
echo "    port: ${TARGET_PORT}" >> ${MONGOD_CONF}
echo -e "\n" >> ${MONGOD_CONF}
echo "replication:" >> ${MONGOD_CONF}
echo "    replSetName: ${REPL_SET_NAME}" >> ${MONGOD_CONF}
echo -e "\n" >> ${MONGOD_CONF}
echo "processManagement:" >> ${MONGOD_CONF}
echo "    fork: true" >> ${MONGOD_CONF}

echo "mongod conf:${MONGOD_CONF} create success..."
