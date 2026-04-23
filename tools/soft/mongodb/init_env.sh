#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 给指定机器初始化mongodb 环境, 包括安装包, 环境变量等
# sh ./init_env.sh 压缩包绝对路径 目标机器工作路径 目标机器安装路径

# 脚本所在路径
ENV_MONGODB_SCRIPT_PATH=${1}
# 所有要安装的mongodb压缩文件, 以及脚本打包
LOCAL_INSTALL_PATH=${2}

if [ -z "${LOCAL_INSTALL_PATH}" ]; then
    echo "LOCAL_INSTALL_PATH is empty please check!!!"
    exit 1
fi

# 安装
echo "执行 source ${ENV_MONGODB_SCRIPT_PATH}/install_mongodb.sh ${LOCAL_INSTALL_PATH}..."
source ${ENV_MONGODB_SCRIPT_PATH}/install_mongodb.sh ${LOCAL_INSTALL_PATH} || {
    echo "错误： 执行 source ${ENV_MONGODB_SCRIPT_PATH}/install_mongodb.sh 失败" >&2
    exit 1
}

echo "init env success, LOCAL_INSTALL_PATH:${LOCAL_INSTALL_PATH} script path:${ENV_MONGODB_SCRIPT_PATH} enjoy!!"
