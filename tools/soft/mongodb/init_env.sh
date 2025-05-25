#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 给指定机器初始化mongodb 环境, 包括安装包, 环境变量等

# 当前脚本路径
SCRIPT_PATH="$(cd $(dirname $0); pwd)"

# 所有要安装的mongodb压缩文件, 以及脚本打包
TGZ_FILE_PATH=${SCRIPT_PATH}
MACHINE_IP=${1}
TARGET_MACHINE_WORK_PATH=${2}
INSTALL_PATH=${3}

TMP_DIR=/root/build_env_tmp

if [ -z "${MACHINE_IP}" ]; then
    echo "MACHINE_IP is empty please check!!!"
    exit 1
fi
if [ -z "${TARGET_MACHINE_WORK_PATH}" ]; then
    echo "TARGET_MACHINE_WORK_PATH is empty please check!!!"
    exit 1
fi
if [ -z "${INSTALL_PATH}" ]; then
    echo "INSTALL_PATH is empty please check!!!"
    exit 1
fi

TGZ_FILE_NAME="mongodb.tar.gz"

# 打包构建环境所需的文件
rm -rf ${TMP_DIR}
mkdir -f ${TMP_DIR}

echo "打包需要的文件 => ${TMP_DIR} ..."
tar -zcvf ${TMP_DIR}/${TGZ_FILE_NAME} ${TGZ_FILE_PATH}

MONGODB_NAME=mongodb-linux-x86_64-rhel8-8.0.6
MONGODB_TOOLS_NAME=mongodb-database-tools-rhel88-x86_64-100.11.0
MONGOSH_NAME=mongosh-2.4.2-linux-x64

echo "${MACHINE_IP}: 创建目录: TARGET_MACHINE_WORK_PATH:${TARGET_MACHINE_WORK_PATH} ..."
ssh root@${MACHINE_IP} "rm -rf ${TARGET_MACHINE_WORK_PATH}"
ssh root@${MACHINE_IP} "mkdir -f ${TARGET_MACHINE_WORK_PATH}"

echo "${MACHINE_IP}: 创建目录: INSTALL_PATH:${INSTALL_PATH} ..."
ssh root@${MACHINE_IP} "rm -rf ${INSTALL_PATH}"
ssh root@${MACHINE_IP} "mkdir -f ${INSTALL_PATH}"

echo "拷贝压缩文件 ${TMP_DIR}/${TGZ_FILE_NAME} =>  ${MACHINE_IP}:${TARGET_MACHINE_WORK_PATH} ..."
scp -r ${TMP_DIR}/${TGZ_FILE_NAME} root@${MACHINE_IP}:${TARGET_MACHINE_WORK_PATH}

echo "解压  ${MACHINE_IP}:${TARGET_MACHINE_WORK_PATH}/${TGZ_FILE_NAME}..."
ssh root@${MACHINE_IP} "tar -zxvf ${TARGET_MACHINE_WORK_PATH}/${TGZ_FILE_NAME}"

# 安装
echo "执行 source ${TARGET_MACHINE_WORK_PATH}/mongodb/install_mongodb.sh ${INSTALL_PATH}..."
ssh root@${MACHINE_IP} "source ${TARGET_MACHINE_WORK_PATH}/mongodb/install_mongodb.sh ${INSTALL_PATH}"

echo "init env success, INSTALL_PATH:${INSTALL_PATH}@${MACHINE_IP} script path:${TARGET_MACHINE_WORK_PATH}/mongodb enjoy!!"
