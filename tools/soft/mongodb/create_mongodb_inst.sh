#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 创建mongodb 实例


# 当前脚本路径
SCRIPT_PATH="$1"

# 变量
# 目标db目录
LOCAL_TARGET_DB_PATH=$2
# 指定ip
LOCAL_TARGET_IP=$3
# 端口号
LOCAL_TARGET_PORT=$4
# 复制集名
LOCAL_REPL_SET_NAME=$5
# keyfile 绝对路径
LOCAL_KEYFILE_PATH=$6
# sharding 分片角色
LOCAL_SHARDING_CLUSTER_ROLE=$7
# 是否mongos
LOCAL_IS_MONGOS=$8
# mongos configsvr地址
LOCAL_MONGOS_CONFIG_ADDR=$9
# 是否不需要启动验证
LOCAL_IS_NO_AUTH=${10}

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
    # 检查目录是否包含数据文件（排除 mongod.conf 和 keyfile）
    DATA_FILES=$(ls ${LOCAL_TARGET_DB_PATH}/ 2>/dev/null | grep -vE '^(mongod\.conf|mongod\.log|keyfile)$' | wc -l)
    if [ "${DATA_FILES}" -gt 0 ]; then
        echo "警告: 数据目录 ${LOCAL_TARGET_DB_PATH} 包含 ${DATA_FILES} 个数据文件"
        echo "这些文件可能是之前 mongod 遗留的，将被复用"
    fi
else
    echo "create dir:${LOCAL_TARGET_DB_PATH} ..."
    mkdir -p ${LOCAL_TARGET_DB_PATH}
fi

if [ -n "${LOCAL_IS_MONGOS}" ]; then
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
    sh ${SCRIPT_PATH}/create_mongod_conf.sh ${LOCAL_TARGET_DB_PATH} ${LOCAL_TARGET_IP} ${LOCAL_TARGET_PORT} ${LOCAL_REPL_SET_NAME} mongod.conf

    # 拷贝keyfile,并设置文件掩码, 否则会启动失败
    TARGET_KEYFILE_PATH=${LOCAL_TARGET_DB_PATH}/keyfile
    echo "准备复制 keyfile: ${LOCAL_KEYFILE_PATH} => ${TARGET_KEYFILE_PATH}"
    if [ ! -f "${LOCAL_KEYFILE_PATH}" ]; then
        echo "错误: 源 keyfile 不存在: ${LOCAL_KEYFILE_PATH}" >&2
        exit 1
    fi
    cp -rf ${LOCAL_KEYFILE_PATH} ${TARGET_KEYFILE_PATH}
    chmod 600 ${TARGET_KEYFILE_PATH}
    echo "keyfile 复制完成: ${TARGET_KEYFILE_PATH}, 权限: $(ls -la ${TARGET_KEYFILE_PATH} | awk '{print $1}')"

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

    # 检查端口是否被占用，如果被占用则关闭已有进程
    echo "检查端口 ${LOCAL_TARGET_PORT} 是否被占用..."
    PORT_PID=$(lsof -ti:${LOCAL_TARGET_PORT} 2>/dev/null || echo "")
    if [ -n "${PORT_PID}" ]; then
        echo "警告: 端口 ${LOCAL_TARGET_PORT} 被进程 ${PORT_PID} 占用"
        echo "尝试关闭进程 ${PORT_PID}..."
        kill -15 ${PORT_PID} 2>/dev/null
        sleep 2
        # 检查是否已关闭
        PORT_PID=$(lsof -ti:${LOCAL_TARGET_PORT} 2>/dev/null || echo "")
        if [ -n "${PORT_PID}" ]; then
            echo "强制关闭进程 ${PORT_PID}..."
            kill -9 ${PORT_PID} 2>/dev/null
            sleep 1
        fi
        # 再检查一次
        PORT_PID=$(lsof -ti:${LOCAL_TARGET_PORT} 2>/dev/null || echo "")
        if [ -n "${PORT_PID}" ]; then
            echo "错误: 无法关闭占用端口 ${LOCAL_TARGET_PORT} 的进程 ${PORT_PID}" >&2
            echo "请手动处理后重试" >&2
            exit 1
        fi
        echo "端口 ${LOCAL_TARGET_PORT} 已释放"
    else
        echo "端口 ${LOCAL_TARGET_PORT} 可用"
    fi

    # 显示最终的 mongod.conf 内容
    echo "========== mongod.conf 内容 =========="
    cat ${MONGOD_CONF}
    echo "======================================"

    # 启动mongodb 实例, 创建用户并赋予权限
    echo "正在启动 mongod..."
    mongod -f ${MONGOD_CONF}
    MONGOD_EXIT_CODE=$?

    if [ ${MONGOD_EXIT_CODE} -ne 0 ]; then
        echo "错误：mongod 启动失败，退出码: ${MONGOD_EXIT_CODE}" >&2
        echo "========== mongod.log 错误信息 ==========" >&2
        if [ -f "${LOCAL_TARGET_DB_PATH}/mongod.log" ]; then
            tail -50 "${LOCAL_TARGET_DB_PATH}/mongod.log" >&2
        else
            echo "mongod.log 文件不存在" >&2
        fi
        echo "=========================================" >&2
        echo "MONGOD_CONF 内容:" >&2
        cat ${MONGOD_CONF} >&2
        exit 1
    fi
fi


echo "start mongod LOCAL_SHARDING_CLUSTER_ROLE:${LOCAL_SHARDING_CLUSTER_ROLE} success, mongo conf:${MONGOD_CONF}"





