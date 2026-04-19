#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 创建mongo 分片集群
# create_mongo_shard_cluster.sh 复制集名 复制集公网ip 主节点端口号 用户名 密码 复制集安装目标路径
# create_mongo_shard_cluster.sh IP列表文本文件 目标机器工作路径(数据库运行路径) 目标机器安装路径(数据库安装路径)
# 先初始化各个节点环境 => 启动config/shard各个节点脚本创建3节点复制集 => 创建mongos => 测试连通性 
# IP_LIST_FILE: ip 节点类型 端口: config 127.0.0.1 27010

# 当前脚本路径
SCRIPT_PATH="$(cd $(dirname $0); pwd)"

# IP列表文件
IP_LIST_FILE=$1
TARGET_USER=$2
TARGET_PWD=$3
# 数据库名(由业务决定)
DB_NAME=$4
# 复制集名
RS_NAME=$5
if [ -z "${DB_NAME}" ]; then
    DB_NAME="TEST_MONGO"
fi
if [ -z "${RS_NAME}" ]; then
    RS_NAME="TEST_MONGO_RS"
fi

# 工作路径
WORK_PATH=/root/mongo_work_place
# 安装mongodb的目录
INSTALL_PATH=/root/mongo_install
# 最终复制集工作目录
REPLISET_INSTALL_PATH=/root/Mongo_${DB_NAME}_ReplisetWorkPlace/

echo "${IP_LIST_FILE}, ${TARGET_USER}, ${TARGET_PWD}, ${DB_NAME}, ${RS_NAME}, ${WORK_PATH}, ${INSTALL_PATH}, ${REPLISET_INSTALL_PATH}"

. ${SCRIPT_PATH}/create_mongo_shard_cluster.sh ${IP_LIST_FILE} ${WORK_PATH} ${INSTALL_PATH} ${TARGET_USER} ${TARGET_PWD} ${REPLISET_INSTALL_PATH} ${DB_NAME} ${RS_NAME}