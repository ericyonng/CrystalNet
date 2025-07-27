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

# 获取公网ip
GET_PUBLIC_IP_HOST_ADDR=https://ifconfig.me
get_public_ip(){
    HAS_NS_LOOKUP=1
    if ! command -v curl &> /dev/null; then
        echo "当前环境未安装curl正在安装 curl..."
        if sudo yum install curl -y &> /dev/null; then
            echo "curl 安装成功！"
        else
            echo "curl 安装失败！"
            HAS_NS_LOOKUP=0
        fi
    fi
    
    if [ $HAS_NS_LOOKUP = 0 ]; then
        return ""
    fi

    return $(curl ${GET_PUBLIC_IP_HOST_ADDR})
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