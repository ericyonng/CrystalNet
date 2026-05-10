#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 安全关闭 MongoDB 分片集群
# 用法: sh stop_mongo_shard_cluster.sh <iplist.txt> <用户名> <密码>
#
# IP_LIST_FILE: 节点类型 复制集前缀 IP 端口: config testsuit_rs 127.0.0.1 27010 / shard1 testsuit_rs 127.0.0.1 27011 / mongos testsuit_rs 127.0.0.1 27017
# TARGET_USER: 用户名
# TARGET_PWD: 密码
#
# 注意: 无需传入数据路径和数据库名, 脚本通过 ps 自动检测 mongod/mongos 进程的 -f conf 路径
#
# 关闭顺序(官方推荐):
#   1. 先关闭 mongos 路由节点
#   2. 再关闭 shard 从节点(secondary)
#   3. 再关闭 shard 主节点(primary) — 降级为从节点后关闭
#   4. 最后关闭 config 从节点, 再关闭 config 主节点

# 当前脚本路径
SCRIPT_PATH="$(cd $(dirname $0); pwd)"

# IP列表文件
IP_LIST_FILE=$1
# 用户名
TARGET_USER=$2
# 密码
TARGET_PWD=$3

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

# 加载公共
. ${SCRIPT_PATH}/common/common_define.sh
. ${SCRIPT_PATH}/common/funcs.sh

echo "=========================================="
echo "MongoDB 分片集群安全关闭脚本"
echo "=========================================="
echo "IP_LIST_FILE    : ${IP_LIST_FILE}"
echo "TARGET_USER     : ${TARGET_USER}"
echo "TARGET_PWD      : ******"
echo "=========================================="

##############################
# 解析 iplist.txt, 按类型分组
##############################
declare -A SHARD_GROUPS    # key=shard名, value=节点列表(分号分隔: "ip port;ip port")
CONFIG_SVR_ARRAY=()        # config节点数组, 元素格式: "ip port"
MONGOS_SVR_ARRAY=()        # mongos节点数组, 元素格式: "ip port"
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

# 校验
if [ ${#CONFIG_SVR_ARRAY[@]} -eq 0 ]; then
    echo "错误：iplist.txt中没有config节点"
    exit 1
fi

if [ ${#SHARD_NAME_LIST[@]} -eq 0 ]; then
    echo "错误：iplist.txt中没有shard节点"
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
# 辅助函数: 通过监听端口获取对应 mongod/mongos 进程的 -f conf 路径
# mongod/mongos 启动命令为 "mongod -f xxx.conf" 或 "mongos -f xxx.conf",
# 进程命令行中不包含端口号, 因此需先通过 netstat/ss 找到监听该端口的PID,
# 再从 /proc/<PID>/cmdline 提取 -f 参数值
# $1: ip, $2: port, $3: 进程类型(mongod/mongos)
# 输出: conf文件绝对路径, 如果找不到则为空
##############################
get_conf_path_by_port() {
    local ip=$1
    local port=$2
    local proc_type=$3

    if is_local_host "${ip}" "${LOCAL_IP_LIST}"; then
        # 通过 ss 找到监听该端口的进程PID, 再从 /proc/PID/cmdline 提取 -f 参数
        local pid=$(ss -tlnp "sport = :${port}" 2>/dev/null | grep -oP 'pid=\K[0-9]+' | head -1)
        if [ -n "${pid}" ] && [ -r "/proc/${pid}/cmdline" ]; then
            tr '\0' ' ' < "/proc/${pid}/cmdline" | grep -oP '(?<=-f\s)\S+' | head -1
        else
            # ss 不可用时回退: 遍历所有 proc_type 进程, 检查其监听端口
            for p in $(ps -aux | grep "${proc_type}" | sed '/grep/d' | awk '{print $2}'); do
                if [ -r "/proc/${p}/cmdline" ]; then
                    local listen_ports=$(ss -tlnp "pid = ${p}" 2>/dev/null | grep -oP 'pid=\K[0-9]+|:\K[0-9]+' | grep -v "${p}")
                    if echo "${listen_ports}" | grep -qw "${port}"; then
                        tr '\0' ' ' < "/proc/${p}/cmdline" | grep -oP '(?<=-f\s)\S+' | head -1
                        return
                    fi
                fi
            done
        fi
    else
        ssh root@${ip} "source ~/.bash_profile 2>/dev/null; pid=\$(ss -tlnp 'sport = :${port}' 2>/dev/null | grep -oP 'pid=\K[0-9]+' | head -1); if [ -n \"\$pid\" ] && [ -r \"/proc/\$pid/cmdline\" ]; then tr '\0' ' ' < \"/proc/\$pid/cmdline\" | grep -oP '(?<=-f\s)\S+' | head -1; else for p in \$(ps -aux | grep '${proc_type}' | sed '/grep/d' | awk '{print \$2}'); do if [ -r \"/proc/\$p/cmdline\" ]; then listen_ports=\$(ss -tlnp 'pid = \$p' 2>/dev/null | grep -oP 'pid=\K[0-9]+|:\K[0-9]+' | grep -v \"\$p\"); if echo \"\$listen_ports\" | grep -qw '${port}'; then tr '\0' ' ' < \"/proc/\$p/cmdline\" | grep -oP '(?<=-f\s)\S+' | head -1; exit 0; fi; fi; done; fi"
    fi
}

##############################
# 辅助函数: 通过监听端口获取对应进程的PID
# $1: ip, $2: port
# 输出: PID, 如果找不到则为空
##############################
get_pid_by_port() {
    local ip=$1
    local port=$2

    if is_local_host "${ip}" "${LOCAL_IP_LIST}"; then
        ss -tlnp "sport = :${port}" 2>/dev/null | grep -oP 'pid=\K[0-9]+' | head -1
    else
        ssh root@${ip} "source ~/.bash_profile 2>/dev/null; ss -tlnp 'sport = :${port}' 2>/dev/null | grep -oP 'pid=\K[0-9]+' | head -1"
    fi
}

##############################
# 辅助函数: 在mongosh中执行命令(带认证)
# $1: host, $2: port, $3: eval表达式
##############################
mongosh_eval_auth() {
    local host="$1"
    local port="$2"
    local eval_expr="$3"
    local host_port=$(format_host_port ${host} ${port})
    mongosh --host ${host_port} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "${eval_expr}"
}

##############################
# 辅助函数: 通过SSH在远程主机的mongosh中执行命令(带认证)
# $1: remote_ip, $2: mongosh_host, $3: mongosh_port, $4: eval表达式
##############################
mongosh_eval_auth_remote() {
    local remote_ip="$1"
    local mongosh_host="$2"
    local mongosh_port="$3"
    local eval_expr="$4"
    local host_port=$(format_host_port ${mongosh_host} ${mongosh_port})
    ssh root@${remote_ip} "source ~/.bash_profile 2>/dev/null; mongosh --host ${host_port} -u \"${TARGET_USER}\" -p \"${TARGET_PWD}\" --authenticationDatabase admin --eval \"${eval_expr}\""
}

##############################
# 辅助函数: 在最接近的主机上执行mongosh命令
# $1: target_host(节点IP), $2: target_port, $3: eval表达式
##############################
mongosh_exec_on_nearest() {
    local target_host="$1"
    local target_port="$2"
    local eval_expr="$3"

    if is_local_host "${target_host}" "${LOCAL_IP_LIST}"; then
        mongosh_eval_auth "${LOCAL_IP}" "${target_port}" "${eval_expr}"
    else
        mongosh_eval_auth_remote "${target_host}" "${target_host}" "${target_port}" "${eval_expr}"
    fi
}

##############################
# 步骤1: 关闭 mongos 路由节点
##############################
echo ""
echo "========== 步骤1: 关闭 mongos 路由节点 =========="

for i in "${!MONGOS_SVR_ARRAY[@]}"; do
    item="${MONGOS_SVR_ARRAY[$i]}"
    ip=$(echo "${item}" | awk '{print $1}')
    node_port=$(echo "${item}" | awk '{print $2}')

    # mongos 通过 -f conf 启动, ps中不包含端口号, 需先获取conf路径再通过conf匹配进程
    mongos_conf_path=$(get_conf_path_by_port "${ip}" "${node_port}" "mongos")

    echo "关闭 mongos 节点 [$i]: $(format_host_port ${ip} ${node_port})..."

    if is_local_host "${ip}" "${LOCAL_IP_LIST}"; then
        # 本机: mongos 不支持 --shutdown, 必须通过 SIGTERM/SIGINT 关闭
        if [ -n "${mongos_conf_path}" ]; then
            PID_LIST="$(ps -aux | grep "mongos" | grep "${mongos_conf_path}" | sed '/grep/d' | awk '{print $2}')"
        else
            PID_LIST="$(ps -aux | grep "mongos" | sed '/grep/d' | awk '{print $2}')"
        fi
        if [ -z "${PID_LIST}" ]; then
            echo "mongos $(format_host_port ${ip} ${node_port}) 未在运行, 跳过"
            continue
        fi
        for pid in ${PID_LIST}; do
            kill -2 ${pid}
            echo "发送 SIGINT(2) 到 mongos pid:${pid}..."
        done
        # 等待进程退出(最多30秒)
        for pid in ${PID_LIST}; do
            local_wait_count=0
            while [ -n "$(ps -p ${pid} -o pid= 2>/dev/null)" ] && [ ${local_wait_count} -lt 30 ]; do
                sleep 1
                local_wait_count=$((local_wait_count + 1))
            done
            if [ -n "$(ps -p ${pid} -o pid= 2>/dev/null)" ]; then
                echo "警告: mongos pid:${pid} 未在30秒内退出, 强制终止"
                kill -9 ${pid} 2>/dev/null
            fi
        done
        echo "mongos $(format_host_port ${ip} ${node_port}) 已关闭"
    else
        # 远程: 通过SSH关闭
        if [ -n "${mongos_conf_path}" ]; then
            ssh root@${ip} "source ~/.bash_profile 2>/dev/null; PID_LIST=\$(ps -aux | grep 'mongos' | grep '${mongos_conf_path}' | sed '/grep/d' | awk '{print \$2}'); if [ -z \"\$PID_LIST\" ]; then echo 'mongos $(format_host_port ${ip} ${node_port}) not running, skip'; exit 0; fi; for pid in \${PID_LIST}; do kill -2 \$pid; echo \"SIGINT(2) -> mongos pid:\$pid\"; done; for pid in \${PID_LIST}; do wait_count=0; while [ -n \"\$(ps -p \$pid -o pid= 2>/dev/null)\" ] && [ \$wait_count -lt 30 ]; do sleep 1; wait_count=\$((wait_count+1)); done; if [ -n \"\$(ps -p \$pid -o pid= 2>/dev/null)\" ]; then echo 'force kill mongos'; kill -9 \$pid 2>/dev/null; fi; done; echo 'mongos closed'"
        else
            ssh root@${ip} "source ~/.bash_profile 2>/dev/null; PID_LIST=\$(ps -aux | grep 'mongos' | sed '/grep/d' | awk '{print \$2}'); if [ -z \"\$PID_LIST\" ]; then echo 'mongos $(format_host_port ${ip} ${node_port}) not running, skip'; exit 0; fi; for pid in \${PID_LIST}; do kill -2 \$pid; echo \"SIGINT(2) -> mongos pid:\$pid\"; done; for pid in \${PID_LIST}; do wait_count=0; while [ -n \"\$(ps -p \$pid -o pid= 2>/dev/null)\" ] && [ \$wait_count -lt 30 ]; do sleep 1; wait_count=\$((wait_count+1)); done; if [ -n \"\$(ps -p \$pid -o pid= 2>/dev/null)\" ]; then echo 'force kill mongos'; kill -9 \$pid 2>/dev/null; fi; done; echo 'mongos closed'"
        fi
    fi

    sleep 2
done

echo "所有 mongos 路由节点已关闭."

##############################
# 步骤2: 关闭 shard 从节点, 再关闭主节点
##############################
echo ""
echo "========== 步骤2: 关闭 shard 分片节点 =========="

for shard_name in "${SHARD_NAME_LIST[@]}"; do
    echo ""
    echo "--- 关闭分片: ${shard_name} ---"

    SHARD_RS_NAME="${RS_PREFIX_MAP[$shard_name]}_${shard_name}"
    IFS=';' read -ra shard_nodes <<< "${SHARD_GROUPS[$shard_name]}"

    if [ ${#shard_nodes[@]} -eq 0 ]; then
        echo "警告：分片 ${shard_name} 没有节点, 跳过"
        continue
    fi

    # 2.1 先关闭所有从节点(secondary)
    echo "  关闭 ${shard_name} 从节点(secondary)..."
    for j in "${!shard_nodes[@]}"; do
        if [ $j -eq 0 ]; then
            continue  # 跳过主节点(第一个节点)
        fi

        node="${shard_nodes[$j]}"
        ip=$(echo "${node}" | awk '{print $1}')
        node_port=$(echo "${node}" | awk '{print $2}')
        mongod_conf_path=$(get_conf_path_by_port "${ip}" "${node_port}" "mongod")

        echo "  关闭 ${shard_name} 从节点 [$j]: $(format_host_port ${ip} ${node_port})..."

        if is_local_host "${ip}" "${LOCAL_IP_LIST}"; then
            if [ -n "${mongod_conf_path}" ] && [ -f "${mongod_conf_path}" ]; then
                mongod -f ${mongod_conf_path} --shutdown 2>/dev/null || {
                    echo "  警告: mongod --shutdown 失败, 尝试 SIGTERM..."
                    PID=$(get_pid_by_port "${ip}" "${node_port}")
                    if [ -n "${PID}" ]; then
                        kill -2 ${PID}
                        sleep 3
                        if [ -n "$(ps -p ${PID} -o pid= 2>/dev/null)" ]; then
                            kill -9 ${PID} 2>/dev/null
                        fi
                    fi
                }
            else
                echo "  未检测到conf文件, 使用SIGTERM关闭..."
                PID=$(get_pid_by_port "${ip}" "${node_port}")
                if [ -n "${PID}" ]; then
                    kill -2 ${PID}
                    sleep 3
                    if [ -n "$(ps -p ${PID} -o pid= 2>/dev/null)" ]; then
                        kill -9 ${PID} 2>/dev/null
                    fi
                else
                    echo "  进程未运行, 跳过"
                fi
            fi
        else
            if [ -n "${mongod_conf_path}" ]; then
                ssh root@${ip} "source ~/.bash_profile 2>/dev/null; mongod -f ${mongod_conf_path} --shutdown 2>/dev/null || { echo 'warn: mongod --shutdown fail, try SIGTERM...'; PID=\$(ss -tlnp 'sport = :${node_port}' 2>/dev/null | grep -oP 'pid=\K[0-9]+' | head -1); if [ -n \"\$PID\" ]; then kill -2 \$PID; sleep 3; if [ -n \"\$(ps -p \$PID -o pid= 2>/dev/null)\" ]; then kill -9 \$PID 2>/dev/null; fi; fi; }"
            else
                echo "  未检测到conf文件, 使用SIGTERM关闭..."
                ssh root@${ip} "source ~/.bash_profile 2>/dev/null; PID=\$(ss -tlnp 'sport = :${node_port}' 2>/dev/null | grep -oP 'pid=\K[0-9]+' | head -1); if [ -n \"\$PID\" ]; then kill -2 \$PID; sleep 3; if [ -n \"\$(ps -p \$PID -o pid= 2>/dev/null)\" ]; then kill -9 \$PID 2>/dev/null; fi; else echo '进程未运行, 跳过'; fi"
            fi
        fi

        sleep 2
        echo "  ${shard_name} 从节点 $(format_host_port ${ip} ${node_port}) 已关闭."
    done

    # 2.2 将主节点(primary)降级为从节点, 然后关闭
    primary_node="${shard_nodes[0]}"
    primary_ip=$(echo "${primary_node}" | awk '{print $1}')
    primary_port=$(echo "${primary_node}" | awk '{print $2}')
    primary_mongod_conf_path=$(get_conf_path_by_port "${primary_ip}" "${primary_port}" "mongod")

    echo "  将 ${shard_name} 主节点降级: rs.stepDown()..."
    mongosh_exec_on_nearest "${primary_ip}" "${primary_port}" "rs.stepDown(120)" 2>/dev/null || {
        echo "  警告: rs.stepDown 失败(可能已经是从节点或不影响关闭), 继续关闭..."
    }

    sleep 5

    echo "  关闭 ${shard_name} 主节点: $(format_host_port ${primary_ip} ${primary_port})..."
    if is_local_host "${primary_ip}" "${LOCAL_IP_LIST}"; then
        if [ -n "${primary_mongod_conf_path}" ] && [ -f "${primary_mongod_conf_path}" ]; then
            mongod -f ${primary_mongod_conf_path} --shutdown 2>/dev/null || {
                echo "  警告: mongod --shutdown 失败, 尝试 SIGTERM..."
                PID=$(get_pid_by_port "${primary_ip}" "${primary_port}")
                if [ -n "${PID}" ]; then
                    kill -2 ${PID}
                    sleep 3
                    if [ -n "$(ps -p ${PID} -o pid= 2>/dev/null)" ]; then
                        kill -9 ${PID} 2>/dev/null
                    fi
                fi
            }
        else
            echo "  未检测到conf文件, 使用SIGTERM关闭..."
            PID=$(get_pid_by_port "${primary_ip}" "${primary_port}")
            if [ -n "${PID}" ]; then
                kill -2 ${PID}
                sleep 3
                if [ -n "$(ps -p ${PID} -o pid= 2>/dev/null)" ]; then
                    kill -9 ${PID} 2>/dev/null
                fi
            else
                echo "  进程未运行, 跳过"
            fi
        fi
    else
        if [ -n "${primary_mongod_conf_path}" ]; then
            ssh root@${primary_ip} "source ~/.bash_profile 2>/dev/null; mongod -f ${primary_mongod_conf_path} --shutdown 2>/dev/null || { echo 'warn: mongod --shutdown fail, try SIGTERM...'; PID=\$(ss -tlnp 'sport = :${primary_port}' 2>/dev/null | grep -oP 'pid=\K[0-9]+' | head -1); if [ -n \"\$PID\" ]; then kill -2 \$PID; sleep 3; if [ -n \"\$(ps -p \$PID -o pid= 2>/dev/null)\" ]; then kill -9 \$PID 2>/dev/null; fi; fi; }"
        else
            echo "  未检测到conf文件, 使用SIGTERM关闭..."
            ssh root@${primary_ip} "source ~/.bash_profile 2>/dev/null; PID=\$(ss -tlnp 'sport = :${primary_port}' 2>/dev/null | grep -oP 'pid=\K[0-9]+' | head -1); if [ -n \"\$PID\" ]; then kill -2 \$PID; sleep 3; if [ -n \"\$(ps -p \$PID -o pid= 2>/dev/null)\" ]; then kill -9 \$PID 2>/dev/null; fi; else echo '进程未运行, 跳过'; fi"
        fi
    fi

    sleep 2
    echo "  ${shard_name} 主节点 $(format_host_port ${primary_ip} ${primary_port}) 已关闭."
    echo "分片 ${shard_name} 所有节点已关闭."
done

echo "所有 shard 分片节点已关闭."

##############################
# 步骤3: 关闭 config 从节点, 再关闭主节点
##############################
echo ""
echo "========== 步骤3: 关闭 config 配置节点 =========="

# 3.1 先关闭所有config从节点
echo "关闭 config 从节点..."
for i in "${!CONFIG_SVR_ARRAY[@]}"; do
    if [ $i -eq 0 ]; then
        continue  # 跳过主节点
    fi

    item="${CONFIG_SVR_ARRAY[$i]}"
    ip=$(echo "${item}" | awk '{print $1}')
    node_port=$(echo "${item}" | awk '{print $2}')
    mongod_conf_path=$(get_conf_path_by_port "${ip}" "${node_port}" "mongod")

    echo "关闭 config 从节点 [$i]: $(format_host_port ${ip} ${node_port})..."

    if is_local_host "${ip}" "${LOCAL_IP_LIST}"; then
        if [ -n "${mongod_conf_path}" ] && [ -f "${mongod_conf_path}" ]; then
            mongod -f ${mongod_conf_path} --shutdown 2>/dev/null || {
                echo "  警告: mongod --shutdown 失败, 尝试 SIGTERM..."
                PID=$(get_pid_by_port "${ip}" "${node_port}")
                if [ -n "${PID}" ]; then
                    kill -2 ${PID}
                    sleep 3
                    if [ -n "$(ps -p ${PID} -o pid= 2>/dev/null)" ]; then
                        kill -9 ${PID} 2>/dev/null
                    fi
                fi
            }
        else
            echo "  未检测到conf文件, 使用SIGTERM关闭..."
            PID=$(get_pid_by_port "${ip}" "${node_port}")
            if [ -n "${PID}" ]; then
                kill -2 ${PID}
                sleep 3
                if [ -n "$(ps -p ${PID} -o pid= 2>/dev/null)" ]; then
                    kill -9 ${PID} 2>/dev/null
                fi
            else
                echo "  进程未运行, 跳过"
            fi
        fi
    else
        if [ -n "${mongod_conf_path}" ]; then
            ssh root@${ip} "source ~/.bash_profile 2>/dev/null; mongod -f ${mongod_conf_path} --shutdown 2>/dev/null || { echo 'warn: mongod --shutdown fail, try SIGTERM...'; PID=\$(ss -tlnp 'sport = :${node_port}' 2>/dev/null | grep -oP 'pid=\K[0-9]+' | head -1); if [ -n \"\$PID\" ]; then kill -2 \$PID; sleep 3; if [ -n \"\$(ps -p \$PID -o pid= 2>/dev/null)\" ]; then kill -9 \$PID 2>/dev/null; fi; fi; }"
        else
            echo "  未检测到conf文件, 使用SIGTERM关闭..."
            ssh root@${ip} "source ~/.bash_profile 2>/dev/null; PID=\$(ss -tlnp 'sport = :${node_port}' 2>/dev/null | grep -oP 'pid=\K[0-9]+' | head -1); if [ -n \"\$PID\" ]; then kill -2 \$PID; sleep 3; if [ -n \"\$(ps -p \$PID -o pid= 2>/dev/null)\" ]; then kill -9 \$PID 2>/dev/null; fi; else echo '进程未运行, 跳过'; fi"
        fi
    fi

    sleep 2
    echo "config 从节点 $(format_host_port ${ip} ${node_port}) 已关闭."
done

# 3.2 将config主节点降级, 然后关闭
CONFIG_PRIMARY_IP=$(echo "${CONFIG_SVR_ARRAY[0]}" | awk '{print $1}')
CONFIG_PRIMARY_PORT=$(echo "${CONFIG_SVR_ARRAY[0]}" | awk '{print $2}')
CONFIG_PRIMARY_MONGOD_CONF=$(get_conf_path_by_port "${CONFIG_PRIMARY_IP}" "${CONFIG_PRIMARY_PORT}" "mongod")

echo "将 config 主节点降级: rs.stepDown()..."
mongosh_exec_on_nearest "${CONFIG_PRIMARY_IP}" "${CONFIG_PRIMARY_PORT}" "rs.stepDown(120)" 2>/dev/null || {
    echo "警告: rs.stepDown 失败(可能已经是从节点或不影响关闭), 继续关闭..."
}

sleep 5

echo "关闭 config 主节点: $(format_host_port ${CONFIG_PRIMARY_IP} ${CONFIG_PRIMARY_PORT})..."
if is_local_host "${CONFIG_PRIMARY_IP}" "${LOCAL_IP_LIST}"; then
    if [ -n "${CONFIG_PRIMARY_MONGOD_CONF}" ] && [ -f "${CONFIG_PRIMARY_MONGOD_CONF}" ]; then
        mongod -f ${CONFIG_PRIMARY_MONGOD_CONF} --shutdown 2>/dev/null || {
            echo "  警告: mongod --shutdown 失败, 尝试 SIGTERM..."
            PID=$(get_pid_by_port "${CONFIG_PRIMARY_IP}" "${CONFIG_PRIMARY_PORT}")
            if [ -n "${PID}" ]; then
                kill -2 ${PID}
                sleep 3
                if [ -n "$(ps -p ${PID} -o pid= 2>/dev/null)" ]; then
                    kill -9 ${PID} 2>/dev/null
                fi
            fi
        }
    else
        echo "  未检测到conf文件, 使用SIGTERM关闭..."
        PID=$(get_pid_by_port "${CONFIG_PRIMARY_IP}" "${CONFIG_PRIMARY_PORT}")
        if [ -n "${PID}" ]; then
            kill -2 ${PID}
            sleep 3
            if [ -n "$(ps -p ${PID} -o pid= 2>/dev/null)" ]; then
                kill -9 ${PID} 2>/dev/null
            fi
        else
            echo "  进程未运行, 跳过"
        fi
    fi
else
    if [ -n "${CONFIG_PRIMARY_MONGOD_CONF}" ]; then
        ssh root@${CONFIG_PRIMARY_IP} "source ~/.bash_profile 2>/dev/null; mongod -f ${CONFIG_PRIMARY_MONGOD_CONF} --shutdown 2>/dev/null || { echo 'warn: mongod --shutdown fail, try SIGTERM...'; PID=\$(ss -tlnp 'sport = :${CONFIG_PRIMARY_PORT}' 2>/dev/null | grep -oP 'pid=\K[0-9]+' | head -1); if [ -n \"\$PID\" ]; then kill -2 \$PID; sleep 3; if [ -n \"\$(ps -p \$PID -o pid= 2>/dev/null)\" ]; then kill -9 \$PID 2>/dev/null; fi; fi; }"
    else
        echo "  未检测到conf文件, 使用SIGTERM关闭..."
        ssh root@${CONFIG_PRIMARY_IP} "source ~/.bash_profile 2>/dev/null; PID=\$(ss -tlnp 'sport = :${CONFIG_PRIMARY_PORT}' 2>/dev/null | grep -oP 'pid=\K[0-9]+' | head -1); if [ -n \"\$PID\" ]; then kill -2 \$PID; sleep 3; if [ -n \"\$(ps -p \$PID -o pid= 2>/dev/null)\" ]; then kill -9 \$PID 2>/dev/null; fi; else echo '进程未运行, 跳过'; fi"
    fi
fi

sleep 2
echo "config 主节点 $(format_host_port ${CONFIG_PRIMARY_IP} ${CONFIG_PRIMARY_PORT}) 已关闭."

##############################
# 完成
##############################
echo ""
echo "=========================================="
echo "MongoDB 分片集群已安全关闭!"
echo "=========================================="
echo "关闭顺序:"
echo "  1. mongos 路由节点 (${#MONGOS_SVR_ARRAY[@]}个)"
echo "  2. shard 分片从节点 + 主节点 (${#SHARD_NAME_LIST[@]}个分片)"
echo "  3. config 配置从节点 + 主节点 (${#CONFIG_SVR_ARRAY[@]}个)"
echo ""
echo "如需重新启动, 请执行:"
echo "  sh install_mongo_shard_cluster.sh ${IP_LIST_FILE} <用户名> <密码> <安装路径> <数据路径> [库名]"
echo "  (注意: 重新启动时脚本会检测已有数据, 不需要重新初始化复制集)"
echo "=========================================="
