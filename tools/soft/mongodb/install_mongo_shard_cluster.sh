#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 创建mongo 分片集群
# IP_LIST_FILE: ip 节点类型 端口: config 127.0.0.1 27010 / shard1 127.0.0.1 27011 / mongos 127.0.0.1 27017
# INSTALL_PATH: 安装mongodb的程序目录
# DATA_PATH: 安装mongodb的数据库目录
# TARGET_USER: 用户名
# TARGET_PWD: 密码
# RS_NAME_PREFIX: 复制集名前缀, 如 rs, 则config复制集名为rs_config, shard1为rs_shard1
# DB_NAME: db名(可选, 默认admin)
# SHARD_CONFIG: 分片配置(可选, 格式 dbname.collection:shardkey, 如 mydb.users:_id)

# 当前脚本路径
SCRIPT_PATH="$(cd $(dirname $0); pwd)"

# IP列表文件
IP_LIST_FILE=$1
# 用户名
TARGET_USER=$2
# 密码
TARGET_PWD=$3
# 复制集名前缀
RS_NAME_PREFIX=$4
# 安装mongodb的目录
INSTALL_PATH=$5
# 数据库数据路径
DATA_PATH=$6
# 数据库名(可选, 默认admin)
DB_NAME=${7:-admin}
# 分片配置(可选, 格式 dbname.collection:shardkey)
SHARD_CONFIG=$8

##############################
# 参数校验
##############################
if [ ! -e "${IP_LIST_FILE}" ]; then
    echo "IP_LIST_FILE:${IP_LIST_FILE} not exist please check"
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

if [ -z "${RS_NAME_PREFIX}" ]; then
    echo "RS_NAME_PREFIX is empty please check!!!"
    exit 1
fi

if [ -z "${INSTALL_PATH}" ]; then
    echo "INSTALL_PATH is empty please check!!!"
    exit 1
fi

if [ -z "${DATA_PATH}" ]; then
    echo "DATA_PATH is empty please check!!!"
    exit 1
fi

# 加载公共
. ${SCRIPT_PATH}/common/common_define.sh
. ${SCRIPT_PATH}/common/funcs.sh

echo "=========================================="
echo "MongoDB 分片集群自动搭建脚本"
echo "=========================================="
echo "IP_LIST_FILE    : ${IP_LIST_FILE}"
echo "TARGET_USER     : ${TARGET_USER}"
echo "RS_NAME_PREFIX  : ${RS_NAME_PREFIX}"
echo "INSTALL_PATH    : ${INSTALL_PATH}"
echo "DATA_PATH       : ${DATA_PATH}"
echo "DB_NAME         : ${DB_NAME}"
echo "SHARD_CONFIG    : ${SHARD_CONFIG}"
echo "=========================================="

##############################
# 解析 iplist.txt, 按类型分组
##############################
declare -A SHARD_GROUPS    # key=shard名(shard1/shard2/...), value=节点列表(分号分隔: "ip port;ip port")
CONFIG_SVR_ARRAY=()        # config节点数组, 元素格式: "ip port"
MONGOS_SVR_ARRAY=()        # mongos节点数组, 元素格式: "ip port"
SHARD_NAME_LIST=()         # 分片名有序列表, 保证添加分片顺序

# 本机公共ip
LOCAL_IP="127.0.0.1"
if check_internet; then
    LOCAL_IP=$(get_public_ip)
    echo "外网连接正常 LOCAL_IP:${LOCAL_IP}"
else
    LOCAL_IP="127.0.0.1"
    echo "无法连接外网 LOCAL_IP:${LOCAL_IP}"
fi

# 读取iplist
IP_LIST_ARRAY=()
read_file_to_array ${IP_LIST_FILE} IP_LIST_ARRAY || {
    echo "错误： read_file_to_array fail IP_LIST_FILE: ${IP_LIST_FILE} 失败" >&2
    exit 1
}

if [ ${#IP_LIST_ARRAY[@]} -eq 0 ]; then
    echo "IP_LIST_ARRAY ip列表是空的"
    exit 1
fi

# 解析每行, 按类型分组
for index in "${!IP_LIST_ARRAY[@]}"; do
    elem="${IP_LIST_ARRAY[$index]}"

    # 过滤空行和注释行
    if [ -z "${elem}" ] || [[ "$elem" =~ ^[[:space:]]*$ ]]; then
        continue
    fi
    # 去掉前导空格后检查注释
    trimmed=$(echo "${elem}" | sed 's/^[[:space:]]*//')
    if [[ "$trimmed" =~ ^# ]]; then
        continue
    fi

    fields=($(echo "${elem}" | awk '{print $1, $2, $3}'))
    node_type="${fields[0]}"
    ip="${fields[1]}"
    node_port="${fields[2]}"

    if [ -z "${node_type}" ] || [ -z "${ip}" ] || [ -z "${node_port}" ]; then
        echo "警告：跳过无效行: ${elem}"
        continue
    fi

    echo "解析节点: type=${node_type}, ip=${ip}, port=${node_port}"

    if [ "${node_type}" = "config" ]; then
        CONFIG_SVR_ARRAY+=("${ip} ${node_port}")
    elif [ "${node_type}" = "mongos" ]; then
        MONGOS_SVR_ARRAY+=("${ip} ${node_port}")
    elif [[ "${node_type}" =~ ^shard[0-9]+$ ]]; then
        # 分片类型: shard1, shard2, ...
        if [ -z "${SHARD_GROUPS[$node_type]}" ]; then
            SHARD_GROUPS[$node_type]="${ip} ${node_port}"
            SHARD_NAME_LIST+=("${node_type}")
        else
            SHARD_GROUPS[$node_type]="${SHARD_GROUPS[$node_type]};${ip} ${node_port}"
        fi
    else
        echo "错误：未知的节点类型: ${node_type}, 支持的类型: config, mongos, shard1, shard2, ..."
        exit 1
    fi
done

# 校验必须有config和至少一个shard
if [ ${#CONFIG_SVR_ARRAY[@]} -eq 0 ]; then
    echo "错误：iplist.txt中没有config节点"
    exit 1
fi

if [ ${#SHARD_NAME_LIST[@]} -eq 0 ]; then
    echo "错误：iplist.txt中没有shard节点(需要shard1/shard2/...格式)"
    exit 1
fi

if [ ${#MONGOS_SVR_ARRAY[@]} -eq 0 ]; then
    echo "错误：iplist.txt中没有mongos节点"
    exit 1
fi

# 打印分组结果
echo ""
echo "========== 节点分组结果 =========="
echo "Config 节点 (${#CONFIG_SVR_ARRAY[@]}个):"
for i in "${!CONFIG_SVR_ARRAY[@]}"; do
    echo "  [$i] ${CONFIG_SVR_ARRAY[$i]}"
done

echo "Shard 分片:"
for shard_name in "${SHARD_NAME_LIST[@]}"; do
    # 统计该分片节点数
    IFS=';' read -ra shard_nodes <<< "${SHARD_GROUPS[$shard_name]}"
    echo "  ${shard_name} (${#shard_nodes[@]}个): ${SHARD_GROUPS[$shard_name]}"
done

echo "Mongos 节点 (${#MONGOS_SVR_ARRAY[@]}个):"
for i in "${!MONGOS_SVR_ARRAY[@]}"; do
    echo "  [$i] ${MONGOS_SVR_ARRAY[$i]}"
done
echo "=================================="

##############################
# 生成keyfile
##############################
if ! command -v openssl &> /dev/null; then
    echo "当前环境未安装openssl正在安装 OpenSSL..."
    if sudo yum install openssl -y &> /dev/null; then
        echo "OpenSSL 安装成功！"
    else
        echo "安装失败，请手动执行：sudo yum install openssl -y"
        exit 1
    fi
fi

KEYFILE_PATH=${SCRIPT_PATH}/keyfile
openssl rand -base64 756 > ${KEYFILE_PATH}
echo "创建keyfile 成功 ${KEYFILE_PATH}"

##############################
# 打包脚本并分发到各机器, 初始化环境
##############################
TMP_DIR=/root/build_mongo_temp
TGZ_FILE_NAME=mongodb.tar.gz

sh ${SCRIPT_PATH}/pack_tar.sh ${TMP_DIR} ${SCRIPT_PATH} ${TGZ_FILE_NAME} || {
    echo "错误： pack_tar.sh fail ${TMP_DIR} ${TGZ_FILE_NAME} 失败" >&2
    exit 1
}
echo "pack TGZ_FILE_NAME:${TGZ_FILE_NAME} success."

# 收集所有ip(去重)
declare -A ALL_IPS
for item in "${CONFIG_SVR_ARRAY[@]}"; do
    ip=$(echo "${item}" | awk '{print $1}')
    ALL_IPS[$ip]=1
done
for shard_name in "${SHARD_NAME_LIST[@]}"; do
    IFS=';' read -ra shard_nodes <<< "${SHARD_GROUPS[$shard_name]}"
    for node in "${shard_nodes[@]}"; do
        ip=$(echo "${node}" | awk '{print $1}')
        ALL_IPS[$ip]=1
    done
done
for item in "${MONGOS_SVR_ARRAY[@]}"; do
    ip=$(echo "${item}" | awk '{print $1}')
    ALL_IPS[$ip]=1
done

TARGET_SCRIPT_PATH=/root/mongodb_script
declare -A is_ip_init_dict

for ip in "${!ALL_IPS[@]}"; do
    echo "初始化环境 ip:${ip}..."

    if [ "${ip}" = "127.0.0.1" ] || [ "${ip}" = "${LOCAL_IP}" ]; then
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

            echo "拷贝keyfile到 ${ip}..."
            scp -r ${KEYFILE_PATH} root@${ip}:${TARGET_SCRIPT_PATH}/keyfile || {
                echo "错误： scp 拷贝 keyfile => ${ip}:${TARGET_SCRIPT_PATH}/keyfile 失败" >&2
                exit 1
            }

            ssh root@${ip} "source ~/.bash_profile 2>/dev/null; sh ${TARGET_SCRIPT_PATH}/init_env.sh ${TARGET_SCRIPT_PATH} ${INSTALL_PATH}" || {
                echo "错误：${ip} ${TARGET_SCRIPT_PATH}/init_env.sh 失败" >&2
                exit 1
            }
            is_ip_init_dict[$ip]=1
            echo "$ip 执行 init_env.sh 成功"
        fi
    fi
done

echo "所有节点环境初始化完成."

##############################
# 辅助函数: 判断是否本机ip
##############################
is_local_ip() {
    local ip=$1
    if [ "${ip}" = "127.0.0.1" ] || [ "${ip}" = "${LOCAL_IP}" ]; then
        return 0
    else
        return 1
    fi
}

##############################
# 辅助函数: 在指定机器上执行命令
# $1: ip
# $2: 命令
##############################
exec_on_host() {
    local ip=$1
    local cmd=$2
    if is_local_ip ${ip}; then
        bash -c "${cmd}"
    else
        ssh root@${ip} "source ~/.bash_profile 2>/dev/null; ${cmd}"
    fi
}

##############################
# 步骤1: 初始化 config 复制集
##############################
echo ""
echo "========== 步骤1: 初始化 config 复制集 =========="

# 提取config主节点(第一个节点)
CONFIG_PRIMARY_IP=$(echo "${CONFIG_SVR_ARRAY[0]}" | awk '{print $1}')
CONFIG_PRIMARY_PORT=$(echo "${CONFIG_SVR_ARRAY[0]}" | awk '{print $2}')
CONFIG_RS_NAME="${RS_NAME_PREFIX}_config"
CONFIG_DB_NAME="${DB_NAME}_config_1"

echo "Config主节点: ${CONFIG_PRIMARY_IP}:${CONFIG_PRIMARY_PORT}, RS_NAME: ${CONFIG_RS_NAME}"

# 1.1 初始化config主节点 (no_auth → rs.initiate → createUser → shutdown)
if is_local_ip ${CONFIG_PRIMARY_IP}; then
    sh ${TARGET_SCRIPT_PATH}/init_primary.sh ${TARGET_SCRIPT_PATH} ${TARGET_USER} ${TARGET_PWD} ${DATA_PATH} ${LOCAL_IP} ${CONFIG_PRIMARY_PORT} ${CONFIG_DB_NAME} ${CONFIG_RS_NAME} ${TARGET_SCRIPT_PATH}/keyfile configsvr || {
        echo "错误：config主节点 init_primary 失败" >&2
        exit 1
    }
else
    # 拷贝keyfile到远程
    scp -r ${KEYFILE_PATH} root@${CONFIG_PRIMARY_IP}:${TARGET_SCRIPT_PATH}/keyfile 2>/dev/null
    ssh root@${CONFIG_PRIMARY_IP} "source ~/.bash_profile 2>/dev/null; sh ${TARGET_SCRIPT_PATH}/init_primary.sh ${TARGET_SCRIPT_PATH} ${TARGET_USER} ${TARGET_PWD} ${DATA_PATH} ${CONFIG_PRIMARY_IP} ${CONFIG_PRIMARY_PORT} ${CONFIG_DB_NAME} ${CONFIG_RS_NAME} ${TARGET_SCRIPT_PATH}/keyfile configsvr" || {
        echo "错误：config主节点 ${CONFIG_PRIMARY_IP} init_primary 失败" >&2
        exit 1
    }
fi

echo "Config主节点初始化(no_auth → createUser → shutdown) 成功."

# 1.2 启动config主节点(带认证)
if is_local_ip ${CONFIG_PRIMARY_IP}; then
    sh ${TARGET_SCRIPT_PATH}/create_mongodb_inst.sh ${TARGET_SCRIPT_PATH} ${DATA_PATH}/${CONFIG_DB_NAME} ${LOCAL_IP} ${CONFIG_PRIMARY_PORT} "${CONFIG_RS_NAME}" ${TARGET_SCRIPT_PATH}/keyfile configsvr || {
        echo "错误：config主节点带认证启动失败" >&2
        exit 1
    }
else
    ssh root@${CONFIG_PRIMARY_IP} "source ~/.bash_profile 2>/dev/null; sh ${TARGET_SCRIPT_PATH}/create_mongodb_inst.sh ${TARGET_SCRIPT_PATH} ${DATA_PATH}/${CONFIG_DB_NAME} ${CONFIG_PRIMARY_IP} ${CONFIG_PRIMARY_PORT} \"${CONFIG_RS_NAME}\" ${TARGET_SCRIPT_PATH}/keyfile configsvr" || {
        echo "错误：config主节点 ${CONFIG_PRIMARY_IP} 带认证启动失败" >&2
        exit 1
    }
fi

echo "Config主节点带认证启动成功."

sleep 5

# 1.3 启动config从节点并添加到复制集
for i in "${!CONFIG_SVR_ARRAY[@]}"; do
    if [ $i -eq 0 ]; then
        continue  # 跳过主节点
    fi

    item="${CONFIG_SVR_ARRAY[$i]}"
    ip=$(echo "${item}" | awk '{print $1}')
    node_port=$(echo "${item}" | awk '{print $2}')
    node_db_name="${DB_NAME}_config_$(($i + 1))"

    echo "启动config从节点 [$i]: ${ip}:${node_port}..."

    # 启动从节点(带认证, 不需要no_auth)
    if is_local_ip ${ip}; then
        sh ${TARGET_SCRIPT_PATH}/create_mongodb_inst.sh ${TARGET_SCRIPT_PATH} ${DATA_PATH}/${node_db_name} ${LOCAL_IP} ${node_port} "${CONFIG_RS_NAME}" ${TARGET_SCRIPT_PATH}/keyfile configsvr || {
            echo "错误：config从节点 ${ip}:${node_port} 启动失败" >&2
            exit 1
        }
    else
        ssh root@${ip} "source ~/.bash_profile 2>/dev/null; sh ${TARGET_SCRIPT_PATH}/create_mongodb_inst.sh ${TARGET_SCRIPT_PATH} ${DATA_PATH}/${node_db_name} ${ip} ${node_port} \"${CONFIG_RS_NAME}\" ${TARGET_SCRIPT_PATH}/keyfile configsvr" || {
            echo "错误：config从节点 ${ip}:${node_port} 启动失败" >&2
            exit 1
        }
    fi

    sleep 3

    # 从主节点添加该从节点到复制集
    local_ip=${ip}
    if is_local_ip ${ip}; then
        local_ip=${LOCAL_IP}
    fi

    echo "从主节点添加config从节点: ${local_ip}:${node_port}..."
    if is_local_ip ${CONFIG_PRIMARY_IP}; then
        mongosh --host ${LOCAL_IP}:${CONFIG_PRIMARY_PORT} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "rs.add({_id: ${i}, host: \"${local_ip}:${node_port}\", priority: 1, votes: 1})" || {
            echo "错误：添加config从节点 ${local_ip}:${node_port} 失败" >&2
            exit 1
        }
    else
        ssh root@${CONFIG_PRIMARY_IP} "source ~/.bash_profile 2>/dev/null; mongosh --host ${CONFIG_PRIMARY_IP}:${CONFIG_PRIMARY_PORT} -u \"${TARGET_USER}\" -p \"${TARGET_PWD}\" --authenticationDatabase admin --eval \"rs.add({_id: ${i}, host: \\\"${local_ip}:${node_port}\\\", priority: 1, votes: 1})\"" || {
            echo "错误：添加config从节点 ${local_ip}:${node_port} 失败" >&2
            exit 1
        }
    fi

    echo "Config从节点 ${local_ip}:${node_port} 添加成功."
    sleep 2
done

# 验证config复制集状态
echo "验证config复制集状态..."
if is_local_ip ${CONFIG_PRIMARY_IP}; then
    mongosh --host ${LOCAL_IP}:${CONFIG_PRIMARY_PORT} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "rs.status()" || {
        echo "警告：config复制集状态查询失败" >&2
    }
else
    ssh root@${CONFIG_PRIMARY_IP} "source ~/.bash_profile 2>/dev/null; mongosh --host ${CONFIG_PRIMARY_IP}:${CONFIG_PRIMARY_PORT} -u \"${TARGET_USER}\" -p \"${TARGET_PWD}\" --authenticationDatabase admin --eval \"rs.status()\"" || {
        echo "警告：config复制集状态查询失败" >&2
    }
fi

echo "Config复制集初始化完成."

##############################
# 步骤2: 初始化每个 shard 分片的复制集
##############################
echo ""
echo "========== 步骤2: 初始化 shard 分片复制集 =========="

# 记录每个分片的地址串, 用于后续 addShard
declare -A SHARD_ADDR_STRINGS

for shard_name in "${SHARD_NAME_LIST[@]}"; do
    echo ""
    echo "--- 初始化分片: ${shard_name} ---"

    SHARD_RS_NAME="${RS_NAME_PREFIX}_${shard_name}"

    # 解析该分片的节点列表
    IFS=';' read -ra shard_nodes <<< "${SHARD_GROUPS[$shard_name]}"

    if [ ${#shard_nodes[@]} -eq 0 ]; then
        echo "错误：分片 ${shard_name} 没有节点" >&2
        exit 1
    fi

    # 提取主节点(第一个节点)
    primary_node="${shard_nodes[0]}"
    primary_ip=$(echo "${primary_node}" | awk '{print $1}')
    primary_port=$(echo "${primary_node}" | awk '{print $2}')
    primary_db_name="${DB_NAME}_${shard_name}_1"

    echo "Shard ${shard_name} 主节点: ${primary_ip}:${primary_port}, RS_NAME: ${SHARD_RS_NAME}"

    # 2.1 初始化shard主节点 (no_auth → rs.initiate → createUser → shutdown)
    if is_local_ip ${primary_ip}; then
        sh ${TARGET_SCRIPT_PATH}/init_primary.sh ${TARGET_SCRIPT_PATH} ${TARGET_USER} ${TARGET_PWD} ${DATA_PATH} ${LOCAL_IP} ${primary_port} ${primary_db_name} ${SHARD_RS_NAME} ${TARGET_SCRIPT_PATH}/keyfile shardsvr || {
            echo "错误：shard ${shard_name} 主节点 init_primary 失败" >&2
            exit 1
        }
    else
        scp -r ${KEYFILE_PATH} root@${primary_ip}:${TARGET_SCRIPT_PATH}/keyfile 2>/dev/null
        ssh root@${primary_ip} "source ~/.bash_profile 2>/dev/null; sh ${TARGET_SCRIPT_PATH}/init_primary.sh ${TARGET_SCRIPT_PATH} ${TARGET_USER} ${TARGET_PWD} ${DATA_PATH} ${primary_ip} ${primary_port} ${primary_db_name} ${SHARD_RS_NAME} ${TARGET_SCRIPT_PATH}/keyfile shardsvr" || {
            echo "错误：shard ${shard_name} 主节点 ${primary_ip} init_primary 失败" >&2
            exit 1
        }
    fi

    echo "Shard ${shard_name} 主节点初始化(no_auth → createUser → shutdown) 成功."

    # 2.2 启动shard主节点(带认证)
    if is_local_ip ${primary_ip}; then
        sh ${TARGET_SCRIPT_PATH}/create_mongodb_inst.sh ${TARGET_SCRIPT_PATH} ${DATA_PATH}/${primary_db_name} ${LOCAL_IP} ${primary_port} "${SHARD_RS_NAME}" ${TARGET_SCRIPT_PATH}/keyfile shardsvr || {
            echo "错误：shard ${shard_name} 主节点带认证启动失败" >&2
            exit 1
        }
    else
        ssh root@${primary_ip} "source ~/.bash_profile 2>/dev/null; sh ${TARGET_SCRIPT_PATH}/create_mongodb_inst.sh ${TARGET_SCRIPT_PATH} ${DATA_PATH}/${primary_db_name} ${primary_ip} ${primary_port} \"${SHARD_RS_NAME}\" ${TARGET_SCRIPT_PATH}/keyfile shardsvr" || {
            echo "错误：shard ${shard_name} 主节点 ${primary_ip} 带认证启动失败" >&2
            exit 1
        }
    fi

    echo "Shard ${shard_name} 主节点带认证启动成功."

    sleep 5

    # 2.3 启动shard从节点并添加到复制集
    for j in "${!shard_nodes[@]}"; do
        if [ $j -eq 0 ]; then
            continue  # 跳过主节点
        fi

        node="${shard_nodes[$j]}"
        ip=$(echo "${node}" | awk '{print $1}')
        node_port=$(echo "${node}" | awk '{print $2}')
        node_db_name="${DB_NAME}_${shard_name}_$(($j + 1))"

        echo "启动shard ${shard_name} 从节点 [$j]: ${ip}:${node_port}..."

        if is_local_ip ${ip}; then
            sh ${TARGET_SCRIPT_PATH}/create_mongodb_inst.sh ${TARGET_SCRIPT_PATH} ${DATA_PATH}/${node_db_name} ${LOCAL_IP} ${node_port} "${SHARD_RS_NAME}" ${TARGET_SCRIPT_PATH}/keyfile shardsvr || {
                echo "错误：shard ${shard_name} 从节点 ${ip}:${node_port} 启动失败" >&2
                exit 1
            }
        else
            ssh root@${ip} "source ~/.bash_profile 2>/dev/null; sh ${TARGET_SCRIPT_PATH}/create_mongodb_inst.sh ${TARGET_SCRIPT_PATH} ${DATA_PATH}/${node_db_name} ${ip} ${node_port} \"${SHARD_RS_NAME}\" ${TARGET_SCRIPT_PATH}/keyfile shardsvr" || {
                echo "错误：shard ${shard_name} 从节点 ${ip}:${node_port} 启动失败" >&2
                exit 1
            }
        fi

        sleep 3

        # 从主节点添加该从节点到复制集
        local_ip=${ip}
        if is_local_ip ${ip}; then
            local_ip=${LOCAL_IP}
        fi

        member_id=$j
        echo "从主节点添加shard ${shard_name} 从节点: ${local_ip}:${node_port}..."
        if is_local_ip ${primary_ip}; then
            mongosh --host ${LOCAL_IP}:${primary_port} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "rs.add({_id: ${member_id}, host: \"${local_ip}:${node_port}\", priority: 1, votes: 1})" || {
                echo "错误：添加shard ${shard_name} 从节点 ${local_ip}:${node_port} 失败" >&2
                exit 1
            }
        else
            ssh root@${primary_ip} "source ~/.bash_profile 2>/dev/null; mongosh --host ${primary_ip}:${primary_port} -u \"${TARGET_USER}\" -p \"${TARGET_PWD}\" --authenticationDatabase admin --eval \"rs.add({_id: ${member_id}, host: \\\"${local_ip}:${node_port}\\\", priority: 1, votes: 1})\"" || {
                echo "错误：添加shard ${shard_name} 从节点 ${local_ip}:${node_port} 失败" >&2
                exit 1
            }
        fi

        echo "Shard ${shard_name} 从节点 ${local_ip}:${node_port} 添加成功."
        sleep 2
    done

    # 构建该分片的地址串: rs_shard1/ip1:port1,ip2:port2,...
    shard_addr_str="${SHARD_RS_NAME}/"
    max_idx=$((${#shard_nodes[@]} - 1))
    for j in "${!shard_nodes[@]}"; do
        node="${shard_nodes[$j]}"
        ip=$(echo "${node}" | awk '{print $1}')
        node_port=$(echo "${node}" | awk '{print $2}')
        if is_local_ip ${ip}; then
            ip=${LOCAL_IP}
        fi
        shard_addr_str="${shard_addr_str}${ip}:${node_port}"
        if [ $j -ne $max_idx ]; then
            shard_addr_str="${shard_addr_str},"
        fi
    done
    SHARD_ADDR_STRINGS[$shard_name]=${shard_addr_str}

    echo "Shard ${shard_name} 地址串: ${shard_addr_str}"
    echo "Shard ${shard_name} 复制集初始化完成."
done

echo "所有 shard 分片复制集初始化完成."

##############################
# 步骤3: 初始化 mongos 路由节点
##############################
echo ""
echo "========== 步骤3: 初始化 mongos 路由节点 =========="

# 构建 configSvr 地址串: rs_config/ip1:port1,ip2:port2,ip3:port3
CONFIG_SVR_ADDR_STR="${CONFIG_RS_NAME}/"
max_config_idx=$((${#CONFIG_SVR_ARRAY[@]} - 1))
for i in "${!CONFIG_SVR_ARRAY[@]}"; do
    item="${CONFIG_SVR_ARRAY[$i]}"
    ip=$(echo "${item}" | awk '{print $1}')
    node_port=$(echo "${item}" | awk '{print $2}')
    if is_local_ip ${ip}; then
        ip=${LOCAL_IP}
    fi
    CONFIG_SVR_ADDR_STR="${CONFIG_SVR_ADDR_STR}${ip}:${node_port}"
    if [ $i -ne $max_config_idx ]; then
        CONFIG_SVR_ADDR_STR="${CONFIG_SVR_ADDR_STR},"
    fi
done

echo "ConfigSvr 地址串: ${CONFIG_SVR_ADDR_STR}"

# 3.1 初始化每个mongos节点 (no_auth → createUser → shutdown)
for i in "${!MONGOS_SVR_ARRAY[@]}"; do
    item="${MONGOS_SVR_ARRAY[$i]}"
    ip=$(echo "${item}" | awk '{print $1}')
    node_port=$(echo "${item}" | awk '{print $2}')
    mongos_db_name="${DB_NAME}_mongos_$(($i + 1))"

    echo "初始化mongos节点 [$i]: ${ip}:${node_port}..."

    if is_local_ip ${ip}; then
        sh ${TARGET_SCRIPT_PATH}/init_mongos.sh ${TARGET_SCRIPT_PATH} ${TARGET_USER} ${TARGET_PWD} ${DATA_PATH}/${mongos_db_name} ${LOCAL_IP} ${node_port} ${mongos_db_name} "${CONFIG_RS_NAME}" ${TARGET_SCRIPT_PATH}/keyfile "${CONFIG_SVR_ADDR_STR}" || {
            echo "错误：mongos节点 ${ip}:${node_port} init_mongos 失败" >&2
            exit 1
        }
    else
        scp -r ${KEYFILE_PATH} root@${ip}:${TARGET_SCRIPT_PATH}/keyfile 2>/dev/null
        ssh root@${ip} "source ~/.bash_profile 2>/dev/null; sh ${TARGET_SCRIPT_PATH}/init_mongos.sh ${TARGET_SCRIPT_PATH} ${TARGET_USER} \"${TARGET_PWD}\" ${DATA_PATH}/${mongos_db_name} ${ip} ${node_port} ${mongos_db_name} \"${CONFIG_RS_NAME}\" ${TARGET_SCRIPT_PATH}/keyfile \"${CONFIG_SVR_ADDR_STR}\"" || {
            echo "错误：mongos节点 ${ip}:${node_port} init_mongos 失败" >&2
            exit 1
        }
    fi

    echo "Mongos节点 ${ip}:${node_port} 初始化(no_auth → createUser → shutdown) 成功."
done

# 3.2 启动mongos节点(带认证)
for i in "${!MONGOS_SVR_ARRAY[@]}"; do
    item="${MONGOS_SVR_ARRAY[$i]}"
    ip=$(echo "${item}" | awk '{print $1}')
    node_port=$(echo "${item}" | awk '{print $2}')
    mongos_db_name="${DB_NAME}_mongos_$(($i + 1))"

    echo "启动mongos节点(带认证) [$i]: ${ip}:${node_port}..."

    if is_local_ip ${ip}; then
        sh ${TARGET_SCRIPT_PATH}/create_mongos_inst.sh ${TARGET_SCRIPT_PATH} ${DATA_PATH}/${mongos_db_name} ${node_port} "${CONFIG_RS_NAME}" ${TARGET_SCRIPT_PATH}/keyfile "${CONFIG_SVR_ADDR_STR}" || {
            echo "错误：mongos节点 ${ip}:${node_port} 带认证启动失败" >&2
            exit 1
        }
    else
        ssh root@${ip} "source ~/.bash_profile 2>/dev/null; sh ${TARGET_SCRIPT_PATH}/create_mongos_inst.sh ${TARGET_SCRIPT_PATH} ${DATA_PATH}/${mongos_db_name} ${node_port} \"${CONFIG_RS_NAME}\" ${TARGET_SCRIPT_PATH}/keyfile \"${CONFIG_SVR_ADDR_STR}\"" || {
            echo "错误：mongos节点 ${ip}:${node_port} 带认证启动失败" >&2
            exit 1
        }
    fi

    echo "Mongos节点 ${ip}:${node_port} 带认证启动成功."
    sleep 3
done

echo "所有 mongos 路由节点启动完成."

##############################
# 步骤4: 通过 mongos 添加分片 (sh.addShard)
##############################
echo ""
echo "========== 步骤4: mongos 添加分片 =========="

# 选择第一个mongos节点执行addShard
MONGOS_PRIMARY_IP=$(echo "${MONGOS_SVR_ARRAY[0]}" | awk '{print $1}')
MONGOS_PRIMARY_PORT=$(echo "${MONGOS_SVR_ARRAY[0]}" | awk '{print $2}')

if is_local_ip ${MONGOS_PRIMARY_IP}; then
    MONGOS_PRIMARY_IP=${LOCAL_IP}
fi

echo "使用mongos节点 ${MONGOS_PRIMARY_IP}:${MONGOS_PRIMARY_PORT} 执行addShard..."

sleep 5  # 等待mongos完全启动

for shard_name in "${SHARD_NAME_LIST[@]}"; do
    shard_addr="${SHARD_ADDR_STRINGS[$shard_name]}"
    echo "添加分片: sh.addShard(\"${shard_addr}\")..."

    if is_local_ip $(echo "${MONGOS_SVR_ARRAY[0]}" | awk '{print $1}'); then
        mongosh --host ${MONGOS_PRIMARY_IP}:${MONGOS_PRIMARY_PORT} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "sh.addShard(\"${shard_addr}\")" || {
            echo "错误：添加分片 ${shard_name} 失败" >&2
            exit 1
        }
    else
        ssh root@$(echo "${MONGOS_SVR_ARRAY[0]}" | awk '{print $1}') "source ~/.bash_profile 2>/dev/null; mongosh --host ${MONGOS_PRIMARY_IP}:${MONGOS_PRIMARY_PORT} -u \"${TARGET_USER}\" -p \"${TARGET_PWD}\" --authenticationDatabase admin --eval \"sh.addShard(\\\"${shard_addr}\\\")\"" || {
            echo "错误：添加分片 ${shard_name} 失败" >&2
            exit 1
        }
    fi

    echo "分片 ${shard_name} 添加成功."
    sleep 3
done

echo "所有分片添加完成."

##############################
# 步骤5: 启用分片和设置分片键
##############################
echo ""
echo "========== 步骤5: 启用分片和设置分片键 =========="

if [ -n "${SHARD_CONFIG}" ]; then
    # 解析分片配置: dbname.collection:shardkey
    SHARD_DB_NAME=$(echo "${SHARD_CONFIG}" | cut -d'.' -f1)
    SHARD_COLLECTION=$(echo "${SHARD_CONFIG}" | cut -d'.' -f2 | cut -d':' -f1)
    SHARD_KEY=$(echo "${SHARD_CONFIG}" | cut -d':' -f2)

    if [ -z "${SHARD_DB_NAME}" ] || [ -z "${SHARD_COLLECTION}" ] || [ -z "${SHARD_KEY}" ]; then
        echo "错误：分片配置格式不正确: ${SHARD_CONFIG}, 正确格式: dbname.collection:shardkey" >&2
        exit 1
    fi

    echo "启用数据库分片: sh.enableSharding(\"${SHARD_DB_NAME}\")..."

    if is_local_ip $(echo "${MONGOS_SVR_ARRAY[0]}" | awk '{print $1}'); then
        mongosh --host ${MONGOS_PRIMARY_IP}:${MONGOS_PRIMARY_PORT} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "sh.enableSharding(\"${SHARD_DB_NAME}\")" || {
            echo "错误：启用数据库分片 ${SHARD_DB_NAME} 失败" >&2
            exit 1
        }
    else
        ssh root@$(echo "${MONGOS_SVR_ARRAY[0]}" | awk '{print $1}') "source ~/.bash_profile 2>/dev/null; mongosh --host ${MONGOS_PRIMARY_IP}:${MONGOS_PRIMARY_PORT} -u \"${TARGET_USER}\" -p \"${TARGET_PWD}\" --authenticationDatabase admin --eval \"sh.enableSharding(\\\"${SHARD_DB_NAME}\\\")\"" || {
            echo "错误：启用数据库分片 ${SHARD_DB_NAME} 失败" >&2
            exit 1
        }
    fi

    echo "数据库 ${SHARD_DB_NAME} 分片已启用."

    sleep 2

    echo "设置分片键: sh.shardCollection(\"${SHARD_DB_NAME}.${SHARD_COLLECTION}\", {${SHARD_KEY}: 1})..."

    if is_local_ip $(echo "${MONGOS_SVR_ARRAY[0]}" | awk '{print $1}'); then
        mongosh --host ${MONGOS_PRIMARY_IP}:${MONGOS_PRIMARY_PORT} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "sh.shardCollection(\"${SHARD_DB_NAME}.${SHARD_COLLECTION}\", {${SHARD_KEY}: 1})" || {
            echo "错误：设置分片键 ${SHARD_DB_NAME}.${SHARD_COLLECTION} 失败" >&2
            exit 1
        }
    else
        ssh root@$(echo "${MONGOS_SVR_ARRAY[0]}" | awk '{print $1}') "source ~/.bash_profile 2>/dev/null; mongosh --host ${MONGOS_PRIMARY_IP}:${MONGOS_PRIMARY_PORT} -u \"${TARGET_USER}\" -p \"${TARGET_PWD}\" --authenticationDatabase admin --eval \"sh.shardCollection(\\\"${SHARD_DB_NAME}.${SHARD_COLLECTION}\\\", {${SHARD_KEY}: 1})\"" || {
            echo "错误：设置分片键 ${SHARD_DB_NAME}.${SHARD_COLLECTION} 失败" >&2
            exit 1
        }
    fi

    echo "分片键设置成功: ${SHARD_DB_NAME}.${SHARD_COLLECTION} -> {${SHARD_KEY}: 1}"
else
    echo "未指定分片配置(SHARD_CONFIG), 跳过enableSharding和shardCollection."
    echo "如需启用, 请使用格式: dbname.collection:shardkey"
fi

##############################
# 完成
##############################
echo ""
echo "=========================================="
echo "MongoDB 分片集群搭建完成!"
echo "=========================================="
echo "Config 复制集: ${CONFIG_RS_NAME}"
echo "  主节点: ${CONFIG_PRIMARY_IP}:${CONFIG_PRIMARY_PORT}"
echo ""
echo "Shard 分片:"
for shard_name in "${SHARD_NAME_LIST[@]}"; do
    echo "  ${shard_name}: ${SHARD_ADDR_STRINGS[$shard_name]}"
done
echo ""
echo "Mongos 路由:"
for i in "${!MONGOS_SVR_ARRAY[@]}"; do
    item="${MONGOS_SVR_ARRAY[$i]}"
    ip=$(echo "${item}" | awk '{print $1}')
    node_port=$(echo "${item}" | awk '{print $2}')
    echo "  ${ip}:${node_port}"
done
echo ""
echo "连接方式:"
echo "  mongosh --host ${MONGOS_PRIMARY_IP}:${MONGOS_PRIMARY_PORT} -u ${TARGET_USER} -p <密码> --authenticationDatabase admin"
echo "=========================================="
