# @file funcs.sh
# @author EricYonng<120453674@qq.com>
# @brief 公共函数
#!/usr/bin/env bash

FUNCS_SCRIPT_PATH="$(cd $(dirname $0); pwd)"

# 是否可以联外网
check_internet() {
    HAS_WGET=1
    if ! command -v wget &> /dev/null; then
        echo "当前环境未安装wget正在安装 wget..."
        if sudo yum install wget -y &> /dev/null; then
            echo "wget 安装成功！"
        else
            echo "wget 安装失败！"
            HAS_WGET=0
        fi
    fi

    if [ ${HAS_WGET} = 1 ]; then
        if wget -q --spider --timeout=3 https://www.baidu.com; then
            echo "wget 外网连接正常"
            return 0
        else
            echo "wget 连接 https://www.baidu.com 失败"
        fi
    fi

    HAS_PING=1
    if ! command -v ping &> /dev/null; then
        echo "当前环境未安装ping正在安装 ping..."
        if sudo yum install iputils -y &> /dev/null; then
            echo "ping 安装成功！"
        else
            echo "ping 安装失败！"
            HAS_PING=0
        fi
    fi
    if [ ${HAS_PING} = 1 ]; then
        if ping -c 1 -W 2 8.8.8.8 &> /dev/null; then
            echo "ping 8.8.8.8 正常"
            return 0
        else
            echo "ping 8.8.8.8 失败"
        fi
        # 尝试 IPv6 ping
        if ping6 -c 1 -W 2 2001:4860:4860::8888 &> /dev/null 2>&1; then
            echo "ping6 2001:4860:4860::8888 正常"
            return 0
        else
            echo "ping6 2001:4860:4860::8888 失败"
        fi
    fi

    HAS_NS_LOOKUP=1
    if ! command -v nslookup &> /dev/null; then
        echo "当前环境未安装nslookup正在安装 nslookup..."
        if sudo yum install bind-utils -y &> /dev/null; then
            echo "nslookup 安装成功！"
        else
            echo "nslookup 安装失败！"
            HAS_NS_LOOKUP=0
        fi
    fi
    if [ ${HAS_NS_LOOKUP} = 1 ]; then
        if nslookup baidu.com &> /dev/null; then
            echo "nslookup baidu.com 正常"
            return 0
        else
            echo "nslookup baidu.com 失败"
        fi
    fi

    return 1
}

# 获取公网ip 要输出字符串 需要通过echo return 只能返回错误码
get_public_ip(){
    HAS_CURL=1
    if ! command -v curl &> /dev/null; then
        if sudo yum install curl -y &> /dev/null; then
            HAS_CURL=1
        else
            HAS_CURL=0
        fi
    fi
    
    if [ $HAS_CURL = 0 ]; then
        return 1
    fi

    # 先尝试 IPv4
    local ip=$(curl -s --connect-timeout 5 ${GET_PUBLIC_IP_HOST_ADDR} 2>/dev/null)
    if [ -n "${ip}" ]; then
        echo ${ip}
        return 0
    fi

    # IPv4 失败, 尝试 IPv6
    ip=$(curl -s --connect-timeout 5 ${GET_PUBLIC_IP_HOST_ADDR_V6} 2>/dev/null)
    if [ -n "${ip}" ]; then
        echo ${ip}
        return 0
    fi

    return 1
}

#!/bin/bash

# 函数：从文件中读取行到数组
# 参数:
#   $1: 文件路径 (必需)
#   $2: 接收结果的数组名称 (必需)
#   $3: 空行处理选项 (可选)
#        skip_empty: 跳过空行（长度为0）
#        skip_blank: 跳过空白行（仅含空白字符）
#        默认: 保留所有行（包括空行和空白行）
# 返回值:
#   0: 成功
#   非0: 文件不存在或读取错误
read_file_to_array() {
    local file_path="$1"
    local array_name="$2"

    # 检查文件是否存在
    if [[ ! -f "$file_path" ]]; then
        echo "错误: 文件 '$file_path' 不存在或不可访问!" >&2
        return 1
    fi

    # 初始化目标数组
    eval "$array_name=()"

    # 逐行读取处理
    while IFS= read -r line || [[ -n "$line" ]]; do
        # if [ -z "${line}" ]; then
        #     continue
        # fi
        # if [[ "$STR" =~ ^[[:space:]]*$ ]]; then
        #     continue
        # fi
        # 安全添加行到数组
        eval "$array_name+=(\"\$line\")"
    done < "$file_path"

    return 0
}

##############################
# 地址工具函数 (IPv4/IPv6/域名 统一支持)
##############################

# 判断是否为 IPv4 地址
# IPv4 为点分十进制格式, 如 192.168.1.1
# 返回: 0=true, 1=false
is_ipv4() {
    local host="$1"
    [[ "${host}" =~ ^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$ ]]
}

# 判断是否为 IPv6 地址
# IPv6 地址含冒号且不含点号(排除域名), 如 2001:db8::1
# 返回: 0=true, 1=false
is_ipv6() {
    local host="$1"
    # 含冒号且不含点号 => IPv6 (域名含点号, IPv4含点号)
    if [[ "${host}" == *":"* ]] && [[ "${host}" != *"."* ]]; then
        return 0
    fi
    return 1
}

# 格式化 host:port
# IPv6 自动加方括号: [2001:db8::1]:27017
# IPv4/域名直接拼接: 192.168.1.1:27017 或 mongo1.example.com:27017
format_host_port() {
    local host="$1"
    local port="$2"
    if is_ipv6 "${host}"; then
        echo "[${host}]:${port}"
    else
        echo "${host}:${port}"
    fi
}

# DNS 解析域名获取所有 IP (空格分隔)
# 对于 IP 地址直接返回本身
# "mongo1.example.com" → "1.2.3.4 2001:db8::1"
# "192.168.1.1" → "192.168.1.1"
resolve_host() {
    local host="$1"

    # 已经是 IP 地址, 直接返回
    if is_ipv4 "${host}" || is_ipv6 "${host}"; then
        echo "${host}"
        return 0
    fi

    # 尝试 getent (支持 /etc/hosts, 最通用)
    local ips=""
    if command -v getent &> /dev/null; then
        ips=$(getent hosts "${host}" 2>/dev/null | awk '{print $1}' | tr '\n' ' ' | sed 's/ *$//')
    fi

    # getent 失败时 fallback 到 dig
    if [ -z "${ips}" ] && command -v dig &> /dev/null; then
        # 获取 IPv4 和 IPv6
        local ipv4=$(dig +short A "${host}" 2>/dev/null | grep -E '^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$')
        local ipv6=$(dig +short AAAA "${host}" 2>/dev/null | grep -E ':')
        ips=$(echo -e "${ipv4}\n${ipv6}" | grep -v '^$' | tr '\n' ' ' | sed 's/ *$//')
    fi

    if [ -n "${ips}" ]; then
        echo "${ips}"
        return 0
    fi

    # 都失败, 返回原始 host (fallback 到字符串比较)
    echo "${host}"
    return 1
}

# 获取本机所有 IP 地址列表 (空格分隔)
# 包括 127.0.0.1, ::1, 以及所有接口的 IPv4/IPv6 地址
get_local_ip_list() {
    local ip_list="127.0.0.1 ::1"

    # hostname -I 列出所有接口 IP (不含 loopback)
    if command -v hostname &> /dev/null; then
        local host_ips=$(hostname -I 2>/dev/null)
        if [ -n "${host_ips}" ]; then
            ip_list="${ip_list} ${host_ips}"
        fi
    fi

    # 获取公网 IP (如果可联网)
    if command -v curl &> /dev/null; then
        local public_ip=$(curl -s --connect-timeout 3 ${GET_PUBLIC_IP_HOST_ADDR} 2>/dev/null)
        if [ -n "${public_ip}" ]; then
            ip_list="${ip_list} ${public_ip}"
        fi
        local public_ip_v6=$(curl -s --connect-timeout 3 ${GET_PUBLIC_IP_HOST_ADDR_V6} 2>/dev/null)
        if [ -n "${public_ip_v6}" ]; then
            ip_list="${ip_list} ${public_ip_v6}"
        fi
    fi

    echo "${ip_list}" | tr -s ' ' | sed 's/^ *//;s/ *$//'
}

# 获取本机最优IP: 优先公网IPv4 > 公网IPv6 > 内网IPv4 > 内网IPv6 > 127.0.0.1
# $1: local_ip_list (由 get_local_ip_list 获取)
get_local_ip() {
    local ip_list="$1"

    local public_ipv4=""
    local public_ipv6=""
    local private_ipv4=""
    local private_ipv6=""

    for ip in ${ip_list}; do
        if [ "${ip}" = "127.0.0.1" ] || [ "${ip}" = "::1" ]; then
            continue
        fi
        # 公网IP判断: RFC1918私网范围之外的IPv4视为公网
        if is_ipv6 "${ip}"; then
            if [ -z "${public_ipv6}" ]; then
                public_ipv6="${ip}"
            fi
        else
            # 判断是否为私网IPv4
            if is_private_ipv4 "${ip}"; then
                if [ -z "${private_ipv4}" ]; then
                    private_ipv4="${ip}"
                fi
            else
                if [ -z "${public_ipv4}" ]; then
                    public_ipv4="${ip}"
                fi
            fi
        fi
    done

    # 优先级: 公网IPv4 > 公网IPv6 > 内网IPv4 > 内网IPv6 > 127.0.0.1
    if [ -n "${public_ipv4}" ]; then
        echo "${public_ipv4}"
    elif [ -n "${public_ipv6}" ]; then
        echo "${public_ipv6}"
    elif [ -n "${private_ipv4}" ]; then
        echo "${private_ipv4}"
    elif [ -n "${private_ipv6}" ]; then
        echo "${private_ipv6}"
    else
        echo "127.0.0.1"
    fi
}

# 获取本机最优IPv4地址
# $1: local_ip_list
get_local_ipv4() {
    local ip_list="$1"
    local public_ipv4=""
    local private_ipv4=""

    for ip in ${ip_list}; do
        if [ "${ip}" = "127.0.0.1" ] || [ "${ip}" = "::1" ]; then
            continue
        fi
        if ! is_ipv6 "${ip}"; then
            if is_private_ipv4 "${ip}"; then
                [ -z "${private_ipv4}" ] && private_ipv4="${ip}"
            else
                [ -z "${public_ipv4}" ] && public_ipv4="${ip}"
            fi
        fi
    done

    [ -n "${public_ipv4}" ] && echo "${public_ipv4}" && return
    [ -n "${private_ipv4}" ] && echo "${private_ipv4}" && return
    echo "127.0.0.1"
}

# 获取本机最优IPv6地址
# $1: local_ip_list
get_local_ipv6() {
    local ip_list="$1"
    local public_ipv6=""
    local private_ipv6=""

    for ip in ${ip_list}; do
        if [ "${ip}" = "127.0.0.1" ] || [ "${ip}" = "::1" ]; then
            continue
        fi
        if is_ipv6 "${ip}"; then
            # 简单判断: 以 fe80: 开头的为 link-local(私网), 其余视为公网
            if [[ "${ip}" == fe80:* ]]; then
                [ -z "${private_ipv6}" ] && private_ipv6="${ip}"
            else
                [ -z "${public_ipv6}" ] && public_ipv6="${ip}"
            fi
        fi
    done

    [ -n "${public_ipv6}" ] && echo "${public_ipv6}" && return
    [ -n "${private_ipv6}" ] && echo "${private_ipv6}" && return
    echo "::1"
}

# 根据给定 host 的类型, 返回本机同类型的最优IP
# 如果 ref_host 是 IPv6, 返回本机最优 IPv6; 如果是 IPv4, 返回本机最优 IPv4
# 如果是域名, 先 DNS 解析再根据解析结果的类型决定
# $1: ref_host (参考IP或域名, 用于判断类型), $2: local_ip_list
get_local_ip_by_type() {
    local ref_host="$1"
    local ip_list="$2"

    if is_ipv6 "${ref_host}"; then
        get_local_ipv6 "${ip_list}"
    elif is_ipv4 "${ref_host}"; then
        get_local_ipv4 "${ip_list}"
    else
        # 域名: 先 DNS 解析, 根据解析结果的IP类型决定
        local resolved=$(resolve_host "${ref_host}" | awk '{print $1}')
        if [ -n "${resolved}" ]; then
            if is_ipv6 "${resolved}"; then
                get_local_ipv6 "${ip_list}"
            else
                get_local_ipv4 "${ip_list}"
            fi
        else
            # DNS 解析失败, fallback 到 IPv4 (域名通常解析到 IPv4)
            get_local_ipv4 "${ip_list}"
        fi
    fi
}
    local ip="$1"
    # 10.0.0.0/8
    if [[ "${ip}" == 10.* ]]; then
        return 0
    fi
    # 172.16.0.0/12: 172.16.x.x ~ 172.31.x.x
    if [[ "${ip}" == 172.1[6-9].* ]] || [[ "${ip}" == 172.2[0-9].* ]] || [[ "${ip}" == 172.3[0-1].* ]]; then
        return 0
    fi
    # 192.168.0.0/16
    if [[ "${ip}" == 192.168.* ]]; then
        return 0
    fi
    # 100.64.0.0/10 (CGNAT, 常见于云厂商内网)
    if [[ "${ip}" == 100.6[4-9].* ]] || [[ "${ip}" == 100.[7-9]?.* ]] || [[ "${ip}" == 100.1[0-1]?.* ]] || [[ "${ip}" == 100.12[0-7].* ]]; then
        return 0
    fi
    return 1
}

# 判断是否 loopback 地址
# 返回: 0=loopback, 1=非loopback
is_loopback() {
    local host="$1"
    [ "${host}" = "127.0.0.1" ] || [ "${host}" = "::1" ] || [ "${host}" = "localhost" ]
}

# 获取可达地址: 如果原始地址是 loopback 则替换为 LOCAL_IP(公网/内网IP), 否则保持原样
# 域名和非 loopback IP 不需要替换(域名本身已可从外部访问)
# $1: original_host, $2: local_ip
get_reachable_host() {
    local original_host="$1"
    local local_ip="$2"
    if is_loopback "${original_host}"; then
        echo "${local_ip}"
    else
        echo "${original_host}"
    fi
}

# 判断是否本机地址 (支持 IPv4/IPv6/域名)
# $1: host (IPv4/IPv6/域名)
# $2: local_ip_list (由 get_local_ip_list 获取)
# 域名先 DNS 解析再与本机 IP 列表比较
# 返回: 0=本机, 1=非本机
is_local_host() {
    local host="$1"
    local local_ip_list="$2"

    # 直接比较 loopback
    if [ "${host}" = "127.0.0.1" ] || [ "${host}" = "::1" ] || [ "${host}" = "localhost" ]; then
        return 0
    fi

    # 直接比较本机 IP 列表
    local ip
    for ip in ${local_ip_list}; do
        if [ "${host}" = "${ip}" ]; then
            return 0
        fi
    done

    # 域名: DNS 解析后比较
    local resolved_ips=$(resolve_host "${host}")
    for resolved_ip in ${resolved_ips}; do
        for local_ip in ${local_ip_list}; do
            if [ "${resolved_ip}" = "${local_ip}" ]; then
                return 0
            fi
        done
    done

    return 1
}

# scp 封装, IPv6 地址自动处理方括号
# $1: host, $2: src, $3: dst
scp_to_host() {
    local host="$1"
    local src="$2"
    local dst="$3"

    if is_ipv6 "${host}"; then
        scp -r "${src}" "root@[${host}]:${dst}"
    else
        scp -r "${src}" "root@${host}:${dst}"
    fi
}

# 从连接串中解析 host 和 port
# "[2001:db8::1]:27017" → host="2001:db8::1" port="27017"
# "192.168.1.1:27017" → host="192.168.1.1" port="27017"
# "mongo1.example.com:27017" → host="mongo1.example.com" port="27017"
# $1: host_port_str, $2: out_host_var_name, $3: out_port_var_name
parse_host_port() {
    local host_port_str="$1"
    local out_host_var="$2"
    local out_port_var="$3"

    if [[ "${host_port_str}" == "["* ]]; then
        # IPv6 格式: [host]:port
        local inner="${host_port_str#\[}"
        local host_part="${inner%%\]*}"
        local port_part="${inner##*\]:}"
        eval "${out_host_var}=\"${host_part}\""
        eval "${out_port_var}=\"${port_part}\""
    else
        # IPv4/域名格式: host:port, 从最后一个冒号切分
        eval "${out_host_var}=\"${host_port_str%:*}\""
        eval "${out_port_var}=\"${host_port_str##*:}\""
    fi
}