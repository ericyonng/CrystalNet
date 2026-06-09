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
PACKAGE_TGZ_FILE_NAME=$(basename "${TGZ_FILE_PATH}")

echo "创建目录: INIT_TARGET_SCRIPT_PATH:${INIT_TARGET_SCRIPT_PATH} ..."

# 内联 safe_rm_rf 函数（此脚本可能被单独 scp 到远程执行，无法依赖外部 common/funcs.sh）
safe_rm_rf() {
    local target_path="$1"
    if [ -z "${target_path}" ]; then
        echo "错误: safe_rm_rf 拒绝删除空路径!" >&2
        return 1
    fi
    local normalized_path="${target_path%/}"
    local dangerous_paths="/ /root /home /etc /usr /var /opt /boot /sys /proc /bin /sbin /lib /tmp"
    for dp in ${dangerous_paths}; do
        if [ "${normalized_path}" = "${dp}" ]; then
            echo "错误: safe_rm_rf 拒绝删除危险路径: ${target_path}!" >&2
            return 1
        fi
    done
    local depth=$(echo "${normalized_path}" | tr -cd '/' | wc -c)
    if [ ${depth} -lt 2 ]; then
        echo "错误: safe_rm_rf 拒绝删除路径(深度不足): ${target_path}!" >&2
        return 1
    fi
    echo "safe_rm_rf: 即将删除 ${target_path}"
    rm -rf "${target_path}"
    return $?
}

safe_rm_rf "${INIT_TARGET_SCRIPT_PATH}" || {
    echo "错误： 移除 ${INIT_TARGET_SCRIPT_PATH} 失败" >&2
    exit 1
}

mkdir -p "${INIT_TARGET_SCRIPT_PATH}" || {
    echo "错误： 创建 ${INIT_TARGET_SCRIPT_PATH} 失败" >&2
    exit 1
}

echo "拷贝压缩文件 ${TGZ_FILE_PATH} =>  ${INIT_TARGET_SCRIPT_PATH} ..."
cp -Rf -r "${TGZ_FILE_PATH}" "${INIT_TARGET_SCRIPT_PATH}" || {
    echo "错误： scp 拷贝 ${TGZ_FILE_PATH} => ${INIT_TARGET_SCRIPT_PATH} 失败" >&2
    exit 1
}

echo "解压  ${INIT_TARGET_SCRIPT_PATH}/${PACKAGE_TGZ_FILE_NAME}..."
tar -zxvf "${INIT_TARGET_SCRIPT_PATH}/${PACKAGE_TGZ_FILE_NAME}" -C "${INIT_TARGET_SCRIPT_PATH}" || {
    echo "错误： 解压 拷贝 ${INIT_TARGET_SCRIPT_PATH}/${PACKAGE_TGZ_FILE_NAME} 失败" >&2
    exit 1
}

echo "init_package success INIT_TARGET_SCRIPT_PATH:${INIT_TARGET_SCRIPT_PATH}, TGZ_FILE_NAME:${PACKAGE_TGZ_FILE_NAME}"
