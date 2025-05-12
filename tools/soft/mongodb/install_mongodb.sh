#!/usr/bin/env bash
# source install_mongodb.sh tgzs_path install_target_path 需要执行source, 否则环境变量不生效

# 当前脚本路径
SCRIPT_PATH="$(cd $(dirname $0); pwd)"

# 变量定义
TEMP_DIR=$SCRIPT_PATH/INSTALL_TEMP
MONGODB_NAME=mongodb-linux-x86_64-rhel8-8.0.6
MONGODB_TOOLS_NAME=mongodb-database-tools-rhel88-x86_64-100.11.0
MONGOSH_NAME=mongosh-2.4.2-linux-x64

MONGODB_TGZ_NAME=${MONGODB_NAME}.tgz
MONGODB_TOOLS_TGZ_NAME=${MONGODB_TOOLS_NAME}.tgz
MONGOSH_TGZ_NAME=${MONGOSH_NAME}.tgz
INSTALL_PATH=$2

# 创建临时目录
rm -rf ${TEMP_DIR}
mkdir ${TEMP_DIR}

# 判断tgz安装包路径
if [ -z "$1" ]
then
    echo "please specify install path"
    exit 1
fi

# 安装路径必须是绝对路径
if [ -d "${INSTALL_PATH}" ]; then
    INSTALL_PATH="$(cd ${INSTALL_PATH}; pwd)"
    echo "will install path:${INSTALL_PATH},  current path:$(pwd)"
else
    mkdir ${INSTALL_PATH}
    INSTALL_PATH="$(cd ${INSTALL_PATH}; pwd)"
    echo "created install path:${INSTALL_PATH}, current path:$(pwd)"
fi

# 拼接包路径
MONGODB_TGZ_PATH=$1/${MONGODB_TGZ_NAME}
MONGODB_TOOLS_TGZ_PATH=$1/${MONGODB_TOOLS_TGZ_NAME}
MONGOSH_TGZ_PATH=$1/${MONGOSH_TGZ_NAME}

# 判断包存不存在
if [ -e "${MONGODB_TGZ_PATH}" ]; then
    echo "mongodb tgz ${MONGODB_TGZ_PATH} ready..."
else
    echo "mongodb tgz ${MONGODB_TGZ_PATH} not exists..."
    exit 1
fi
if [ -e "${MONGODB_TOOLS_TGZ_PATH}" ]; then
    echo "mongodb tools ${MONGODB_TOOLS_TGZ_PATH} ready..."
else
    echo "mongodb tools ${MONGODB_TOOLS_TGZ_PATH} not exists..."
    exit 1
fi
if [ -e "${MONGOSH_TGZ_PATH}" ]; then
    echo "mongosh ${MONGOSH_TGZ_PATH} ready..."

else
    echo "mongosh ${MONGOSH_TGZ_PATH} not exists..."
    exit 1
fi

# 解压到目标文件夹
rm -rf ${INSTALL_PATH}
mkdir ${INSTALL_PATH}

# 目标包路径
TARGET_MONGO_PATH=${INSTALL_PATH}/${MONGODB_NAME}
TARGET_MONGODB_TOOLS_PATH=${INSTALL_PATH}/${MONGODB_TOOLS_NAME}
TARGET_MONGOSH_PATH=${INSTALL_PATH}/${MONGOSH_NAME}

# 解压包到指定路径
echo "unpack ${MONGODB_TGZ_PATH} to ${TARGET_MONGO_PATH}..."
tar -zxvf "${MONGODB_TGZ_PATH}" -C "${INSTALL_PATH}"
echo "unpack ${MONGODB_TGZ_PATH} to ${TARGET_MONGO_PATH} success."

echo "unpack ${MONGODB_TOOLS_TGZ_PATH} to ${TARGET_MONGODB_TOOLS_PATH}..."
tar -zxvf "${MONGODB_TOOLS_TGZ_PATH}" -C "${INSTALL_PATH}"
echo "unpack ${MONGODB_TOOLS_TGZ_PATH} to ${TARGET_MONGODB_TOOLS_PATH} success."

echo "unpack ${MONGOSH_TGZ_PATH} to ${TARGET_MONGOSH_PATH}..."
tar -zxvf "${MONGOSH_TGZ_PATH}" -C "${INSTALL_PATH}"
echo "unpack ${MONGOSH_TGZ_PATH} to ${TARGET_MONGOSH_PATH} success."

# 创建环境变量设置脚本, 并执行脚本
MONGO_PATH_EXPORT_SH=${INSTALL_PATH}/mongodb_export.sh
echo "MONGO_PATH_EXPORT_SH:${MONGO_PATH_EXPORT_SH}"

echo '#!/usr/bin/env bash' > ${MONGO_PATH_EXPORT_SH}
echo '# mongodb_export.sh' >> ${MONGO_PATH_EXPORT_SH}
echo '' >> ${MONGO_PATH_EXPORT_SH}
echo 'export PATH=$PATH:'"${TARGET_MONGODB_TOOLS_PATH}/bin" >> ${MONGO_PATH_EXPORT_SH}
echo 'export PATH=$PATH:'"${TARGET_MONGOSH_PATH}/bin" >> ${MONGO_PATH_EXPORT_SH}
echo 'export PATH=$PATH:'"${INSTALL_PATH}/mongodb-linux-x86_64-rhel88-8.0.6/bin" >> ${MONGO_PATH_EXPORT_SH}
chmod +x ${MONGO_PATH_EXPORT_SH}

# 添加到.bash_profile 
EXPORT_SCRIPT_CONTENT='. '${MONGO_PATH_EXPORT_SH}
echo "will add ${EXPORT_SCRIPT_CONTENT} to .bash_profile"
sed -i "/mongodb_export\.sh/d" ~/.bash_profile
echo "${EXPORT_SCRIPT_CONTENT}" >> ~/.bash_profile

# 执行脚本添加环境变量
echo "add export env to path..."
source ${MONGO_PATH_EXPORT_SH}

echo "install mongodb success target_path:${INSTALL_PATH}, enjoy!"
