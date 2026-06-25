#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 创建 MongoDB 复制集(非分片集群, 适用于中小规模项目)
# 用法: sh create_replicaset.sh <iplist.txt> <用户名> <密码> <工作路径(数据运行路径)> <安装路径(数据库安装路径)>
#
# iplist.txt 格式(3字段): 复制集名 IP地址/域名 端口
#   示例:
#     mydb_rs 192.168.1.1 27017
#     mydb_rs 192.168.1.2 27017
#     mydb_rs 192.168.1.3 27017
#
# IP_LIST_FILE: 节点列表文件(复制集名 IP 端口)
# TARGET_USER: 管理员用户名
# TARGET_PWD: 管理员密码
# WORK_PATH: 数据库运行路径(数据目录父路径, 如 /root/mongo_data)
# INSTALL_PATH: 数据库安装路径(mongodb程序目录, 如 /root/mongo_install)
#
# 复制集名从 iplist.txt 中读取(所有行必须一致), 不作为命令行参数

# 当前脚本路径
SCRIPT_PATH="$(cd $(dirname $0); pwd)"

# 参数定义
IP_LIST_FILE=$1
TARGET_USER=$2
TARGET_PWD=$3
WORK_PATH=$4
INSTALL_PATH=$5

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

if [ -z "${WORK_PATH}" ]; then
    echo "错误：WORK_PATH(工作路径) 为空"
    exit 1
fi

if [ -z "${INSTALL_PATH}" ]; then
    echo "错误：INSTALL_PATH(安装路径) 为空"
    exit 1
fi

# 加载公共函数库
. ${SCRIPT_PATH}/common/common_define.sh
. ${SCRIPT_PATH}/common/funcs.sh

echo "=========================================="
echo "MongoDB 复制集自动搭建脚本"
echo "=========================================="
echo "IP_LIST_FILE    : ${IP_LIST_FILE}"
echo "TARGET_USER     : ${TARGET_USER}"
echo "TARGET_PWD      : ******"
echo "WORK_PATH       : ${WORK_PATH}"
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

# LOCAL_IP: 从 LOCAL_IP_LIST 中选最优IP(优先公网IP), 用于子脚本参数
LOCAL_IP=$(get_local_ip "${LOCAL_IP_LIST}")
echo "LOCAL_IP(用于子脚本): ${LOCAL_IP}"

# LOCAL_IPV4 / LOCAL_IPV6: 本机最优IPv4和IPv6地址, 根据iplist中的IP类型选择使用
LOCAL_IPV4=$(get_local_ipv4 "${LOCAL_IP_LIST}")
LOCAL_IPV6=$(get_local_ipv6 "${LOCAL_IP_LIST}")
echo "LOCAL_IPV4: ${LOCAL_IPV4}, LOCAL_IPV6: ${LOCAL_IPV6}"

##############################
# 解析 iplist.txt, 获取复制集名和节点列表
##############################
RS_NAME=""                # 复制集名(从iplist读取, 所有行必须一致)
NODE_ARRAY=()             # 节点数组, 元素格式: "ip port"

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
    # 去掉前导空格后检查注释
    trimmed=$(echo "${elem}" | sed 's/^[[:space:]]*//')
    if [[ "$trimmed" =~ ^# ]]; then
        continue
    fi

    fields=($(echo "${elem}" | awk '{print $1, $2, $3}'))
    rs_name="${fields[0]}"
    ip="${fields[1]}"
    node_port="${fields[2]}"

    if [ -z "${rs_name}" ] || [ -z "${ip}" ] || [ -z "${node_port}" ]; then
        echo "警告：跳过无效行(需要3字段: 复制集名 IP 端口): ${elem}"
        continue
    fi

    # 校验复制集名一致性
    if [ -z "${RS_NAME}" ]; then
        RS_NAME="${rs_name}"
    else
        if [ "${RS_NAME}" != "${rs_name}" ]; then
            echo "错误：复制集名不一致: 已有 '${RS_NAME}', 当前行 '${rs_name}'" >&2
            echo "提示: iplist.txt 中所有行的复制集名必须一致" >&2
            exit 1
        fi
    fi

    NODE_ARRAY+=("${ip} ${node_port}")
    echo "解析节点: rs_name=${rs_name}, ip=${ip}, port=${node_port}"
done

if [ ${#NODE_ARRAY[@]} -eq 0 ]; then
    echo "错误：iplist.txt 中没有有效节点"
    exit 1
fi

# 建议至少3节点(仅警告, 不强制)
if [ ${#NODE_ARRAY[@]} -lt 3 ]; then
    echo "警告：复制集节点数 ${#NODE_ARRAY[@]} 少于 3, 建议至少 3 节点(1 Primary + 2 Secondary)以保证高可用"
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
    if [ $i -eq 0 ]; then
        echo "  [$i] $(format_host_port ${ip} ${node_port})  (初始主节点, priority: 2)"
    else
        echo "  [$i] $(format_host_port ${ip} ${node_port})  (从节点, priority: 1)"
    fi
done
echo "===================================="

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

# 脚本分发目标路径(与现有脚本保持一致)
TARGET_SCRIPT_PATH=/root/mongodb_script

# 辅助函数: 判断目标机器是否已安装 mongodb
# 检测方式: which mongod 可用 或 INSTALL_PATH/mongodb-linux-x86_64-rhel88-8.0.6/bin/mongod 存在
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

# 收集所有ip(去重)
declare -A ALL_IPS
for item in "${NODE_ARRAY[@]}"; do
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
                # 确保 keyfile 和脚本目录存在
                mkdir -p ${TARGET_SCRIPT_PATH}
                if [ ! -f "${KEYFILE_PATH}" ]; then
                    echo "错误: 源 keyfile 不存在: ${KEYFILE_PATH}" >&2
                    exit 1
                fi
                cp -f ${KEYFILE_PATH} ${TARGET_SCRIPT_PATH}/keyfile || {
                    echo "错误：拷贝 keyfile 失败" >&2
                    exit 1
                }
                chmod 600 ${TARGET_SCRIPT_PATH}/keyfile
                echo "本地 keyfile 已准备: ${TARGET_SCRIPT_PATH}/keyfile"
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
                if [ ! -f "${KEYFILE_PATH}" ]; then
                    echo "错误: 源 keyfile 不存在: ${KEYFILE_PATH}" >&2
                    exit 1
                fi
                echo "拷贝keyfile到 ${ip}..."
                scp_to_host ${ip} ${KEYFILE_PATH} ${TARGET_SCRIPT_PATH}/keyfile || {
                    echo "错误：scp keyfile 到 ${ip} 失败" >&2
                    exit 1
                }
                ssh root@${ip} "chmod 600 ${TARGET_SCRIPT_PATH}/keyfile"
                echo "远程 ${ip} keyfile 已准备: ${TARGET_SCRIPT_PATH}/keyfile"
            else
                echo "远程节点 ip:${ip} 未安装 mongodb, 执行 init_package 和 init_env..."
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
                echo "$ip 执行 init_env.sh 成功"
            fi
            is_ip_init_dict[$ip]=1
        fi
    fi
done

echo "所有节点环境初始化完成."

# source 环境变量确保 mongosh 在 PATH 中
source ~/.bash_profile 2>/dev/null

# 验证 mongosh 命令可用
if ! command -v mongosh > /dev/null 2>&1; then
    echo "错误：mongosh 命令不可用, 请检查 mongodb 是否正确安装"
    exit 1
fi
echo "mongosh 命令可用."

##############################
# 步骤1: 初始化主节点 (第一个节点)
# init_primary.sh: no_auth启动 → rs.initiate → createUser → shutdown
##############################
echo ""
echo "========== 步骤1: 初始化主节点 =========="

# 提取主节点(第一个节点)
PRIMARY_NODE="${NODE_ARRAY[0]}"
PRIMARY_IP=$(echo "${PRIMARY_NODE}" | awk '{print $1}')
PRIMARY_PORT=$(echo "${PRIMARY_NODE}" | awk '{print $2}')
PRIMARY_DB_SUBDIR="${RS_NAME}_1"
PRIMARY_DB_PATH="${WORK_PATH}/${PRIMARY_DB_SUBDIR}"

echo "主节点: $(format_host_port ${PRIMARY_IP} ${PRIMARY_PORT}), RS_NAME: ${RS_NAME}"
echo "主节点数据目录: ${PRIMARY_DB_PATH}"

# init_primary.sh 参数(13个):
# SCRIPT_PATH TARGET_USER TARGET_PWD WORK_PATH PRIMARY_IP PRIMARY_PORT DB_SUBDIR RS_NAME KEYFILE_PATH SHARDING_CLUSTER_ROLE IS_MONGOS MONGOS_CONFIG_ADDR REGISTER_HOST
# 复制集场景: SHARDING_CLUSTER_ROLE="", IS_MONGOS="", MONGOS_CONFIG_ADDR="", REGISTER_HOST=iplist原始IP/域名
# LOCAL_REGISTER_HOST 使用 iplist 中的原始 IP/域名, 保证复制集成员注册地址一致
if is_local_host "${PRIMARY_IP}" "${LOCAL_IP_LIST}"; then
    sh ${TARGET_SCRIPT_PATH}/init_primary.sh ${TARGET_SCRIPT_PATH} ${TARGET_USER} ${TARGET_PWD} ${WORK_PATH} $(get_local_ip_by_type "${PRIMARY_IP}" "${LOCAL_IP_LIST}") ${PRIMARY_PORT} ${PRIMARY_DB_SUBDIR} ${RS_NAME} ${TARGET_SCRIPT_PATH}/keyfile "" "" "" "${PRIMARY_IP}" || {
        echo "错误：主节点 init_primary 失败" >&2
        exit 1
    }
else
    # 拷贝keyfile到远程
    scp_to_host ${PRIMARY_IP} ${KEYFILE_PATH} ${TARGET_SCRIPT_PATH}/keyfile 2>/dev/null
    ssh root@${PRIMARY_IP} "source ~/.bash_profile 2>/dev/null; sh ${TARGET_SCRIPT_PATH}/init_primary.sh ${TARGET_SCRIPT_PATH} ${TARGET_USER} \"${TARGET_PWD}\" ${WORK_PATH} ${PRIMARY_IP} ${PRIMARY_PORT} ${PRIMARY_DB_SUBDIR} ${RS_NAME} ${TARGET_SCRIPT_PATH}/keyfile \"\" \"\" \"\" \"${PRIMARY_IP}\"" || {
        echo "错误：主节点 ${PRIMARY_IP} init_primary 失败" >&2
        exit 1
    }
fi

echo "主节点初始化(no_auth → createUser → shutdown) 成功."

##############################
# 步骤2: 带认证启动主节点
##############################
echo ""
echo "========== 步骤2: 带认证启动主节点 =========="

# create_mongodb_inst.sh 参数(10个):
# SCRIPT_PATH LOCAL_TARGET_DB_PATH LOCAL_TARGET_IP LOCAL_TARGET_PORT LOCAL_REPL_SET_NAME LOCAL_KEYFILE_PATH LOCAL_SHARDING_CLUSTER_ROLE IS_MONGOS MONGOS_CONFIG_ADDR IS_NO_AUTH
# 复制集场景: SHARDING_CLUSTER_ROLE="", IS_MONGOS="", MONGOS_CONFIG_ADDR="", IS_NO_AUTH=""(启用认证)
if is_local_host "${PRIMARY_IP}" "${LOCAL_IP_LIST}"; then
    sh ${TARGET_SCRIPT_PATH}/create_mongodb_inst.sh ${TARGET_SCRIPT_PATH} ${PRIMARY_DB_PATH} $(get_local_ip_by_type "${PRIMARY_IP}" "${LOCAL_IP_LIST}") ${PRIMARY_PORT} "${RS_NAME}" ${TARGET_SCRIPT_PATH}/keyfile "" || {
        echo "错误：主节点带认证启动失败" >&2
        exit 1
    }
else
    ssh root@${PRIMARY_IP} "source ~/.bash_profile 2>/dev/null; sh ${TARGET_SCRIPT_PATH}/create_mongodb_inst.sh ${TARGET_SCRIPT_PATH} ${PRIMARY_DB_PATH} ${PRIMARY_IP} ${PRIMARY_PORT} \"${RS_NAME}\" ${TARGET_SCRIPT_PATH}/keyfile \"\"" || {
        echo "错误：主节点 ${PRIMARY_IP} 带认证启动失败" >&2
        exit 1
    }
fi

echo "主节点带认证启动成功."

sleep 5

##############################
# 步骤3: 启动从节点并添加到复制集
##############################
echo ""
echo "========== 步骤3: 启动从节点并添加到复制集 =========="

for i in "${!NODE_ARRAY[@]}"; do
    if [ $i -eq 0 ]; then
        continue  # 跳过主节点
    fi

    item="${NODE_ARRAY[$i]}"
    ip=$(echo "${item}" | awk '{print $1}')
    node_port=$(echo "${item}" | awk '{print $2}')
    # 从节点序号从2开始(主节点为1)
    node_db_subdir="${RS_NAME}_$(($i + 1))"
    node_db_path="${WORK_PATH}/${node_db_subdir}"

    echo ""
    echo "--- 启动从节点 [$i]: $(format_host_port ${ip} ${node_port}), 数据目录: ${node_db_path} ---"

    # 启动从节点(带认证, 不需要no_auth)
    if is_local_host "${ip}" "${LOCAL_IP_LIST}"; then
        sh ${TARGET_SCRIPT_PATH}/create_mongodb_inst.sh ${TARGET_SCRIPT_PATH} ${node_db_path} $(get_local_ip_by_type "${ip}" "${LOCAL_IP_LIST}") ${node_port} "${RS_NAME}" ${TARGET_SCRIPT_PATH}/keyfile "" || {
            echo "错误：从节点 $(format_host_port ${ip} ${node_port}) 启动失败" >&2
            echo "请检查日志: ${node_db_path}/mongod.log" >&2
            exit 1
        }
    else
        ssh root@${ip} "source ~/.bash_profile 2>/dev/null; sh ${TARGET_SCRIPT_PATH}/create_mongodb_inst.sh ${TARGET_SCRIPT_PATH} ${node_db_path} ${ip} ${node_port} \"${RS_NAME}\" ${TARGET_SCRIPT_PATH}/keyfile \"\"" || {
            echo "错误：从节点 $(format_host_port ${ip} ${node_port}) 启动失败" >&2
            echo "请检查日志: ${node_db_path}/mongod.log" >&2
            exit 1
        }
    fi

    echo "从节点 $(format_host_port ${ip} ${node_port}) 启动成功."
    sleep 3

    # 从主节点添加该从节点到复制集
    # rs.add 中 host 使用 iplist 中的原始 IP/域名, 保证复制集成员注册地址一致
    register_host_port=$(format_host_port ${ip} ${node_port})
    echo "从主节点添加从节点: ${register_host_port}..."
    if is_local_host "${PRIMARY_IP}" "${LOCAL_IP_LIST}"; then
        mongosh --host $(format_host_port $(get_local_ip_by_type "${PRIMARY_IP}" "${LOCAL_IP_LIST}") ${PRIMARY_PORT}) -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "rs.add({_id: ${i}, host: \"${register_host_port}\", priority: 1, votes: 1})" || {
            echo "错误：添加从节点 ${register_host_port} 失败" >&2
            exit 1
        }
    else
        ssh root@${PRIMARY_IP} "source ~/.bash_profile 2>/dev/null; mongosh --host $(format_host_port ${PRIMARY_IP} ${PRIMARY_PORT}) -u \"${TARGET_USER}\" -p \"${TARGET_PWD}\" --authenticationDatabase admin --eval \"rs.add({_id: ${i}, host: \\\"${register_host_port}\\\", priority: 1, votes: 1})\"" || {
            echo "错误：添加从节点 ${register_host_port} 失败" >&2
            exit 1
        }
    fi

    echo "从节点 ${register_host_port} 添加成功."
    sleep 2
done

echo "所有从节点启动并添加完成."

##############################
# 步骤4: 验证复制集状态
##############################
echo ""
echo "========== 步骤4: 验证复制集状态 =========="

sleep 5

echo "复制集配置:"
if is_local_host "${PRIMARY_IP}" "${LOCAL_IP_LIST}"; then
    mongosh --host $(format_host_port $(get_local_ip_by_type "${PRIMARY_IP}" "${LOCAL_IP_LIST}") ${PRIMARY_PORT}) -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "rs.conf()" || {
        echo "警告：复制集配置查询失败" >&2
    }
else
    ssh root@${PRIMARY_IP} "source ~/.bash_profile 2>/dev/null; mongosh --host $(format_host_port ${PRIMARY_IP} ${PRIMARY_PORT}) -u \"${TARGET_USER}\" -p \"${TARGET_PWD}\" --authenticationDatabase admin --eval \"rs.conf()\"" || {
        echo "警告：复制集配置查询失败" >&2
    }
fi

echo ""
echo "复制集状态:"
if is_local_host "${PRIMARY_IP}" "${LOCAL_IP_LIST}"; then
    mongosh --host $(format_host_port $(get_local_ip_by_type "${PRIMARY_IP}" "${LOCAL_IP_LIST}") ${PRIMARY_PORT}) -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "rs.status()" || {
        echo "警告：复制集状态查询失败" >&2
    }
else
    ssh root@${PRIMARY_IP} "source ~/.bash_profile 2>/dev/null; mongosh --host $(format_host_port ${PRIMARY_IP} ${PRIMARY_PORT}) -u \"${TARGET_USER}\" -p \"${TARGET_PWD}\" --authenticationDatabase admin --eval \"rs.status()\"" || {
        echo "警告：复制集状态查询失败" >&2
    }
fi

##############################
# 完成
##############################
echo ""
echo "=========================================="
echo "MongoDB 复制集搭建完成!"
echo "=========================================="
echo "复制集名: ${RS_NAME}"
echo ""
echo "节点列表:"
for i in "${!NODE_ARRAY[@]}"; do
    item="${NODE_ARRAY[$i]}"
    ip=$(echo "${item}" | awk '{print $1}')
    node_port=$(echo "${item}" | awk '{print $2}')
    if [ $i -eq 0 ]; then
        echo "  [初始主节点] $(format_host_port ${ip} ${node_port})"
    else
        echo "  [从节点 $i]   $(format_host_port ${ip} ${node_port})"
    fi
done
echo ""
echo "数据目录:"
echo "  ${WORK_PATH}/${RS_NAME}_1  (主节点)"
for i in "${!NODE_ARRAY[@]}"; do
    if [ $i -eq 0 ]; then
        continue
    fi
    echo "  ${WORK_PATH}/${RS_NAME}_$(($i + 1))  (从节点 $i)"
done
echo ""
echo "连接方式:"
echo "  mongosh --host $(format_host_port ${PRIMARY_IP} ${PRIMARY_PORT}) -u ${TARGET_USER} -p <密码> --authenticationDatabase admin"
echo ""
echo "查看复制集状态:"
echo "  mongosh --host $(format_host_port ${PRIMARY_IP} ${PRIMARY_PORT}) -u ${TARGET_USER} -p <密码> --authenticationDatabase admin --eval \"rs.status()\""
echo "=========================================="
