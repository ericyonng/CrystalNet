#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 创建mongo 分片集群
# 用法: sh install_mongo_shard_cluster.sh <iplist.txt> <用户名> <密码> <软件包安装路径> <数据库数据路径> [数据库名]
#
# IP_LIST_FILE: 节点类型 复制集前缀 IP 端口: config testsuit_rs 127.0.0.1 27010 / shard1 testsuit_rs 127.0.0.1 27011 / mongos testsuit_rs 127.0.0.1 27017
# TARGET_USER: 用户名
# TARGET_PWD: 密码
# INSTALL_PATH: 安装mongodb的程序目录
# DATA_PATH: 安装mongodb的数据库目录
# DB_NAME: db名(可选, 默认admin)

# 当前脚本路径
SCRIPT_PATH="$(cd $(dirname $0); pwd)"

# IP列表文件
IP_LIST_FILE=$1
# 用户名
TARGET_USER=$2
# 密码
TARGET_PWD=$3
# 安装mongodb的目录
INSTALL_PATH=$4
# 数据库数据路径
DATA_PATH=$5
# 数据库名(可选, 默认admin)
DB_NAME=${6:-admin}

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
echo "TARGET_PWD      : ******"
echo "INSTALL_PATH    : ${INSTALL_PATH}"
echo "DATA_PATH       : ${DATA_PATH}"
echo "DB_NAME         : ${DB_NAME}"
echo "=========================================="

##############################
# 解析 iplist.txt, 按类型分组
##############################
declare -A SHARD_GROUPS    # key=shard名(shard1/shard2/...), value=节点列表(分号分隔: "ip port;ip port")
CONFIG_SVR_ARRAY=()        # config节点数组, 元素格式: "ip port"
MONGOS_SVR_ARRAY=()        # mongos节点数组, 元素格式: "ip port"
SHARD_NAME_LIST=()         # 分片名有序列表, 保证添加分片顺序

# 复制集前缀相关: 从 iplist 中读取, 同一类型组内必须一致
declare -A RS_PREFIX_MAP   # key=节点类型(config/shard1/mongos), value=该组的复制集前缀

# 本机所有IP地址列表
LOCAL_IP_LIST="127.0.0.1 ::1"
if check_internet; then
    LOCAL_IP_LIST=$(get_local_ip_list)
    echo "外网连接正常 LOCAL_IP_LIST:${LOCAL_IP_LIST}"
else
    LOCAL_IP_LIST="127.0.0.1 ::1"
    echo "无法连接外网 LOCAL_IP_LIST:${LOCAL_IP_LIST}"
fi

# LOCAL_IP: 从 LOCAL_IP_LIST 中选最优IP(优先公网IP), 用于子脚本参数
LOCAL_IP=$(get_local_ip "${LOCAL_IP_LIST}")
echo "LOCAL_IP(用于子脚本): ${LOCAL_IP}"

# LOCAL_IPV4 / LOCAL_IPV6: 本机最优IPv4和IPv6地址, 根据iplist中的IP类型选择使用
LOCAL_IPV4=$(get_local_ipv4 "${LOCAL_IP_LIST}")
LOCAL_IPV6=$(get_local_ipv6 "${LOCAL_IP_LIST}")
echo "LOCAL_IPV4: ${LOCAL_IPV4}, LOCAL_IPV6: ${LOCAL_IPV6}"

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

    fields=($(echo "${elem}" | awk '{print $1, $2, $3, $4}'))
    node_type="${fields[0]}"
    rs_prefix="${fields[1]}"
    ip="${fields[2]}"
    node_port="${fields[3]}"

    if [ -z "${node_type}" ] || [ -z "${rs_prefix}" ] || [ -z "${ip}" ] || [ -z "${node_port}" ]; then
        echo "警告：跳过无效行(需要4字段: 类型 复制集前缀 IP 端口): ${elem}"
        continue
    fi

    # 校验同一类型组内的复制集前缀必须一致
    if [ -n "${RS_PREFIX_MAP[$node_type]}" ]; then
        if [ "${RS_PREFIX_MAP[$node_type]}" != "${rs_prefix}" ]; then
            echo "错误：类型 ${node_type} 的复制集前缀不一致: 已有 '${RS_PREFIX_MAP[$node_type]}', 当前行 '${rs_prefix}'" >&2
            exit 1
        fi
    else
        RS_PREFIX_MAP[$node_type]="${rs_prefix}"
    fi

    echo "解析节点: type=${node_type}, rs_prefix=${rs_prefix}, ip=${ip}, port=${node_port}"

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

    if is_local_host "${ip}" "${LOCAL_IP_LIST}"; then
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
            # 安全检查: 确保路径深度足够(至少2层)，防止误删危险目录
            TMP_DIR_DEPTH=$(echo "${TMP_DIR}" | tr -cd '/' | wc -c)
            if [ ${TMP_DIR_DEPTH} -lt 2 ] || [ -z "${TMP_DIR}" ]; then
                echo "错误： TMP_DIR 路径不安全: ${TMP_DIR}" >&2
                exit 1
            fi
            ssh root@${ip} "rm -rf ${TMP_DIR}" || {
                echo "错误： 移除 ${TMP_DIR} 失败" >&2
                exit 1
            }
            ssh root@${ip} "mkdir -p ${TMP_DIR}" || {
                echo "错误：${ip} 创建 ${TMP_DIR} 失败" >&2
                exit 1
            }

            echo "拷贝压缩文件 ${TMP_DIR}/${TGZ_FILE_NAME} =>  ${ip}:${TMP_DIR} ..."
            scp_to_host ${ip} ${TMP_DIR}/${TGZ_FILE_NAME} ${TMP_DIR} || {
                echo "错误： scp 拷贝 ${TMP_DIR}/${TGZ_FILE_NAME} => ${ip}:${TMP_DIR} 失败" >&2
                exit 1
            }

            echo "拷贝 init_package.sh =>  ${ip}:${TMP_DIR} ..."
            scp_to_host ${ip} ${SCRIPT_PATH}/init_package.sh ${TMP_DIR} || {
                echo "错误： scp 拷贝 ${SCRIPT_PATH}/init_package.sh => ${ip}:${TMP_DIR} 失败" >&2
                exit 1
            }
            echo "$ip 执行 init_package.sh => ..."
            ssh root@${ip} "sh ${TMP_DIR}/init_package.sh ${TMP_DIR}/${TGZ_FILE_NAME} ${TARGET_SCRIPT_PATH}" || {
                echo "错误：${ip} ${TMP_DIR}/init_package.sh 失败" >&2
                exit 1
            }

            echo "拷贝keyfile到 ${ip}..."
            scp_to_host ${ip} ${KEYFILE_PATH} ${TARGET_SCRIPT_PATH}/keyfile || {
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
# 辅助函数: 在指定机器上执行命令
# $1: ip
# $2: 命令
##############################
exec_on_host() {
    local ip=$1
    local cmd=$2
    if is_local_host "${ip}" "${LOCAL_IP_LIST}"; then
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
CONFIG_RS_NAME="${RS_PREFIX_MAP[config]}_config"
# 每个节点的数据子目录名: <DB_NAME>_config_<序号>
CONFIG_PRIMARY_DB_SUBDIR="${DB_NAME}_config_1"
# 完整数据目录: DATA_PATH/子目录名
CONFIG_PRIMARY_DB_PATH="${DATA_PATH}/${CONFIG_PRIMARY_DB_SUBDIR}"

echo "Config主节点: $(format_host_port ${CONFIG_PRIMARY_IP} ${CONFIG_PRIMARY_PORT}), RS_NAME: ${CONFIG_RS_NAME}"
echo "Config主节点数据目录: ${CONFIG_PRIMARY_DB_PATH}"

# 1.1 初始化config主节点 (no_auth → rs.initiate → createUser → shutdown)
# init_primary.sh 参数: SCRIPT_PATH TARGET_USER TARGET_PWD LOCAL_REPLISET_INSTALL_PATH LOCAL_PRIMARY_IP LOCAL_PRIMARY_PORT LOCAL_DB_NAME LOCAL_RS_NAME LOCAL_KEYFILE_PATH LOCAL_SHARDING_CLUSTER_ROLE [LOCAL_IS_MONGOS] [LOCAL_MONGOS_CONFIG_ADDR] [LOCAL_REGISTER_HOST]
# LOCAL_REPLISET_INSTALL_PATH: 数据目录的父路径(如 /root/mongo_data1)
# LOCAL_DB_NAME: 子目录名(如 mydb_config_1), init_primary.sh内部会拼接为 LOCAL_REPLISET_INSTALL_PATH/LOCAL_DB_NAME
# LOCAL_REGISTER_HOST: 注册到复制集的host(iplist中的原始域名或IP), rs.initiate用此host注册
if is_local_host "${CONFIG_PRIMARY_IP}" "${LOCAL_IP_LIST}"; then
    sh ${TARGET_SCRIPT_PATH}/init_primary.sh ${TARGET_SCRIPT_PATH} ${TARGET_USER} ${TARGET_PWD} ${DATA_PATH} $(get_local_ip_by_type "${CONFIG_PRIMARY_IP}" "${LOCAL_IP_LIST}") ${CONFIG_PRIMARY_PORT} ${CONFIG_PRIMARY_DB_SUBDIR} ${CONFIG_RS_NAME} ${TARGET_SCRIPT_PATH}/keyfile configsvr "" "" "${CONFIG_PRIMARY_IP}" || {
        echo "错误：config主节点 init_primary 失败" >&2
        exit 1
    }
else
    # 拷贝keyfile到远程
    scp_to_host ${CONFIG_PRIMARY_IP} ${KEYFILE_PATH} ${TARGET_SCRIPT_PATH}/keyfile 2>/dev/null
    ssh root@${CONFIG_PRIMARY_IP} "source ~/.bash_profile 2>/dev/null; sh ${TARGET_SCRIPT_PATH}/init_primary.sh ${TARGET_SCRIPT_PATH} ${TARGET_USER} ${TARGET_PWD} ${DATA_PATH} ${CONFIG_PRIMARY_IP} ${CONFIG_PRIMARY_PORT} ${CONFIG_PRIMARY_DB_SUBDIR} ${CONFIG_RS_NAME} ${TARGET_SCRIPT_PATH}/keyfile configsvr \"\" \"\" \"${CONFIG_PRIMARY_IP}\"" || {
        echo "错误：config主节点 ${CONFIG_PRIMARY_IP} init_primary 失败" >&2
        exit 1
    }
fi

echo "Config主节点初始化(no_auth → createUser → shutdown) 成功."

# 1.2 启动config主节点(带认证)
# create_mongodb_inst.sh 参数: SCRIPT_PATH LOCAL_TARGET_DB_PATH LOCAL_TARGET_IP LOCAL_TARGET_PORT LOCAL_REPL_SET_NAME LOCAL_KEYFILE_PATH LOCAL_SHARDING_CLUSTER_ROLE [LOCAL_IS_MONGOS] [LOCAL_MONGOS_CONFIG_ADDR] [LOCAL_IS_NO_AUTH]
# LOCAL_TARGET_DB_PATH: 完整数据目录路径
if is_local_host "${CONFIG_PRIMARY_IP}" "${LOCAL_IP_LIST}"; then
    sh ${TARGET_SCRIPT_PATH}/create_mongodb_inst.sh ${TARGET_SCRIPT_PATH} ${CONFIG_PRIMARY_DB_PATH} $(get_local_ip_by_type "${CONFIG_PRIMARY_IP}" "${LOCAL_IP_LIST}") ${CONFIG_PRIMARY_PORT} "${CONFIG_RS_NAME}" ${TARGET_SCRIPT_PATH}/keyfile configsvr || {
        echo "错误：config主节点带认证启动失败" >&2
        exit 1
    }
else
    ssh root@${CONFIG_PRIMARY_IP} "source ~/.bash_profile 2>/dev/null; sh ${TARGET_SCRIPT_PATH}/create_mongodb_inst.sh ${TARGET_SCRIPT_PATH} ${CONFIG_PRIMARY_DB_PATH} ${CONFIG_PRIMARY_IP} ${CONFIG_PRIMARY_PORT} \"${CONFIG_RS_NAME}\" ${TARGET_SCRIPT_PATH}/keyfile configsvr" || {
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
    node_db_subdir="${DB_NAME}_config_$(($i + 1))"
    node_db_path="${DATA_PATH}/${node_db_subdir}"

    echo "启动config从节点 [$i]: $(format_host_port ${ip} ${node_port}), 数据目录: ${node_db_path}..."

    # 启动从节点(带认证, 不需要no_auth)
    if is_local_host "${ip}" "${LOCAL_IP_LIST}"; then
        sh ${TARGET_SCRIPT_PATH}/create_mongodb_inst.sh ${TARGET_SCRIPT_PATH} ${node_db_path} $(get_local_ip_by_type "${ip}" "${LOCAL_IP_LIST}") ${node_port} "${CONFIG_RS_NAME}" ${TARGET_SCRIPT_PATH}/keyfile configsvr || {
            echo "错误：config从节点 $(format_host_port ${ip} ${node_port}) 启动失败" >&2
            exit 1
        }
    else
        ssh root@${ip} "source ~/.bash_profile 2>/dev/null; sh ${TARGET_SCRIPT_PATH}/create_mongodb_inst.sh ${TARGET_SCRIPT_PATH} ${node_db_path} ${ip} ${node_port} \"${CONFIG_RS_NAME}\" ${TARGET_SCRIPT_PATH}/keyfile configsvr" || {
            echo "错误：config从节点 $(format_host_port ${ip} ${node_port}) 启动失败" >&2
            exit 1
        }
    fi

    sleep 3

    # 从主节点添加该从节点到复制集
    # rs.add 中 host 使用 iplist 中的原始 IP/域名, 保证复制集成员注册地址一致
    register_host_port=$(format_host_port ${ip} ${node_port})
    echo "从主节点添加config从节点: ${register_host_port}..."
    if is_local_host "${CONFIG_PRIMARY_IP}" "${LOCAL_IP_LIST}"; then
        mongosh --host $(format_host_port $(get_local_ip_by_type "${CONFIG_PRIMARY_IP}" "${LOCAL_IP_LIST}") ${CONFIG_PRIMARY_PORT}) -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "rs.add({_id: ${i}, host: \"${register_host_port}\", priority: 1, votes: 1})" || {
            echo "错误：添加config从节点 ${register_host_port} 失败" >&2
            exit 1
        }
    else
        ssh root@${CONFIG_PRIMARY_IP} "source ~/.bash_profile 2>/dev/null; mongosh --host $(format_host_port ${CONFIG_PRIMARY_IP} ${CONFIG_PRIMARY_PORT}) -u \"${TARGET_USER}\" -p \"${TARGET_PWD}\" --authenticationDatabase admin --eval \"rs.add({_id: ${i}, host: \\\"${register_host_port}\\\", priority: 1, votes: 1})\"" || {
            echo "错误：添加config从节点 ${register_host_port} 失败" >&2
            exit 1
        }
    fi

    echo "Config从节点 ${register_host_port} 添加成功."
    sleep 2
done

# 验证config复制集状态
echo "验证config复制集状态..."
if is_local_host "${CONFIG_PRIMARY_IP}" "${LOCAL_IP_LIST}"; then
    mongosh --host $(format_host_port $(get_local_ip_by_type "${CONFIG_PRIMARY_IP}" "${LOCAL_IP_LIST}") ${CONFIG_PRIMARY_PORT}) -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "rs.status()" || {
        echo "警告：config复制集状态查询失败" >&2
    }
else
    ssh root@${CONFIG_PRIMARY_IP} "source ~/.bash_profile 2>/dev/null; mongosh --host $(format_host_port ${CONFIG_PRIMARY_IP} ${CONFIG_PRIMARY_PORT}) -u \"${TARGET_USER}\" -p \"${TARGET_PWD}\" --authenticationDatabase admin --eval \"rs.status()\"" || {
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

    SHARD_RS_NAME="${RS_PREFIX_MAP[$shard_name]}_${shard_name}"

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
    primary_db_subdir="${DB_NAME}_${shard_name}_1"
    primary_db_path="${DATA_PATH}/${primary_db_subdir}"

    echo "Shard ${shard_name} 主节点: $(format_host_port ${primary_ip} ${primary_port}), RS_NAME: ${SHARD_RS_NAME}"
    echo "Shard ${shard_name} 主节点数据目录: ${primary_db_path}"

    # 2.1 初始化shard主节点 (no_auth → rs.initiate → createUser → shutdown)
    # LOCAL_REGISTER_HOST 使用 iplist 中的原始 IP/域名, 保证复制集成员注册地址一致
    if is_local_host "${primary_ip}" "${LOCAL_IP_LIST}"; then
        sh ${TARGET_SCRIPT_PATH}/init_primary.sh ${TARGET_SCRIPT_PATH} ${TARGET_USER} ${TARGET_PWD} ${DATA_PATH} $(get_local_ip_by_type "${primary_ip}" "${LOCAL_IP_LIST}") ${primary_port} ${primary_db_subdir} ${SHARD_RS_NAME} ${TARGET_SCRIPT_PATH}/keyfile shardsvr "" "" "${primary_ip}" || {
            echo "错误：shard ${shard_name} 主节点 init_primary 失败" >&2
            exit 1
        }
    else
        scp_to_host ${primary_ip} ${KEYFILE_PATH} ${TARGET_SCRIPT_PATH}/keyfile 2>/dev/null
        ssh root@${primary_ip} "source ~/.bash_profile 2>/dev/null; sh ${TARGET_SCRIPT_PATH}/init_primary.sh ${TARGET_SCRIPT_PATH} ${TARGET_USER} ${TARGET_PWD} ${DATA_PATH} ${primary_ip} ${primary_port} ${primary_db_subdir} ${SHARD_RS_NAME} ${TARGET_SCRIPT_PATH}/keyfile shardsvr \"\" \"\" \"${primary_ip}\"" || {
            echo "错误：shard ${shard_name} 主节点 ${primary_ip} init_primary 失败" >&2
            exit 1
        }
    fi

    echo "Shard ${shard_name} 主节点初始化(no_auth → createUser → shutdown) 成功."

    # 2.2 启动shard主节点(带认证)
    if is_local_host "${primary_ip}" "${LOCAL_IP_LIST}"; then
        sh ${TARGET_SCRIPT_PATH}/create_mongodb_inst.sh ${TARGET_SCRIPT_PATH} ${primary_db_path} $(get_local_ip_by_type "${primary_ip}" "${LOCAL_IP_LIST}") ${primary_port} "${SHARD_RS_NAME}" ${TARGET_SCRIPT_PATH}/keyfile shardsvr || {
            echo "错误：shard ${shard_name} 主节点带认证启动失败" >&2
            exit 1
        }
    else
        ssh root@${primary_ip} "source ~/.bash_profile 2>/dev/null; sh ${TARGET_SCRIPT_PATH}/create_mongodb_inst.sh ${TARGET_SCRIPT_PATH} ${primary_db_path} ${primary_ip} ${primary_port} \"${SHARD_RS_NAME}\" ${TARGET_SCRIPT_PATH}/keyfile shardsvr" || {
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
        node_db_subdir="${DB_NAME}_${shard_name}_$(($j + 1))"
        node_db_path="${DATA_PATH}/${node_db_subdir}"

        echo "启动shard ${shard_name} 从节点 [$j]: $(format_host_port ${ip} ${node_port}), 数据目录: ${node_db_path}..."

        if is_local_host "${ip}" "${LOCAL_IP_LIST}"; then
            sh ${TARGET_SCRIPT_PATH}/create_mongodb_inst.sh ${TARGET_SCRIPT_PATH} ${node_db_path} $(get_local_ip_by_type "${ip}" "${LOCAL_IP_LIST}") ${node_port} "${SHARD_RS_NAME}" ${TARGET_SCRIPT_PATH}/keyfile shardsvr || {
                echo "错误：shard ${shard_name} 从节点 $(format_host_port ${ip} ${node_port}) 启动失败" >&2
                exit 1
            }
        else
            ssh root@${ip} "source ~/.bash_profile 2>/dev/null; sh ${TARGET_SCRIPT_PATH}/create_mongodb_inst.sh ${TARGET_SCRIPT_PATH} ${node_db_path} ${ip} ${node_port} \"${SHARD_RS_NAME}\" ${TARGET_SCRIPT_PATH}/keyfile shardsvr" || {
                echo "错误：shard ${shard_name} 从节点 $(format_host_port ${ip} ${node_port}) 启动失败" >&2
                exit 1
            }
        fi

        sleep 3

        # 从主节点添加该从节点到复制集
        # rs.add 中 host 使用 iplist 中的原始 IP/域名, 保证复制集成员注册地址一致
        register_host_port=$(format_host_port ${ip} ${node_port})
        echo "从主节点添加shard ${shard_name} 从节点: ${register_host_port}..."
        if is_local_host "${primary_ip}" "${LOCAL_IP_LIST}"; then
            mongosh --host $(format_host_port $(get_local_ip_by_type "${primary_ip}" "${LOCAL_IP_LIST}") ${primary_port}) -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "rs.add({_id: ${j}, host: \"${register_host_port}\", priority: 1, votes: 1})" || {
                echo "错误：添加shard ${shard_name} 从节点 ${register_host_port} 失败" >&2
                exit 1
            }
        else
            ssh root@${primary_ip} "source ~/.bash_profile 2>/dev/null; mongosh --host $(format_host_port ${primary_ip} ${primary_port}) -u \"${TARGET_USER}\" -p \"${TARGET_PWD}\" --authenticationDatabase admin --eval \"rs.add({_id: ${j}, host: \\\"${register_host_port}\\\", priority: 1, votes: 1})\"" || {
                echo "错误：添加shard ${shard_name} 从节点 ${register_host_port} 失败" >&2
                exit 1
            }
        fi

        echo "Shard ${shard_name} 从节点 ${register_host_port} 添加成功."
        sleep 2
    done

    # 构建该分片的地址串: rs_shard1/[host1]:port1,[host2]:port2,...
    # 使用 iplist 中的原始 IP/域名, 保证与复制集成员注册地址一致
    shard_addr_str="${SHARD_RS_NAME}/"
    max_idx=$((${#shard_nodes[@]} - 1))
    for j in "${!shard_nodes[@]}"; do
        node="${shard_nodes[$j]}"
        ip=$(echo "${node}" | awk '{print $1}')
        node_port=$(echo "${node}" | awk '{print $2}')
        shard_addr_str="${shard_addr_str}$(format_host_port ${ip} ${node_port})"
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

# 构建 configSvr 地址串: rs_config/[host1]:port1,[host2]:port2,[host3]:port3
# 使用 iplist 中的原始 IP/域名, 保证与 config 复制集成员注册地址一致
CONFIG_SVR_ADDR_STR="${CONFIG_RS_NAME}/"
max_config_idx=$((${#CONFIG_SVR_ARRAY[@]} - 1))
for i in "${!CONFIG_SVR_ARRAY[@]}"; do
    item="${CONFIG_SVR_ARRAY[$i]}"
    ip=$(echo "${item}" | awk '{print $1}')
    node_port=$(echo "${item}" | awk '{print $2}')
    CONFIG_SVR_ADDR_STR="${CONFIG_SVR_ADDR_STR}$(format_host_port ${ip} ${node_port})"
    if [ $i -ne $max_config_idx ]; then
        CONFIG_SVR_ADDR_STR="${CONFIG_SVR_ADDR_STR},"
    fi
done

echo "ConfigSvr 地址串: ${CONFIG_SVR_ADDR_STR}"

# 3.1 初始化每个mongos节点 (no_auth → createUser → shutdown)
# init_mongos.sh 参数: SCRIPT_PATH TARGET_USER TARGET_PWD LOCAL_REPLISET_INSTALL_PATH LOCAL_PRIMARY_IP LOCAL_PRIMARY_PORT LOCAL_DB_NAME LOCAL_RS_NAME LOCAL_KEYFILE_PATH LOCAL_MONGOS_CONFIG_ADDR
# 注意: init_mongos.sh 内部调用 create_mongos_inst.sh, 拼接 LOCAL_REPLISET_INSTALL_PATH/LOCAL_DB_NAME 作为数据目录
for i in "${!MONGOS_SVR_ARRAY[@]}"; do
    item="${MONGOS_SVR_ARRAY[$i]}"
    ip=$(echo "${item}" | awk '{print $1}')
    node_port=$(echo "${item}" | awk '{print $2}')
    mongos_db_subdir="${DB_NAME}_mongos_$(($i + 1))"
    mongos_db_path="${DATA_PATH}/${mongos_db_subdir}"

    echo "初始化mongos节点 [$i]: $(format_host_port ${ip} ${node_port}), 数据目录: ${mongos_db_path}..."

    if is_local_host "${ip}" "${LOCAL_IP_LIST}"; then
        sh ${TARGET_SCRIPT_PATH}/init_mongos.sh ${TARGET_SCRIPT_PATH} ${TARGET_USER} ${TARGET_PWD} ${DATA_PATH} $(get_local_ip_by_type "${ip}" "${LOCAL_IP_LIST}") ${node_port} ${mongos_db_subdir} "${CONFIG_RS_NAME}" ${TARGET_SCRIPT_PATH}/keyfile "${CONFIG_SVR_ADDR_STR}" || {
            echo "错误：mongos节点 $(format_host_port ${ip} ${node_port}) init_mongos 失败" >&2
            exit 1
        }
    else
        scp_to_host ${ip} ${KEYFILE_PATH} ${TARGET_SCRIPT_PATH}/keyfile 2>/dev/null
        ssh root@${ip} "source ~/.bash_profile 2>/dev/null; sh ${TARGET_SCRIPT_PATH}/init_mongos.sh ${TARGET_SCRIPT_PATH} ${TARGET_USER} \"${TARGET_PWD}\" ${DATA_PATH} ${ip} ${node_port} ${mongos_db_subdir} \"${CONFIG_RS_NAME}\" ${TARGET_SCRIPT_PATH}/keyfile \"${CONFIG_SVR_ADDR_STR}\"" || {
            echo "错误：mongos节点 $(format_host_port ${ip} ${node_port}) init_mongos 失败" >&2
            exit 1
        }
    fi

    echo "Mongos节点 $(format_host_port ${ip} ${node_port}) 初始化(no_auth → createUser → shutdown) 成功."
done

# 3.2 启动mongos节点(带认证)
# create_mongos_inst.sh 参数: SCRIPT_PATH TARGET_DB_PATH TARGET_PORT REPL_SET_NAME KEYFILE_PATH MONGOS_CONFIG_ADDR [IS_NO_AUTH]
for i in "${!MONGOS_SVR_ARRAY[@]}"; do
    item="${MONGOS_SVR_ARRAY[$i]}"
    ip=$(echo "${item}" | awk '{print $1}')
    node_port=$(echo "${item}" | awk '{print $2}')
    mongos_db_subdir="${DB_NAME}_mongos_$(($i + 1))"
    mongos_db_path="${DATA_PATH}/${mongos_db_subdir}"

    echo "启动mongos节点(带认证) [$i]: $(format_host_port ${ip} ${node_port})..."

    if is_local_host "${ip}" "${LOCAL_IP_LIST}"; then
        sh ${TARGET_SCRIPT_PATH}/create_mongos_inst.sh ${TARGET_SCRIPT_PATH} ${mongos_db_path} ${node_port} "${CONFIG_RS_NAME}" ${TARGET_SCRIPT_PATH}/keyfile "${CONFIG_SVR_ADDR_STR}" || {
            echo "错误：mongos节点 $(format_host_port ${ip} ${node_port}) 带认证启动失败" >&2
            exit 1
        }
    else
        ssh root@${ip} "source ~/.bash_profile 2>/dev/null; sh ${TARGET_SCRIPT_PATH}/create_mongos_inst.sh ${TARGET_SCRIPT_PATH} ${mongos_db_path} ${node_port} \"${CONFIG_RS_NAME}\" ${TARGET_SCRIPT_PATH}/keyfile \"${CONFIG_SVR_ADDR_STR}\"" || {
            echo "错误：mongos节点 $(format_host_port ${ip} ${node_port}) 带认证启动失败" >&2
            exit 1
        }
    fi

    echo "Mongos节点 $(format_host_port ${ip} ${node_port}) 带认证启动成功."
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

MONGOS_PRIMARY_IP=$(get_reachable_host "${MONGOS_PRIMARY_IP}" "$(get_local_ip_by_type "${MONGOS_PRIMARY_IP}" "${LOCAL_IP_LIST}")")

MONGOS_PRIMARY_HOST_PORT=$(format_host_port ${MONGOS_PRIMARY_IP} ${MONGOS_PRIMARY_PORT})
echo "使用mongos节点 ${MONGOS_PRIMARY_HOST_PORT} 执行addShard..."

sleep 5  # 等待mongos完全启动

for shard_name in "${SHARD_NAME_LIST[@]}"; do
    shard_addr="${SHARD_ADDR_STRINGS[$shard_name]}"
    echo "添加分片: sh.addShard(\"${shard_addr}\")..."

    if is_local_host "$(echo "${MONGOS_SVR_ARRAY[0]}" | awk '{print $1}')" "${LOCAL_IP_LIST}"; then
        mongosh --host ${MONGOS_PRIMARY_HOST_PORT} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "sh.addShard(\"${shard_addr}\")" || {
            echo "错误：添加分片 ${shard_name} 失败" >&2
            exit 1
        }
    else
        ssh root@$(echo "${MONGOS_SVR_ARRAY[0]}" | awk '{print $1}') "source ~/.bash_profile 2>/dev/null; mongosh --host ${MONGOS_PRIMARY_HOST_PORT} -u \"${TARGET_USER}\" -p \"${TARGET_PWD}\" --authenticationDatabase admin --eval \"sh.addShard(\\\"${shard_addr}\\\")\"" || {
            echo "错误：添加分片 ${shard_name} 失败" >&2
            exit 1
        }
    fi

    echo "分片 ${shard_name} 添加成功."
    sleep 3
done

echo "所有分片添加完成."

##############################
# 步骤5: 启用数据库分片
##############################
echo ""
echo "========== 步骤5: 启用数据库分片 =========="

echo "启用数据库分片: sh.enableSharding(\"${DB_NAME}\")..."

if is_local_host "$(echo "${MONGOS_SVR_ARRAY[0]}" | awk '{print $1}')" "${LOCAL_IP_LIST}"; then
    mongosh --host ${MONGOS_PRIMARY_HOST_PORT} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "sh.enableSharding(\"${DB_NAME}\")" || {
        echo "错误：启用数据库分片 ${DB_NAME} 失败" >&2
        exit 1
    }
else
    ssh root@$(echo "${MONGOS_SVR_ARRAY[0]}" | awk '{print $1}') "source ~/.bash_profile 2>/dev/null; mongosh --host ${MONGOS_PRIMARY_HOST_PORT} -u \"${TARGET_USER}\" -p \"${TARGET_PWD}\" --authenticationDatabase admin --eval \"sh.enableSharding(\\\"${DB_NAME}\\\")\"" || {
        echo "错误：启用数据库分片 ${DB_NAME} 失败" >&2
        exit 1
    }
fi

echo "数据库 ${DB_NAME} 分片已启用."
echo "提示: 分片键(shardCollection)属于业务逻辑, 请在业务初始化时通过 mongosh 手动设置."

##############################
# 完成
##############################
echo ""
echo "=========================================="
echo "MongoDB 分片集群搭建完成!"
echo "=========================================="
echo "Config 复制集: ${CONFIG_RS_NAME}"
echo "  主节点: $(format_host_port ${CONFIG_PRIMARY_IP} ${CONFIG_PRIMARY_PORT})"
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
    echo "  $(format_host_port ${ip} ${node_port})"
done
echo ""
echo "连接方式:"
echo "  mongosh --host $(format_host_port ${MONGOS_PRIMARY_IP} ${MONGOS_PRIMARY_PORT}) -u ${TARGET_USER} -p <密码> --authenticationDatabase admin"
echo "=========================================="
