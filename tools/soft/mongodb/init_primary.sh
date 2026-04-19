#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 初始化主节点
# sh ./init_primary.sh

# 当前脚本路径
SCRIPT_PATH="$1"

# 复制集路径
TARGET_USER=$2
TARGET_PWD=$3
LOCAL_REPLISET_INSTALL_PATH=$4
LOCAL_PRIMARY_IP=$5
LOCAL_PRIMARY_PORT=$6
LOCAL_DB_NAME=$7
# 复制集名 config/数据集的rs_name必须唯一
LOCAL_RS_NAME=$8
# keyfile路径
LOCAL_KEYFILE_PATH=$9
# sharding角色 mongos填写:""
LOCAL_SHARDING_CLUSTER_ROLE=$10
# 是否mongos
LOCAL_IS_MONGOS="$11"
# 如果是mongos需要configDB
LOCAL_MONGOS_CONFIG_ADDR="$12"

if [ -z "${TARGET_USER}" ] || [ -z "${TARGET_PWD}" ]; then
    echo "TARGET_USER:${TARGET_USER} TARGET_PWD:${TARGET_PWD} lack of pwd info"
    exit 1
fi

# 校验参数0
if [ -e "${LOCAL_REPLISET_INSTALL_PATH}" ]; then
    echo "LOCAL_REPLISET_INSTALL_PATH:${LOCAL_REPLISET_INSTALL_PATH} exists"
else
    echo "LOCAL_REPLISET_INSTALL_PATH:${LOCAL_REPLISET_INSTALL_PATH} not exists will create"
    mkdir -p ${LOCAL_REPLISET_INSTALL_PATH}
fi

if [ -z "${LOCAL_PRIMARY_IP}" ] || [ -z "${LOCAL_PRIMARY_PORT}" ]; then
    echo "LOCAL_PRIMARY_IP:${LOCAL_PRIMARY_IP} LOCAL_PRIMARY_PORT:${LOCAL_PRIMARY_PORT} lack of primary info"
    exit 1
fi

if [ -z "${LOCAL_DB_NAME}" ]; then
    echo "LOCAL_DB_NAME:${LOCAL_DB_NAME} is empty"
    exit 1
fi

if [ -z "${LOCAL_RS_NAME}" ]; then
    echo "LOCAL_RS_NAME:${LOCAL_RS_NAME} is empty"
    exit 1
fi

if [ -e "${LOCAL_KEYFILE_PATH}" ]; then
    echo "LOCAL_KEYFILE_PATH:${LOCAL_KEYFILE_PATH} is exists"
else
    echo "LOCAL_KEYFILE_PATH:${LOCAL_KEYFILE_PATH} is not exists"
    exit 1
fi

echo "TARGET_USER:${TARGET_USER}"
echo "TARGET_PWD:${TARGET_PWD}"
echo "LOCAL_REPLISET_INSTALL_PATH:${LOCAL_REPLISET_INSTALL_PATH}"
echo "LOCAL_PRIMARY_IP:${LOCAL_PRIMARY_IP}"
echo "LOCAL_PRIMARY_PORT:${LOCAL_PRIMARY_PORT}"
echo "LOCAL_DB_NAME:${LOCAL_DB_NAME}"
echo "LOCAL_RS_NAME:${LOCAL_RS_NAME}"
echo "LOCAL_KEYFILE_PATH:${LOCAL_KEYFILE_PATH}"
echo "LOCAL_SHARDING_CLUSTER_ROLE:${LOCAL_SHARDING_CLUSTER_ROLE}"
echo "LOCAL_IS_MONGOS:${LOCAL_IS_MONGOS}"
echo "LOCAL_MONGOS_CONFIG_ADDR:${LOCAL_MONGOS_CONFIG_ADDR}"

# 先启动并创建用户, 密码与授权
sh ${SCRIPT_PATH}/create_mongodb_inst.sh ${LOCAL_REPLISET_INSTALL_PATH}/${LOCAL_DB_NAME} ${LOCAL_PRIMARY_PORT} "${LOCAL_RS_NAME}" ${LOCAL_KEYFILE_PATH} "${LOCAL_SHARDING_CLUSTER_ROLE}" "${LOCAL_IS_MONGOS}" "${LOCAL_MONGOS_CONFIG_ADDR}" "no_auth" || {
    echo "创建主节点db1实例失败！" >&2
    exit 1
}
echo "创建主节点 ${LOCAL_DB_NAME} 实例成功"

# 必须先初始化复制集否则会报不是主节点的错
mongosh --host 127.0.0.1:${LOCAL_PRIMARY_PORT} --eval "rs.initiate({_id: \"${LOCAL_RS_NAME}\", members: [{_id: 0, host: \"${LOCAL_PRIMARY_IP}:${LOCAL_PRIMARY_PORT}\", priority: 2}], settings: {heartbeatIntervalMillis: 2000, electionTimeoutMillis: 10000}})" || {
    echo "错误：初始化复制集失败 LOCAL_DB_NAME:${LOCAL_DB_NAME}" >&2
    exit 1
}

# 等待选举完成
echo "wait ${LOCAL_DB_NAME} 选举完成..."
sleep 5

echo "创建权限用户:${TARGET_USER}..."

# 创建用户并赋予权限
mongosh 127.0.0.1:${LOCAL_PRIMARY_PORT}/admin --eval "db.createUser({user: \"${TARGET_USER}\", pwd: \"${TARGET_PWD}\", roles:[{role: \"userAdminAnyDatabase\", db: \"admin\"}, {role: \"readWriteAnyDatabase\", db: \"admin\"}, {role: \"clusterAdmin\", db: \"admin\"}]})" || {
    echo "错误：创建用户并赋予权限:${LOCAL_REPLISET_INSTALL_PATH}/db1！" >&2
    exit 1
}

echo "创建用户:${TARGET_USER} 并授权成功..."

# 关闭mongo
if [ -z "${LOCAL_IS_MONGOS}" ]; then
    mongod -f ${LOCAL_REPLISET_INSTALL_PATH}/${LOCAL_DB_NAME}/mongod.conf --shutdown || {
        echo "错误：关闭 MongoDB 失败，请检查配置或日志 MONGOD_CONF:${LOCAL_REPLISET_INSTALL_PATH}/${LOCAL_DB_NAME}/mongod.conf！" >&2
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


