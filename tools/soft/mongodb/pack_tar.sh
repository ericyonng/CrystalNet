#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 打包脚本
# pack_tar.sh TGZ_FILE_NAME

# 参数
TMP_DIR=$1
TGZ_FILE_NAME="mongodb.tar.gz"

if [ -e "${TMP_DIR}" ]; then
    echo "TMP_DIR:${TMP_DIR} exists"
else
    echo "TMP_DIR:${TMP_DIR} not exists"
    exit 1
fi

if [ -n "$2" ]; then
    TGZ_FILE_NAME=$2
fi

# 打包构建环境所需的文件
rm -rf ${TMP_DIR}
mkdir ${TMP_DIR}

echo "打包需要的文件 => ${TMP_DIR} ..."
tar -zcvf ${TMP_DIR}/${TGZ_FILE_NAME} ./*

echo "pack finish:${TMP_DIR}/${TGZ_FILE_NAME}"
