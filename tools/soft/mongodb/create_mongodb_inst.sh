#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 创建mongodb 实例


# 当前脚本路径
SCRIPT_PATH="$(cd $(dirname $0); pwd)"

# 变量
# 目标db目录
LOCAL_TARGET_DB_PATH=$1
# 端口号
LOCAL_TARGET_PORT=$2
# 复制集名
LOCAL_REPL_SET_NAME=$3
# keyfile 绝对路径
LOCAL_KEYFILE_PATH=$4
# sharding 分片角色
LOCAL_SHARDING_CLUSTER_ROLE=$5
# 是否mongos
LOCAL_IS_MONGOS=$6
# mongos configsvr地址
LOCAL_MONGOS_CONFIG_ADDR=$7
# 是否不需要启动验证
LOCAL_IS_NO_AUTH=$8

echo "LOCAL_TARGET_DB_PATH:${LOCAL_TARGET_DB_PATH}"
echo "LOCAL_TARGET_PORT:${LOCAL_TARGET_PORT}"
echo "LOCAL_REPL_SET_NAME:${LOCAL_REPL_SET_NAME}"
echo "LOCAL_KEYFILE_PATH:${LOCAL_KEYFILE_PATH}"
echo "LOCAL_SHARDING_CLUSTER_ROLE:${LOCAL_SHARDING_CLUSTER_ROLE}"
echo "LOCAL_IS_MONGOS:${LOCAL_IS_MONGOS}"
echo "LOCAL_MONGOS_CONFIG_ADDR:${LOCAL_MONGOS_CONFIG_ADDR}"
echo "LOCAL_IS_NO_AUTH:${LOCAL_IS_NO_AUTH}"

if [ -z "${LOCAL_KEYFILE_PATH}" ]; then
    echo "please specify a keyfile!!!"
    exit 1
fi

if [ -z "${LOCAL_TARGET_PORT}" ]; then
    echo "please specify a port!!!"
    exit 1
fi

if [ -z "${LOCAL_REPL_SET_NAME}" ]; then
    echo "please specify a LOCAL_REPL_SET_NAME!!!"
    exit 1
fi

# 创建目录
if [ -e "${LOCAL_TARGET_DB_PATH}" ]; then
    echo "exists dir:${LOCAL_TARGET_DB_PATH} ..."
else
    echo "create dir:${LOCAL_TARGET_DB_PATH} ..."
    mkdir ${LOCAL_TARGET_DB_PATH}
fi

if [ -n = "${LOCAL_IS_MONGOS}" ]; then
    # 创建conf文件
    MONGOS_CONF=${LOCAL_TARGET_DB_PATH}/mongos.conf
    sh ${SCRIPT_PATH}/create_mongos_conf.sh ${LOCAL_TARGET_DB_PATH} ${LOCAL_TARGET_PORT} ${LOCAL_REPL_SET_NAME} mongos.conf ${LOCAL_MONGOS_CONFIG_ADDR}

    # 拷贝keyfile,并设置文件掩码, 否则会启动失败
    TARGET_KEYFILE_PATH=${LOCAL_TARGET_DB_PATH}/keyfile
    cp -rf ${LOCAL_KEYFILE_PATH} ${TARGET_KEYFILE_PATH}
    chmod 600 ${TARGET_KEYFILE_PATH}

    echo "keyfile:${TARGET_KEYFILE_PATH}"

    # 启用认证
    if [ -z "${LOCAL_IS_NO_AUTH}" ]; then
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
    MONGOD_CONF=${LOCAL_TARGET_DB_PATH}/mongod.conf
    sh ${SCRIPT_PATH}/create_mongod_conf.sh ${LOCAL_TARGET_DB_PATH} ${LOCAL_TARGET_PORT} ${LOCAL_REPL_SET_NAME} mongod.conf

    # 拷贝keyfile,并设置文件掩码, 否则会启动失败
    TARGET_KEYFILE_PATH=${LOCAL_TARGET_DB_PATH}/keyfile
    cp -rf ${LOCAL_KEYFILE_PATH} ${TARGET_KEYFILE_PATH}
    chmod 600 ${TARGET_KEYFILE_PATH}

    echo "keyfile:${TARGET_KEYFILE_PATH}"

    # 启用认证
    if [ -z "${LOCAL_IS_NO_AUTH}" ]; then
        echo "enable security..."
        echo "security:" >> ${MONGOD_CONF}
        echo "    authorization: enabled" >> ${MONGOD_CONF}
        echo "    keyFile: ${TARGET_KEYFILE_PATH}" >> ${MONGOD_CONF}
    else
        echo "disable security..."
    fi

    # 启动分片
    if [ -n "${LOCAL_SHARDING_CLUSTER_ROLE}" ]; then
        echo "enable sharding LOCAL_SHARDING_CLUSTER_ROLE:${LOCAL_SHARDING_CLUSTER_ROLE}..."
        echo "sharding:" >> ${MONGOD_CONF}
        echo "    clusterRole: ${LOCAL_SHARDING_CLUSTER_ROLE}" >> ${MONGOD_CONF}
    fi

    # 启动mongodb 实例, 创建用户并赋予权限
    mongod -f ${MONGOD_CONF} || {
        echo "错误：启动 MongoDB 失败， LOCAL_SHARDING_CLUSTER_ROLE:${LOCAL_SHARDING_CLUSTER_ROLE} 请检查配置或日志 MONGOD_CONF:${MONGOD_CONF}！" >&2
        cat ${MONGOD_CONF} >&2
        exit 1
    }
fi


echo "start mongod LOCAL_SHARDING_CLUSTER_ROLE:${LOCAL_SHARDING_CLUSTER_ROLE} success, mongo conf:${MONGOD_CONF}"





