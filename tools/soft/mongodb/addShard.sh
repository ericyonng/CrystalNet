#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 新增分片到已有的MongoDB分片集群
# 用法: sh addShard.sh <iplist.txt> <mongos_ip> <mongos_port> <用户名> <密码> <软件包安装路径> <数据库数据路径> <keyfile绝对路径> [数据库名]
#
# iplist.txt 中只需填写新增的 shard 节点, 格式:
#   shardN 复制集前缀 IP 端口   (shardN 的 N 必须是集群中尚不存在的分片编号)
#
# 示例: 已有 shard1, 要新增 shard2
#   shard2 testsuit_rs 192.168.1.4 27011
#   shard2 testsuit_rs 192.168.1.5 27011
#   shard2 testsuit_rs 192.168.1.6 27011
#
# IP_LIST_FILE: 节点类型 复制集前缀 IP 端口: shard1 testsuit_rs 127.0.0.1 27011
# MONGOS_IP: 已有集群的 mongos 节点IP
# MONGOS_PORT: 已有集群的 mongos 节点端口
# TARGET_USER: 已有集群的管理员用户名
# TARGET_PWD: 已有集群的管理员密码
# INSTALL_PATH: mongodb程序目录
# DATA_PATH: mongodb数据库数据目录
# KEYFILE_PATH: 集群keyfile的绝对路径(必须与已有集群的keyfile一致)
# DB_NAME: db名(可选, 默认admin)

# 当前脚本路径
SCRIPT_PATH="$(cd $(dirname $0); pwd)"

# IP列表文件
IP_LIST_FILE=$1
# mongos IP
MONGOS_IP=$2
# mongos 端口
MONGOS_PORT=$3
# 用户名
TARGET_USER=$4
# 密码
TARGET_PWD=$5
# 安装mongodb的目录
INSTALL_PATH=$6
# 数据库数据路径
DATA_PATH=$7
# keyfile绝对路径
KEYFILE_PATH=${8}
# 数据库名(可选, 默认admin)
DB_NAME="${9:-admin}"

##############################
# 参数校验
##############################
if [ ! -e "${IP_LIST_FILE}" ]; then
    echo "IP_LIST_FILE:${IP_LIST_FILE} not exist please check"
    exit 1
fi

if [ -z "${MONGOS_IP}" ]; then
    echo "MONGOS_IP is empty please check!!!"
    exit 1
fi

if [ -z "${MONGOS_PORT}" ]; then
    echo "MONGOS_PORT is empty please check!!!"
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

if [ -z "${KEYFILE_PATH}" ]; then
    echo "KEYFILE_PATH is empty please check!!!"
    exit 1
fi

if [ ! -e "${KEYFILE_PATH}" ]; then
    echo "错误：keyfile ${KEYFILE_PATH} 不存在, 请提供集群的keyfile绝对路径"
    exit 1
fi

# 加载公共
. ${SCRIPT_PATH}/common/common_define.sh
. ${SCRIPT_PATH}/common/funcs.sh

echo "=========================================="
echo "MongoDB 分片集群 - 新增分片"
echo "=========================================="
echo "IP_LIST_FILE    : ${IP_LIST_FILE}"
echo "MONGOS_IP       : ${MONGOS_IP}"
echo "MONGOS_PORT     : ${MONGOS_PORT}"
echo "TARGET_USER     : ${TARGET_USER}"
echo "TARGET_PWD      : ******"
echo "INSTALL_PATH    : ${INSTALL_PATH}"
echo "DATA_PATH       : ${DATA_PATH}"
echo "KEYFILE_PATH    : ${KEYFILE_PATH}"
echo "DB_NAME         : ${DB_NAME}"
echo "=========================================="

##############################
# 解析 iplist.txt, 按类型分组
##############################
declare -A SHARD_GROUPS    # key=shard名(shard2/shard3/...), value=节点列表(分号分隔: "ip port;ip port")
SHARD_NAME_LIST=()         # 分片名有序列表, 保证添加分片顺序

# 复制集前缀相关: 从 iplist 中读取, 同一类型组内必须一致
declare -A RS_PREFIX_MAP   # key=节点类型(shard2/shard3/...), value=该组的复制集前缀

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

    # 只接受 shard 类型
    if [[ "${node_type}" =~ ^shard[0-9]+$ ]]; then
        # 校验同一类型组内的复制集前缀必须一致
        if [ -n "${RS_PREFIX_MAP[$node_type]}" ]; then
            if [ "${RS_PREFIX_MAP[$node_type]}" != "${rs_prefix}" ]; then
                echo "错误：类型 ${node_type} 的复制集前缀不一致: 已有 '${RS_PREFIX_MAP[$node_type]}', 当前行 '${rs_prefix}'" >&2
                exit 1
            fi
        else
            RS_PREFIX_MAP[$node_type]="${rs_prefix}"
        fi

        if [ -z "${SHARD_GROUPS[$node_type]}" ]; then
            SHARD_GROUPS[$node_type]="${ip} ${node_port}"
            SHARD_NAME_LIST+=("${node_type}")
        else
            SHARD_GROUPS[$node_type]="${SHARD_GROUPS[$node_type]};${ip} ${node_port}"
        fi
    else
        echo "警告：addShard 只支持 shard 类型, 跳过: ${node_type} ${ip} ${node_port}"
        echo "提示: config 和 mongos 节点不需要在 iplist.txt 中指定, 新增分片会通过参数指定的 mongos 连接"
    fi
done

if [ ${#SHARD_NAME_LIST[@]} -eq 0 ]; then
    echo "错误：iplist.txt中没有shard节点(需要shard1/shard2/...格式)"
    exit 1
fi

# 打印分组结果
echo ""
echo "========== 新增分片列表 =========="
for shard_name in "${SHARD_NAME_LIST[@]}"; do
    IFS=';' read -ra shard_nodes <<< "${SHARD_GROUPS[$shard_name]}"
    echo "  ${shard_name} (${#shard_nodes[@]}个节点): ${SHARD_GROUPS[$shard_name]}"
done
echo "=================================="

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

# 判断目标机器是否已安装 mongodb (检查 INSTALL_PATH 目录和 mongod 二进制是否存在)
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
# 环境准备: 检查本机mongo安装 & 打包分发到各节点
##############################
TARGET_SCRIPT_PATH=/root/mongodb_script

echo "使用 keyfile: ${KEYFILE_PATH}"

# 首先检查本机是否已安装 mongodb (后续需要在本机执行 mongosh)
echo "检查本机 mongodb 安装状态..."
if is_mongo_installed "127.0.0.1"; then
    echo "本机已安装 mongodb, mongosh 可用."
else
    echo "本机未安装 mongodb, 需要先安装才能执行 mongosh 命令."
    echo "执行本机 init_package 和 init_env..."
    TMP_DIR=/root/build_mongo_temp
    TGZ_FILE_NAME=mongodb.tar.gz
    sh ${SCRIPT_PATH}/pack_tar.sh ${TMP_DIR} ${SCRIPT_PATH} ${TGZ_FILE_NAME} || {
        echo "错误： pack_tar.sh fail ${TMP_DIR} ${TGZ_FILE_NAME} 失败" >&2
        exit 1
    }
    echo "pack TGZ_FILE_NAME:${TGZ_FILE_NAME} success."
    . ${SCRIPT_PATH}/init_package.sh ${TMP_DIR}/${TGZ_FILE_NAME} ${TARGET_SCRIPT_PATH}
    . ${TARGET_SCRIPT_PATH}/init_env.sh ${TARGET_SCRIPT_PATH} ${INSTALL_PATH} || {
        echo "错误：本机 ${TARGET_SCRIPT_PATH}/init_env.sh 失败" >&2
        exit 1
    }
    echo "本机 mongodb 安装完成, mongosh 可用."
fi

# source 环境变量确保 mongosh 在 PATH 中
source ~/.bash_profile 2>/dev/null

# 验证 mongosh 命令可用
if ! command -v mongosh > /dev/null 2>&1; then
    echo "错误：mongosh 命令不可用, 请检查 mongodb 是否正确安装"
    exit 1
fi
echo "mongosh 命令可用."

##############################
# 验证 mongos 连接 & 检查已有分片
##############################
# 验证 mongos 连接
echo "验证 mongos 连接 $(format_host_port ${MONGOS_IP} ${MONGOS_PORT})..."
mongosh --host $(format_host_port ${MONGOS_IP} ${MONGOS_PORT}) -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "db.runCommand({ping:1})" > /dev/null 2>&1 || {
    echo "错误：无法连接到 mongos $(format_host_port ${MONGOS_IP} ${MONGOS_PORT}), 请检查IP端口、用户名密码和集群状态"
    exit 1
}
echo "mongos 连接验证成功."

# 获取已有分片列表, 检查是否重复添加
echo "检查已有分片列表..."
EXISTING_SHARDS=$(mongosh --host $(format_host_port ${MONGOS_IP} ${MONGOS_PORT}) -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --quiet --eval "db.adminCommand({listShards:1}).shards.map(s => s._id).join(',')")
echo "已有分片: ${EXISTING_SHARDS}"

for shard_name in "${SHARD_NAME_LIST[@]}"; do
    if echo "${EXISTING_SHARDS}" | grep -qw "${shard_name}"; then
        echo "错误：分片 ${shard_name} 已存在于集群中, 不能重复添加! 如需扩容节点请修改 iplist.txt 使用新的分片编号"
        exit 1
    fi
done

##############################
# 打包脚本并分发到各 shard 节点, 初始化环境
##############################
TMP_DIR=/root/build_mongo_temp
TGZ_FILE_NAME=mongodb.tar.gz

# 如果本机已安装则 pack_tar 可能已执行过, 仍需确保包存在
if [ ! -e "${TMP_DIR}/${TGZ_FILE_NAME}" ]; then
    sh ${SCRIPT_PATH}/pack_tar.sh ${TMP_DIR} ${SCRIPT_PATH} ${TGZ_FILE_NAME} || {
        echo "错误： pack_tar.sh fail ${TMP_DIR} ${TGZ_FILE_NAME} 失败" >&2
        exit 1
    }
    echo "pack TGZ_FILE_NAME:${TGZ_FILE_NAME} success."
fi

# 收集所有ip(去重)
declare -A ALL_IPS
for shard_name in "${SHARD_NAME_LIST[@]}"; do
    IFS=';' read -ra shard_nodes <<< "${SHARD_GROUPS[$shard_name]}"
    for node in "${shard_nodes[@]}"; do
        ip=$(echo "${node}" | awk '{print $1}')
        ALL_IPS[$ip]=1
    done
done

declare -A is_ip_init_dict

for ip in "${!ALL_IPS[@]}"; do
    echo "检查节点 ip:${ip} 的 mongodb 安装状态..."

    if is_local_host "${ip}" "${LOCAL_IP_LIST}"; then
        if [ -z "${is_ip_init_dict[$ip]}" ]; then
            if is_mongo_installed ${ip}; then
                echo "本地节点 ip:${ip} 已安装 mongodb, 跳过 init_package 和 init_env."
                # 确保 keyfile 和脚本目录存在
                mkdir -p ${TARGET_SCRIPT_PATH}
                cp -f ${KEYFILE_PATH} ${TARGET_SCRIPT_PATH}/keyfile || {
                    echo "错误：拷贝 keyfile ${KEYFILE_PATH} => ${TARGET_SCRIPT_PATH}/keyfile 失败" >&2
                    exit 1
                }
            else
                echo "本地节点 ip:${ip} 未安装 mongodb, 执行 init_package 和 init_env..."
                . ${SCRIPT_PATH}/init_package.sh ${TMP_DIR}/${TGZ_FILE_NAME} ${TARGET_SCRIPT_PATH}

                . ${TARGET_SCRIPT_PATH}/init_env.sh ${TARGET_SCRIPT_PATH} ${INSTALL_PATH} || {
                    echo "错误：本地:$ip ${TARGET_SCRIPT_PATH}/init_env.sh 失败" >&2
                    exit 1
                }
            fi
            is_ip_init_dict[$ip]=1
        fi
    else
        if [ -z "${is_ip_init_dict[$ip]}" ]; then
            if is_mongo_installed ${ip}; then
                echo "远程节点 ip:${ip} 已安装 mongodb, 跳过 init_package 和 init_env."
                # 确保 keyfile 和脚本目录存在
                ssh root@${ip} "mkdir -p ${TARGET_SCRIPT_PATH}" || {
                    echo "错误：${ip} 创建 ${TARGET_SCRIPT_PATH} 失败" >&2
                    exit 1
                }
                echo "拷贝keyfile到 ${ip}..."
                scp_to_host ${ip} ${KEYFILE_PATH} ${TARGET_SCRIPT_PATH}/keyfile || {
                    echo "错误： scp 拷贝 keyfile => ${ip}:${TARGET_SCRIPT_PATH}/keyfile 失败" >&2
                    exit 1
                }
            else
                echo "远程节点 ip:${ip} 未安装 mongodb, 执行 init_package 和 init_env..."
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
                echo "$ip 执行 init_env.sh 成功"
            fi
            is_ip_init_dict[$ip]=1
        fi
    fi
done

echo "所有新节点环境检查/初始化完成."

##############################
# 步骤1: 初始化每个新 shard 分片的复制集
##############################
echo ""
echo "========== 步骤1: 初始化新 shard 分片复制集 =========="

# 记录每个分片的地址串
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

    # 1.1 初始化shard主节点 (no_auth → rs.initiate → createUser → shutdown)
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

    # 1.2 启动shard主节点(带认证)
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

    # 1.3 启动shard从节点并添加到复制集
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
            mongosh --host $(format_host_port $(get_local_ip_by_type "${primary_ip}" "${LOCAL_IP_LIST}") ${primary_port}) -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "rs.add({_id: ${member_id}, host: \"${register_host_port}\", priority: 1, votes: 1})" || {
                echo "错误：添加shard ${shard_name} 从节点 ${register_host_port} 失败" >&2
                exit 1
            }
        else
            ssh root@${primary_ip} "source ~/.bash_profile 2>/dev/null; mongosh --host $(format_host_port ${primary_ip} ${primary_port}) -u \"${TARGET_USER}\" -p \"${TARGET_PWD}\" --authenticationDatabase admin --eval \"rs.add({_id: ${member_id}, host: \\\"${register_host_port}\\\", priority: 1, votes: 1})\"" || {
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

echo "所有新 shard 分片复制集初始化完成."

##############################
# 步骤2: 通过 mongos 添加新分片 (sh.addShard)
##############################
echo ""
echo "========== 步骤2: mongos 添加新分片 =========="

sleep 3  # 等待确保复制集稳定

for shard_name in "${SHARD_NAME_LIST[@]}"; do
    shard_addr="${SHARD_ADDR_STRINGS[$shard_name]}"
    echo "添加分片: sh.addShard(\"${shard_addr}\")..."

    mongosh --host $(format_host_port ${MONGOS_IP} ${MONGOS_PORT}) -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "sh.addShard(\"${shard_addr}\")" || {
        echo "错误：添加分片 ${shard_name} 失败" >&2
        exit 1
    }

    echo "分片 ${shard_name} 添加成功."
    sleep 3
done

echo "所有新分片添加完成."

##############################
# 完成
##############################
echo ""
echo "=========================================="
echo "MongoDB 分片集群 - 新增分片完成!"
echo "=========================================="
echo "新增的 Shard 分片:"
for shard_name in "${SHARD_NAME_LIST[@]}"; do
    echo "  ${shard_name}: ${SHARD_ADDR_STRINGS[$shard_name]}"
done
echo ""
echo "Mongos 路由:"
echo "  $(format_host_port ${MONGOS_IP} ${MONGOS_PORT})"
echo ""
echo "连接方式:"
echo "  mongosh --host $(format_host_port ${MONGOS_IP} ${MONGOS_PORT}) -u ${TARGET_USER} -p <密码> --authenticationDatabase admin"
echo ""
echo "查看分片状态:"
echo "  mongosh --host $(format_host_port ${MONGOS_IP} ${MONGOS_PORT}) -u ${TARGET_USER} -p <密码> --authenticationDatabase admin --eval \"sh.status()\""
echo "=========================================="
