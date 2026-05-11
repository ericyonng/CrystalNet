#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 向已有MongoDB复制集扩展节点
# 用法: sh add_replica_node.sh <iplist.txt> <用户名> <密码> <软件包安装路径> <数据库数据路径> <keyfile绝对路径> <mongod主节点地址> <mongod主节点端口>
#
# iplist.txt 格式: 类型 复制集前缀 IP 端口
#   支持的类型: shard1, shard2, ..., config
#   示例:
#     shard1 testsuit_rs 192.168.1.10 27011
#     shard1 testsuit_rs 192.168.1.11 27011
#
# IP_LIST_FILE: 节点IP列表文件
# TARGET_USER: 已有复制集的管理员用户名
# TARGET_PWD: 已有复制集的管理员密码
# INSTALL_PATH: mongodb程序目录
# DATA_PATH: mongodb数据库数据目录
# KEYFILE_PATH: 已有复制集的keyfile绝对路径
# MONGOD_PRIMARY_ADDR: 复制集主节点地址（IP/域名）
# MONGOD_PRIMARY_PORT: 复制集主节点端口

# 当前脚本路径
SCRIPT_PATH="$(cd $(dirname $0); pwd)"

# 参数定义
IP_LIST_FILE=$1
TARGET_USER=$2
TARGET_PWD=$3
INSTALL_PATH=$4
DATA_PATH=$5
KEYFILE_PATH=$6
MONGOD_PRIMARY_ADDR=$7
MONGOD_PRIMARY_PORT=$8

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

if [ -z "${DATA_PATH}" ]; then
    echo "错误：DATA_PATH 为空"
    exit 1
fi

if [ -z "${KEYFILE_PATH}" ]; then
    echo "错误：KEYFILE_PATH 为空"
    exit 1
fi

if [ ! -e "${KEYFILE_PATH}" ]; then
    echo "错误：keyfile ${KEYFILE_PATH} 不存在"
    exit 1
fi

if [ -z "${MONGOD_PRIMARY_ADDR}" ]; then
    echo "错误：MONGOD_PRIMARY_ADDR 为空"
    exit 1
fi

if [ -z "${MONGOD_PRIMARY_PORT}" ]; then
    echo "错误：MONGOD_PRIMARY_PORT 为空"
    exit 1
fi

# 加载公共函数库
. ${SCRIPT_PATH}/common/common_define.sh
. ${SCRIPT_PATH}/common/funcs.sh

echo "=========================================="
echo "MongoDB 复制集扩展节点脚本"
echo "=========================================="
echo "IP_LIST_FILE       : ${IP_LIST_FILE}"
echo "TARGET_USER        : ${TARGET_USER}"
echo "TARGET_PWD         : ******"
echo "INSTALL_PATH       : ${INSTALL_PATH}"
echo "DATA_PATH          : ${DATA_PATH}"
echo "KEYFILE_PATH       : ${KEYFILE_PATH}"
echo "MONGOD_PRIMARY_ADDR: ${MONGOD_PRIMARY_ADDR}"
echo "MONGOD_PRIMARY_PORT: ${MONGOD_PRIMARY_PORT}"
echo "=========================================="

##############################
# 解析 iplist.txt 获取新节点信息
##############################
NEW_NODE_ARRAY=()      # 节点数组, 元素格式: "ip port"
RS_PREFIX=""           # 复制集前缀
RS_NAME=""             # 完整复制集名
NODE_ROLE=""           # shardsvr 或 configsvr
NODE_TYPE=""           # 节点类型 (shardN 或 config)

# 读取iplist
IP_LIST_ARRAY=()
read_file_to_array ${IP_LIST_FILE} IP_LIST_ARRAY || {
    echo "错误：read_file_to_array 失败 IP_LIST_FILE: ${IP_LIST_FILE}" >&2
    exit 1
}

if [ ${#IP_LIST_ARRAY[@]} -eq 0 ]; then
    echo "错误：IP_LIST_ARRAY 为空"
    exit 1
fi

# 解析每行
for index in "${!IP_LIST_ARRAY[@]}"; do
    elem="${IP_LIST_ARRAY[$index]}"

    # 过滤空行和注释行
    if [ -z "${elem}" ] || [[ "$elem" =~ ^[[:space:]]*$ ]]; then
        continue
    fi
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

    # 校验节点类型
    if [[ "${node_type}" =~ ^shard[0-9]+$ ]]; then
        NODE_ROLE="shardsvr"
    elif [ "${node_type}" = "config" ]; then
        NODE_ROLE="configsvr"
    else
        echo "警告：add_replica_node 只支持 shard1/shard2/... 和 config 类型, 跳过: ${node_type}"
        continue
    fi

    # 复制集前缀和类型取第一个非空值
    if [ -z "${RS_PREFIX}" ]; then
        RS_PREFIX="${rs_prefix}"
        NODE_TYPE="${node_type}"
        RS_NAME="${rs_prefix}_${node_type}"
    else
        # 校验复制集前缀一致性
        if [ "${RS_PREFIX}" != "${rs_prefix}" ]; then
            echo "错误：复制集前缀不一致: 已有 '${RS_PREFIX}', 当前行 '${rs_prefix}'" >&2
            exit 1
        fi
        # 校验节点类型一致性 (同一次扩展只能针对同一复制集)
        if [ "${NODE_TYPE}" != "${node_type}" ]; then
            echo "错误：节点类型不一致: 已有 '${NODE_TYPE}', 当前行 '${node_type}'" >&2
            exit 1
        fi
    fi

    NEW_NODE_ARRAY+=("${ip} ${node_port}")
    echo "解析新节点: type=${node_type}, ip=${ip}, port=${node_port}"
done

if [ ${#NEW_NODE_ARRAY[@]} -eq 0 ]; then
    echo "错误：没有有效的节点"
    exit 1
fi

echo ""
echo "========== 扩展信息 =========="
echo "复制集名称: ${RS_NAME}"
echo "节点角色: ${NODE_ROLE}"
echo "新节点数量: ${#NEW_NODE_ARRAY[@]}"
for i in "${!NEW_NODE_ARRAY[@]}"; do
    echo "  [${i}] ${NEW_NODE_ARRAY[$i]}"
done
echo "=============================="

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

LOCAL_IP=$(get_local_ip "${LOCAL_IP_LIST}")
echo "LOCAL_IP: ${LOCAL_IP}"

LOCAL_IPV4=$(get_local_ipv4 "${LOCAL_IP_LIST}")
LOCAL_IPV6=$(get_local_ipv6 "${LOCAL_IP_LIST}")

##############################
# 辅助函数
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

# 判断目标机器是否已安装 mongodb
is_mongo_installed() {
    local ip=$1
    local check_cmd="[ -d '${INSTALL_PATH}' ] && [ -x '${INSTALL_PATH}/mongodb-linux-x86_64-rhel88-8.0.6/bin/mongod' ]"
    if is_local_host "${ip}" "${LOCAL_IP_LIST}"; then
        bash -c "${check_cmd}" 2>/dev/null
    else
        ssh root@${ip} "${check_cmd}" 2>/dev/null
    fi
}

##############################
# 环境准备: 检查本机mongo安装
##############################
TARGET_SCRIPT_PATH=/root/mongodb_script

echo ""
echo "========== 检查本机 MongoDB 安装状态 =========="

if is_mongo_installed "127.0.0.1"; then
    echo "本机已安装 mongodb, mongosh 可用."
else
    echo "本机未安装 mongodb, 需要先安装才能执行 mongosh 命令."
    echo "执行本机 init_package 和 init_env..."

    TMP_DIR=/root/build_mongo_temp
    TGZ_FILE_NAME=mongodb.tar.gz

    sh ${SCRIPT_PATH}/pack_tar.sh ${TMP_DIR} ${SCRIPT_PATH} ${TGZ_FILE_NAME} || {
        echo "错误：pack_tar.sh 失败" >&2
        exit 1
    }
    echo "pack TGZ_FILE_NAME:${TGZ_FILE_NAME} success."

    . ${SCRIPT_PATH}/init_package.sh ${TMP_DIR}/${TGZ_FILE_NAME} ${TARGET_SCRIPT_PATH}
    . ${TARGET_SCRIPT_PATH}/init_env.sh ${TARGET_SCRIPT_PATH} ${INSTALL_PATH} || {
        echo "错误：本机 ${TARGET_SCRIPT_PATH}/init_env.sh 失败" >&2
        exit 1
    }

    echo "本机 mongodb 安装完成."
fi

# source 环境变量
source ~/.bash_profile 2>/dev/null

# 验证 mongosh 命令可用
if ! command -v mongosh > /dev/null 2>&1; then
    echo "错误：mongosh 命令不可用"
    exit 1
fi
echo "mongosh 命令可用."

##############################
# 验证主节点连接 & 获取复制集配置
##############################
echo ""
echo "========== 验证主节点连接 =========="

PRIMARY_HOST_PORT=$(format_host_port ${MONGOD_PRIMARY_ADDR} ${MONGOD_PRIMARY_PORT})
echo "连接主节点: ${PRIMARY_HOST_PORT}..."

# 验证主节点连接
mongosh --host ${PRIMARY_HOST_PORT} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "db.runCommand({ping:1})" > /dev/null 2>&1 || {
    echo "错误：无法连接到主节点 ${PRIMARY_HOST_PORT}, 请检查地址、用户名密码和集群状态"
    exit 1
}
echo "主节点连接验证成功."

# 获取复制集名称
echo "获取复制集配置..."
RS_CONFIG=$(mongosh --host ${PRIMARY_HOST_PORT} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --quiet --eval "JSON.stringify(rs.conf())")

# 验证复制集名称匹配
ACTUAL_RS_NAME=$(echo "${RS_CONFIG}" | mongosh --quiet --eval 'printjson' 2>/dev/null | grep -o '"_id"[[:space:]]*:[^,}]*' | head -1 | sed 's/.*"\([^"]*\)".*/\1/')

if [ -z "${ACTUAL_RS_NAME}" ]; then
    # 尝试另一种方式获取复制集名称
    ACTUAL_RS_NAME=$(mongosh --host ${PRIMARY_HOST_PORT} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --quiet --eval "rs.conf()._id")
fi

echo "复制集名称: ${ACTUAL_RS_NAME}"

# 校验复制集名称是否匹配
if [ -n "${ACTUAL_RS_NAME}" ] && [ "${ACTUAL_RS_NAME}" != "${RS_NAME}" ]; then
    echo "警告：iplist.txt 中的复制集名称 '${RS_NAME}' 与实际 '${ACTUAL_RS_NAME}' 不一致"
    echo "将使用实际复制集名称: ${ACTUAL_RS_NAME}"
    RS_NAME="${ACTUAL_RS_NAME}"
fi

# 获取当前成员数量，计算新节点 _id
MEMBER_COUNT=$(mongosh --host ${PRIMARY_HOST_PORT} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --quiet --eval "rs.conf().members.length")
echo "当前复制集成员数量: ${MEMBER_COUNT}"

# 打印当前复制集状态
echo ""
echo "========== 当前复制集成员 =========="
mongosh --host ${PRIMARY_HOST_PORT} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "rs.status().members.forEach(m => printjson({host: m.name, stateStr: m.stateStr}))"

##############################
# 打包脚本并分发到各节点, 初始化环境
##############################
TMP_DIR=/root/build_mongo_temp
TGZ_FILE_NAME=mongodb.tar.gz

if [ ! -e "${TMP_DIR}/${TGZ_FILE_NAME}" ]; then
    sh ${SCRIPT_PATH}/pack_tar.sh ${TMP_DIR} ${SCRIPT_PATH} ${TGZ_FILE_NAME} || {
        echo "错误：pack_tar.sh 失败" >&2
        exit 1
    }
    echo "pack TGZ_FILE_NAME:${TGZ_FILE_NAME} success."
fi

# 收集所有ip(去重)
declare -A ALL_IPS
for item in "${NEW_NODE_ARRAY[@]}"; do
    ip=$(echo "${item}" | awk '{print $1}')
    ALL_IPS[$ip]=1
done

declare -A is_ip_init_dict

echo ""
echo "========== 初始化各节点环境 =========="

for ip in "${!ALL_IPS[@]}"; do
    echo "检查节点 ip:${ip} 的 mongodb 安装状态..."

    if is_local_host "${ip}" "${LOCAL_IP_LIST}"; then
        if [ -z "${is_ip_init_dict[$ip]}" ]; then
            if is_mongo_installed ${ip}; then
                echo "本地节点 ip:${ip} 已安装 mongodb, 跳过安装."
                mkdir -p ${TARGET_SCRIPT_PATH}
                cp -f ${KEYFILE_PATH} ${TARGET_SCRIPT_PATH}/keyfile || {
                    echo "错误：拷贝 keyfile 失败" >&2
                    exit 1
                }
            else
                echo "本地节点 ip:${ip} 未安装 mongodb, 执行 init_package 和 init_env..."
                . ${SCRIPT_PATH}/init_package.sh ${TMP_DIR}/${TGZ_FILE_NAME} ${TARGET_SCRIPT_PATH}
                . ${TARGET_SCRIPT_PATH}/init_env.sh ${TARGET_SCRIPT_PATH} ${INSTALL_PATH} || {
                    echo "错误：本地:$ip init_env.sh 失败" >&2
                    exit 1
                }
            fi
            is_ip_init_dict[$ip]=1
        fi
    else
        if [ -z "${is_ip_init_dict[$ip]}" ]; then
            if is_mongo_installed ${ip}; then
                echo "远程节点 ip:${ip} 已安装 mongodb, 跳过安装."
                ssh root@${ip} "mkdir -p ${TARGET_SCRIPT_PATH}" || {
                    echo "错误：${ip} 创建 ${TARGET_SCRIPT_PATH} 失败" >&2
                    exit 1
                }
                echo "拷贝keyfile到 ${ip}..."
                scp_to_host ${ip} ${KEYFILE_PATH} ${TARGET_SCRIPT_PATH}/keyfile || {
                    echo "错误：scp keyfile 到 ${ip} 失败" >&2
                    exit 1
                }
            else
                echo "远程节点 ip:${ip} 未安装 mongodb, 执行 init_package 和 init_env..."
                echo "创建目录: TMP_DIR:${TMP_DIR} ..."
                ssh root@${ip} "rm -rf ${TMP_DIR}" || {
                    echo "错误：移除 ${TMP_DIR} 失败" >&2
                    exit 1
                }
                ssh root@${ip} "mkdir -p ${TMP_DIR}" || {
                    echo "错误：${ip} 创建 ${TMP_DIR} 失败" >&2
                    exit 1
                }

                echo "拷贝压缩文件到 ${ip}..."
                scp_to_host ${ip} ${TMP_DIR}/${TGZ_FILE_NAME} ${TMP_DIR} || {
                    echo "错误：scp 压缩文件到 ${ip} 失败" >&2
                    exit 1
                }

                scp_to_host ${ip} ${SCRIPT_PATH}/init_package.sh ${TMP_DIR} || {
                    echo "错误：scp init_package.sh 到 ${ip} 失败" >&2
                    exit 1
                }
                echo "$ip 执行 init_package.sh ..."
                ssh root@${ip} "sh ${TMP_DIR}/init_package.sh ${TMP_DIR}/${TGZ_FILE_NAME} ${TARGET_SCRIPT_PATH}" || {
                    echo "错误：${ip} init_package.sh 失败" >&2
                    exit 1
                }

                echo "拷贝keyfile到 ${ip}..."
                scp_to_host ${ip} ${KEYFILE_PATH} ${TARGET_SCRIPT_PATH}/keyfile || {
                    echo "错误：scp keyfile 到 ${ip} 失败" >&2
                    exit 1
                }

                ssh root@${ip} "source ~/.bash_profile 2>/dev/null; sh ${TARGET_SCRIPT_PATH}/init_env.sh ${TARGET_SCRIPT_PATH} ${INSTALL_PATH}" || {
                    echo "错误：${ip} init_env.sh 失败" >&2
                    exit 1
                }
                echo "$ip init_env.sh 成功"
            fi
            is_ip_init_dict[$ip]=1
        fi
    fi
done

echo "所有新节点环境初始化完成."

##############################
# 启动各节点 mongod 实例
##############################
echo ""
echo "========== 启动新节点 mongod =========="

# 为每个节点分配 _id
NODE_INDEX=0
for item in "${NEW_NODE_ARRAY[@]}"; do
    ip=$(echo "${item}" | awk '{print $1}')
    node_port=$(echo "${item}" | awk '{print $2}')
    node_id=$((MEMBER_COUNT + NODE_INDEX))
    node_db_subdir="${RS_NAME}_${NODE_INDEX}"
    node_db_path="${DATA_PATH}/${node_db_subdir}"

    echo "启动新节点 [$NODE_INDEX]: $(format_host_port ${ip} ${node_port}), _id: ${node_id}, 数据目录: ${node_db_path}..."

    if is_local_host "${ip}" "${LOCAL_IP_LIST}"; then
        sh ${TARGET_SCRIPT_PATH}/create_mongodb_inst.sh ${TARGET_SCRIPT_PATH} ${node_db_path} $(get_local_ip_by_type "${ip}" "${LOCAL_IP_LIST}") ${node_port} "${RS_NAME}" ${TARGET_SCRIPT_PATH}/keyfile ${NODE_ROLE} || {
            echo "错误：新节点 $(format_host_port ${ip} ${node_port}) 启动失败" >&2
            exit 1
        }
    else
        ssh root@${ip} "source ~/.bash_profile 2>/dev/null; sh ${TARGET_SCRIPT_PATH}/create_mongodb_inst.sh ${TARGET_SCRIPT_PATH} ${node_db_path} ${ip} ${node_port} \"${RS_NAME}\" ${TARGET_SCRIPT_PATH}/keyfile ${NODE_ROLE}" || {
            echo "错误：新节点 $(format_host_port ${ip} ${node_port}) 启动失败" >&2
            exit 1
        }
    fi

    echo "新节点 $(format_host_port ${ip} ${node_port}) 启动成功."
    sleep 3

    NODE_INDEX=$((NODE_INDEX + 1))
done

##############################
# 从主节点添加新节点到复制集
##############################
echo ""
echo "========== 添加新节点到复制集 =========="

NODE_INDEX=0
for item in "${NEW_NODE_ARRAY[@]}"; do
    ip=$(echo "${item}" | awk '{print $1}')
    node_port=$(echo "${item}" | awk '{print $2}')
    node_id=$((MEMBER_COUNT + NODE_INDEX))
    register_host_port=$(format_host_port ${ip} ${node_port})

    echo "添加新节点到复制集: ${register_host_port}, _id: ${node_id}..."

    if is_local_host "${MONGOD_PRIMARY_ADDR}" "${LOCAL_IP_LIST}"; then
        mongosh --host $(format_host_port $(get_local_ip_by_type "${MONGOD_PRIMARY_ADDR}" "${LOCAL_IP_LIST}") ${MONGOD_PRIMARY_PORT}) -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "rs.add({_id: ${node_id}, host: \"${register_host_port}\", priority: 1, votes: 1})" || {
            echo "错误：添加节点 ${register_host_port} 失败" >&2
            exit 1
        }
    else
        ssh root@${MONGOD_PRIMARY_ADDR} "source ~/.bash_profile 2>/dev/null; mongosh --host $(format_host_port ${MONGOD_PRIMARY_ADDR} ${MONGOD_PRIMARY_PORT}) -u \"${TARGET_USER}\" -p \"${TARGET_PWD}\" --authenticationDatabase admin --eval \"rs.add({_id: ${node_id}, host: \\\"${register_host_port}\\\", priority: 1, votes: 1})\"" || {
            echo "错误：添加节点 ${register_host_port} 失败" >&2
            exit 1
        }
    fi

    echo "节点 ${register_host_port} 添加成功."
    sleep 3

    NODE_INDEX=$((NODE_INDEX + 1))
done

##############################
# 验证复制集状态
##############################
echo ""
echo "========== 验证复制集状态 =========="

sleep 5

echo "复制集配置:"
if is_local_host "${MONGOD_PRIMARY_ADDR}" "${LOCAL_IP_LIST}"; then
    mongosh --host $(format_host_port $(get_local_ip_by_type "${MONGOD_PRIMARY_ADDR}" "${LOCAL_IP_LIST}") ${MONGOD_PRIMARY_PORT}) -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "rs.conf()"
else
    ssh root@${MONGOD_PRIMARY_ADDR} "source ~/.bash_profile 2>/dev/null; mongosh --host $(format_host_port ${MONGOD_PRIMARY_ADDR} ${MONGOD_PRIMARY_PORT}) -u \"${TARGET_USER}\" -p \"${TARGET_PWD}\" --authenticationDatabase admin --eval \"rs.conf()\""
fi

echo ""
echo "复制集状态:"
if is_local_host "${MONGOD_PRIMARY_ADDR}" "${LOCAL_IP_LIST}"; then
    mongosh --host $(format_host_port $(get_local_ip_by_type "${MONGOD_PRIMARY_ADDR}" "${LOCAL_IP_LIST}") ${MONGOD_PRIMARY_PORT}) -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "rs.status()"
else
    ssh root@${MONGOD_PRIMARY_ADDR} "source ~/.bash_profile 2>/dev/null; mongosh --host $(format_host_port ${MONGOD_PRIMARY_ADDR} ${MONGOD_PRIMARY_PORT}) -u \"${TARGET_USER}\" -p \"${TARGET_PWD}\" --authenticationDatabase admin --eval \"rs.status()\""
fi

##############################
# 完成
##############################
echo ""
echo "=========================================="
echo "MongoDB 复制集扩展节点完成!"
echo "=========================================="
echo "复制集名称: ${RS_NAME}"
echo "节点角色: ${NODE_ROLE}"
echo "新增节点:"
for item in "${NEW_NODE_ARRAY[@]}"; do
    echo "  $(format_host_port $(echo "${item}" | awk '{print $1}') $(echo "${item}" | awk '{print $2}'))"
done
echo ""
echo "主节点: ${PRIMARY_HOST_PORT}"
echo ""
echo "连接方式:"
echo "  mongosh --host ${PRIMARY_HOST_PORT} -u ${TARGET_USER} -p <密码> --authenticationDatabase admin"
echo "=========================================="
