#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 给指定机器初始化mongodb 环境, 包括安装包, 环境变量等
# sh ./init_package.sh 压缩包绝对路径 解压脚本文件的目标路径

# 所有要安装的mongodb压缩文件, 以及脚本打包
TGZ_FILE_PATH=${1}
# 解压脚本文件的目标路径
INIT_TARGET_SCRIPT_PATH=${2}

if [ -e "${TGZ_FILE_PATH}" ]; then
    echo "TGZ_FILE_PATH:${TGZ_FILE_PATH} exist!!!"
else
    echo "TGZ_FILE_PATH:${TGZ_FILE_PATH} not exist please check!!!"
    exit 1
fi

if [ -z "${INIT_TARGET_SCRIPT_PATH}" ]; then
    echo "INIT_TARGET_SCRIPT_PATH is empty please check!!!"
    exit 1
fi

# 压缩文件名
PACKAGE_TGZ_FILE_NAME=$(basename ${TGZ_FILE_PATH})

echo "创建目录: INIT_TARGET_SCRIPT_PATH:${INIT_TARGET_SCRIPT_PATH} ..."

rm -rf ${INIT_TARGET_SCRIPT_PATH} || {
    echo "错误： 移除 ${INIT_TARGET_SCRIPT_PATH} 失败" >&2
    exit 1
}

mkdir -p ${INIT_TARGET_SCRIPT_PATH} || {
    echo "错误： 创建 ${INIT_TARGET_SCRIPT_PATH} 失败" >&2
    exit 1
}

echo "拷贝压缩文件 ${TGZ_FILE_PATH} =>  ${INIT_TARGET_SCRIPT_PATH} ..."
cp -Rf -r ${TGZ_FILE_PATH} ${INIT_TARGET_SCRIPT_PATH} || {
    echo "错误： scp 拷贝 ${TGZ_FILE_PATH} => ${INIT_TARGET_SCRIPT_PATH} 失败" >&2
    exit 1
}

echo "解压  ${INIT_TARGET_SCRIPT_PATH}/${PACKAGE_TGZ_FILE_NAME}..."
tar -zxvf ${INIT_TARGET_SCRIPT_PATH}/${PACKAGE_TGZ_FILE_NAME} -C ${INIT_TARGET_SCRIPT_PATH} || {
    echo "错误： 解压 拷贝 ${INIT_TARGET_SCRIPT_PATH}/${PACKAGE_TGZ_FILE_NAME} 失败" >&2
    exit 1
}

echo "init_package success INIT_TARGET_SCRIPT_PATH:${INIT_TARGET_SCRIPT_PATH}, TGZ_FILE_NAME:${PACKAGE_TGZ_FILE_NAME}"
