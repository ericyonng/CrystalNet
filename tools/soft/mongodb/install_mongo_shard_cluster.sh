#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 创建mongo 分片集群
# IP_LIST_FILE: ip 节点类型 端口: config 127.0.0.1 27010
# INSTALL_PATH: 安装mongodb的程序目录
# DATA_PATH: 安装mongodb的数据库目录
# TARGET_USER: 用户名
# TARGET_PWD: 密码
# DB_NAME: db名
# RS_NAME: 复制集名

# 当前脚本路径
SCRIPT_PATH="$(cd $(dirname $0); pwd)"

# IP列表文件
IP_LIST_FILE=$1
# 安装mongodb的目录
INSTALL_PATH=$2
# 数据库数据路径
DATA_PATH=$3
# 用户名
TARGET_USER=$4
# 密码
TARGET_PWD=$5
# 数据库名(由业务决定)
DB_NAME=$6
# 复制集名
RS_NAME=$7

# 校验参数
if [ -e ${IP_LIST_FILE} ]; then
    echo "IP_LIST_FILE:${IP_LIST_FILE} exist"
else
    echo "IP_LIST_FILE:${IP_LIST_FILE} not exist please check"
fi

if [ -z "${INSTALL_PATH}" ]; then
    echo "INSTALL_PATH is empty please check!!!"
    exit 1
fi

if [ -z "${DATA_PATH}" ]; then
    echo "DATA_PATH is empty please check!!!"
    exit 1
fi

if [ -z "${TARGET_USER}" ]; then
    echo "TARGET_USER is empty please check!!!"
    exit 1
fi
if [ -z "${TARGET_PWD}" ]; then
    echo "TARGET_PWD is empty please check!!!"
    exit 1
fi

if [ -z "${DB_NAME}" ]; then
    echo "DB_NAME is empty please check!!!"
    exit 1
fi

if [ -z "${RS_NAME}" ]; then
    echo "RS_NAME is empty please check!!!"
    exit 1
fi

# 加载公共
. ${SCRIPT_PATH}/common/common_define.sh
. ${SCRIPT_PATH}/common/funcs.sh


# 从文件读取ip列表
IP_LIST_ARRAY=()
read_file_to_array ${IP_LIST_FILE} IP_LIST_ARRAY || {
    echo "错误： read_file_to_array fail IP_LIST_FILE: ${IP_LIST_FILE} 失败" >&2
    exit 1
}
# ip列表打印
for i in "${!IP_LIST_ARRAY[@]}"; do
    echo "索引 $i: ${IP_LIST_ARRAY[$i]}"
done

# 判断是否为空
if [ ${#IP_LIST_ARRAY[@]} -eq 0 ]; then
    echo "IP_LIST_ARRAY ip列表是空的"
    exit 1
fi

# 本机公共ip
LOCAL_IP="127.0.0.1"
if check_internet; then
    LOCAL_IP=$(get_public_ip)
    echo "外网连接正常 LOCAL_IP:${LOCAL_IP}"
else
    LOCAL_IP="127.0.0.1"
    echo "无法连接外网 LOCAL_IP:${LOCAL_IP}"
fi

echo "LOCAL_IP:${LOCAL_IP}..."

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


# 1.打包脚本
TMP_DIR=/root/build_mongo_temp

# 生成keyfile
KEYFILE_PATH=${SCRIPT_PATH}/keyfile
openssl rand -base64 756 > ${KEYFILE_PATH}

echo "创建keyfile 成功 ${KEYFILE_PATH}"

TGZ_FILE_NAME=mongodb.tar.gz
sh ${SCRIPT_PATH}/pack_tar.sh ${TMP_DIR} ${SCRIPT_PATH} ${TGZ_FILE_NAME} || {
    echo "错误： pack_tar.sh fail ${TMP_DIR} ${TGZ_FILE_NAME} 失败" >&2
    exit 1
}

echo "pack TGZ_FILE_NAME:${TGZ_FILE_NAME} success."

# 初始化环境: 安装mongodb软件, 并设置环境变量
TARGET_SCRIPT_PATH=/root/mongodb_script
NODE_CONFIG_ARR=()
COUNT=0
declare -A is_ip_init_dict
for index in "${!IP_LIST_ARRAY[@]}"; do
    # ip file 一行的数据: DATA ip
    elem="${IP_LIST_ARRAY[$index]}"
    echo "elem:${elem}"

    # 过滤空行
    if [ -z "${elem}" ] || [[ "$elem" =~ ^[[:space:]]*$ ]]; then
        continue
    fi
    fields=($(echo "${elem}" | awk '{print $1, $2, $3}'))
    ip="${fields[1]}"
    node_port="${fields[2]}"
    NODE_CONFIG_ARR[$COUNT]="${fields[0]} ${fields[1]} ${fields[2]}"
    echo "第 $index 个 IP 地址: elem:${elem}, $ip, $node_port, $fields[0] COUNT:{$COUNT}, ${NODE_CONFIG_ARR[$COUNT]}"
    COUNT=$(($COUNT + 1))

    # 本地机器, 则不需要远程拷贝
    if [ ${ip} = "127.0.0.1" ] || [ ${ip} = ${LOCAL_IP} ]; then
        if [ -z "${is_ip_init_dict[$ip]}" ]; then
            echo "local init_package ..."
            . ${SCRIPT_PATH}/init_package.sh ${TMP_DIR}/${TGZ_FILE_NAME} ${TARGET_SCRIPT_PATH}

            . ${TARGET_SCRIPT_PATH}/init_env.sh ${TARGET_SCRIPT_PATH} ${INSTALL_PATH} || {
                echo "错误：本地:$ip ${TARGET_SCRIPT_PATH}/init_env.sh 失败" >&2
                exit 1
            }
            is_ip_init_dict[$ip]=1
        fi
    else
        if [ -z "${is_ip_init_dict[$ip]}" ]; then
            echo "remote: ${ip}: 创建目录: TMP_DIR:${TMP_DIR} ..."
            ssh root@${ip} "rm -rf ${TMP_DIR}" || {
                echo "错误： 移除 ${TMP_DIR} 失败" >&2
                exit 1
            }
            ssh root@${ip} "mkdir -p ${TMP_DIR}" || {
                echo "错误：${ip} 创建 ${TMP_DIR} 失败" >&2
                exit 1
            }

            echo "拷贝压缩文件 ${TMP_DIR}/${TGZ_FILE_NAME} =>  ${ip}:${TMP_DIR} ..."
            scp -r ${TMP_DIR}/${TGZ_FILE_NAME} root@${ip}:${TMP_DIR} || {
                echo "错误： scp 拷贝 ${TMP_DIR}/${TGZ_FILE_NAME} => ${ip}:${TMP_DIR} 失败" >&2
                exit 1
            }

            echo "拷贝 init_package.sh =>  ${ip}:${TMP_DIR} ..."
            scp -r ${SCRIPT_PATH}/init_package.sh root@${ip}:${TMP_DIR} || {
                echo "错误： scp 拷贝 ${SCRIPT_PATH}/init_package.sh => ${ip}:${TMP_DIR} 失败" >&2
                exit 1
            }
            echo "$ip 执行 init_package.sh => ..."
            ssh root@${ip} "sh ${TMP_DIR}/init_package.sh ${TMP_DIR}/${TGZ_FILE_NAME} ${TARGET_SCRIPT_PATH}" || {
                echo "错误：${ip} ${TMP_DIR}/init_package.sh 失败" >&2
                exit 1
            }
            ssh root@${ip} "sh ${TARGET_SCRIPT_PATH}/init_env.sh ${TARGET_SCRIPT_PATH} ${INSTALL_PATH}" || {
                echo "错误：${ip} ${TARGET_SCRIPT_PATH}/init_env.sh 失败" >&2
                exit 1
            }
            is_ip_init_dict[$ip]=1
            echo "$ip 执行 init_env.sh 成功"
        fi
    fi
done

TEST_MONGOD_PATH=$(whereis mongod)
echo "create_mongo_shard_cluster init env success mongod path:${TEST_MONGOD_PATH}."


# 获取主节点ip
CONFIG_SVR_PRIMARY=""
CONFIG_SVR_PRIMARY_PORT=""
MONGOD_SVR_PRIMARY=""
MONGOD_SVR_PRIMARY_PORT=""
MONGOS_SVR_PRIMARY=""
MONGOS_SVR_PRIMARY_PORT=""

MONGOD_SVR_ARRAY=()
MONGO_CONFIG_SVR_ARRAY=()
MONGOS_SVR_ARRAY=()
COUNT_CONFIG=0
COUNT_MONGOD=0
COUNT_MONGOS=0

for index in "${!NODE_CONFIG_ARR[@]}"; do
    # ip file 一行的数据: DATA ip
    elem="${IP_LIST_ARRAY[$index]}"
    echo "NODE_CONFIG_ARR elem:${elem}"
    fields=($(echo "${elem}" | awk '{print $1, $2, $3}'))
    node_type="${fields[0]}"
    ip="${fields[1]}"
    node_port="${fields[2]}"
    echo "第 $index 个 IP 地址: elem:${elem}, $ip, ${node_type}, $fields[0]"
    
    if [ ${node_type} = "config" ]; then
        # 添加到config_svr数组
        MONGO_CONFIG_SVR_ARRAY[$COUNT_CONFIG]="$elem"
        COUNT_CONFIG=$(($COUNT_CONFIG + 1))

        if [ -z "${CONFIG_SVR_PRIMARY}" ]; then
            CONFIG_SVR_PRIMARY=${ip}
            CONFIG_SVR_PRIMARY_PORT=${node_port}
        fi

    elif [ ${node_type} = "mongod" ]; then
        # 添加到mongod_svr数组
        MONGOD_SVR_ARRAY[$COUNT_MONGOD]="$elem"
        COUNT_MONGOD=$(($COUNT_MONGOD + 1))

        if [ -z "${MONGOD_SVR_PRIMARY}" ]; then
            MONGOD_SVR_PRIMARY=${ip}
            MONGOD_SVR_PRIMARY_PORT=${node_port}
        fi

    elif [ ${node_type} = "mongos" ]; then
        # 添加到mongos_svr数组
        MONGOS_SVR_ARRAY[$COUNT_MONGOS]="$elem"
        COUNT_MONGOS=$(($COUNT_MONGOS + 1))

        if [ -z "${MONGOS_SVR_PRIMARY}" ]; then
            MONGOS_SVR_PRIMARY=${ip}
            MONGOS_SVR_PRIMARY_PORT=${node_port}
        fi
    else
        echo "bad node_type:${node_type}"
        exit 1
    fi
done
# ip列表打印
for i in "${!MONGO_CONFIG_SVR_ARRAY[@]}"; do
    echo "MONGO_CONFIG_SVR_ARRAY 索引 $i: ${MONGO_CONFIG_SVR_ARRAY[$i]}"
done
for i in "${!MONGOD_SVR_ARRAY[@]}"; do
    echo "MONGOD_SVR_ARRAY 索引 $i: ${MONGOD_SVR_ARRAY[$i]}"
done
for i in "${!MONGOS_SVR_ARRAY[@]}"; do
    echo "MONGOS_SVR_ARRAY 索引 $i: ${MONGOS_SVR_ARRAY[$i]}"
done

echo "CONFIG_SVR_PRIMARY:${CONFIG_SVR_PRIMARY}:${CONFIG_SVR_PRIMARY_PORT}"
echo "MONGOD_SVR_PRIMARY:${MONGOD_SVR_PRIMARY}:${MONGOD_SVR_PRIMARY_PORT}"
echo "MONGOS_SVR_PRIMARY:${MONGOS_SVR_PRIMARY}:${MONGOS_SVR_PRIMARY_PORT}"

if [ -z "${CONFIG_SVR_PRIMARY}" ] || [ -z "${CONFIG_SVR_PRIMARY_PORT}" ] || [ -z "${MONGOD_SVR_PRIMARY}" ] || [ -z "${MONGOD_SVR_PRIMARY_PORT}" ] || [ -z "${MONGOS_SVR_PRIMARY}" ] || [ -z "${MONGOS_SVR_PRIMARY_PORT}" ]; then
    echo "lack primary info "
    exit 1
fi

echo "scan config, mongod, mongos ip list success."


echo "create_mongo_shard_cluster init configsvr nodes..."

# 初始化Configsvr主节点
if [ ${CONFIG_SVR_PRIMARY} = "127.0.0.1" ] || [ ${CONFIG_SVR_PRIMARY} = ${LOCAL_IP} ]; then
    echo "create_mongo_shard_cluster init local config primary DB_NAME:${DB_NAME}..."
    sh ${TARGET_SCRIPT_PATH}/init_primary.sh ${TARGET_SCRIPT_PATH} ${TARGET_USER} ${TARGET_PWD} ${DATA_PATH} ${CONFIG_SVR_PRIMARY} ${CONFIG_SVR_PRIMARY_PORT} ${DB_NAME}_config_1 ${RS_NAME}_config ${TARGET_SCRIPT_PATH}/keyfile configsvr || {
        echo "错误：${CONFIG_SVR_PRIMARY} init_primary fail ${SCRIPT_PATH}/init_primary.sh 失败" >&2
        exit 1
    }

    echo "create_mongo_shard_cluster init local config primary success."
else
    echo "create_mongo_shard_cluster init remote ${CONFIG_SVR_PRIMARY} config primary..."

    ssh root@${CONFIG_SVR_PRIMARY} "sh ${TARGET_SCRIPT_PATH}/init_primary.sh ${TARGET_SCRIPT_PATH} ${TARGET_USER} ${TARGET_PWD} ${DATA_PATH} ${CONFIG_SVR_PRIMARY} ${CONFIG_SVR_PRIMARY_PORT} ${DB_NAME}_config_1 ${RS_NAME}_config ${TARGET_SCRIPT_PATH}/keyfile configsvr" || {
        echo "错误：${CONFIG_SVR_PRIMARY} init_primary fail ${WORK_PATH}/init_primary.sh 失败" >&2
        exit 1
    }

    echo "create_mongo_shard_cluster init remote ${CONFIG_SVR_PRIMARY} config primary success."
fi

echo "create_mongo_shard_cluster init mongod nodes..."

# 初始化数据主节点
if [ ${MONGOD_SVR_PRIMARY} = "127.0.0.1" ] || [ ${MONGOD_SVR_PRIMARY} = ${LOCAL_IP} ]; then
    echo "create_mongo_shard_cluster init local mongod primary DB_NAME:${DB_NAME}..."
    sh ${TARGET_SCRIPT_PATH}/init_primary.sh ${TARGET_SCRIPT_PATH} ${TARGET_USER} ${TARGET_PWD} ${DATA_PATH} ${MONGOD_SVR_PRIMARY} ${MONGOD_SVR_PRIMARY_PORT} ${DB_NAME}_mongod_1 ${RS_NAME}_mongod ${TARGET_SCRIPT_PATH}/keyfile shardsvr || {
        echo "错误：${MONGOD_SVR_PRIMARY} init_primary fail ${SCRIPT_PATH}/init_primary.sh 失败" >&2
        exit 1
    }

    echo "create_mongo_shard_cluster init local mongod primary success."
else
    echo "create_mongo_shard_cluster init remote ${MONGOD_SVR_PRIMARY} mongod primary..."

    ssh root@${MONGOD_SVR_PRIMARY} "sh ${TARGET_SCRIPT_PATH}/init_primary.sh ${TARGET_SCRIPT_PATH} ${TARGET_USER} ${TARGET_PWD} ${DATA_PATH} ${MONGOD_SVR_PRIMARY} ${MONGOD_SVR_PRIMARY_PORT} ${DB_NAME}_mongod_1 ${RS_NAME}_mongod ${TARGET_SCRIPT_PATH}/keyfile shardsvr" || {
        echo "错误：${MONGOD_SVR_PRIMARY} init_primary fail ${TARGET_SCRIPT_PATH}/init_primary.sh 失败" >&2
        exit 1
    }

    echo "create_mongo_shard_cluster init remote ${MONGOD_SVR_PRIMARY} mongod primary success."
fi

echo "create_mongo_shard_cluster init primary node success."


# 启动节点函数
start_nodes() {
    local NODES_STR="$1"
    local NODES_LEN=$2
    local SHARDING_ROLE="$3"
    local PRINT_STR=""
    for ((i=1; i<=${NODES_LEN}; i++)); do
        echo "当前: $i"
        PRINT_STR="${PRINT_STR}\$$i"
        if [ $i -ne ${NODES_LEN} ]; then
            PRINT_STR="${PRINT_STR}, "
        fi
    done

    echo "PRINT_STR:${PRINT_STR}"

    local items=($(echo "${NODES_STR}" | awk -F';' '{print '${PRINT_STR}'}'))
    echo "start_nodes..."

    DB_INDEX=1
    for index in "${!items[@]}"; do
        # ip file 一行的数据: DATA ip
        elem="${items[$index]}"
        fields=($(echo "${elem}" | awk '{print $1, $2, $3}'))
        node_type="${fields[0]}"
        ip="${fields[1]}"
        node_port="${fields[2]}"
        echo "第 $index 个 IP 地址: $ip, ${node_type}, ${node_port}"
        
        # db_name
        DB_INDEX=$(($DB_INDEX + $index))
        FINAL_DB_NAME=${DB_NAME}_${DB_TYPE}_${DB_INDEX}

        # 是不是本地地址
        if [ ${ip} = "127.0.0.1" ] || [ ${ip} = ${LOCAL_IP} ]; then
            sh ${TARGET_SCRIPT_PATH}/create_mongodb_inst.sh ${TARGET_SCRIPT_PATH} ${DATA_PATH}/${FINAL_DB_NAME} ${node_port} "${RS_NAME}_${DB_TYPE}" ${TARGET_SCRIPT_PATH}/keyfile ${SHARDING_ROLE} || {
                echo "创建db1实例失败！" >&2
                return 1
            }  
            echo "启动mongodb节点 IP 地址: FINAL_DB_NAME:${FINAL_DB_NAME} elem:${elem}, $ip, ${node_port} 成功."

        else
            ssh root@${ip} "sh ${TARGET_SCRIPT_PATH}/create_mongodb_inst.sh ${TARGET_SCRIPT_PATH} ${DATA_PATH}/${FINAL_DB_NAME} ${node_port} \"${RS_NAME}_${DB_TYPE}\" ${TARGET_SCRIPT_PATH}/keyfile ${SHARDING_ROLE}" || {
                echo "错误：${ip} create_mongodb_inst fail ${TARGET_SCRIPT_PATH}/create_mongodb_inst.sh 失败" >&2
                return 1
            }

            echo "启动mongodb节点 IP 地址: FINAL_DB_NAME:${FINAL_DB_NAME} elem:${elem}, $ip, ${node_port} 成功."
        fi
    done

    local PRIMARY_ADDR=""
    local PRIMARY_PORT_TMP=""
    local TMP_ADDRS=()
    for index in "${!items[@]}"; do
        if [ $index -eq 0 ]; then
            # ip file 一行的数据: DATA ip
            elem="${nodes_arr[$index]}"
            fields=($(echo "${elem}" | awk '{print $1, $2, $3}'))
            ip="${fields[1]}"
            node_port="${fields[2]}"
            echo "PRIMARY: 第 $index 个 IP 地址: elem:${elem}, $ip, ${node_type}, ${node_port}"
        
            PRIMARY_ADDR=${ip}
            PRIMARY_PORT_TMP=${node_port}
        else
            elem="${nodes_arr[$index]}"
            TMP_ADDRS+=("$elem")
        fi
    done
    
    # 添加节点
    echo "start nodes primary add nodes..."
    for index in "${!TMP_ADDRS[@]}"; do
        elem="${TMP_ADDRS[$index]}"
        fields=($(echo "${elem}" | awk '{print $1, $2, $3}'))
        ip="${fields[1]}"
        node_port="${fields[2]}"
        echo "${PRIMARY_ADDR} add TMP_ADDRS node: ${ip}:${node_port}..."
        local cnt=$(($index + 1))
        
        if [ ${PRIMARY_ADDR} = "127.0.0.1" ] || [ ${PRIMARY_ADDR} = ${LOCAL_IP} ]; then
            echo "${PRIMARY_ADDR} excute mongsosh "
            mongosh --host ${PRIMARY_ADDR}:${PRIMARY_PORT_TMP} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "rs.add({_id: ${cnt}, host: \"${ip}:${node_port}\", priority: 1, votes: 1})" || {
                echo "错误：初始化复制集失败 添加节点失败:${ip}:${node_port}" >&2
                exit 1
            }
        else
            ssh root@${PRIMARY_ADDR} "mongosh --host ${PRIMARY_ADDR}:${PRIMARY_PORT_TMP} -u \"${TARGET_USER}\" -p \"${TARGET_PWD}\" --authenticationDatabase admin --eval \"rs.add({_id: ${cnt}, host: \\\"${ip}:${node_port}\\\", priority: 1, votes: 1})\"" || {
                echo "错误：${PRIMARY_ADDR} add node fail ${TARGET_SCRIPT_PATH}/create_mongodb_inst.sh 失败" >&2
                return 1
            }
        fi
        echo "${PRIMARY_ADDR} add node: ${ip}:${node_port} success."
    done

    sleep 2
    return 0
}


ARRAY_STR_TMP=""
MAX_INDEX=${#MONGO_CONFIG_SVR_ARRAY[@]}
MAX_INDEX=$(($MAX_INDEX - 1))
for index in "${!MONGO_CONFIG_SVR_ARRAY[@]}"; do
    item="${MONGO_CONFIG_SVR_ARRAY[$index]}"
    ARRAY_STR_TMP=${ARRAY_STR_TMP}$item
    if [ $index -ne $MAX_INDEX ]; then
        ARRAY_STR_TMP=${ARRAY_STR_TMP}";"
    fi
done

# 启动配置服复制集
echo "ARRAY_STR_TMP:${ARRAY_STR_TMP}" 
start_nodes ARRAY_STR_TMP ${#MONGO_CONFIG_SVR_ARRAY[@]} configsvr  || {
    echo "错误： start_nodes fail ARRAY_STR_TMP:${ARRAY_STR_TMP} 失败" >&2
    exit 1
}


ARRAY_STR_TMP=""
MAX_INDEX=${#MONGOD_SVR_ARRAY[@]}
MAX_INDEX=$(($MAX_INDEX - 1))
for index in "${!MONGOD_SVR_ARRAY[@]}"; do
    item="${MONGOD_SVR_ARRAY[$index]}"
    ARRAY_STR_TMP=${ARRAY_STR_TMP}$item
    if [ $index -ne $MAX_INDEX ]; then
        ARRAY_STR_TMP=${ARRAY_STR_TMP}";"
    fi
done

# 启动mongod复制集
echo "ARRAY_STR_TMP:${ARRAY_STR_TMP}" 
start_nodes ARRAY_STR_TMP ${#MONGOD_SVR_ARRAY[@]} shardsvr || {
    echo "错误： start_nodes fail ARRAY_STR_TMP:${ARRAY_STR_TMP} 失败" >&2
    exit 1
}

echo "start nodes success."