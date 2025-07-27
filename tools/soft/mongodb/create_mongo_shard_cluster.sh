#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 创建mongo 分片集群
# create_mongo_shard_cluster.sh 复制集名 复制集公网ip 主节点端口号 用户名 密码 复制集安装目标路径
# create_mongo_shard_cluster.sh IP列表文本文件 目标机器工作路径(数据库运行路径) 目标机器安装路径(数据库安装路径)
# 先初始化各个节点环境 => 启动config/shard各个节点脚本创建3节点复制集 => 创建mongos => 测试连通性 

# 当前脚本路径
SCRIPT_PATH="$(cd $(dirname $0); pwd)"

# 参数
IP_LIST_FILE=$1
WORK_PATH=$2
INSTALL_PATH=$3
TARGET_USER=$4
TARGET_PWD=$5
REPLISET_INSTALL_PATH=$6

# 校验参数
if [ -e ${IP_LIST_FILE} ]; then
    echo "IP_LIST_FILE:${IP_LIST_FILE} exist"
else
    echo "IP_LIST_FILE:${IP_LIST_FILE} not exist please check"
fi

if [ -z "${WORK_PATH}" ]; then
    echo "WORK_PATH is empty please check!!!"
    exit 1
fi
if [ -z "${INSTALL_PATH}" ]; then
    echo "INSTALL_PATH is empty please check!!!"
    exit 1
fi
if [ -z "${TARGET_USER}" ]; then
    echo "TARGET_USER is empty please check!!!"
    exit 1
fi
if [ -z "${TARGET_PWD}" ]; then
    echo "TARGET_PWD is empty please check!!!"
    exit 1
fi

# 加载公共
. ${SCRIPT_PATH}/common/common_define.sh
. ${SCRIPT_PATH}/common/funcs.sh

# 从文件读取ip列表
IP_LIST_ARRAY=()
read_file_to_array ${IP_LIST_FILE} IP_LIST_ARRAY || {
    echo "错误： read_file_to_array fail IP_LIST_FILE: ${IP_LIST_FILE} 失败" >&2
    exit 1
}
# ip列表打印
for i in "${!IP_LIST_ARRAY[@]}"; do
    echo "索引 $i: ${IP_LIST_ARRAY[$i]}"
done

# 判断是否为空
if [ ${#IP_LIST_ARRAY[@]} -eq 0 ]; then
    echo "IP_LIST_ARRAY ip列表是空的"
    exit 1
fi

# 本机公共ip
LOCAL_IP="127.0.0.1"
if check_internet; then
    LOCAL_IP=$(get_public_ip)
    echo "外网连接正常 LOCAL_IP:${LOCAL_IP}"
else
    LOCAL_IP="127.0.0.1"
    echo "无法连接外网 LOCAL_IP:${LOCAL_IP}"
fi

# 检查 OpenSSL 是否已安装
if ! command -v openssl &> /dev/null; then
    echo "当前环境未安装openssl正在安装 OpenSSL..."
    if sudo yum install openssl -y &> /dev/null; then
        echo "OpenSSL 安装成功！"
    else
        echo "安装失败，请手动执行：sudo yum install openssl -y"
        exit 1
    fi
fi

# 1.打包脚本
TMP_DIR=/root/build_mongo_temp

# 生成keyfile
KEYFILE_PATH=${SCRIPT_PATH}/keyfile
openssl rand -base64 756 > ${KEYFILE_PATH}

echo "创建keyfile 成功 ${KEYFILE_PATH}"

TGZ_FILE_NAME=mongodb.tar.gz
sh ${SCRIPT_PATH}/pack_tar.sh ${TMP_DIR} ${SCRIPT_PATH} ${TGZ_FILE_NAME} || {
    echo "错误： pack_tar.sh fail ${TMP_DIR} ${TGZ_FILE_NAME} 失败" >&2
    exit 1
}

# 初始化环境
NODE_CONFIG_ARR=()
COUNT=0
declare -A is_ip_init_dict
for index in "${!IP_LIST_ARRAY[@]}"; do
    # ip file 一行的数据: DATA ip
    elem="${IP_LIST_ARRAY[$index]}"
    echo "elem:${elem}"

    # 过滤空行
    if [ -z "${elem}" ] || [[ "$elem" =~ ^[[:space:]]*$ ]]; then
        continue
    fi
    fields=($(echo "${elem}" | awk '{print $1, $2, $3}'))
    ip="${fields[1]}"
    node_port="${fields[2]}"
    NODE_CONFIG_ARR[$COUNT]="${fields[0]} ${fields[1]} ${fields[2]}"
    echo "第 $index 个 IP 地址: elem:${elem}, $ip, $node_port, $fields[0] COUNT:{$COUNT}, ${NODE_CONFIG_ARR[$COUNT]}"
    COUNT=$(($COUNT + 1))

    # 本地机器, 则不需要远程拷贝
    if [ ${ip} = "127.0.0.1" ] || [ ${ip} = ${LOCAL_IP} ]; then
        echo "init_package ..."
        sh ${SCRIPT_PATH}/init_package.sh /root/build_mongo_temp/mongodb.tar.gz ${WORK_PATH} ${INSTALL_PATH}

        if [ -z "${is_ip_init_dict[$ip]}" ]; then
            sh ${SCRIPT_PATH}/init_env.sh ${WORK_PATH} ${INSTALL_PATH} || {
                echo "错误：本地:$ip ${TMP_DIR}/init_env.sh 失败" >&2
                exit 1
            }
            is_ip_init_dict[$ip]=1
        fi
    else
        if [ -z "${is_ip_init_dict[$ip]}" ]; then
            echo "${ip}: 创建目录: TMP_DIR:${TMP_DIR} ..."
            ssh root@${ip} "rm -rf ${TMP_DIR}" || {
                echo "错误： 移除 ${TMP_DIR} 失败" >&2
                exit 1
            }
            ssh root@${ip} "mkdir -p ${TMP_DIR}" || {
                echo "错误：${ip} 创建 ${TMP_DIR} 失败" >&2
                exit 1
            }

            echo "拷贝压缩文件 ${TMP_DIR}/${TGZ_FILE_NAME} =>  ${ip}:${TMP_DIR} ..."
            scp -r ${TMP_DIR}/${TGZ_FILE_NAME} root@${ip}:${TMP_DIR} || {
                echo "错误： scp 拷贝 ${TMP_DIR}/${TGZ_FILE_NAME} => ${ip}:${TMP_DIR} 失败" >&2
                exit 1
            }

            echo "拷贝init_env.sh =>  ${ip}:${TMP_DIR} ..."
            scp -r ${SCRIPT_PATH}/init_package.sh root@${ip}:${TMP_DIR} || {
                echo "错误： scp 拷贝 ${SCRIPT_PATH}/init_package.sh => ${ip}:${TMP_DIR} 失败" >&2
                exit 1
            }
            echo "$ip 执行 init_package.sh => ..."
            ssh root@${ip} "sh ${TMP_DIR}/init_package.sh ${TMP_DIR}/${TGZ_FILE_NAME} ${WORK_PATH} ${INSTALL_PATH}" || {
                echo "错误：${ip} ${TMP_DIR}/init_package.sh 失败" >&2
                exit 1
            }
            ssh root@${ip} "sh ${WORK_PATH}/init_env.sh ${WORK_PATH} ${INSTALL_PATH}" || {
                echo "错误：${ip} ${WORK_PATH}/init_env.sh 失败" >&2
                exit 1
            }
            is_ip_init_dict[$ip]=1
            echo "$ip 执行 init_env.sh 成功"
        fi
    fi
done

echo "create_mongo_shard_cluster init env success."

echo "create_mongo_shard_cluster primary init..."
echo ""

# 获取主节点ip
CONFIG_SVR_PRIMARY=""
CONFIG_SVR_PRIMARY_PORT=""
MONGOD_SVR_PRIMARY=""
MONGOD_SVR_PRIMARY_PORT=""
MONGOS_SVR_PRIMARY=""
MONGOS_SVR_PRIMARY_PORT=""

for index in "${!NODE_CONFIG_ARR[@]}"; do
    # ip file 一行的数据: DATA ip
    elem="${IP_LIST_ARRAY[$index]}"
    echo "NODE_CONFIG_ARR elem:${elem}"
    fields=($(echo "${elem}" | awk '{print $1, $2, $3}'))
    node_type="${fields[0]}"
    ip="${fields[1]}"
    node_port="${fields[2]}"
    echo "第 $index 个 IP 地址: elem:${elem}, $ip, ${node_type}, $fields[0]"
    
    if [ ${ip} = "127.0.0.1" ] || [ ${ip} = ${LOCAL_IP} ]; then
        if [ ${node_type} = "config" ]; then

            if [ -z "${CONFIG_SVR_PRIMARY}" ]; then
                CONFIG_SVR_PRIMARY=${ip}
                CONFIG_SVR_PRIMARY_PORT=${node_port}
            fi

        elif [ ${node_type} = "mongod" ]; then
            
            if [ -z "${MONGOD_SVR_PRIMARY}" ]; then
                MONGOD_SVR_PRIMARY=${ip}
                MONGOD_SVR_PRIMARY_PORT=${node_port}
            fi

        elif [ ${node_type} = "mongos" ]; then

            if [ -z "${MONGOS_SVR_PRIMARY}" ]; then
                MONGOS_SVR_PRIMARY=${ip}
                MONGOS_SVR_PRIMARY_PORT=${node_port}
            fi

        else
            echo "bad node_type:${node_type}"
            exit 1
        fi
    else
    
    fi
done

echo "CONFIG_SVR_PRIMARY:${CONFIG_SVR_PRIMARY}:${CONFIG_SVR_PRIMARY_PORT}"
echo "MONGOD_SVR_PRIMARY:${MONGOD_SVR_PRIMARY}:${MONGOD_SVR_PRIMARY_PORT}"
echo "MONGOS_SVR_PRIMARY:${MONGOS_SVR_PRIMARY}:${MONGOS_SVR_PRIMARY_PORT}"

if [ -z "${CONFIG_SVR_PRIMARY}" ] || [ -z "${CONFIG_SVR_PRIMARY_PORT}" ] || [ -z "${MONGOD_SVR_PRIMARY}" ] || [ -z "${MONGOD_SVR_PRIMARY_PORT}" ] || [ -z "${MONGOS_SVR_PRIMARY}" ] || [ -z "${MONGOS_SVR_PRIMARY_PORT}" ]; then
    echo "lack primary info "
    exit 1
fi

echo "create_mongo_shard_cluster init primary nodes..."

# TODO:初始化组节点
sh ${SCRIPT_PATH}/init_primary.sh ${TARGET_USER} ${TARGET_PWD} ${REPLISET_INSTALL_PATH}

# TODO:启动主节点
# TODO:启动configsvr的复制集
# TODO:启动数据节点复制集
# TODO:在mongos的主节点 添加分片addShare
# 分片键可以在程序中自动创建, 或者另外启一个脚本来创建分片键, 不宜在程序中创建, 因为会导致并发

for index in "${!NODE_CONFIG_ARR[@]}"; do
    # ip file 一行的数据: DATA ip
    elem="${IP_LIST_ARRAY[$index]}"
    fields=($(echo "${elem}" | awk '{print $1, $2, $3}'))
    node_type="${fields[0]}"
    ip="${fields[1]}"
    node_port="${fields[2]}"
    echo "第 $index 个 IP 地址: elem:${elem}, $ip, ${node_type}, ${node_port}"
    
done

echo "create_mongo_shard_cluster create nodes success."


