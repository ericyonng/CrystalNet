#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 初始化主节点
# sh ./init_primary.sh

# 当前脚本路径
SCRIPT_PATH="$(cd $(dirname $0); pwd)"

# 复制集路径
TARGET_USER=$1
TARGET_PWD=$2
REPLISET_INSTALL_PATH=$3
PRIMARY_IP=$4
PRIMARY_PORT=$5
DB_NAME=$6
# 复制集名 config/数据集的rs_name必须唯一
RS_NAME=$7
# keyfile路径
KEYFILE_PATH=$8
# sharding角色 mongos填写:""
SHARDING_CLUSTER_ROLE=$9
# 是否mongos
IS_MONGOS=$10
# 如果是mongos需要configDB
MONGOS_CONFIG_ADDR="$11"

if [ -z "${TARGET_USER}" ] || [ -z "${TARGET_PWD}" ]; then
    echo "TARGET_USER:${TARGET_USER} TARGET_PWD:${TARGET_PWD} lack of pwd info"
    exit 1
fi

# 校验参数0
if [ -e "${REPLISET_INSTALL_PATH}" ]; then
    echo "REPLISET_INSTALL_PATH:${REPLISET_INSTALL_PATH} exists"
else
    echo "REPLISET_INSTALL_PATH:${REPLISET_INSTALL_PATH} not exists"
    exit 1
fi

if [ -z "${PRIMARY_IP}" ] || [ -z "${PRIMARY_PORT}" ]; then
    echo "PRIMARY_IP:${PRIMARY_IP} PRIMARY_PORT:${PRIMARY_PORT} lack of primary info"
    exit 1
fi

if [ -z "${DB_NAME}" ]; then
    echo "DB_NAME:${DB_NAME} is empty"
    exit 1
fi

if [ -z "${RS_NAME}" ]; then
    echo "RS_NAME:${RS_NAME} is empty"
    exit 1
fi

if [ -e "${KEYFILE_PATH}" ]; then
    echo "KEYFILE_PATH:${KEYFILE_PATH} is exists"
else
    echo "KEYFILE_PATH:${KEYFILE_PATH} is not exists"
    exit 1
fi

echo "TARGET_USER:${TARGET_USER}"
echo "TARGET_PWD:${TARGET_PWD}"
echo "REPLISET_INSTALL_PATH:${REPLISET_INSTALL_PATH}"
echo "PRIMARY_IP:${PRIMARY_IP}"
echo "PRIMARY_PORT:${PRIMARY_PORT}"
echo "DB_NAME:${DB_NAME}"
echo "RS_NAME:${RS_NAME}"
echo "KEYFILE_PATH:${KEYFILE_PATH}"
echo "SHARDING_CLUSTER_ROLE:${SHARDING_CLUSTER_ROLE}"
echo "IS_MONGOS:${IS_MONGOS}"
echo "MONGOS_CONFIG_ADDR:${MONGOS_CONFIG_ADDR}"

# 先启动并创建用户, 密码与授权
sh ${SCRIPT_PATH}/create_mongodb_inst.sh ${REPLISET_INSTALL_PATH}/${DB_NAME} ${PRIMARY_PORT} "${RS_NAME}" ${KEYFILE_PATH} "${SHARDING_CLUSTER_ROLE}" "${IS_MONGOS}" "${MONGOS_CONFIG_ADDR}" "no_auth" || {
    echo "创建主节点db1实例失败！" >&2
    exit 1
}
echo "创建主节点 ${DB_NAME} 实例成功"

# 必须先初始化复制集否则会报不是主节点的错
mongosh --host 127.0.0.1:${PRIMARY_PORT} --eval "rs.initiate({_id: \"${RS_NAME}\", members: [{_id: 0, host: \"${PRIMARY_IP}:${PRIMARY_PORT}\", priority: 2}], settings: {heartbeatIntervalMillis: 2000, electionTimeoutMillis: 10000}})" || {
    echo "错误：初始化复制集失败 DB_NAME:${DB_NAME}" >&2
    exit 1
}

# 等待选举完成
echo "wait ${DB_NAME} 选举完成..."
sleep 5

echo "创建权限用户:${TARGET_USER}..."

# 创建用户并赋予权限
mongosh 127.0.0.1:${PRIMARY_PORT}/admin --eval "db.createUser({user: \"${TARGET_USER}\", pwd: \"${TARGET_PWD}\", roles:[{role: \"userAdminAnyDatabase\", db: \"admin\"}, {role: \"readWriteAnyDatabase\", db: \"admin\"}, {role: \"clusterAdmin\", db: \"admin\"}]})" || {
    echo "错误：创建用户并赋予权限:${REPLISET_INSTALL_PATH}/db1！" >&2
    exit 1
}

echo "创建用户:${TARGET_USER} 并授权成功..."

# 关闭mongo
if [ -z "${IS_MONGOS}" ]; then
    mongod -f ${REPLISET_INSTALL_PATH}/${DB_NAME}/mongod.conf --shutdown || {
        echo "错误：关闭 MongoDB 失败，请检查配置或日志 MONGOD_CONF:${REPLISET_INSTALL_PATH}/${DB_NAME}/mongod.conf！" >&2
        exit 1
    }
        
    sleep 5
    echo "关闭 mongod 成功..."
else
    # 关闭所有 mongos 进程的完整脚本
    PID_LIST="$(ps -aux | grep mongos | sed '/grep/d' | sed '/init_primary/d' | sed 's/^[^ ]* //' | sed 's/^ *//' | sed 's/ .*$//')"

    for pid in $PID_LIST
    do
        kill -2 ${pid}
        echo "wait mongos pid:${pid} close..."
        while [ -n "$(ps -p $pid | sed '1d')" ]
        do
            echo "wait mongos pid:${pid} close..."
            sleep 1
        done

        echo "mongos pid:${pid}, has closed."
    done

    echo "关闭 mongos 成功..."
fi


