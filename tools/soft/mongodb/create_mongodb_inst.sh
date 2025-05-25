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
# 是否不需要启动验证
IS_NO_AUTH=$5

echo "TARGET_DB_PATH:${TARGET_DB_PATH}"
echo "TARGET_PORT:${TARGET_PORT}"
echo "REPL_SET_NAME:${REPL_SET_NAME}"
echo "KEYFILE_PATH:${KEYFILE_PATH}"
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

# 启动mongodb 实例, 创建用户并赋予权限
mongod -f ${MONGOD_CONF} || {
    echo "错误：启动 MongoDB 失败，请检查配置或日志 MONGOD_CONF:${MONGOD_CONF}！" >&2
    exit 1
}

echo "start mongod success, mongo conf:${MONGOD_CONF}"





