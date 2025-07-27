#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 给指定机器初始化mongodb 环境, 包括安装包, 环境变量等
# sh ./init_package.sh 压缩包绝对路径 目标机器工作路径 目标机器安装路径

# 当前脚本路径
SCRIPT_PATH="$(cd $(dirname $0); pwd)"


# 所有要安装的mongodb压缩文件, 以及脚本打包
TGZ_FILE_PATH=${1}
TARGET_MACHINE_WORK_PATH=${2}
INSTALL_PATH=${3}

if [ -e "${TGZ_FILE_PATH}" ]; then
    echo "TGZ_FILE_PATH:${TGZ_FILE_PATH} exist!!!"
else
    echo "TGZ_FILE_PATH:${TGZ_FILE_PATH} not exist please check!!!"
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

# 压缩文件名
TGZ_FILE_NAME=$(basename ${TGZ_FILE_PATH})

echo "创建目录: TARGET_MACHINE_WORK_PATH:${TARGET_MACHINE_WORK_PATH} ..."

rm -rf ${TARGET_MACHINE_WORK_PATH} || {
    echo "错误： 移除 ${TARGET_MACHINE_WORK_PATH} 失败" >&2
    exit 1
}

mkdir -p ${TARGET_MACHINE_WORK_PATH} || {
    echo "错误： 创建 ${TARGET_MACHINE_WORK_PATH} 失败" >&2
    exit 1
}

echo "创建目录: INSTALL_PATH:${INSTALL_PATH} ..."
rm -rf ${INSTALL_PATH} || {
    echo "错误： 移除 ${INSTALL_PATH} 失败" >&2
    exit 1
}

mkdir -p ${INSTALL_PATH} || {
    echo "错误： 创建 ${INSTALL_PATH} 失败" >&2
    exit 1
}

echo "拷贝压缩文件 ${TGZ_FILE_PATH} =>  ${TARGET_MACHINE_WORK_PATH} ..."
cp -Rf -r ${TGZ_FILE_PATH} ${TARGET_MACHINE_WORK_PATH} || {
    echo "错误： scp 拷贝 ${TGZ_FILE_PATH} => ${TARGET_MACHINE_WORK_PATH} 失败" >&2
    exit 1
}

echo "解压  ${TARGET_MACHINE_WORK_PATH}/${TGZ_FILE_NAME}..."
tar -zxvf ${TARGET_MACHINE_WORK_PATH}/${TGZ_FILE_NAME} -C ${TARGET_MACHINE_WORK_PATH} || {
    echo "错误： 解压 拷贝 ${TARGET_MACHINE_WORK_PATH}/${TGZ_FILE_NAME} 失败" >&2
    exit 1
}

echo "init_package success TARGET_MACHINE_WORK_PATH:${TARGET_MACHINE_WORK_PATH}, TGZ_FILE_NAME:${TGZ_FILE_NAME}, INSTALL_PATH:${INSTALL_PATH}"
