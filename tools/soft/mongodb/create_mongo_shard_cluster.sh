#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 创建mongo 分片集群
# create_mongo_shard_cluster.sh 复制集名 复制集公网ip 主节点端口号 用户名 密码 复制集安装目标路径
# create_mongo_shard_cluster.sh IP列表文本文件 目标机器工作路径(数据库运行路径) 目标机器安装路径(数据库安装路径)
# 先初始化各个节点环境 => 启动config/shard各个节点脚本创建3节点复制集 => 创建mongos => 测试连通性 
# IP_LIST_FILE: ip 节点类型 端口: config 127.0.0.1 27010

# 当前脚本路径
SCRIPT_PATH="$(cd $(dirname $0); pwd)"

# IP列表文件
IP_LIST_FILE=$1
# 工作路径
WORK_PATH=$2
# 安装mongodb的目录
INSTALL_PATH=$3
TARGET_USER=$4
TARGET_PWD=$5
# 最终复制集工作目录
REPLISET_INSTALL_PATH=$6
# 数据库名(由业务决定)
DB_NAME=$7
# 复制集名
RS_NAME=$8

# 校验参数
if [ -e ${IP_LIST_FILE} ]; then
    echo "IP_LIST_FILE:${IP_LIST_FILE} exist"
else
    echo "IP_LIST_FILE:${IP_LIST_FILE} not exist please check"
fi

if [ -z "${WORK_PATH}" ]; then
    echo "WORK_PATH is empty please check!!!"
    exit 1
fi
if [ -z "${INSTALL_PATH}" ]; then
    echo "INSTALL_PATH is empty please check!!!"
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

# 初始化环境
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
            . ${SCRIPT_PATH}/init_package.sh /root/build_mongo_temp/mongodb.tar.gz ${WORK_PATH} ${INSTALL_PATH}

            . ${SCRIPT_PATH}/init_env.sh ${WORK_PATH} ${INSTALL_PATH} || {
                echo "错误：本地:$ip ${TMP_DIR}/init_env.sh 失败" >&2
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

            echo "拷贝init_env.sh =>  ${ip}:${TMP_DIR} ..."
            scp -r ${SCRIPT_PATH}/init_package.sh root@${ip}:${TMP_DIR} || {
                echo "错误： scp 拷贝 ${SCRIPT_PATH}/init_package.sh => ${ip}:${TMP_DIR} 失败" >&2
                exit 1
            }
            echo "$ip 执行 init_package.sh => ..."
            ssh root@${ip} "sh ${TMP_DIR}/init_package.sh ${TMP_DIR}/${TGZ_FILE_NAME} ${WORK_PATH} ${INSTALL_PATH}" || {
                echo "错误：${ip} ${TMP_DIR}/init_package.sh 失败" >&2
                exit 1
            }
            ssh root@${ip} "sh ${WORK_PATH}/init_env.sh ${WORK_PATH} ${INSTALL_PATH}" || {
                echo "错误：${ip} ${WORK_PATH}/init_env.sh 失败" >&2
                exit 1
            }
            is_ip_init_dict[$ip]=1
            echo "$ip 执行 init_env.sh 成功"
        fi
    fi
done

echo "create_mongo_shard_cluster init env success."

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
        MONGO_CONFIG_SVR_ARRAY[$index]="$elem"

        if [ -z "${CONFIG_SVR_PRIMARY}" ]; then
            CONFIG_SVR_PRIMARY=${ip}
            CONFIG_SVR_PRIMARY_PORT=${node_port}
        fi

    elif [ ${node_type} = "mongod" ]; then
        # 添加到mongod_svr数组
        MONGOD_SVR_ARRAY[$index]="$elem"

        if [ -z "${MONGOD_SVR_PRIMARY}" ]; then
            MONGOD_SVR_PRIMARY=${ip}
            MONGOD_SVR_PRIMARY_PORT=${node_port}
        fi

    elif [ ${node_type} = "mongos" ]; then
        # 添加到mongos_svr数组
        MONGOS_SVR_ARRAY[$index]="$elem"

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

echo "create_mongo_shard_cluster init configsvr nodes..."

# 初始化Configsvr主节点
if [ ${CONFIG_SVR_PRIMARY} = "127.0.0.1" ] || [ ${CONFIG_SVR_PRIMARY} = ${LOCAL_IP} ]; then
    echo "create_mongo_shard_cluster init local config primary DB_NAME:${DB_NAME}..."
    sh ${SCRIPT_PATH}/init_primary.sh ${SCRIPT_PATH} ${TARGET_USER} ${TARGET_PWD} ${REPLISET_INSTALL_PATH} ${CONFIG_SVR_PRIMARY} ${CONFIG_SVR_PRIMARY_PORT} ${DB_NAME}_config_1 ${RS_NAME}_config ${KEYFILE_PATH} configsvr || {
        echo "错误：${CONFIG_SVR_PRIMARY} init_primary fail ${SCRIPT_PATH}/init_primary.sh 失败" >&2
        exit 1
    }

    echo "create_mongo_shard_cluster init local config primary success."
else
    echo "create_mongo_shard_cluster init remote ${CONFIG_SVR_PRIMARY} config primary..."

    ssh root@${CONFIG_SVR_PRIMARY} "sh ${WORK_PATH}/init_primary.sh ${WORK_PATH} ${TARGET_USER} ${TARGET_PWD} ${REPLISET_INSTALL_PATH} ${CONFIG_SVR_PRIMARY} ${CONFIG_SVR_PRIMARY_PORT} ${DB_NAME}_config_1 ${RS_NAME}_config ${KEYFILE_PATH} configsvr" || {
        echo "错误：${CONFIG_SVR_PRIMARY} init_primary fail ${WORK_PATH}/init_primary.sh 失败" >&2
        exit 1
    }

    echo "create_mongo_shard_cluster init remote ${CONFIG_SVR_PRIMARY} config primary success."
fi

echo "create_mongo_shard_cluster init mongod nodes..."

# 初始化数据主节点
if [ ${MONGOD_SVR_PRIMARY} = "127.0.0.1" ] || [ ${MONGOD_SVR_PRIMARY} = ${LOCAL_IP} ]; then
    echo "create_mongo_shard_cluster init local mongod primary DB_NAME:${DB_NAME}..."
    sh ${SCRIPT_PATH}/init_primary.sh ${SCRIPT_PATH} ${TARGET_USER} ${TARGET_PWD} ${REPLISET_INSTALL_PATH} ${MONGOD_SVR_PRIMARY} ${MONGOD_SVR_PRIMARY_PORT} ${DB_NAME}_mongod_1 ${RS_NAME}_mongod ${KEYFILE_PATH} shardsvr || {
        echo "错误：${MONGOD_SVR_PRIMARY} init_primary fail ${SCRIPT_PATH}/init_primary.sh 失败" >&2
        exit 1
    }

    echo "create_mongo_shard_cluster init local mongod primary success."
else
    echo "create_mongo_shard_cluster init remote ${MONGOD_SVR_PRIMARY} mongod primary..."

    ssh root@${MONGOD_SVR_PRIMARY} "sh ${WORK_PATH}/init_primary.sh ${WORK_PATH} ${TARGET_USER} ${TARGET_PWD} ${REPLISET_INSTALL_PATH} ${MONGOD_SVR_PRIMARY} ${MONGOD_SVR_PRIMARY_PORT} ${DB_NAME}_mongod_1 ${RS_NAME}_mongod ${KEYFILE_PATH} shardsvr" || {
        echo "错误：${MONGOD_SVR_PRIMARY} init_primary fail ${WORK_PATH}/init_primary.sh 失败" >&2
        exit 1
    }

    echo "create_mongo_shard_cluster init remote ${MONGOD_SVR_PRIMARY} mongod primary success."
fi

# 启动节点函数
start_nodes() {
    declare -n nodes_arr="$1"
    echo "start_nodes..."

    DB_INDEX=1
    for index in "${!nodes_arr[@]}"; do
        # ip file 一行的数据: DATA ip
        elem="${nodes_arr[$index]}"
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
            sh ${SCRIPT_PATH}/create_mongodb_inst.sh ${SCRIPT_PATH} ${REPLISET_INSTALL_PATH}/${FINAL_DB_NAME} ${node_port} "${RS_NAME}_${DB_TYPE}" ${KEYFILE_PATH} || {
                echo "创建db1实例失败！" >&2
                return 1
            }  
            echo "启动mongodb节点 IP 地址: elem:${elem}, $ip, ${node_port} 成功."

        else
            ssh root@${ip} "sh ${WORK_PATH}/create_mongodb_inst.sh ${WORK_PATH} ${REPLISET_INSTALL_PATH}/${FINAL_DB_NAME} ${node_port} \"${RS_NAME}_${DB_TYPE}\" ${KEYFILE_PATH}" || {
                echo "错误：${ip} create_mongodb_inst fail ${WORK_PATH}/create_mongodb_inst.sh 失败" >&2
                return 1
            }

            echo "启动mongodb节点 IP 地址: elem:${elem}, $ip, ${node_port} 成功."
        fi
    done

    local PRIMARY_ADDR=""
    local PRIMARY_PORT_TMP=""
    local TMP_ADDRS=()
    for index in "${!nodes_arr[@]}"; do
        if [ $index -eq 0 ]; then
            # ip file 一行的数据: DATA ip
            elem="${nodes_arr[$index]}"
            fields=($(echo "${elem}" | awk '{print $1, $2, $3}'))
            ip="${fields[1]}"
            node_port="${fields[2]}"
            echo "第 $index 个 IP 地址: elem:${elem}, $ip, ${node_type}, ${node_port}"
        
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
        echo "${PRIMARY_ADDR} add node: ${ip}:${node_port}..."
        local cnt=$(($index + 1))
        
        if [ ${PRIMARY_ADDR} = "127.0.0.1" ] || [ ${PRIMARY_ADDR} = ${LOCAL_IP} ]; then
            echo "${PRIMARY_ADDR} excute mongsosh "
            mongosh --host ${PRIMARY_ADDR}:${PRIMARY_PORT_TMP} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "rs.add({_id: ${cnt}, host: \"${ip}:${node_port}\", priority: 1, votes: 1})" || {
                echo "错误：初始化复制集失败" >&2
                exit 1
            }
        else
            ssh root@${PRIMARY_ADDR} "mongosh --host ${PRIMARY_ADDR}:${PRIMARY_PORT_TMP} -u \"${TARGET_USER}\" -p \"${TARGET_PWD}\" --authenticationDatabase admin --eval \"rs.add({_id: ${cnt}, host: \\\"${ip}:${node_port}\\\", priority: 1, votes: 1})\"" || {
                echo "错误：${PRIMARY_ADDR} add node fail ${WORK_PATH}/create_mongodb_inst.sh 失败" >&2
                return 1
            }
        fi
        echo "${PRIMARY_ADDR} add node: ${ip}:${node_port} success."
    done

    sleep 2
    return 0
}

echo "create_mongo_shard_cluster start config nodes..."

# 启动配置集群
start_nodes ${MONGO_CONFIG_SVR_ARRAY} || {
    echo "错误： start_nodes 配置集群 fail 失败" >&2
    exit 1
}

echo "start nodes MONGO_CONFIG_SVR success"

echo "create_mongo_shard_cluster start mongod nodes..."

# 启动数据集群
start_nodes ${MONGOD_SVR_ARRAY} || {
    echo "错误： start_nodes 数据集群 fail 失败" >&2
    exit 1
}

echo "start nodes MONGOD_SVR success"


# 获取数组长度
CONFIG_ADDR_LEN=${#MONGO_CONFIG_SVR_ARRAY[@]}
CONFIG_ADDR_MAX_INDEX=$(($CONFIG_ADDR_LEN - 1))

# mongos的配置服务器列表
MONGO_CONFIG_SVR_ARRAY_STR=""
for index in "${!MONGO_CONFIG_SVR_ARRAY[@]}"; do
    # ip file 一行的数据: DATA ip
    elem="${MONGO_CONFIG_SVR_ARRAY[$index]}"
    echo "MONGO_CONFIG_SVR_ARRAY elem:${elem}"
    fields=($(echo "${elem}" | awk '{print $1, $2, $3}'))
    ip="${fields[1]}"
    node_port="${fields[2]}"
    MONGO_CONFIG_SVR_ARRAY_STR=${MONGO_CONFIG_SVR_ARRAY_STR}${ip}:${node_port}
    if [ $index -ne $CONFIG_ADDR_MAX_INDEX ]; then
        MONGO_CONFIG_SVR_ARRAY_STR=${MONGO_CONFIG_SVR_ARRAY_STR},
    fi
done

echo "MONGO_CONFIG_SVR_ARRAY_STR:${MONGO_CONFIG_SVR_ARRAY_STR}"

# 数据复制集
MONGOD_ADDR_LEN=${#MONGOD_SVR_ARRAY[@]}
MONGOD_ADDR_MAX_INDEX=$(($MONGOD_ADDR_LEN - 1))

MONGO_DATA_SVR_ARRAY_STR=""
for index in "${!MONGOD_SVR_ARRAY[@]}"; do
    # ip file 一行的数据: DATA ip
    elem="${MONGOD_SVR_ARRAY[$index]}"
    echo "MONGOD_SVR_ARRAY elem:${elem}"
    fields=($(echo "${elem}" | awk '{print $1, $2, $3}'))
    ip="${fields[1]}"
    node_port="${fields[2]}"
    MONGO_DATA_SVR_ARRAY_STR=${MONGO_DATA_SVR_ARRAY_STR}${ip}:${node_port}
    if [ $index -ne $MONGOD_ADDR_MAX_INDEX ]; then
        MONGO_DATA_SVR_ARRAY_STR=${MONGO_DATA_SVR_ARRAY_STR},
    fi
done

echo "MONGO_DATA_SVR_ARRAY_STR:${MONGO_DATA_SVR_ARRAY_STR}"

echo "create_mongo_shard_cluster init mongos nodes..."

# 初始化mongos
for index in "${!MONGOS_SVR_ARRAY[@]}"; do
    # ip file 一行的数据: DATA ip
    elem="${MONGOS_SVR_ARRAY[$index]}"
    fields=($(echo "${elem}" | awk '{print $1, $2, $3}'))
    node_type="${fields[0]}"
    ip="${fields[1]}"
    node_port="${fields[2]}"
    echo "mongos 第 $index 个 IP 地址: elem:${elem}, $ip, ${node_type}, ${node_port}"
    
    # db_name
    DB_INDEX=$(($DB_INDEX + $index))
    FINAL_DB_NAME=${DB_NAME}_${DB_TYPE}_${DB_INDEX}

    # 是不是本地地址
    if [ ${ip} = "127.0.0.1" ] || [ ${ip} = ${LOCAL_IP} ]; then
        sh ${SCRIPT_PATH}/init_mongos.sh ${SCRIPT_PATH} ${TARGET_USER} ${TARGET_PWD} ${REPLISET_INSTALL_PATH}/${FINAL_DB_NAME} ${ip} ${node_port} ${FINAL_DB_NAME} "${RS_NAME}" ${KEYFILE_PATH} ${MONGO_CONFIG_SVR_ARRAY_STR} || {
            echo "创建db1实例失败！" >&2
            return 1
        }  
        echo "初始化 mongos 节点 IP 地址: elem:${elem}, $ip, ${node_port} 成功."

    else
        ssh root@${ip} "sh ${WORK_PATH}/init_mongos.sh ${WORK_PATH} ${TARGET_USER} \"${TARGET_PWD}\" ${REPLISET_INSTALL_PATH}/${FINAL_DB_NAME} ${ip} ${node_port} ${FINAL_DB_NAME} \"${RS_NAME}\" ${KEYFILE_PATH} ${MONGO_CONFIG_SVR_ARRAY_STR} || { return 1 }" || {
            echo "错误：${ip} init_mongos fail ${WORK_PATH}/init_mongos.sh 失败" >&2
            return 1
        }

        echo "初始化 mongos 节点 IP 地址: elem:${elem}, $ip, ${node_port} 成功."
    fi
done
echo "create_mongo_shard_cluster init mongos nodes success."

echo "create_mongo_shard_cluster start mongos nodes..."

# 启动 mongos
for index in "${!MONGOS_SVR_ARRAY[@]}"; do
    # ip file 一行的数据: DATA ip
    elem="${MONGOS_SVR_ARRAY[$index]}"
    fields=($(echo "${elem}" | awk '{print $1, $2, $3}'))
    node_type="${fields[0]}"
    ip="${fields[1]}"
    node_port="${fields[2]}"
    echo "mongos 第 $index 个 IP 地址: elem:${elem}, $ip, ${node_type}, ${node_port}"
    
    # db_name
    DB_INDEX=$(($DB_INDEX + $index))
    FINAL_DB_NAME=${DB_NAME}_${DB_TYPE}_${DB_INDEX}

    # 是不是本地地址
    if [ ${ip} = "127.0.0.1" ] || [ ${ip} = ${LOCAL_IP} ]; then
        sh ${SCRIPT_PATH}/create_mongos_inst.sh ${SCRIPT_PATH} ${REPLISET_INSTALL_PATH}/${FINAL_DB_NAME} ${node_port} "${RS_NAME}" ${KEYFILE_PATH} ${MONGO_CONFIG_SVR_ARRAY_STR} || {
            echo "创建mongos实例失败！" >&2
            return 1
        }  
        echo "启动 mongos 节点 IP 地址: elem:${elem}, $ip, ${node_port} 成功."

    else
        ssh root@${ip} "sh ${WORK_PATH}/create_mongos_inst.sh  ${WORK_PATH} ${REPLISET_INSTALL_PATH}/${FINAL_DB_NAME} ${node_port} \"${RS_NAME}\" ${KEYFILE_PATH} ${MONGO_CONFIG_SVR_ARRAY_STR} || { return 1 }" || {
            echo "错误：${ip} init_mongos fail ${WORK_PATH}/init_mongos.sh 失败" >&2
            return 1
        }

        echo "启动 mongos 节点 IP 地址: elem:${elem}, $ip, ${node_port} 成功."
    fi
done

echo "create_mongo_shard_cluster start mongos nodes success."

echo "create_mongo_shard_cluster mongos addShard mongods: ${MONGO_DATA_SVR_ARRAY_STR}..."

# mongos 添加分片数据复制集
for index in "${!MONGOS_SVR_ARRAY[@]}"; do
    # ip file 一行的数据: DATA ip
    elem="${MONGOS_SVR_ARRAY[$index]}"
    fields=($(echo "${elem}" | awk '{print $1, $2, $3}'))
    node_type="${fields[0]}"
    ip="${fields[1]}"
    node_port="${fields[2]}"
    echo "mongos 第 $index 个 IP 地址: elem:${elem}, $ip, ${node_type}, ${node_port}"
    
    # db_name
    DB_INDEX=$(($DB_INDEX + $index))
    FINAL_DB_NAME=${DB_NAME}_${DB_TYPE}_${DB_INDEX}

    # 是不是本地地址
    if [ ${ip} = "127.0.0.1" ] || [ ${ip} = ${LOCAL_IP} ]; then
        mongosh --host 127.0.0.1:${node_port} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "sh.addShard(${RS_NAME}/${MONGO_DATA_SVR_ARRAY_STR})" || {
            echo "错误: 添加分片集失败 ${MONGO_DATA_SVR_ARRAY_STR}" >&2
            exit 1
        }

        echo "mongos 节点 IP 地址: elem:${elem}, $ip, ${node_port} 添加分片集 ${MONGO_DATA_SVR_ARRAY_STR} 成功."

    else
        ssh root@${ip} "mongosh --host 127.0.0.1:${node_port} -u \"${TARGET_USER}\" -p \"${TARGET_PWD}\" --authenticationDatabase admin --eval \"sh.addShard(${RS_NAME}/${MONGO_DATA_SVR_ARRAY_STR})\"" || {
            echo "错误：${ip} 添加分片集 ${MONGO_DATA_SVR_ARRAY_STR} 失败" >&2
            return 1
        }

        echo "启动 mongos 节点 IP 地址: elem:${elem}, $ip, ${node_port} 添加分片集 ${MONGO_DATA_SVR_ARRAY_STR} 成功."
    fi
done
echo "create_mongo_shard_cluster mongos addShard mongods: ${MONGO_DATA_SVR_ARRAY_STR} success."

echo "create_mongo_shard_cluster create nodes success."
