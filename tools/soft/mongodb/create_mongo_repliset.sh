#!/usr/bin/env bash
# 创建mongodb复制集
# create_mongo_repliset.sh 复制集名 主节点端口号 用户名 密码 复制集安装目标路径

# 当前脚本路径
SCRIPT_PATH="$(cd $(dirname $0); pwd)"

echo "SCRIPT_PATH:${SCRIPT_PATH}"

# 变量
RS_NAME=$1
PRIMARY_PORT=$2
TARGET_USER=$3
TARGET_PWD=$4
REPLISET_INSTALL_PATH=$5

# 先切换到执行目录
TEMP_DIR=${SCRIPT_PATH}/TEMP_DIR
rm -rf ${TEMP_DIR}
mkdir ${TEMP_DIR}

# host name 必须正常
# hostname
HOST_NAME_RESULT=$(hostname -f 2>/dev/null)

# host name 必须正常
if [ -z "${HOST_NAME_RESULT}" ]; then
    echo "host name is null please check!!!"
    exit 1
fi
echo "HOST_NAME_RESULT:${HOST_NAME_RESULT}"

if [ -z "${RS_NAME}" ]; then
    echo "repliset name is empty please check!!!"
    exit 1
fi

if [ -z "${PRIMARY_PORT}" ]; then
    echo "primary port is empty please check!!!"
    exit 1
fi

if [ -z "${REPLISET_INSTALL_PATH}" ]; then
    echo "repliset install path is empty please check!!!"
    exit 1
fi

if [ -z "${TARGET_USER}" ]; then
    echo "user name is empty please check!!!"
    exit 1
fi

if [ -z "${TARGET_PWD}" ]; then
    echo "user pwd is empty please check!!!"
    exit 1
fi

# 检查 OpenSSL 是否已安装
if ! command -v openssl &> /dev/null; then
    echo "当前环境未安装openssl正在安装 OpenSSL..."
    if sudo yum install openssl -y &> /dev/null; then
        echo "OpenSSL 安装成功！"
    else
        echo "安装失败，请手动执行：sudo yum install openssl -y"
        exit 1
    fi
fi

# 创建目录
if [ -e "${REPLISET_INSTALL_PATH}" ]; then
    echo "will remove exists dir:${REPLISET_INSTALL_PATH} ..."
    rm -rf ${REPLISET_INSTALL_PATH}
fi
mkdir ${REPLISET_INSTALL_PATH}

# 生成keyfile
KEYFILE_PATH=${TEMP_DIR}/keyfile
openssl rand -base64 756 > ${KEYFILE_PATH}

echo "创建keyfile 成功 ${KEYFILE_PATH}"

# 先启动并创建用户, 密码与授权
DB1_PORT=${PRIMARY_PORT}
. ${SCRIPT_PATH}/create_mongodb_inst.sh ${REPLISET_INSTALL_PATH}/db1 ${DB1_PORT} "${RS_NAME}" ${KEYFILE_PATH} "no_auth" || {
    echo "创建主节点db1实例失败！" >&2
    exit 1
}
echo "创建主节点db1实例成功"

# 初始化复制集
mongosh --host 127.0.0.1:${DB1_PORT} --eval 'rs.initiate()' || {
    echo "错误：初始化复制集失败" >&2
    exit 1
}

echo "initiate rs success..."

# 创建用户并赋予权限
mongosh 127.0.0.1:${PRIMARY_PORT}/admin --eval "db.createUser({user: \"${TARGET_USER}\", pwd: \"${TARGET_PWD}\", roles:[{role: \"userAdminAnyDatabase\", db: \"admin\"}, {role: \"readWriteAnyDatabase\", db: \"admin\"}, {role: \"clusterAdmin\", db: \"admin\"}]})" || {
    echo "错误：创建用户并赋予权限:${REPLISET_INSTALL_PATH}/db1！" >&2
    exit 1
}

echo "创建用户:${TARGET_USER} 并授权成功..."

# 关闭mongod
mongod -f ${REPLISET_INSTALL_PATH}/db1/mongod.conf --shutdown || {
    echo "错误：关闭 MongoDB 失败，请检查配置或日志 MONGOD_CONF:${REPLISET_INSTALL_PATH}/db1/mongod.conf！" >&2
    exit 1
}

sleep 1
echo "关闭 mongod 成功..."

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

mongosh --host 127.0.0.1:${DB1_PORT} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "rs.add(\"${HOST_NAME_RESULT}:${DB2_PORT}\")" || {
    echo "主节点:${DB1_PORT} 添加从节点失败 ${HOST_NAME_RESULT}:${DB2_PORT}" >&2
    exit 1
}
sleep 2
echo "add secondary success port:${DB2_PORT}..."

mongosh --host 127.0.0.1:${DB1_PORT} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "rs.add(\"${HOST_NAME_RESULT}:${DB3_PORT}\")" || {
    echo "主节点:${DB1_PORT} 添加从节点失败 ${HOST_NAME_RESULT}:${DB3_PORT}" >&2
    exit 1
}
echo "add secondary success port:${DB3_PORT}..."

mongosh --host 127.0.0.1:${DB1_PORT} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "rs.status()"

echo "create mongo repliset success PRIMARY PORT:${DB1_PORT}, path:${REPLISET_INSTALL_PATH}, enjoy!"
