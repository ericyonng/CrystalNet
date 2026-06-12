#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 向已有MongoDB分片集群扩展mongos路由节点
# 用法: sh add_mongos.sh <iplist.txt> <用户名> <密码> <软件包安装路径> <数据库数据路径> [keyfile路径] [数据库名]
#
# IP_LIST_FILE: 包含config节点信息(用于构建configDB)和新增的mongos节点
#   格式: 类型 复制集前缀 IP 端口
#   示例:
#     config testsuit_rs 192.168.1.1 27010
#     config testsuit_rs 192.168.1.2 27010
#     config testsuit_rs 192.168.1.3 27010
#     mongos testsuit_rs 192.168.1.10 27017
#     mongos testsuit_rs 192.168.1.11 27017
# TARGET_USER: 集群管理员用户名
# TARGET_PWD: 集群管理员密码
# INSTALL_PATH: mongodb程序安装目录
# DATA_PATH: mongodb数据库数据目录
# KEYFILE_PATH: 集群keyfile绝对路径(可选, 默认/root/mongodb_script/keyfile)
# DB_NAME: 数据库名称（用于目录命名, 可选, 默认admin）

# 当前脚本路径
SCRIPT_PATH="$(cd $(dirname $0); pwd)"

# IP列表文件
IP_LIST_FILE=$1
# 用户名
TARGET_USER=$2
# 密码
TARGET_PWD=$3
# mongodb程序安装目录
INSTALL_PATH=$4
# 数据库数据路径
DATA_PATH=$5
# keyfile路径(可选, 默认/root/mongodb_script/keyfile)
KEYFILE_PATH=${6:-/root/mongodb_script/keyfile}
# 数据库名(可选, 默认admin)
DB_NAME=${7:-admin}

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
echo "MongoDB 扩展 Mongos 路由节点脚本"
echo "=========================================="
echo "IP_LIST_FILE    : ${IP_LIST_FILE}"
echo "TARGET_USER     : ${TARGET_USER}"
echo "TARGET_PWD      : ******"
echo "INSTALL_PATH    : ${INSTALL_PATH}"
echo "DATA_PATH       : ${DATA_PATH}"
echo "KEYFILE_PATH    : ${KEYFILE_PATH}"
echo "DB_NAME         : ${DB_NAME}"
echo "=========================================="

##############################
# 解析 iplist.txt, 提取 config 和 mongos 节点
##############################
CONFIG_SVR_ARRAY=()        # config节点数组, 元素格式: "ip port", 用于构建configDB
MONGOS_NEW_ARRAY=()        # 新增mongos节点数组, 元素格式: "ip port"
declare -A RS_PREFIX_MAP   # key=节点类型(config/mongos), value=该组的复制集前缀

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
        MONGOS_NEW_ARRAY+=("${ip} ${node_port}")
    else
        echo "警告：add_mongos 只处理 config(构建configDB) 和 mongos(新增) 类型, 跳过: ${node_type}"
    fi
done

# 校验必须有config和至少一个mongos
if [ ${#CONFIG_SVR_ARRAY[@]} -eq 0 ]; then
    echo "错误：iplist.txt中没有config节点(需要config条目来构建configDB连接串)"
    exit 1
fi

if [ ${#MONGOS_NEW_ARRAY[@]} -eq 0 ]; then
    echo "错误：iplist.txt中没有mongos节点(没有需要新增的mongos)"
    exit 1
fi

##############################
# 构建 configDB 地址串
##############################
CONFIG_RS_NAME="${RS_PREFIX_MAP[config]}_config"
echo "Config 复制集名: ${CONFIG_RS_NAME}"

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

##############################
# 打印分组结果
##############################
echo ""
echo "========== 节点分组结果 =========="
echo "Config 节点 (${#CONFIG_SVR_ARRAY[@]}个, 仅用于构建configDB):"
for i in "${!CONFIG_SVR_ARRAY[@]}"; do
    echo "  [$i] ${CONFIG_SVR_ARRAY[$i]}"
done
echo ""
echo "新增 Mongos 节点 (${#MONGOS_NEW_ARRAY[@]}个):"
for i in "${!MONGOS_NEW_ARRAY[@]}"; do
    echo "  [$i] ${MONGOS_NEW_ARRAY[$i]}"
done
echo "=================================="

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
echo "LOCAL_IPV4: ${LOCAL_IPV4}, LOCAL_IPV6: ${LOCAL_IPV6}"

##############################
# 辅助函数
##############################

# SSH 封装, IPv6 地址自动处理方括号 (类似 scp_to_host)
# $1: host, $2...: ssh 参数和命令
ssh_to_host() {
    local host="$1"
    shift
    if is_ipv6 "${host}"; then
        ssh "root@[${host}]" "$@"
    else
        ssh "root@${host}" "$@"
    fi
}

exec_on_host() {
    local ip=$1
    local cmd=$2
    if is_local_host "${ip}" "${LOCAL_IP_LIST}"; then
        bash -c "${cmd}"
    else
        ssh_to_host ${ip} "source ~/.bash_profile 2>/dev/null; ${cmd}"
    fi
}

# 判断目标机器是否已安装 mongodb (检测 mongos 命令)
is_mongo_installed() {
    local ip=$1
    local INSTALL_PATH_CLEAN=${INSTALL_PATH%/}
    if is_local_host "${ip}" "${LOCAL_IP_LIST}"; then
        if which mongos &>/dev/null && [ -x "$(which mongos)" ]; then
            return 0
        fi
        if [ -d "${INSTALL_PATH_CLEAN}" ] && [ -x "${INSTALL_PATH_CLEAN}/mongodb-linux-x86_64-rhel88-8.0.6/bin/mongos" ]; then
            return 0
        fi
        return 1
    else
        if ssh_to_host ${ip} "which mongos &>/dev/null && [ -x \"\$(which mongos)\""; then
            return 0
        fi
        if ssh_to_host ${ip} "[ -d '${INSTALL_PATH_CLEAN}' ] && [ -x '${INSTALL_PATH_CLEAN}/mongodb-linux-x86_64-rhel88-8.0.6/bin/mongos' ]"; then
            return 0
        fi
        return 1
    fi
}

##############################
# 环境准备: 检查本机 mongo 安装
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

# 收集所有新增mongos节点的IP(去重)
declare -A ALL_IPS
for item in "${MONGOS_NEW_ARRAY[@]}"; do
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
                # keyfile 处理: 默认路径不存在则检查源keyfile
                if [ ! -f "${TARGET_SCRIPT_PATH}/keyfile" ]; then
                    if [ -f "${KEYFILE_PATH}" ]; then
                        cp -f ${KEYFILE_PATH} ${TARGET_SCRIPT_PATH}/keyfile || {
                            echo "错误：拷贝 keyfile 失败" >&2
                            exit 1
                        }
                        chmod 600 ${TARGET_SCRIPT_PATH}/keyfile
                        echo "本地 keyfile 从 ${KEYFILE_PATH} 拷贝到 ${TARGET_SCRIPT_PATH}/keyfile"
                    else
                        echo "错误: keyfile 不存在: ${KEYFILE_PATH}" >&2
                        exit 1
                    fi
                else
                    echo "本地 keyfile 已存在: ${TARGET_SCRIPT_PATH}/keyfile"
                fi
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
                ssh_to_host ${ip} "mkdir -p ${TARGET_SCRIPT_PATH}" || {
                    echo "错误：${ip} 创建 ${TARGET_SCRIPT_PATH} 失败" >&2
                    exit 1
                }
                # keyfile 处理
                echo "拷贝keyfile到 ${ip}..."
                if [ ! -f "${KEYFILE_PATH}" ]; then
                    echo "错误: 源 keyfile 不存在: ${KEYFILE_PATH}" >&2
                    exit 1
                fi
                scp_to_host ${ip} ${KEYFILE_PATH} ${TARGET_SCRIPT_PATH}/keyfile || {
                    echo "错误：scp keyfile 到 ${ip} 失败" >&2
                    exit 1
                }
                ssh_to_host ${ip} "chmod 600 ${TARGET_SCRIPT_PATH}/keyfile"
                echo "远程 ${ip} keyfile 已准备: ${TARGET_SCRIPT_PATH}/keyfile"
            else
                echo "远程节点 ip:${ip} 未安装 mongodb, 执行 init_package 和 init_env..."

                # 安全检查: 确保路径深度足够(至少2层)
                TMP_DIR_DEPTH=$(echo "${TMP_DIR}" | tr -cd '/' | wc -c)
                if [ ${TMP_DIR_DEPTH} -lt 2 ] || [ -z "${TMP_DIR}" ]; then
                    echo "错误： TMP_DIR 路径不安全: ${TMP_DIR}" >&2
                    exit 1
                fi
                ssh_to_host ${ip} "rm -rf ${TMP_DIR}" || {
                    echo "错误：移除 ${TMP_DIR} 失败" >&2
                    exit 1
                }
                ssh_to_host ${ip} "mkdir -p ${TMP_DIR}" || {
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
                ssh_to_host ${ip} "sh ${TMP_DIR}/init_package.sh ${TMP_DIR}/${TGZ_FILE_NAME} ${TARGET_SCRIPT_PATH}" || {
                    echo "错误：${ip} init_package.sh 失败" >&2
                    exit 1
                }

                echo "拷贝keyfile到 ${ip}..."
                if [ ! -f "${KEYFILE_PATH}" ]; then
                    echo "错误: 源 keyfile 不存在: ${KEYFILE_PATH}" >&2
                    exit 1
                fi
                scp_to_host ${ip} ${KEYFILE_PATH} ${TARGET_SCRIPT_PATH}/keyfile || {
                    echo "错误：scp keyfile 到 ${ip} 失败" >&2
                    exit 1
                }

                ssh_to_host ${ip} "source ~/.bash_profile 2>/dev/null; sh ${TARGET_SCRIPT_PATH}/init_env.sh ${TARGET_SCRIPT_PATH} ${INSTALL_PATH}" || {
                    echo "错误：${ip} init_env.sh 失败" >&2
                    exit 1
                }
                echo "$ip init_env.sh 成功"
            fi
            is_ip_init_dict[$ip]=1
        fi
    fi
done

echo "所有新增节点环境初始化完成."

##############################
# 启动各新增 mongos 节点
##############################
echo ""
echo "========== 启动新增 mongos 节点 =========="

for i in "${!MONGOS_NEW_ARRAY[@]}"; do
    item="${MONGOS_NEW_ARRAY[$i]}"
    ip=$(echo "${item}" | awk '{print $1}')
    node_port=$(echo "${item}" | awk '{print $2}')
    mongos_db_subdir="${DB_NAME}_mongos_$(($i + 1))"
    # 移除 DATA_PATH 末尾的斜杠，避免路径中出现双斜杠
    DATA_PATH_CLEAN=${DATA_PATH%/}
    mongos_db_path="${DATA_PATH_CLEAN}/${mongos_db_subdir}"

    echo "启动新增mongos节点 [$(($i + 1))]: $(format_host_port ${ip} ${node_port}), 数据目录: ${mongos_db_path}..."

    # 确保 keyfile 存在于目标机器的 TARGET_SCRIPT_PATH 目录
    if is_local_host "${ip}" "${LOCAL_IP_LIST}"; then
        # 本地节点: 确保 keyfile 存在
        if [ ! -f "${TARGET_SCRIPT_PATH}/keyfile" ]; then
            echo "警告: 本地 keyfile 不存在, 尝试从源 keyfile 复制..."
            mkdir -p ${TARGET_SCRIPT_PATH}
            if [ ! -f "${KEYFILE_PATH}" ]; then
                echo "错误: 源 keyfile 不存在: ${KEYFILE_PATH}" >&2
                exit 1
            fi
            cp -f ${KEYFILE_PATH} ${TARGET_SCRIPT_PATH}/keyfile || {
                echo "错误: 复制 keyfile ${KEYFILE_PATH} => ${TARGET_SCRIPT_PATH}/keyfile 失败" >&2
                exit 1
            }
            chmod 600 ${TARGET_SCRIPT_PATH}/keyfile
        fi

        local_ip_for_bind=$(get_local_ip_by_type "${ip}" "${LOCAL_IP_LIST}")
        echo "执行本地启动: create_mongos_inst.sh ${TARGET_SCRIPT_PATH} ${mongos_db_path} ${node_port} ${CONFIG_RS_NAME} ${TARGET_SCRIPT_PATH}/keyfile ${CONFIG_SVR_ADDR_STR}"
        sh ${TARGET_SCRIPT_PATH}/create_mongos_inst.sh ${TARGET_SCRIPT_PATH} ${mongos_db_path} ${node_port} "${CONFIG_RS_NAME}" ${TARGET_SCRIPT_PATH}/keyfile "${CONFIG_SVR_ADDR_STR}" || {
            echo "错误：mongos节点 $(format_host_port ${ip} ${node_port}) 启动失败" >&2
            echo "请检查日志: ${mongos_db_path}/mongos.log" >&2
            exit 1
        }
    else
        # 远程节点: 确保 keyfile 存在
        KEYFILE_EXISTS=$(ssh_to_host ${ip} "[ -f '${TARGET_SCRIPT_PATH}/keyfile' ] && echo 'yes' || echo 'no'")
        if [ "${KEYFILE_EXISTS}" != "yes" ]; then
            echo "警告: 远程 ${ip} keyfile 不存在, 尝试从源 keyfile 复制..."
            ssh_to_host ${ip} "mkdir -p ${TARGET_SCRIPT_PATH}" || {
                echo "错误: ${ip} 创建目录 ${TARGET_SCRIPT_PATH} 失败" >&2
                exit 1
            }
            if [ ! -f "${KEYFILE_PATH}" ]; then
                echo "错误: 源 keyfile 不存在: ${KEYFILE_PATH}" >&2
                exit 1
            fi
            scp_to_host ${ip} ${KEYFILE_PATH} ${TARGET_SCRIPT_PATH}/keyfile || {
                echo "错误: scp keyfile 到 ${ip} 失败" >&2
                exit 1
            }
            ssh_to_host ${ip} "chmod 600 ${TARGET_SCRIPT_PATH}/keyfile"
        fi

        echo "执行远程启动 ${ip}: create_mongos_inst.sh ..."
        ssh_to_host ${ip} "source ~/.bash_profile 2>/dev/null; sh ${TARGET_SCRIPT_PATH}/create_mongos_inst.sh ${TARGET_SCRIPT_PATH} ${mongos_db_path} ${node_port} \"${CONFIG_RS_NAME}\" ${TARGET_SCRIPT_PATH}/keyfile \"${CONFIG_SVR_ADDR_STR}\"" || {
            echo "错误：mongos节点 $(format_host_port ${ip} ${node_port}) 启动失败" >&2
            echo "请检查日志: ${mongos_db_path}/mongos.log" >&2
            exit 1
        }
    fi

    echo "Mongos节点 $(format_host_port ${ip} ${node_port}) 启动成功."
    sleep 3
done

echo "所有新增 mongos 路由节点启动完成."

##############################
# 连接验证
##############################
echo ""
echo "========== 验证新增 mongos 节点 =========="

MONGOS_FIRST_IP=$(echo "${MONGOS_NEW_ARRAY[0]}" | awk '{print $1}')
MONGOS_FIRST_PORT=$(echo "${MONGOS_NEW_ARRAY[0]}" | awk '{print $2}')

# 获取可达地址
MONGOS_FIRST_IP=$(get_reachable_host "${MONGOS_FIRST_IP}" "$(get_local_ip_by_type "${MONGOS_FIRST_IP}" "${LOCAL_IP_LIST}")")
MONGOS_FIRST_HOST_PORT=$(format_host_port ${MONGOS_FIRST_IP} ${MONGOS_FIRST_PORT})

echo "连接第一个新增mongos节点验证: ${MONGOS_FIRST_HOST_PORT}..."
sleep 3  # 等待mongos完全连接config server

# 验证连接
echo "验证 ping..."
if is_local_host "$(echo "${MONGOS_NEW_ARRAY[0]}" | awk '{print $1}')" "${LOCAL_IP_LIST}"; then
    mongosh --host ${MONGOS_FIRST_HOST_PORT} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "db.runCommand({ping:1})" || {
        echo "警告：新mongos节点 ${MONGOS_FIRST_HOST_PORT} ping 失败，请检查集群状态"
    }
else
    local verify_ip=$(echo "${MONGOS_NEW_ARRAY[0]}" | awk '{print $1}')
    ssh_to_host ${verify_ip} "source ~/.bash_profile 2>/dev/null; mongosh --host ${MONGOS_FIRST_HOST_PORT} -u \"${TARGET_USER}\" -p \"${TARGET_PWD}\" --authenticationDatabase admin --eval \"db.runCommand({ping:1})\"" || {
        echo "警告：新mongos节点 ${MONGOS_FIRST_HOST_PORT} ping 失败，请检查集群状态"
    }
fi

echo ""
echo "查询分片状态 sh.status():"
if is_local_host "$(echo "${MONGOS_NEW_ARRAY[0]}" | awk '{print $1}')" "${LOCAL_IP_LIST}"; then
    mongosh --host ${MONGOS_FIRST_HOST_PORT} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "sh.status()" || {
        echo "警告：查询分片状态失败" >&2
    }
else
    local verify_ip=$(echo "${MONGOS_NEW_ARRAY[0]}" | awk '{print $1}')
    ssh_to_host ${verify_ip} "source ~/.bash_profile 2>/dev/null; mongosh --host ${MONGOS_FIRST_HOST_PORT} -u \"${TARGET_USER}\" -p \"${TARGET_PWD}\" --authenticationDatabase admin --eval \"sh.status()\"" || {
        echo "警告：查询分片状态失败" >&2
    }
fi

##############################
# 完成
##############################
echo ""
echo "=========================================="
echo "MongoDB 扩展 Mongos 路由节点完成!"
echo "=========================================="
echo "Config 复制集: ${CONFIG_RS_NAME}"
echo "ConfigDB 地址串: ${CONFIG_SVR_ADDR_STR}"
echo ""
echo "新增 Mongos 路由节点 (${#MONGOS_NEW_ARRAY[@]}个):"
for i in "${!MONGOS_NEW_ARRAY[@]}"; do
    item="${MONGOS_NEW_ARRAY[$i]}"
    ip=$(echo "${item}" | awk '{print $1}')
    node_port=$(echo "${item}" | awk '{print $2}')
    echo "  [$(($i + 1))] $(format_host_port ${ip} ${node_port})"
done
echo ""
echo "连接方式:"
echo "  mongosh --host ${MONGOS_FIRST_HOST_PORT} -u ${TARGET_USER} -p <密码> --authenticationDatabase admin"
echo "=========================================="
