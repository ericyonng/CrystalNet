#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 备份 MongoDB 复制集数据
# 用法: sh mongod_replicaset_dump.sh <主节点地址> <主节点port> <账号> <密码> <生成dump的绝对路径名> <db名> <collectionName>
#
# 主节点地址: 支持 IPv4 / IPv6 / 域名 (IPv6 不需加方括号, 脚本自动处理)
# 主节点port: mongod 监听端口
# 账号: 管理员用户名
# 密码: 管理员密码
# 生成dump的绝对路径名: 备份文件输出目录或文件路径
# db名: (可选) 指定要备份的数据库名, 为空则备份所有db
# collectionName: (可选) 指定要备份的集合名, 为空则备份指定db下所有集合
#
# 备份规则:
#   - db名 和 collectionName 都为空 => 备份整个复制集下所有db的数据
#   - db名 非空, collectionName 为空 => 备份指定db名的整个数据
#   - db名 和 collectionName 都非空 => 备份指定db的指定collection
#
# 备份工具: mongodump (由 mongodb-database-tools 提供)

# 当前脚本路径
SCRIPT_PATH="$(cd $(dirname $0); pwd)"

# 参数定义
PRIMARY_ADDR=$1
PRIMARY_PORT=$2
TARGET_USER=$3
TARGET_PWD=$4
DUMP_PATH=$5
DB_NAME=$6
COLLECTION_NAME=$7

##############################
# 参数校验
##############################
if [ -z "${PRIMARY_ADDR}" ]; then
    echo "错误：主节点地址 为空"
    exit 1
fi

if [ -z "${PRIMARY_PORT}" ]; then
    echo "错误：主节点端口 为空"
    exit 1
fi

if [ -z "${TARGET_USER}" ]; then
    echo "错误：账号 为空"
    exit 1
fi

if [ -z "${TARGET_PWD}" ]; then
    echo "错误：密码 为空"
    exit 1
fi

if [ -z "${DUMP_PATH}" ]; then
    echo "错误：生成dump的绝对路径名 为空"
    exit 1
fi

# 加载公共函数库
. ${SCRIPT_PATH}/common/common_define.sh
. ${SCRIPT_PATH}/common/funcs.sh

# 格式化主节点地址(处理IPv6方括号)
PRIMARY_HOST_PORT=$(format_host_port ${PRIMARY_ADDR} ${PRIMARY_PORT})

echo "=========================================="
echo "MongoDB 复制集数据备份脚本"
echo "=========================================="
echo "主节点地址        : ${PRIMARY_ADDR}"
echo "主节点端口        : ${PRIMARY_PORT}"
echo "TARGET_USER       : ${TARGET_USER}"
echo "TARGET_PWD        : ******"
echo "DUMP_PATH         : ${DUMP_PATH}"
echo "DB_NAME           : ${DB_NAME:-<空, 备份所有db>}"
echo "COLLECTION_NAME   : ${COLLECTION_NAME:-<空, 备份整个db>}"
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

LOCAL_IP=$(get_local_ip "${LOCAL_IP_LIST}")
LOCAL_IPV4=$(get_local_ipv4 "${LOCAL_IP_LIST}")
LOCAL_IPV6=$(get_local_ipv6 "${LOCAL_IP_LIST}")

##############################
# 辅助函数: 判断目标机器是否已安装 mongodb
##############################
INSTALL_PATH=${INSTALL_PATH:-/root/mongo_install}
is_mongo_installed() {
    local ip=$1
    local INSTALL_PATH_CLEAN=${INSTALL_PATH%/}
    if is_local_host "${ip}" "${LOCAL_IP_LIST}"; then
        # 检查 mongodump 是否可用(database-tools)
        if which mongodump &>/dev/null && [ -x "$(which mongodump)" ]; then
            return 0
        fi
        # 检查常见安装路径
        if [ -d "${INSTALL_PATH_CLEAN}" ] && [ -x "${INSTALL_PATH_CLEAN}/mongodb-database-tools-rhel88-x86_64-100.11.0/bin/mongodump" ]; then
            return 0
        fi
        return 1
    else
        if ssh root@${ip} "which mongodump &>/dev/null && [ -x \"\$(which mongodump)\""; then
            return 0
        fi
        if ssh root@${ip} "[ -d '${INSTALL_PATH_CLEAN}' ] && [ -x '${INSTALL_PATH_CLEAN}/mongodb-database-tools-rhel88-x86_64-100.11.0/bin/mongodump' ]"; then
            return 0
        fi
        return 1
    fi
}

##############################
# 环境准备: 检查本机mongo安装
##############################
TARGET_SCRIPT_PATH=/root/mongodb_script

echo ""
echo "========== 检查本机 MongoDB 安装状态 =========="

if is_mongo_installed "127.0.0.1"; then
    echo "本机已安装 mongodb (含 mongodump), 可用."
else
    echo "本机未安装 mongodb 或缺少 mongodump, 执行安装..."
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

# 验证 mongodump 命令可用
if ! command -v mongodump > /dev/null 2>&1; then
    echo "错误：mongodump 命令不可用, 请检查 mongodb-database-tools 是否正确安装"
    exit 1
fi
echo "mongodump 命令可用."

# 验证 mongosh 命令可用
if ! command -v mongosh > /dev/null 2>&1; then
    echo "错误：mongosh 命令不可用, 请检查 mongodb 是否正确安装"
    exit 1
fi
echo "mongosh 命令可用."

##############################
# 验证主节点连接
##############################
echo ""
echo "========== 验证主节点连接 =========="
echo "连接主节点: ${PRIMARY_HOST_PORT}..."

mongosh --host ${PRIMARY_HOST_PORT} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --eval "db.runCommand({ping:1})" > /dev/null 2>&1 || {
    echo "错误：无法连接到主节点 ${PRIMARY_HOST_PORT}, 请检查地址、端口、用户名密码和集群状态"
    exit 1
}
echo "主节点连接验证成功."

##############################
# 执行备份
##############################
echo ""
echo "========== 执行数据备份 =========="

# 准备备份输出目录
DUMP_DIR=$(dirname "${DUMP_PATH}")
if [ ! -d "${DUMP_DIR}" ]; then
    echo "创建备份目录: ${DUMP_DIR}..."
    mkdir -p "${DUMP_DIR}" || {
        echo "错误：创建备份目录 ${DUMP_DIR} 失败" >&2
        exit 1
    }
fi

# 构建 mongodump 命令参数
# 公共参数: --host, -u, -p, --authenticationDatabase, --out
DUMP_CMD="mongodump --host ${PRIMARY_HOST_PORT} -u \"${TARGET_USER}\" -p \"${TARGET_PWD}\" --authenticationDatabase admin --out \"${DUMP_PATH}\""

# 根据参数决定备份范围
if [ -n "${DB_NAME}" ] && [ -n "${COLLECTION_NAME}" ]; then
    # 备份指定db的指定collection
    echo "备份范围: db=${DB_NAME}, collection=${COLLECTION_NAME}"
    DUMP_CMD="mongodump --host ${PRIMARY_HOST_PORT} -u \"${TARGET_USER}\" -p \"${TARGET_PWD}\" --authenticationDatabase admin --db \"${DB_NAME}\" --collection \"${COLLECTION_NAME}\" --out \"${DUMP_PATH}\""
elif [ -n "${DB_NAME}" ]; then
    # 备份指定db的所有collection
    echo "备份范围: db=${DB_NAME} (整个数据库)"
    DUMP_CMD="mongodump --host ${PRIMARY_HOST_PORT} -u \"${TARGET_USER}\" -p \"${TARGET_PWD}\" --authenticationDatabase admin --db \"${DB_NAME}\" --out \"${DUMP_PATH}\""
else
    # 备份所有db
    echo "备份范围: 所有数据库"
    DUMP_CMD="mongodump --host ${PRIMARY_HOST_PORT} -u \"${TARGET_USER}\" -p \"${TARGET_PWD}\" --authenticationDatabase admin --out \"${DUMP_PATH}\""
fi

echo "执行备份命令: ${DUMP_CMD}"
echo ""

eval ${DUMP_CMD}
DUMP_EXIT_CODE=$?

if [ ${DUMP_EXIT_CODE} -ne 0 ]; then
    echo "错误：mongodump 备份失败, 退出码: ${DUMP_EXIT_CODE}" >&2
    exit 1
fi

echo ""
echo "备份成功, 备份文件路径: ${DUMP_PATH}"

# 显示备份目录结构
echo ""
echo "========== 备份目录结构 =========="
if [ -d "${DUMP_PATH}" ]; then
    echo "备份目录 ${DUMP_PATH} 内容:"
    ls -la "${DUMP_PATH}" 2>/dev/null || true
    # 显示子目录(db名目录)
    if [ -n "${DB_NAME}" ]; then
        echo ""
        echo "数据库 ${DB_NAME} 备份内容:"
        ls -la "${DUMP_PATH}/${DB_NAME}" 2>/dev/null || echo "  (无 ${DB_NAME} 子目录)"
    fi
else
    echo "警告：备份目录 ${DUMP_PATH} 不存在"
fi

##############################
# 完成
##############################
echo ""
echo "=========================================="
echo "MongoDB 复制集数据备份完成!"
echo "=========================================="
echo "主节点: ${PRIMARY_HOST_PORT}"
echo "备份路径: ${DUMP_PATH}"
if [ -n "${DB_NAME}" ] && [ -n "${COLLECTION_NAME}" ]; then
    echo "备份范围: ${DB_NAME}.${COLLECTION_NAME}"
elif [ -n "${DB_NAME}" ]; then
    echo "备份范围: ${DB_NAME} (整个数据库)"
else
    echo "备份范围: 所有数据库"
fi
echo ""
echo "恢复数据请执行:"
echo "  sh mongod_replicaset_restore.sh ${PRIMARY_ADDR} ${PRIMARY_PORT} ${TARGET_USER} <密码> ${DUMP_PATH}"
echo "=========================================="
