#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>

SCRIPT_PATH="$(cd $(dirname $0); pwd)"

# 变量
REPLISET_PATH=$1
PRIMARY_PATH=${REPLISET_PATH}/db1
SECONDARY1_PATH=${REPLISET_PATH}/db2
SECONDARY2_PATH=${REPLISET_PATH}/db3

echo "复制集路径:${REPLISET_PATH}"

# 检查路径是否存在
if [ -e "${PRIMARY_PATH}" ]; then
    echo "主节点路径:${PRIMARY_PATH}"
else
    echo "主节点路径不存在:${PRIMARY_PATH}..."
    exit 1
fi
if [ -e "${SECONDARY1_PATH}" ]; then
    echo "从节点1路径:${SECONDARY1_PATH}"
else
    echo "从节点1路径不存在:${SECONDARY1_PATH}..."
    exit 1
fi
if [ -e "${SECONDARY2_PATH}" ]; then
    echo "从节点2路径:${SECONDARY2_PATH}"
else
    echo "从节点2路径不存在:${SECONDARY2_PATH}..."
    exit 1
fi

# 配置文件
if [ -e "${PRIMARY_PATH}/mongod.conf" ]; then
    echo "主节点配置文件路径:${PRIMARY_PATH}"
else
    echo "主节点配置文件不存在:${PRIMARY_PATH}..."
    exit 1
fi
if [ -e "${SECONDARY1_PATH}/mongod.conf" ]; then
    echo "从节点1配置文件:${SECONDARY1_PATH}"
else
    echo "从节点1配置文件不存在:${SECONDARY1_PATH}..."
    exit 1
fi
if [ -e "${SECONDARY2_PATH}/mongod.conf" ]; then
    echo "从节点2配置文件:${SECONDARY2_PATH}"
else
    echo "从节点2配置文件不存在:${SECONDARY2_PATH}..."
    exit 1
fi

mongod -f ${PRIMARY_PATH}/mongod.conf
sleep 1
echo "start PRIMARY_PATH:${PRIMARY_PATH}..."

mongod -f ${SECONDARY1_PATH}/mongod.conf
sleep 1
echo "start SECONDARY1_PATH:${SECONDARY1_PATH}..."

mongod -f ${SECONDARY2_PATH}/mongod.conf
echo "start SECONDARY2_PATH:${SECONDARY2_PATH}..."


