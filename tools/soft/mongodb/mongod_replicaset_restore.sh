#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 恢复 MongoDB 复制集数据
# 用法: sh mongod_replicaset_restore.sh <主节点地址> <主节点port> <账号> <密码> <dump的绝对路径名>
#
# 主节点地址: 支持 IPv4 / IPv6 / 域名 (IPv6 不需加方括号, 脚本自动处理)
# 主节点port: mongod 监听端口
# 账号: 管理员用户名
# 密码: 管理员密码
# dump的绝对路径名: mongod_replicaset_dump.sh 生成的备份目录路径
#
# 恢复规则:
#   - 根据 dump 目录结构自动恢复, dump 目录下每个子目录对应一个db
#   - 恢复时会合并到已有数据(不会清空已有数据), 如需覆盖请先手动删除目标库
#
# 恢复工具: mongorestore (由 mongodb-database-tools 提供)

# 当前脚本路径
SCRIPT_PATH="$(cd $(dirname $0); pwd)"

# 参数定义
PRIMARY_ADDR=$1
PRIMARY_PORT=$2
TARGET_USER=$3
TARGET_PWD=$4
DUMP_PATH=$5

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
    echo "错误：dump的绝对路径名 为空"
    exit 1
fi

if [ ! -d "${DUMP_PATH}" ]; then
    echo "错误：dump目录不存在: ${DUMP_PATH}"
    exit 1
fi

# 加载公共函数库
. ${SCRIPT_PATH}/common/common_define.sh
. ${SCRIPT_PATH}/common/funcs.sh

# 格式化主节点地址(处理IPv6方括号)
PRIMARY_HOST_PORT=$(format_host_port ${PRIMARY_ADDR} ${PRIMARY_PORT})

echo "=========================================="
echo "MongoDB 复制集数据恢复脚本"
echo "=========================================="
echo "主节点地址        : ${PRIMARY_ADDR}"
echo "主节点端口        : ${PRIMARY_PORT}"
echo "TARGET_USER       : ${TARGET_USER}"
echo "TARGET_PWD        : ******"
echo "DUMP_PATH         : ${DUMP_PATH}"
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
        # 检查 mongorestore 是否可用(database-tools)
        if which mongorestore &>/dev/null && [ -x "$(which mongorestore)" ]; then
            return 0
        fi
        # 检查常见安装路径
        if [ -d "${INSTALL_PATH_CLEAN}" ] && [ -x "${INSTALL_PATH_CLEAN}/mongodb-database-tools-rhel88-x86_64-100.11.0/bin/mongorestore" ]; then
            return 0
        fi
        return 1
    else
        if ssh root@${ip} "which mongorestore &>/dev/null && [ -x \"\$(which mongorestore)\""; then
            return 0
        fi
        if ssh root@${ip} "[ -d '${INSTALL_PATH_CLEAN}' ] && [ -x '${INSTALL_PATH_CLEAN}/mongodb-database-tools-rhel88-x86_64-100.11.0/bin/mongorestore' ]"; then
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
    echo "本机已安装 mongodb (含 mongorestore), 可用."
else
    echo "本机未安装 mongodb 或缺少 mongorestore, 执行安装..."
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

# 验证 mongorestore 命令可用
if ! command -v mongorestore > /dev/null 2>&1; then
    echo "错误：mongorestore 命令不可用, 请检查 mongodb-database-tools 是否正确安装"
    exit 1
fi
echo "mongorestore 命令可用."

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
# 显示dump目录结构
##############################
echo ""
echo "========== dump目录结构 =========="
echo "dump目录 ${DUMP_PATH} 内容:"
ls -la "${DUMP_PATH}" 2>/dev/null || true

# 检查dump目录是否为空
DUMP_CONTENT_COUNT=$(ls -A "${DUMP_PATH}" 2>/dev/null | wc -l)
if [ "${DUMP_CONTENT_COUNT}" -eq 0 ]; then
    echo "错误：dump目录 ${DUMP_PATH} 为空, 无数据可恢复"
    exit 1
fi

echo ""
echo "可恢复的数据库:"
for db_dir in "${DUMP_PATH}"/*/; do
    if [ -d "${db_dir}" ]; then
        db_name=$(basename "${db_dir}")
        bson_count=$(ls "${db_dir}"*.bson 2>/dev/null | wc -l)
        echo "  ${db_name} (${bson_count} 个集合)"
    fi
done

##############################
# 执行恢复
##############################
echo ""
echo "========== 执行数据恢复 =========="

# 构建 mongorestore 命令参数
# --drop 可选: 恢复前删除目标集合(慎用, 默认不启用, 合并恢复)
# 公共参数: --host, -u, -p, --authenticationDatabase
# 这里不加 --drop, 恢复时合并数据, 避免误删已有数据
RESTORE_CMD="mongorestore --host ${PRIMARY_HOST_PORT} -u \"${TARGET_USER}\" -p \"${TARGET_PWD}\" --authenticationDatabase admin \"${DUMP_PATH}\""

echo "执行恢复命令: ${RESTORE_CMD}"
echo "注意: 恢复为合并模式(不会清空已有数据), 如需覆盖请先手动删除目标库"
echo ""

eval ${RESTORE_CMD}
RESTORE_EXIT_CODE=$?

if [ ${RESTORE_EXIT_CODE} -ne 0 ]; then
    echo "错误：mongorestore 恢复失败, 退出码: ${RESTORE_EXIT_CODE}" >&2
    exit 1
fi

echo ""
echo "恢复成功, 数据已从 ${DUMP_PATH} 恢复到 ${PRIMARY_HOST_PORT}"

##############################
# 验证恢复结果
##############################
echo ""
echo "========== 验证恢复结果 =========="
echo "当前数据库列表:"
mongosh --host ${PRIMARY_HOST_PORT} -u "${TARGET_USER}" -p "${TARGET_PWD}" --authenticationDatabase admin --quiet --eval "db.adminCommand({listDatabases:1}).databases.forEach(d => print(d.name + ' (' + (d.sizeOnDisk/1024/1024).toFixed(2) + ' MB)'))" || echo "警告：无法获取数据库列表"

##############################
# 完成
##############################
echo ""
echo "=========================================="
echo "MongoDB 复制集数据恢复完成!"
echo "=========================================="
echo "主节点: ${PRIMARY_HOST_PORT}"
echo "恢复来源: ${DUMP_PATH}"
echo "恢复模式: 合并模式(已有数据保留, 新数据写入)"
echo ""
echo "连接查看数据:"
echo "  mongosh --host ${PRIMARY_HOST_PORT} -u ${TARGET_USER} -p <密码> --authenticationDatabase admin"
echo "=========================================="
