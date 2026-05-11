#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 启动已存在的 MongoDB 分片集群
# 用法: sh start_mongo_shard_cluster.sh <start_iplist.txt> <账号> <密码> <软件安装路径>
#
# start_iplist.txt 格式: 节点类型 复制集前缀 IP 端口 配置文件绝对路径
#   支持的类型: config, shard1, shard2, ..., mongos
#   示例:
#     config testsuit_rs 127.0.0.1 27010 /root/mongo_data/mydb_config_1/mongod.conf
#     shard1 testsuit_rs 127.0.0.1 27011 /root/mongo_data/mydb_shard1_1/mongod.conf
#     shard1 testsuit_rs 127.0.0.1 27012 /root/mongo_data/mydb_shard1_2/mongod.conf
#     mongos testsuit_rs 127.0.0.1 27017 /root/mongo_data/mydb_mongos_1/mongos.conf
#
# IP_LIST_FILE: start_iplist.txt 路径
# TARGET_USER: 管理员用户名
# TARGET_PWD: 管理员密码
# INSTALL_PATH: MongoDB 软件安装目录

# 当前脚本路径
SCRIPT_PATH="$(cd $(dirname $0); pwd)"

# 参数定义
IP_LIST_FILE=$1
TARGET_USER=$2
TARGET_PWD=$3
INSTALL_PATH=$4

##############################
# 参数校验
##############################
if [ ! -e "${IP_LIST_FILE}" ]; then
    echo "错误：IP_LIST_FILE:${IP_LIST_FILE} 不存在"
    exit 1
fi

if [ -z "${TARGET_USER}" ]; then
    echo "错误：TARGET_USER 为空"
    exit 1
fi

if [ -z "${TARGET_PWD}" ]; then
    echo "错误：TARGET_PWD 为空"
    exit 1
fi

if [ -z "${INSTALL_PATH}" ]; then
    echo "错误：INSTALL_PATH 为空"
    exit 1
fi

# 加载公共函数库
. ${SCRIPT_PATH}/common/common_define.sh
. ${SCRIPT_PATH}/common/funcs.sh

echo "=========================================="
echo "MongoDB 分片集群启动脚本"
echo "=========================================="
echo "IP_LIST_FILE    : ${IP_LIST_FILE}"
echo "TARGET_USER     : ${TARGET_USER}"
echo "TARGET_PWD      : ******"
echo "INSTALL_PATH    : ${INSTALL_PATH}"
echo "=========================================="

##############################
# 解析 start_iplist.txt, 按类型分组
##############################
declare -A SHARD_GROUPS    # key=shard名, value=节点列表(分号分隔: "ip port conf_path;ip port conf_path")
CONFIG_SVR_ARRAY=()        # config节点数组, 元素格式: "ip port conf_path"
MONGOS_SVR_ARRAY=()        # mongos节点数组, 元素格式: "ip port conf_path"
SHARD_NAME_LIST=()         # 分片名有序列表

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

# LOCAL_IPV4 / LOCAL_IPV6: 本机最优IPv4和IPv6地址
LOCAL_IPV4=$(get_local_ipv4 "${LOCAL_IP_LIST}")
LOCAL_IPV6=$(get_local_ipv6 "${LOCAL_IP_LIST}")
echo "LOCAL_IPV4: ${LOCAL_IPV4}, LOCAL_IPV6: ${LOCAL_IPV6}"

# 读取iplist
IP_LIST_ARRAY=()
read_file_to_array ${IP_LIST_FILE} IP_LIST_ARRAY || {
    echo "错误：read_file_to_array fail IP_LIST_FILE: ${IP_LIST_FILE} 失败" >&2
    exit 1
}

if [ ${#IP_LIST_ARRAY[@]} -eq 0 ]; then
    echo "错误：IP_LIST_ARRAY ip列表是空的"
    exit 1
fi

# 解析每行, 按类型分组
for index in "${!IP_LIST_ARRAY[@]}"; do
    elem="${IP_LIST_ARRAY[$index]}"

    if [ -z "${elem}" ] || [[ "$elem" =~ ^[[:space:]]*$ ]]; then
        continue
    fi
    trimmed=$(echo "${elem}" | sed 's/^[[:space:]]*//')
    if [[ "$trimmed" =~ ^# ]]; then
        continue
    fi

    fields=($(echo "${elem}" | awk '{print $1, $2, $3, $4, $5}'))
    node_type="${fields[0]}"
    rs_prefix="${fields[1]}"
    ip="${fields[2]}"
    node_port="${fields[3]}"
    conf_path="${fields[4]}"

    if [ -z "${node_type}" ] || [ -z "${rs_prefix}" ] || [ -z "${ip}" ] || [ -z "${node_port}" ] || [ -z "${conf_path}" ]; then
        echo "警告：跳过无效行(需要5字段: 类型 复制集前缀 IP 端口 配置文件路径): ${elem}"
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

    # 校验配置文件是否存在
    if is_local_host "${ip}" "${LOCAL_IP_LIST}"; then
        if [ ! -f "${conf_path}" ]; then
            echo "错误：配置文件不存在: ${conf_path}"
            exit 1
        fi
    else
        # 远程主机的配置文件校验在启动时进行
        echo "注意：远程节点 ${ip} 的配置文件将在启动时校验"
    fi

    echo "解析节点: type=${node_type}, rs_prefix=${rs_prefix}, ip=${ip}, port=${node_port}, conf=${conf_path}"

    if [ "${node_type}" = "config" ]; then
        CONFIG_SVR_ARRAY+=("${ip} ${node_port} ${conf_path}")
    elif [ "${node_type}" = "mongos" ]; then
        MONGOS_SVR_ARRAY+=("${ip} ${node_port} ${conf_path}")
    elif [[ "${node_type}" =~ ^shard[0-9]+$ ]]; then
        if [ -z "${SHARD_GROUPS[$node_type]}" ]; then
            SHARD_GROUPS[$node_type]="${ip} ${node_port} ${conf_path}"
            SHARD_NAME_LIST+=("${node_type}")
        else
            SHARD_GROUPS[$node_type]="${SHARD_GROUPS[$node_type]};${ip} ${node_port} ${conf_path}"
        fi
    else
        echo "错误：未知的节点类型: ${node_type}, 支持的类型: config, mongos, shard1, shard2, ..."
        exit 1
    fi
done

# 校验
if [ ${#CONFIG_SVR_ARRAY[@]} -eq 0 ]; then
    echo "错误：start_iplist.txt中没有config节点"
    exit 1
fi

if [ ${#SHARD_NAME_LIST[@]} -eq 0 ]; then
    echo "错误：start_iplist.txt中没有shard节点"
    exit 1
fi

if [ ${#MONGOS_SVR_ARRAY[@]} -eq 0 ]; then
    echo "错误：start_iplist.txt中没有mongos节点"
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
    IFS=';' read -ra shard_nodes <<< "${SHARD_GROUPS[$shard_name]}"
    echo "  ${shard_name} (${#shard_nodes[@]}个): ${SHARD_GROUPS[$shard_name]}"
done

echo "Mongos 节点 (${#MONGOS_SVR_ARRAY[@]}个):"
for i in "${!MONGOS_SVR_ARRAY[@]}"; do
    echo "  [$i] ${MONGOS_SVR_ARRAY[$i]}"
done
echo "=================================="

##############################
# 辅助函数: 在指定机器上执行命令
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
# 辅助函数: 判断目标机器是否已安装 mongodb
##############################
is_mongo_installed() {
    local ip=$1
    # 移除 INSTALL_PATH 末尾的斜杠
    local INSTALL_PATH_CLEAN=${INSTALL_PATH%/}
    
    if is_local_host "${ip}" "${LOCAL_IP_LIST}"; then
        if which mongod &>/dev/null && [ -x "$(which mongod)" ]; then
            return 0
        fi
        if [ -d "${INSTALL_PATH_CLEAN}" ] && [ -x "${INSTALL_PATH_CLEAN}/mongodb-linux-x86_64-rhel88-8.0.6/bin/mongod" ]; then
            return 0
        fi
        return 1
    else
        if ssh root@${ip} "which mongod &>/dev/null && [ -x \"\$(which mongod)\""; then
            return 0
        fi
        if ssh root@${ip} "[ -d '${INSTALL_PATH_CLEAN}' ] && [ -x '${INSTALL_PATH_CLEAN}/mongodb-linux-x86_64-rhel88-8.0.6/bin/mongod' ]"; then
            return 0
        fi
        return 1
    fi
}

##############################
# 辅助函数: 检查进程是否已运行
##############################
is_process_running() {
    local ip=$1
    local port=$2
    local proc_type=$3  # mongod 或 mongos

    if is_local_host "${ip}" "${LOCAL_IP_LIST}"; then
        local pid=$(ss -tlnp "sport = :${port}" 2>/dev/null | grep -oP 'pid=\K[0-9]+' | head -1)
        if [ -n "${pid}" ]; then
            return 0
        fi
        return 1
    else
        if ssh root@${ip} "source ~/.bash_profile 2>/dev/null; ss -tlnp 'sport = :${port}' 2>/dev/null | grep -oP 'pid=\K[0-9]+' | head -1" | grep -qE '^[0-9]+'; then
            return 0
        fi
        return 1
    fi
}

##############################
# 辅助函数: 获取复制集名称
##############################
get_repl_set_name() {
    local conf_path=$1
    if [ -f "${conf_path}" ]; then
        grep "replSet" "${conf_path}" | awk -F: '{print $2}' | tr -d ' '
    fi
}

##############################
# 环境准备: 检查本机mongo安装
##############################
echo ""
echo "========== 环境准备 =========="

# 需要初始化的IP集合(去重)
declare -A need_init_ips
for elem in "${IP_LIST_ARRAY[@]}"; do
    trimmed=$(echo "${elem}" | sed 's/^[[:space:]]*//')
    if [[ "$trimmed" =~ ^# ]] || [ -z "${trimmed}" ]; then
        continue
    fi
    fields=($(echo "${trimmed}" | awk '{print $1, $2, $3, $4, $5}'))
    ip="${fields[2]}"
    
    # 只处理本机节点
    if is_local_host "${ip}" "${LOCAL_IP_LIST}"; then
        need_init_ips[$ip]=1
    fi
done

# 处理本机节点
for ip in "${!need_init_ips[@]}"; do
    echo "检查本机 ${ip} MongoDB 安装状态..."
    
    if is_mongo_installed "${ip}"; then
        echo "  ${ip} 已安装 MongoDB"
    else
        echo "  ${ip} 未安装 MongoDB，执行安装..."
        
        # 检查安装包是否存在
        TARGET_SCRIPT_PATH=/root/mongodb_script
        TMP_DIR=/root/build_mongo_temp
        TGZ_FILE_NAME=mongodb.tar.gz

        if [ ! -f "${SCRIPT_PATH}/${TGZ_FILE_NAME}" ]; then
            echo "错误：本机安装包不存在: ${SCRIPT_PATH}/${TGZ_FILE_NAME}" >&2
            exit 1
        fi

        sh ${SCRIPT_PATH}/pack_tar.sh ${TMP_DIR} ${SCRIPT_PATH} ${TGZ_FILE_NAME} || {
            echo "错误：pack_tar.sh 失败" >&2
            exit 1
        }
        echo "打包 TGZ_FILE_NAME:${TGZ_FILE_NAME} 成功"

        . ${SCRIPT_PATH}/init_package.sh ${TMP_DIR}/${TGZ_FILE_NAME} ${TARGET_SCRIPT_PATH}
        . ${TARGET_SCRIPT_PATH}/init_env.sh ${TARGET_SCRIPT_PATH} ${INSTALL_PATH} || {
            echo "错误：本机 ${TARGET_SCRIPT_PATH}/init_env.sh 失败" >&2
            exit 1
        }

        echo "${ip} MongoDB 安装完成"
    fi
done

# source 环境变量
source ~/.bash_profile 2>/dev/null

echo "=================================="

##############################
# 辅助函数: 启动 mongod 节点
##############################
start_mongod_node() {
    local ip=$1
    local port=$2
    local conf_path=$3
    
    echo "  启动 mongod: $(format_host_port ${ip} ${port}), conf: ${conf_path}"
    
    if is_local_host "${ip}" "${LOCAL_IP_LIST}"; then
        # 检查是否已运行
        if is_process_running "${ip}" "${port}" "mongod"; then
            echo "  mongod $(format_host_port ${ip} ${port}) 已在运行，跳过"
            return 0
        fi
        
        # 启动 mongod
        mongod -f "${conf_path}"
        if [ $? -ne 0 ]; then
            echo "错误：mongod 启动失败" >&2
            return 1
        fi
    else
        # 远程启动
        if ssh root@${ip} "source ~/.bash_profile 2>/dev/null; ss -tlnp 'sport = :${port}' 2>/dev/null | grep -q ':${port}'"; then
            echo "  mongod $(format_host_port ${ip} ${port}) 已在运行，跳过"
            return 0
        fi
        
        ssh root@${ip} "source ~/.bash_profile 2>/dev/null; mongod -f '${conf_path}'"
        if [ $? -ne 0 ]; then
            echo "错误：远程 ${ip} mongod 启动失败" >&2
            return 1
        fi
    fi
    
    return 0
}

##############################
# 辅助函数: 启动 mongos 节点
##############################
start_mongos_node() {
    local ip=$1
    local port=$2
    local conf_path=$3
    
    echo "  启动 mongos: $(format_host_port ${ip} ${port}), conf: ${conf_path}"
    
    if is_local_host "${ip}" "${LOCAL_IP_LIST}"; then
        # 检查是否已运行
        if is_process_running "${ip}" "${port}" "mongos"; then
            echo "  mongos $(format_host_port ${ip} ${port}) 已在运行，跳过"
            return 0
        fi
        
        # 启动 mongos
        mongos -f "${conf_path}"
        if [ $? -ne 0 ]; then
            echo "错误：mongos 启动失败" >&2
            return 1
        fi
    else
        # 远程启动
        if ssh root@${ip} "source ~/.bash_profile 2>/dev/null; ss -tlnp 'sport = :${port}' 2>/dev/null | grep -q ':${port}'"; then
            echo "  mongos $(format_host_port ${ip} ${port}) 已在运行，跳过"
            return 0
        fi
        
        ssh root@${ip} "source ~/.bash_profile 2>/dev/null; mongos -f '${conf_path}'"
        if [ $? -ne 0 ]; then
            echo "错误：远程 ${ip} mongos 启动失败" >&2
            return 1
        fi
    fi
    
    return 0
}

##############################
# 步骤1: 启动 config 节点
##############################
echo ""
echo "========== 步骤1: 启动 config 配置节点 =========="

# 先启动 config 主节点
echo "启动 config 主节点..."
CONFIG_PRIMARY_ITEM="${CONFIG_SVR_ARRAY[0]}"
CONFIG_PRIMARY_IP=$(echo "${CONFIG_PRIMARY_ITEM}" | awk '{print $1}')
CONFIG_PRIMARY_PORT=$(echo "${CONFIG_PRIMARY_ITEM}" | awk '{print $2}')
CONFIG_PRIMARY_CONF=$(echo "${CONFIG_PRIMARY_ITEM}" | awk '{print $3}')

start_mongod_node "${CONFIG_PRIMARY_IP}" "${CONFIG_PRIMARY_PORT}" "${CONFIG_PRIMARY_CONF}"
CONFIG_PRIMARY_PID=$?

if [ ${CONFIG_PRIMARY_PID} -ne 0 ]; then
    echo "错误：config 主节点启动失败" >&2
    exit 1
fi

echo "等待 config 主节点就绪..."
sleep 5

# 启动 config 从节点
for i in "${!CONFIG_SVR_ARRAY[@]}"; do
    if [ $i -eq 0 ]; then
        continue  # 跳过主节点
    fi
    
    item="${CONFIG_SVR_ARRAY[$i]}"
    ip=$(echo "${item}" | awk '{print $1}')
    node_port=$(echo "${item}" | awk '{print $2}')
    conf_path=$(echo "${item}" | awk '{print $3}')
    
    start_mongod_node "${ip}" "${node_port}" "${conf_path}"
    if [ $? -ne 0 ]; then
        echo "错误：config 从节点 $(format_host_port ${ip} ${node_port}) 启动失败" >&2
        exit 1
    fi
    
    sleep 2
done

echo "所有 config 节点已启动."

##############################
# 步骤2: 启动 shard 分片节点
##############################
echo ""
echo "========== 步骤2: 启动 shard 分片节点 =========="

for shard_name in "${SHARD_NAME_LIST[@]}"; do
    echo ""
    echo "--- 启动分片: ${shard_name} ---"
    
    SHARD_RS_NAME="${RS_PREFIX_MAP[$shard_name]}_${shard_name}"
    IFS=';' read -ra shard_nodes <<< "${SHARD_GROUPS[$shard_name]}"
    
    if [ ${#shard_nodes[@]} -eq 0 ]; then
        echo "警告：分片 ${shard_name} 没有节点, 跳过"
        continue
    fi
    
    # 先启动主节点
    primary_node="${shard_nodes[0]}"
    primary_ip=$(echo "${primary_node}" | awk '{print $1}')
    primary_port=$(echo "${primary_node}" | awk '{print $2}')
    primary_conf=$(echo "${primary_node}" | awk '{print $3}')
    
    echo "启动 ${shard_name} 主节点: $(format_host_port ${primary_ip} ${primary_port})"
    start_mongod_node "${primary_ip}" "${primary_port}" "${primary_conf}"
    if [ $? -ne 0 ]; then
        echo "错误：${shard_name} 主节点启动失败" >&2
        exit 1
    fi
    
    echo "等待 ${shard_name} 主节点就绪..."
    sleep 5
    
    # 启动从节点
    for j in "${!shard_nodes[@]}"; do
        if [ $j -eq 0 ]; then
            continue  # 跳过主节点
        fi
        
        node="${shard_nodes[$j]}"
        ip=$(echo "${node}" | awk '{print $1}')
        node_port=$(echo "${node}" | awk '{print $2}')
        conf_path=$(echo "${node}" | awk '{print $3}')
        
        start_mongod_node "${ip}" "${node_port}" "${conf_path}"
        if [ $? -ne 0 ]; then
            echo "错误：${shard_name} 从节点 $(format_host_port ${ip} ${node_port}) 启动失败" >&2
            exit 1
        fi
        
        sleep 2
    done
    
    echo "分片 ${shard_name} 所有节点已启动."
done

echo "所有 shard 分片节点已启动."

##############################
# 步骤3: 启动 mongos 路由节点
##############################
echo ""
echo "========== 步骤3: 启动 mongos 路由节点 =========="

for i in "${!MONGOS_SVR_ARRAY[@]}"; do
    item="${MONGOS_SVR_ARRAY[$i]}"
    ip=$(echo "${item}" | awk '{print $1}')
    node_port=$(echo "${item}" | awk '{print $2}')
    conf_path=$(echo "${item}" | awk '{print $3}')
    
    start_mongos_node "${ip}" "${node_port}" "${conf_path}"
    if [ $? -ne 0 ]; then
        echo "错误：mongos 节点 $(format_host_port ${ip} ${node_port}) 启动失败" >&2
        exit 1
    fi
    
    sleep 2
done

echo "所有 mongos 路由节点已启动."

##############################
# 等待集群稳定并验证
##############################
echo ""
echo "========== 等待集群稳定 =========="
echo "等待 10 秒让集群完成初始化..."
sleep 10

##############################
# 验证集群状态
##############################
echo ""
echo "========== 验证集群状态 =========="

# 验证 config 复制集
echo "验证 config 复制集..."
CONFIG_PRIMARY_IP=$(echo "${CONFIG_SVR_ARRAY[0]}" | awk '{print $1}')
CONFIG_PRIMARY_PORT=$(echo "${CONFIG_SVR_ARRAY[0]}" | awk '{print $2}')

CONFIG_RS_NAME="${RS_PREFIX_MAP[config]}_config"
echo "Config 复制集名称: ${CONFIG_RS_NAME}"
mongosh --host $(format_host_port ${CONFIG_PRIMARY_IP} ${CONFIG_PRIMARY_PORT}) -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "rs.status()" --quiet 2>/dev/null | head -20 || echo "警告：无法验证 config 复制集状态"

echo ""

# 验证 shard 复制集
for shard_name in "${SHARD_NAME_LIST[@]}"; do
    echo "验证 ${shard_name} 复制集..."
    SHARD_RS_NAME="${RS_PREFIX_MAP[$shard_name]}_${shard_name}"
    echo "${shard_name} 复制集名称: ${SHARD_RS_NAME}"
    
    # 找到该分片的任意一个节点
    IFS=';' read -ra shard_nodes <<< "${SHARD_GROUPS[$shard_name]}"
    first_node="${shard_nodes[0]}"
    shard_ip=$(echo "${first_node}" | awk '{print $1}')
    shard_port=$(echo "${first_node}" | awk '{print $2}')
    
    mongosh --host $(format_host_port ${shard_ip} ${shard_port}) -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "rs.status()" --quiet 2>/dev/null | head -20 || echo "警告：无法验证 ${shard_name} 复制集状态"
    echo ""
done

# 验证 mongos
echo "验证 mongos 连接..."
if [ ${#MONGOS_SVR_ARRAY[@]} -gt 0 ]; then
    MONGOS_ITEM="${MONGOS_SVR_ARRAY[0]}"
    MONGOS_IP=$(echo "${MONGOS_ITEM}" | awk '{print $1}')
    MONGOS_PORT=$(echo "${MONGOS_ITEM}" | awk '{print $2}')
    
    mongosh --host $(format_host_port ${MONGOS_IP} ${MONGOS_PORT}) -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "sh.status()" --quiet 2>/dev/null | head -30 || echo "警告：无法验证分片集群状态"
fi

##############################
# 完成
##############################
echo ""
echo "=========================================="
echo "MongoDB 分片集群启动完成!"
echo "=========================================="
echo "启动顺序:"
echo "  1. config 配置节点 (${#CONFIG_SVR_ARRAY[@]}个)"
echo "  2. shard 分片节点 (${#SHARD_NAME_LIST[@]}个分片)"
echo "  3. mongos 路由节点 (${#MONGOS_SVR_ARRAY[@]}个)"
echo ""
echo "如需关闭集群, 请执行:"
echo "  sh stop_mongo_shard_cluster.sh ${IP_LIST_FILE} ${TARGET_USER} ${TARGET_PWD}"
echo "=========================================="
