#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 初始化主节点
# sh ./init_primary.sh

# 当前脚本路径
SCRIPT_PATH="$(cd $(dirname $0); pwd)"

# 复制集路径
TARGET_USER=$1
TARGET_PWD=$2
LOCAL_REPLISET_INSTALL_PATH=$3
LOCAL_PRIMARY_IP=$4
LOCAL_PRIMARY_PORT=$5
LOCAL_DB_NAME=$6
# 复制集名 config/数据集的rs_name必须唯一
LOCAL_RS_NAME=$7
# keyfile路径
LOCAL_KEYFILE_PATH=$8
# 如果是mongos需要configDB
LOCAL_MONGOS_CONFIG_ADDR="$9"

if [ -z "${TARGET_USER}" ] || [ -z "${TARGET_PWD}" ]; then
    echo "TARGET_USER:${TARGET_USER} TARGET_PWD:${TARGET_PWD} lack of pwd info"
    exit 1
fi

# 校验参数0
if [ -e "${LOCAL_REPLISET_INSTALL_PATH}" ]; then
    echo "LOCAL_REPLISET_INSTALL_PATH:${LOCAL_REPLISET_INSTALL_PATH} exists"
else
    echo "LOCAL_REPLISET_INSTALL_PATH:${LOCAL_REPLISET_INSTALL_PATH} not exists"
    exit 1
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
echo "LOCAL_MONGOS_CONFIG_ADDR:${LOCAL_MONGOS_CONFIG_ADDR}"

# 先启动并创建用户, 密码与授权
sh ${SCRIPT_PATH}/create_mongos_inst.sh ${LOCAL_REPLISET_INSTALL_PATH}/${LOCAL_DB_NAME} ${LOCAL_PRIMARY_PORT} "${LOCAL_RS_NAME}" ${LOCAL_KEYFILE_PATH} "${LOCAL_MONGOS_CONFIG_ADDR}" "no_auth" || {
    echo "创建主节点db1实例失败！" >&2
    exit 1
}
echo "创建mongos ${LOCAL_DB_NAME} 实例成功"


echo "创建权限用户:${TARGET_USER}..."

# 创建用户并赋予权限
mongosh 127.0.0.1:${LOCAL_PRIMARY_PORT}/admin --eval "db.createUser({user: \"${TARGET_USER}\", pwd: \"${TARGET_PWD}\", roles:[{role: \"userAdminAnyDatabase\", db: \"admin\"}, {role: \"readWriteAnyDatabase\", db: \"admin\"}, {role: \"clusterAdmin\", db: \"admin\"}]})" || {
    echo "错误：创建用户并赋予权限:${LOCAL_REPLISET_INSTALL_PATH}/db1！" >&2
    exit 1
}

echo "创建用户:${TARGET_USER} 并授权成功..."

# 关闭mongo

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

echo "关闭 mongos 成功, init_mongos 成功..."


