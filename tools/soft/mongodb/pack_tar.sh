#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 打包脚本
# pack_tar.sh tmp_dir 源文件路径 TGZ_FILE_NAME

# 参数
TMP_DIR=$1
SRC_PATH=$2
TGZ_FILE_NAME=$3

if [ -z "${TMP_DIR}" ]; then
    echo "TMP_DIR:${TMP_DIR} empty"
    exit 1
fi

if [ -e "${SRC_PATH}" ]; then
    echo "SRC_PATH:${SRC_PATH} exists"
else
    echo "SRC_PATH:${SRC_PATH} not exists"
    exit 1
fi

if [ -z "${TGZ_FILE_NAME}" ]; then
    TGZ_FILE_NAME=mongodb.tar.gz
fi

# 打包构建环境所需的文件
rm -rf ${TMP_DIR}
mkdir ${TMP_DIR}

echo "打包需要的文件 => ${TMP_DIR} ..."
cd ${SRC_PATH} && tar -zcvf ${TMP_DIR}/${TGZ_FILE_NAME} ./* || {
    echo "错误： tar fail SRC_PATH:${SRC_PATH} fail TGZ_FILE_NAME:${TGZ_FILE_NAME} 失败" >&2
    exit 1
}

echo "pack finish SRC_PATH:${SRC_PATH} ${TMP_DIR}/${TGZ_FILE_NAME}"
