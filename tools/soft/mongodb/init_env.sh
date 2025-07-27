#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 给指定机器初始化mongodb 环境, 包括安装包, 环境变量等
# sh ./init_env.sh 压缩包绝对路径 目标机器工作路径 目标机器安装路径

# 当前脚本路径
SCRIPT_PATH="$(cd $(dirname $0); pwd)"

# 所有要安装的mongodb压缩文件, 以及脚本打包
TARGET_MACHINE_WORK_PATH=${1}
INSTALL_PATH=${2}

if [ -z "${TARGET_MACHINE_WORK_PATH}" ]; then
    echo "TARGET_MACHINE_WORK_PATH is empty please check!!!"
    exit 1
fi
if [ -z "${INSTALL_PATH}" ]; then
    echo "INSTALL_PATH is empty please check!!!"
    exit 1
fi

# 安装
echo "执行 source ${TARGET_MACHINE_WORK_PATH}/install_mongodb.sh ${INSTALL_PATH}..."
source ${TARGET_MACHINE_WORK_PATH}/install_mongodb.sh ${INSTALL_PATH} || {
    echo "错误： 执行 source ${TARGET_MACHINE_WORK_PATH}/install_mongodb.sh 失败" >&2
    exit 1
}

echo "init env success, INSTALL_PATH:${INSTALL_PATH} script path:${TARGET_MACHINE_WORK_PATH} enjoy!!"
