#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 创建mongodb复制集
# create_mongo_repliset.sh 复制集名 [复制集公网ip列表(带端口号)] 用户名 密码 复制集安装目标路径

# 当前脚本路径
SCRIPT_PATH="$(cd $(dirname $0); pwd)"

echo "SCRIPT_PATH:${SCRIPT_PATH}"

# 是否可以联外网
check_internet() {
    HAS_WGET=1
    if ! command -v wget &> /dev/null; then
        echo "当前环境未安装wget正在安装 wget..."
        if sudo yum install wget -y &> /dev/null; then
            echo "wget 安装成功！"
        else
            echo "wget 安装失败！"
            HAS_WGET=0
        fi
    fi

    if [ ${HAS_WGET} = 1 ]; then
        if wget -q --spider --timeout=3 https://www.baidu.com; then
            echo "wget 外网连接正常"
            return 0
        else
            echo "wget 连接 https://www.baidu.com 失败"
        fi
    fi

    HAS_PING=1
    if ! command -v ping &> /dev/null; then
        echo "当前环境未安装ping正在安装 ping..."
        if sudo yum install iputils -y &> /dev/null; then
            echo "ping 安装成功！"
        else
            echo "ping 安装失败！"
            HAS_PING=0
        fi
    fi
    if [ ${HAS_PING} = 1 ]; then
        if ping -c 1 -W 2 8.8.8.8 &> /dev/null; then
            echo "ping 8.8.8.8 正常"
            return 0
        else
            echo "ping 8.8.8.8 失败"
        fi
    fi

    HAS_NS_LOOKUP=1
    if ! command -v nslookup &> /dev/null; then
        echo "当前环境未安装nslookup正在安装 nslookup..."
        if sudo yum install bind-utils -y &> /dev/null; then
            echo "nslookup 安装成功！"
        else
            echo "nslookup 安装失败！"
            HAS_NS_LOOKUP=0
        fi
    fi
    if [ ${HAS_NS_LOOKUP} = 1 ]; then
        if nslookup baidu.com &> /dev/null; then
            echo "nslookup baidu.com 正常"
            return 0
        else
            echo "nslookup baidu.com 失败"
        fi
    fi

    return 1
}

# 变量
RS_NAME=$1
IP_ADDR=$2
# PRIMARY_PORT
TARGET_USER=$3
TARGET_PWD=$4
REPLISET_INSTALL_PATH=$6

# 获取本机器的公网ip
if check_internet; then
MACHINE_PUBLIC_IP=$(wget -qO- https://api.ipify.org)
    echo "外网连接正常 MACHINE_PUBLIC_IP:${MACHINE_PUBLIC_IP}"
else
MACHINE_PUBLIC_IP="127.0.0.1"
    echo "无法连接外网 MACHINE_PUBLIC_IP:${MACHINE_PUBLIC_IP}"
fi

echo "RS_NAME:${RS_NAME}, IP_ADDR:${IP_ADDR}, TARGET_USER:${TARGET_USER}, TARGET_PWD:${TARGET_PWD}, REPLISET_INSTALL_PATH:${REPLISET_INSTALL_PATH}, MACHINE_PUBLIC_IP:${MACHINE_PUBLIC_IP}"

if [ -z "${IP_ADDR}" ]; then
    IP_ADDR="[127.0.0.1:28017]"
fi

PRIMARY_IP="127.0.0.1"
PRIMARY_PORT=28017

# 遍历 IP_ADDR 获取primary信息
IP_INFO_LIST=$(echo "${IP_ADDR}" | sed 's/[][]//g')
echo "IP_INFO_LIST:${IP_INFO_LIST}"
# 将节点列表按逗号分割成数组
IFS=',' read -ra nodes <<< "${IP_INFO_LIST}"
# 遍历每个节点并提取IP和端口
LOOP_IP_LIST_INDEX=1
for node in "${nodes[@]}"; do
    IFS=: read ip port <<< "$node"
    if [ ${LOOP_IP_LIST_INDEX} = 1 ]; then
        PRIMARY_IP=${ip}
        PRIMARY_PORT=${port}
    fi
    echo "IP: $ip, Port: $port"
done

echo "PRIMARY_IP:${PRIMARY_IP}, ${PRIMARY_PORT}"

# 先切换到执行目录
TEMP_DIR=${SCRIPT_PATH}/TEMP_DIR
rm -rf ${TEMP_DIR}
mkdir ${TEMP_DIR}

# host name 必须正常
# hostname
# HOST_NAME_RESULT=$(hostname -f 2>/dev/null)

echo "IP_ADDR:${IP_ADDR}"

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

# 必须先初始化复制集否则会报不是主节点的错
mongosh --host 127.0.0.1:${DB1_PORT} --eval "rs.initiate({_id: \"${RS_NAME}\", members: [{_id: 0, host: \"${IP_ADDR}:${DB1_PORT}\", priority: 2}], settings: {heartbeatIntervalMillis: 2000, electionTimeoutMillis: 10000}})" || {
    echo "错误：初始化复制集失败" >&2
    exit 1
}

# 等待选举完成
echo "wait 选举完成..."
sleep 5

echo "创建权限用户:${TARGET_USER}..."

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

sleep 5
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

# 添加从节点2
mongosh --host 127.0.0.1:${DB1_PORT} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "rs.add({_id: 1, host: \"${IP_ADDR}:${DB2_PORT}\", priority: 1, votes: 1})" || {
    echo "错误：初始化复制集失败" >&2
    exit 1
}

sleep 2

# 添加从节点3
mongosh --host 127.0.0.1:${DB1_PORT} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "rs.add({_id: 2, host: \"${IP_ADDR}:${DB3_PORT}\", priority: 1, hidden: false, secondaryDelaySecs: 0})" || {
    echo "错误：初始化复制集失败" >&2
    exit 1
}
echo "add secondary success..."

sleep 2

echo "复制集配置:"
mongosh --host 127.0.0.1:${DB1_PORT} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "rs.conf()"

echo "复制集状态:"
mongosh --host 127.0.0.1:${DB1_PORT} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "rs.status()"

echo "create mongo repliset success PRIMARY: ${IP_ADDR}:${DB1_PORT}, path:${REPLISET_INSTALL_PATH}, enjoy!"
