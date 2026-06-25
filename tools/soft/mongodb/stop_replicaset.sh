#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 安全关闭 MongoDB 复制集
# 用法: sh stop_replicaset.sh <iplist.txt> <用户名> <密码>
#
# iplist.txt 格式(3字段): 复制集名 IP地址/域名 端口
#   示例:
#     mydb_rs 192.168.1.1 27017
#     mydb_rs 192.168.1.2 27017
#     mydb_rs 192.168.1.3 27017
#
# IP_LIST_FILE: 节点列表文件(复制集名 IP 端口)
# TARGET_USER: 用户名
# TARGET_PWD: 密码
#
# 注意: 无需传入数据路径, 脚本通过 ps 自动检测 mongod 进程的 -f conf 路径
#       可直接使用 create_replicaset.sh 部署时使用的同一个 iplist.txt
#
# 关闭顺序(官方推荐):
#   1. 先关闭所有从节点(secondary)
#   2. 再关闭主节点(primary) — 降级为从节点后关闭

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

# 加载公共函数库
. ${SCRIPT_PATH}/common/common_define.sh
. ${SCRIPT_PATH}/common/funcs.sh

echo "=========================================="
echo "MongoDB 复制集安全关闭脚本"
echo "=========================================="
echo "IP_LIST_FILE    : ${IP_LIST_FILE}"
echo "TARGET_USER     : ${TARGET_USER}"
echo "TARGET_PWD      : ******"
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
# 解析 iplist.txt, 获取复制集名和节点列表
##############################
RS_NAME=""                # 复制集名(从iplist读取, 所有行必须一致)
NODE_ARRAY=()             # 节点数组, 元素格式: "ip port"

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
        echo "  [$i] $(format_host_port ${ip} ${node_port})  (初始主节点)"
    else
        echo "  [$i] $(format_host_port ${ip} ${node_port})  (从节点)"
    fi
done
echo "===================================="

##############################
# 辅助函数: 通过监听端口获取对应 mongod 进程的 -f conf 路径
# mongod 启动命令为 "mongod -f xxx.conf", 进程命令行中不包含端口号,
# 因此需先通过 ss 找到监听该端口的PID, 再从 /proc/<PID>/cmdline 提取 -f 参数值
# $1: ip, $2: port
# 输出: conf文件绝对路径, 如果找不到则为空
##############################
get_conf_path_by_port() {
    local ip=$1
    local port=$2

    if is_local_host "${ip}" "${LOCAL_IP_LIST}"; then
        # 通过 ss 找到监听该端口的进程PID, 再从 /proc/PID/cmdline 提取 -f 参数
        local pid=$(ss -tlnp "sport = :${port}" 2>/dev/null | grep -oP 'pid=\K[0-9]+' | head -1)
        if [ -n "${pid}" ] && [ -r "/proc/${pid}/cmdline" ]; then
            tr '\0' ' ' < "/proc/${pid}/cmdline" | grep -oP '(?<=-f\s)\S+' | head -1
        else
            # ss 不可用时回退: 遍历所有 mongod 进程, 检查其监听端口
            for p in $(ps -aux | grep "mongod" | sed '/grep/d' | awk '{print $2}'); do
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
        ssh root@${ip} "source ~/.bash_profile 2>/dev/null; pid=\$(ss -tlnp 'sport = :${port}' 2>/dev/null | grep -oP 'pid=\K[0-9]+' | head -1); if [ -n \"\$pid\" ] && [ -r \"/proc/\$pid/cmdline\" ]; then tr '\0' ' ' < \"/proc/\$pid/cmdline\" | grep -oP '(?<=-f\s)\S+' | head -1; else for p in \$(ps -aux | grep 'mongod' | sed '/grep/d' | awk '{print \$2}'); do if [ -r \"/proc/\$p/cmdline\" ]; then listen_ports=\$(ss -tlnp 'pid = \$p' 2>/dev/null | grep -oP 'pid=\K[0-9]+|:\K[0-9]+' | grep -v \"\$p\"); if echo \"\$listen_ports\" | grep -qw '${port}'; then tr '\0' ' ' < \"/proc/\$p/cmdline\" | grep -oP '(?<=-f\s)\S+' | head -1; exit 0; fi; fi; done; fi"
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
        mongosh_eval_auth "$(get_local_ip_by_type "${target_host}" "${LOCAL_IP_LIST}")" "${target_port}" "${eval_expr}"
    else
        mongosh_eval_auth_remote "${target_host}" "${target_host}" "${target_port}" "${eval_expr}"
    fi
}

##############################
# 步骤1: 关闭所有从节点(secondary)
##############################
echo ""
echo "========== 步骤1: 关闭从节点(secondary) =========="

# 从节点: 跳过第一个节点(初始主节点)
for i in "${!NODE_ARRAY[@]}"; do
    if [ $i -eq 0 ]; then
        continue  # 跳过主节点(第一个节点)
    fi

    item="${NODE_ARRAY[$i]}"
    ip=$(echo "${item}" | awk '{print $1}')
    node_port=$(echo "${item}" | awk '{print $2}')
    mongod_conf_path=$(get_conf_path_by_port "${ip}" "${node_port}")

    echo "关闭从节点 [$i]: $(format_host_port ${ip} ${node_port})..."

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
    echo "从节点 $(format_host_port ${ip} ${node_port}) 已关闭."
done

echo "所有从节点已关闭."

##############################
# 步骤2: 将主节点降级, 然后关闭
##############################
echo ""
echo "========== 步骤2: 关闭主节点(primary) =========="

# 主节点(第一个节点)
PRIMARY_NODE="${NODE_ARRAY[0]}"
PRIMARY_IP=$(echo "${PRIMARY_NODE}" | awk '{print $1}')
PRIMARY_PORT=$(echo "${PRIMARY_NODE}" | awk '{print $2}')
PRIMARY_MONGOD_CONF=$(get_conf_path_by_port "${PRIMARY_IP}" "${PRIMARY_PORT}" "mongod")

echo "将主节点降级: rs.stepDown()..."
# rs.stepDown() 成功后主节点会断开所有连接, mongosh 退出码非0, 这是正常行为
# 从节点已关闭时 stepDown 无法选举新主节点也会失败, 使用 force:true 允许强制降级
# 两种情况都不影响后续关闭流程, 统一忽略错误
mongosh_exec_on_nearest "${PRIMARY_IP}" "${PRIMARY_PORT}" "rs.stepDown(120, 30, {force: true})" 2>/dev/null || true

sleep 5

echo "关闭主节点: $(format_host_port ${PRIMARY_IP} ${PRIMARY_PORT})..."
if is_local_host "${PRIMARY_IP}" "${LOCAL_IP_LIST}"; then
    if [ -n "${PRIMARY_MONGOD_CONF}" ] && [ -f "${PRIMARY_MONGOD_CONF}" ]; then
        mongod -f ${PRIMARY_MONGOD_CONF} --shutdown 2>/dev/null || {
            echo "  警告: mongod --shutdown 失败, 尝试 SIGTERM..."
            PID=$(get_pid_by_port "${PRIMARY_IP}" "${PRIMARY_PORT}")
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
        PID=$(get_pid_by_port "${PRIMARY_IP}" "${PRIMARY_PORT}")
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
    if [ -n "${PRIMARY_MONGOD_CONF}" ]; then
        ssh root@${PRIMARY_IP} "source ~/.bash_profile 2>/dev/null; mongod -f ${PRIMARY_MONGOD_CONF} --shutdown 2>/dev/null || { echo 'warn: mongod --shutdown fail, try SIGTERM...'; PID=\$(ss -tlnp 'sport = :${PRIMARY_PORT}' 2>/dev/null | grep -oP 'pid=\K[0-9]+' | head -1); if [ -n \"\$PID\" ]; then kill -2 \$PID; sleep 3; if [ -n \"\$(ps -p \$PID -o pid= 2>/dev/null)\" ]; then kill -9 \$PID 2>/dev/null; fi; fi; }"
    else
        echo "  未检测到conf文件, 使用SIGTERM关闭..."
        ssh root@${PRIMARY_IP} "source ~/.bash_profile 2>/dev/null; PID=\$(ss -tlnp 'sport = :${PRIMARY_PORT}' 2>/dev/null | grep -oP 'pid=\K[0-9]+' | head -1); if [ -n \"\$PID\" ]; then kill -2 \$PID; sleep 3; if [ -n \"\$(ps -p \$PID -o pid= 2>/dev/null)\" ]; then kill -9 \$PID 2>/dev/null; fi; else echo '进程未运行, 跳过'; fi"
    fi
fi

sleep 2
echo "主节点 $(format_host_port ${PRIMARY_IP} ${PRIMARY_PORT}) 已关闭."

##############################
# 完成
##############################
echo ""
echo "=========================================="
echo "MongoDB 复制集已安全关闭!"
echo "=========================================="
echo "复制集名: ${RS_NAME}"
echo ""
echo "关闭顺序:"
echo "  1. 从节点(secondary) ($((${#NODE_ARRAY[@]} - 1))个)"
echo "  2. 主节点(primary) (1个, 已降级后关闭)"
echo ""
echo "如需重新启动, 请执行:"
echo "  sh start_replicaset.sh <start_iplist.txt> ${TARGET_USER} <密码> <安装路径>"
echo "  (注意: 重新启动时使用 start_iplist.txt, 需包含配置文件路径)"
echo "=========================================="
