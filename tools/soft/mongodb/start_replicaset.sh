#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 启动已存在的 MongoDB 复制集
# 用法: sh start_replicaset.sh <start_iplist.txt> <账号> <密码> <软件安装路径>
#
# start_iplist.txt 格式(4字段): 复制集名 IP地址/域名 端口 配置文件绝对路径
#   示例:
#     mydb_rs 192.168.1.1 27017 /root/mongo_data/mydb_rs_1/mongod.conf
#     mydb_rs 192.168.1.2 27017 /root/mongo_data/mydb_rs_2/mongod.conf
#     mydb_rs 192.168.1.3 27017 /root/mongo_data/mydb_rs_3/mongod.conf
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
echo "MongoDB 复制集启动脚本"
echo "=========================================="
echo "IP_LIST_FILE    : ${IP_LIST_FILE}"
echo "TARGET_USER     : ${TARGET_USER}"
echo "TARGET_PWD      : ******"
echo "INSTALL_PATH    : ${INSTALL_PATH}"
echo "=========================================="

##############################
# 本机IP地址获取
##############################
LOCAL_IP_LIST="127.0.0.1 ::1"
if check_internet; then
    LOCAL_IP_LIST=$(get_local_ip_list)
    echo "外网连接正常 LOCAL_IP_LIST:${LOCAL_IP_LIST}"
else
    LOCAL_IP_LIST="127.0.0.1 ::1"
    echo "无法连接外网 LOCAL_IP_LIST:${LOCAL_IP_LIST}"
fi

# LOCAL_IP: 从 LOCAL_IP_LIST 中选最优IP(优先公网IP)
LOCAL_IP=$(get_local_ip "${LOCAL_IP_LIST}")
echo "LOCAL_IP(用于子脚本): ${LOCAL_IP}"

# LOCAL_IPV4 / LOCAL_IPV6: 本机最优IPv4和IPv6地址
LOCAL_IPV4=$(get_local_ipv4 "${LOCAL_IP_LIST}")
LOCAL_IPV6=$(get_local_ipv6 "${LOCAL_IP_LIST}")
echo "LOCAL_IPV4: ${LOCAL_IPV4}, LOCAL_IPV6: ${LOCAL_IPV6}"

##############################
# 解析 start_iplist.txt, 获取复制集名和节点列表
##############################
RS_NAME=""                # 复制集名(从iplist读取, 所有行必须一致)
NODE_ARRAY=()             # 节点数组, 元素格式: "ip port conf_path"

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

# 解析每行
for index in "${!IP_LIST_ARRAY[@]}"; do
    elem="${IP_LIST_ARRAY[$index]}"

    if [ -z "${elem}" ] || [[ "$elem" =~ ^[[:space:]]*$ ]]; then
        continue
    fi
    trimmed=$(echo "${elem}" | sed 's/^[[:space:]]*//')
    if [[ "$trimmed" =~ ^# ]]; then
        continue
    fi

    fields=($(echo "${elem}" | awk '{print $1, $2, $3, $4}'))
    rs_name="${fields[0]}"
    ip="${fields[1]}"
    node_port="${fields[2]}"
    conf_path="${fields[3]}"

    if [ -z "${rs_name}" ] || [ -z "${ip}" ] || [ -z "${node_port}" ] || [ -z "${conf_path}" ]; then
        echo "警告：跳过无效行(需要4字段: 复制集名 IP 端口 配置文件路径): ${elem}"
        continue
    fi

    # 校验复制集名一致性
    if [ -z "${RS_NAME}" ]; then
        RS_NAME="${rs_name}"
    else
        if [ "${RS_NAME}" != "${rs_name}" ]; then
            echo "错误：复制集名不一致: 已有 '${RS_NAME}', 当前行 '${rs_name}'" >&2
            echo "提示: start_iplist.txt 中所有行的复制集名必须一致" >&2
            exit 1
        fi
    fi

    # 校验配置文件是否存在(本地节点)
    if is_local_host "${ip}" "${LOCAL_IP_LIST}"; then
        if [ ! -f "${conf_path}" ]; then
            echo "错误：配置文件不存在: ${conf_path}"
            exit 1
        fi
    else
        # 远程主机的配置文件校验在启动时进行
        echo "注意：远程节点 ${ip} 的配置文件将在启动时校验"
    fi

    NODE_ARRAY+=("${ip} ${node_port} ${conf_path}")
    echo "解析节点: rs_name=${rs_name}, ip=${ip}, port=${node_port}, conf=${conf_path}"
done

if [ ${#NODE_ARRAY[@]} -eq 0 ]; then
    echo "错误：start_iplist.txt 中没有有效节点"
    exit 1
fi

# 打印节点列表
echo ""
echo "========== 复制集节点列表 =========="
echo "复制集名: ${RS_NAME}"
echo "节点数量: ${#NODE_ARRAY[@]}"
for i in "${!NODE_ARRAY[@]}"; do
    item="${NODE_ARRAY[$i]}"
    ip=$(echo "${item}" | awk '{print $1}')
    node_port=$(echo "${item}" | awk '{print $2}')
    conf_path=$(echo "${item}" | awk '{print $3}')
    echo "  [$i] $(format_host_port ${ip} ${node_port})  conf: ${conf_path}"
done
echo "===================================="

##############################
# 辅助函数: 判断目标机器是否已安装 mongodb
##############################
is_mongo_installed() {
    local ip=$1
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
# 辅助函数: 启动 mongod 节点
##############################
start_mongod_node() {
    local ip=$1
    local port=$2
    local conf_path=$3

    echo "  启动 mongod: $(format_host_port ${ip} ${port}), conf: ${conf_path}"

    if is_local_host "${ip}" "${LOCAL_IP_LIST}"; then
        # 检查是否已运行
        if is_process_running "${ip}" "${port}"; then
            echo "  mongod $(format_host_port ${ip} ${port}) 已在运行，跳过"
            return 0
        fi

        # 检查配置文件
        if [ ! -f "${conf_path}" ]; then
            echo "错误：配置文件不存在: ${conf_path}" >&2
            return 1
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

        # 检查远程配置文件
        if ! ssh root@${ip} "test -f '${conf_path}'"; then
            echo "错误：远程 ${ip} 配置文件不存在: ${conf_path}" >&2
            return 1
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
# 环境准备: 检查本机mongo安装
##############################
echo ""
echo "========== 环境准备 =========="

# 需要初始化的IP集合(去重, 仅本机)
declare -A need_init_ips
for elem in "${NODE_ARRAY[@]}"; do
    ip=$(echo "${elem}" | awk '{print $1}')
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

# 验证 mongosh 命令可用
if ! command -v mongosh > /dev/null 2>&1; then
    echo "错误：mongosh 命令不可用, 请检查 mongodb 是否正确安装"
    exit 1
fi
echo "mongosh 命令可用."

echo "=================================="

##############################
# 步骤1: 启动复制集各节点
##############################
echo ""
echo "========== 步骤1: 启动复制集节点 =========="

for i in "${!NODE_ARRAY[@]}"; do
    item="${NODE_ARRAY[$i]}"
    ip=$(echo "${item}" | awk '{print $1}')
    node_port=$(echo "${item}" | awk '{print $2}')
    conf_path=$(echo "${item}" | awk '{print $3}')

    echo ""
    echo "--- 启动节点 [$i]: $(format_host_port ${ip} ${node_port}) ---"

    start_mongod_node "${ip}" "${node_port}" "${conf_path}"
    if [ $? -ne 0 ]; then
        echo "错误：节点 $(format_host_port ${ip} ${node_port}) 启动失败" >&2
        exit 1
    fi

    echo "节点 $(format_host_port ${ip} ${node_port}) 启动成功."
    sleep 2
done

echo "所有复制集节点已启动."

##############################
# 等待复制集稳定并验证
##############################
echo ""
echo "========== 等待复制集稳定 =========="
echo "等待 10 秒让复制集完成选举..."
sleep 10

##############################
# 验证复制集状态
##############################
echo ""
echo "========== 验证复制集状态 =========="

# 取第一个节点连接验证
VERIFY_NODE="${NODE_ARRAY[0]}"
VERIFY_IP=$(echo "${VERIFY_NODE}" | awk '{print $1}')
VERIFY_PORT=$(echo "${VERIFY_NODE}" | awk '{print $2}')

echo "验证复制集 ${RS_NAME}..."
echo "连接节点: $(format_host_port ${VERIFY_IP} ${VERIFY_PORT})"

if is_local_host "${VERIFY_IP}" "${LOCAL_IP_LIST}"; then
    mongosh --host $(format_host_port $(get_local_ip_by_type "${VERIFY_IP}" "${LOCAL_IP_LIST}") ${VERIFY_PORT}) -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "rs.status()" --quiet 2>/dev/null | head -30 || echo "警告：无法验证复制集状态(节点可能还不是主节点, 请稍后重试)"
else
    ssh root@${VERIFY_IP} "source ~/.bash_profile 2>/dev/null; mongosh --host $(format_host_port ${VERIFY_IP} ${VERIFY_PORT}) -u \"${TARGET_USER}\" -p \"${TARGET_PWD}\" --authenticationDatabase admin --eval \"rs.status()\" --quiet 2>/dev/null | head -30" || echo "警告：无法验证复制集状态(节点可能还不是主节点, 请稍后重试)"
fi

##############################
# 完成
##############################
echo ""
echo "=========================================="
echo "MongoDB 复制集启动完成!"
echo "=========================================="
echo "复制集名: ${RS_NAME}"
echo ""
echo "启动节点:"
for i in "${!NODE_ARRAY[@]}"; do
    item="${NODE_ARRAY[$i]}"
    ip=$(echo "${item}" | awk '{print $1}')
    node_port=$(echo "${item}" | awk '{print $2}')
    echo "  [节点 $i] $(format_host_port ${ip} ${node_port})"
done
echo ""
echo "连接方式:"
echo "  mongosh --host $(format_host_port ${VERIFY_IP} ${VERIFY_PORT}) -u ${TARGET_USER} -p <密码> --authenticationDatabase admin"
echo ""
echo "查看复制集状态:"
echo "  mongosh --host $(format_host_port ${VERIFY_IP} ${VERIFY_PORT}) -u ${TARGET_USER} -p <密码> --authenticationDatabase admin --eval \"rs.status()\""
echo ""
echo "如需关闭复制集, 请执行:"
echo "  sh stop_replicaset.sh ${IP_LIST_FILE} ${TARGET_USER} ${TARGET_PWD}"
echo "=========================================="
