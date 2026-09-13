# @file plugin_building.sh
# @author EricYonng<120453674@qq.com>
# @brief 3rd scripts
#!/usr/bin/env bash

# 路径
FILE_PATH=$1
# 文件名
BASE_NAME=$2
# 扩展名
EXTENSION_NAME=$3

# 时间戳
CUR_TIMESTAMP=$(date +%s)
echo "turn ${FILE_PATH}/${BASE_NAME}${EXTENSION_NAME} => ${FILE_PATH}/${BASE_NAME}.${CUR_TIMESTAMP}${EXTENSION_NAME}"

sudo mv -f ${FILE_PATH}/${BASE_NAME}${EXTENSION_NAME} ${FILE_PATH}/${BASE_NAME}.${CUR_TIMESTAMP}${EXTENSION_NAME}

echo "plugin building finish."