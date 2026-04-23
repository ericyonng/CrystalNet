#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# . install_mongodb.sh 安装db程序的目标路径

# 当前脚本路径
INSTALL_SCRIPT_PATH="$(cd $(dirname $0); pwd)"

echo "install_mongodb.sh script_path:${INSTALL_SCRIPT_PATH}"

# 变量定义
INSTALL_TEMP_DIR=$INSTALL_SCRIPT_PATH/INSTALL_TEMP
INSTALL_MONGODB_NAME=mongodb-linux-x86_64-rhel8-8.0.6
INSTALL_MONGODB_TOOLS_NAME=mongodb-database-tools-rhel88-x86_64-100.11.0
INSTALL_MONGOSH_NAME=mongosh-2.4.2-linux-x64

INSTALL_MONGODB_TGZ_NAME=${INSTALL_MONGODB_NAME}.tgz
INSTALL_MONGODB_TOOLS_TGZ_NAME=${INSTALL_MONGODB_TOOLS_NAME}.tgz
INSTALL_MONGOSH_TGZ_NAME=${INSTALL_MONGOSH_NAME}.tgz
INSTALL_INSTALL_PATH=$1
INSTALL_SOURCE_PATH=${INSTALL_SCRIPT_PATH}

# 创建临时目录
rm -rf ${INSTALL_TEMP_DIR}
mkdir ${INSTALL_TEMP_DIR}

# 安装路径必须是绝对路径
if [ -d "${INSTALL_INSTALL_PATH}" ]; then
    INSTALL_INSTALL_PATH="$(cd ${INSTALL_INSTALL_PATH}; pwd)"
    echo "will install path:${INSTALL_INSTALL_PATH},  current path:$(pwd)"
else
    mkdir ${INSTALL_INSTALL_PATH}
    INSTALL_INSTALL_PATH="$(cd ${INSTALL_INSTALL_PATH}; pwd)"
    echo "created install path:${INSTALL_INSTALL_PATH}, current path:$(pwd)"
fi

# 拼接包路径
MONGODB_TGZ_PATH=${INSTALL_SOURCE_PATH}/${INSTALL_MONGODB_TGZ_NAME}
MONGODB_TOOLS_TGZ_PATH=${INSTALL_SOURCE_PATH}/${INSTALL_MONGODB_TOOLS_TGZ_NAME}
MONGOSH_TGZ_PATH=${INSTALL_SOURCE_PATH}/${INSTALL_MONGOSH_TGZ_NAME}

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
rm -rf ${INSTALL_INSTALL_PATH}
mkdir ${INSTALL_INSTALL_PATH}

# 目标包路径
TARGET_MONGO_PATH=${INSTALL_INSTALL_PATH}/${INSTALL_MONGODB_NAME}
TARGET_MONGODB_TOOLS_PATH=${INSTALL_INSTALL_PATH}/${INSTALL_MONGODB_TOOLS_NAME}
TARGET_MONGOSH_PATH=${INSTALL_INSTALL_PATH}/${INSTALL_MONGOSH_NAME}

# 解压包到指定路径
echo "unpack ${MONGODB_TGZ_PATH} to ${TARGET_MONGO_PATH}..."
tar -zxvf "${MONGODB_TGZ_PATH}" -C "${INSTALL_INSTALL_PATH}"
echo "unpack ${MONGODB_TGZ_PATH} to ${TARGET_MONGO_PATH} success."

echo "unpack ${MONGODB_TOOLS_TGZ_PATH} to ${TARGET_MONGODB_TOOLS_PATH}..."
tar -zxvf "${MONGODB_TOOLS_TGZ_PATH}" -C "${INSTALL_INSTALL_PATH}"
echo "unpack ${MONGODB_TOOLS_TGZ_PATH} to ${TARGET_MONGODB_TOOLS_PATH} success."

echo "unpack ${MONGOSH_TGZ_PATH} to ${TARGET_MONGOSH_PATH}..."
tar -zxvf "${MONGOSH_TGZ_PATH}" -C "${INSTALL_INSTALL_PATH}"
echo "unpack ${MONGOSH_TGZ_PATH} to ${TARGET_MONGOSH_PATH} success."

# 创建环境变量设置脚本, 并执行脚本
MONGO_PATH_EXPORT_SH=${INSTALL_INSTALL_PATH}/mongodb_export.sh
echo "MONGO_PATH_EXPORT_SH:${MONGO_PATH_EXPORT_SH}"

echo '#!/usr/bin/env bash' > ${MONGO_PATH_EXPORT_SH}
echo '# mongodb_export.sh' >> ${MONGO_PATH_EXPORT_SH}
echo '' >> ${MONGO_PATH_EXPORT_SH}
echo 'export PATH=$PATH:'"${TARGET_MONGODB_TOOLS_PATH}/bin" >> ${MONGO_PATH_EXPORT_SH}
echo 'export PATH=$PATH:'"${TARGET_MONGOSH_PATH}/bin" >> ${MONGO_PATH_EXPORT_SH}
echo 'export PATH=$PATH:'"${INSTALL_INSTALL_PATH}/mongodb-linux-x86_64-rhel88-8.0.6/bin" >> ${MONGO_PATH_EXPORT_SH}
chmod +x ${MONGO_PATH_EXPORT_SH}

# 添加到.bash_profile 
EXPORT_SCRIPT_CONTENT='. '${MONGO_PATH_EXPORT_SH}
echo "will add ${EXPORT_SCRIPT_CONTENT} to .bash_profile"
sed -i "/mongodb_export\.sh/d" ~/.bash_profile
echo "${EXPORT_SCRIPT_CONTENT}" >> ~/.bash_profile

# 执行脚本添加环境变量
echo "add export env to path..."
source ${MONGO_PATH_EXPORT_SH}

echo "install mongodb success target_path:${INSTALL_INSTALL_PATH}, enjoy!"
