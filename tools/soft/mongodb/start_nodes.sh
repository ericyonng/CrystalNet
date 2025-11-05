#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 启动节点

SCRIPT_PATH=$1
REPLISET_INSTALL_PATH=$2
NODES_ARRAY=$3
PRIMARY_IP=$4
PRIMARY_PORT=$5
# :config/mongod
DB_TYPE=$6
# db名
DB_NAME=$6
# keyfile
KEYFILE_PATH=$7

if [ -z "${NODES_ARRAY}" ]; then
    echo "NODES_ARRAY empty."
    exit 1
fi

DB_INDEX=1
for index in "${!NODES_ARRAY[@]}"; do
    # ip file 一行的数据: DATA ip
    elem="${NODES_ARRAY[$index]}"
    fields=($(echo "${elem}" | awk '{print $1, $2, $3}'))
    node_type="${fields[0]}"
    ip="${fields[1]}"
    node_port="${fields[2]}"
    echo "第 $index 个 IP 地址: elem:${elem}, $ip, ${node_type}, ${node_port}"
    
    # db_name
    DB_INDEX=$(($DB_INDEX + $index))
    FINAL_DB_NAME=${DB_NAME}_${DB_TYPE}_${DB_INDEX}

    # 是不是本地地址
    if [ ${ip} = "127.0.0.1" ] || [ ${ip} = ${LOCAL_IP} ]; then
      . ${SCRIPT_PATH}/create_mongodb_inst.sh ${REPLISET_INSTALL_PATH}/${FINAL_DB_NAME} ${node_port} "${RS_NAME}_${DB_TYPE}" ${KEYFILE_PATH} || {
            echo "创建db1实例失败！" >&2
            exit 1
        }  
    else
        
    fi
    
done

DB2_PORT=$((${PRIMARY_PORT} + 1))
DB3_PORT=$((${PRIMARY_PORT} + 2))
. ${SCRIPT_PATH}/create_mongodb_inst.sh ${REPLISET_INSTALL_PATH}/db1 ${DB1_PORT} "${RS_NAME}" ${KEYFILE_PATH} || {
    echo "创建db1实例失败！" >&2
    exit 1
}
echo "创建db1实例成功"

. ${SCRIPT_PATH}/create_mongodb_inst.sh ${REPLISET_INSTALL_PATH}/db2 ${DB2_PORT} "${RS_NAME}" ${KEYFILE_PATH} || {
    echo "创建db2实例失败！" >&2
    exit 1
}
echo "创建db2实例成功"

. ${SCRIPT_PATH}/create_mongodb_inst.sh ${REPLISET_INSTALL_PATH}/db3 ${DB3_PORT} "${RS_NAME}" ${KEYFILE_PATH} || {
    echo "创建db3实例失败！" >&2
    exit 1
}
echo "创建db3实例成功"

echo "启动 3 个实例完成, 端口:${DB1_PORT},${DB2_PORT},${DB3_PORT} ..."

echo "开始从主节点添加两个从节点..."

# 添加从节点2
mongosh --host 127.0.0.1:${DB1_PORT} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "rs.add({_id: 1, host: \"${PRIMARY_IP}:${DB2_PORT}\", priority: 1, votes: 1})" || {
    echo "错误：初始化复制集失败" >&2
    exit 1
}

sleep 2

# 添加从节点3
mongosh --host 127.0.0.1:${DB1_PORT} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "rs.add({_id: 2, host: \"${PRIMARY_IP}:${DB3_PORT}\", priority: 1, hidden: false, secondaryDelaySecs: 0})" || {
    echo "错误：初始化复制集失败" >&2
    exit 1
}
echo "add secondary success..."

sleep 2

echo "复制集配置:"
mongosh --host 127.0.0.1:${DB1_PORT} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "rs.conf()"

echo "复制集状态:"
mongosh --host 127.0.0.1:${DB1_PORT} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "rs.status()"

echo "create mongo repliset success PRIMARY: ${PRIMARY_IP}:${DB1_PORT}, path:${REPLISET_INSTALL_PATH}, enjoy!"