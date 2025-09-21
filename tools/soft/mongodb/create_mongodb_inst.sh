#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 创建mongodb 实例


# 当前脚本路径
SCRIPT_PATH="$(cd $(dirname $0); pwd)"

# 变量
# 目标db目录
TARGET_DB_PATH=$1
# 端口号
TARGET_PORT=$2
# 复制集名
REPL_SET_NAME=$3
# keyfile 绝对路径
KEYFILE_PATH=$4
# sharding 分片角色
SHARDING_CLUSTER_ROLE=$5
# 是否mongos
IS_MONGOS=$6
# mongos configsvr地址
MONGOS_CONFIG_ADDR=$7
# 是否不需要启动验证
IS_NO_AUTH=$8

echo "TARGET_DB_PATH:${TARGET_DB_PATH}"
echo "TARGET_PORT:${TARGET_PORT}"
echo "REPL_SET_NAME:${REPL_SET_NAME}"
echo "KEYFILE_PATH:${KEYFILE_PATH}"
echo "SHARDING_CLUSTER_ROLE:${SHARDING_CLUSTER_ROLE}"
echo "IS_MONGOS:${IS_MONGOS}"
echo "MONGOS_CONFIG_ADDR:${MONGOS_CONFIG_ADDR}"
echo "IS_NO_AUTH:${IS_NO_AUTH}"

if [ -z "${KEYFILE_PATH}" ]; then
    echo "please specify a keyfile!!!"
    exit 1
fi

if [ -z "${TARGET_PORT}" ]; then
    echo "please specify a port!!!"
    exit 1
fi

if [ -z "${REPL_SET_NAME}" ]; then
    echo "please specify a REPL_SET_NAME!!!"
    exit 1
fi

# 创建目录
if [ -e "${TARGET_DB_PATH}" ]; then
    echo "exists dir:${TARGET_DB_PATH} ..."
else
    echo "create dir:${TARGET_DB_PATH} ..."
    mkdir ${TARGET_DB_PATH}
fi

if [ -n = "${IS_MONGOS}" ]; then
    # 创建conf文件
    MONGOS_CONF=${TARGET_DB_PATH}/mongos.conf
    sh ${SCRIPT_PATH}/create_mongos_conf.sh ${TARGET_DB_PATH} ${TARGET_PORT} ${REPL_SET_NAME} mongos.conf ${MONGOS_CONFIG_ADDR}

    # 拷贝keyfile,并设置文件掩码, 否则会启动失败
    TARGET_KEYFILE_PATH=${TARGET_DB_PATH}/keyfile
    cp -rf ${KEYFILE_PATH} ${TARGET_KEYFILE_PATH}
    chmod 600 ${TARGET_KEYFILE_PATH}

    echo "keyfile:${TARGET_KEYFILE_PATH}"

    # 启用认证
    if [ -z "${IS_NO_AUTH}" ]; then
        echo "enable security..."
        echo "security:" >> ${MONGOS_CONF}
        echo "    authorization: enabled" >> ${MONGOS_CONF}
        echo "    keyFile: ${TARGET_KEYFILE_PATH}" >> ${MONGOS_CONF}
    else
        echo "disable security..."
    fi

    # 启动mongodb 实例, 创建用户并赋予权限
    mongos -f ${MONGOS_CONF} || {
        echo "错误：启动 mongos 失败，请检查配置或日志 MONGOS_CONF:${MONGOS_CONF}！" >&2
        cat ${MONGOS_CONF} >&2
        exit 1
    }
else
    # 创建conf文件
    MONGOD_CONF=${TARGET_DB_PATH}/mongod.conf
    sh ${SCRIPT_PATH}/create_mongod_conf.sh ${TARGET_DB_PATH} ${TARGET_PORT} ${REPL_SET_NAME} mongod.conf

    # 拷贝keyfile,并设置文件掩码, 否则会启动失败
    TARGET_KEYFILE_PATH=${TARGET_DB_PATH}/keyfile
    cp -rf ${KEYFILE_PATH} ${TARGET_KEYFILE_PATH}
    chmod 600 ${TARGET_KEYFILE_PATH}

    echo "keyfile:${TARGET_KEYFILE_PATH}"

    # 启用认证
    if [ -z "${IS_NO_AUTH}" ]; then
        echo "enable security..."
        echo "security:" >> ${MONGOD_CONF}
        echo "    authorization: enabled" >> ${MONGOD_CONF}
        echo "    keyFile: ${TARGET_KEYFILE_PATH}" >> ${MONGOD_CONF}
    else
        echo "disable security..."
    fi

    # 启动分片
    if [ -n "${SHARDING_CLUSTER_ROLE}" ]; then
        echo "enable sharding SHARDING_CLUSTER_ROLE:${SHARDING_CLUSTER_ROLE}..."
        echo "sharding:" >> ${MONGOD_CONF}
        echo "    clusterRole: ${SHARDING_CLUSTER_ROLE}" >> ${MONGOD_CONF}
    fi

    # 启动mongodb 实例, 创建用户并赋予权限
    mongod -f ${MONGOD_CONF} || {
        echo "错误：启动 MongoDB 失败， SHARDING_CLUSTER_ROLE:${SHARDING_CLUSTER_ROLE} 请检查配置或日志 MONGOD_CONF:${MONGOD_CONF}！" >&2
        cat ${MONGOD_CONF} >&2
        exit 1
    }
fi


echo "start mongod SHARDING_CLUSTER_ROLE:${SHARDING_CLUSTER_ROLE} success, mongo conf:${MONGOD_CONF}"





