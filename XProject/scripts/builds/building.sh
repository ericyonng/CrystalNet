# @file building.sh
# @author EricYonng<120453674@qq.com>
# @brief 3rd scripts
#!/usr/bin/env bash

# 变量与等号之间不可以有空格，否则会被当成命令
SCRIPT_PATH="$(cd $(dirname $0); pwd)"
# DEBUG_LIBS=("libKernel_debug.so" )
# RELEASE_LIBS=("libKernel.so" )
# OPEN_COREDUMP="opencoredump"
# COREDUMPFLAG="$2"
XPROJ_PATH=${SCRIPT_PATH}/../..
ROOT_PATH=${XPROJ_PATH}/..

OUTPUT_NAME="$2"

# 配置环境变量
# sudo export PATH=$PATH:${SCRIPT_PATH}/output/gmake/
OUTPUT_DIR=${XPROJ_PATH}/output/gmake/${OUTPUT_NAME}/
# sudo ln -sv $SCRIPT_PATH/Service/Cfgs ${OUTPUT_DIR}/Cfgs

if [ -n "$1" ]
then
VER="$1"
# sudo export PATH=$PATH:${SCRIPT_PATH}/output/gmake/

	if [ $VER = "debug" ]
	then
		sudo mkdir ${ROOT_PATH}/3rd/kernel
	    sudo rm -f ${ROOT_PATH}/3rd/kernel/libCrystalKernel_debug.so
		sudo cp -rf ${OUTPUT_DIR}libCrystalKernel_debug.so ${ROOT_PATH}/3rd/kernel/libCrystalKernel_debug.so

		sudo cp -rf  ${ROOT_PATH}/3rd/miniz/libs/debug/libminiz.a ${OUTPUT_DIR}/
		sudo cp -rf  ${ROOT_PATH}/3rd/protobuf/lib/libprotobufd.a ${OUTPUT_DIR}/
	else
		sudo mkdir ${ROOT_PATH}/3rd/kernel
	    sudo rm -f ${ROOT_PATH}/3rd/kernel/libCrystalKernel.so
		sudo cp -rf ${OUTPUT_DIR}libCrystalKernel.so ${ROOT_PATH}/3rd/kernel/libCrystalKernel.so

		sudo cp -rf  ${ROOT_PATH}/3rd/miniz/libs/release/libminiz.a ${OUTPUT_DIR}/
		sudo cp -rf  ${ROOT_PATH}/3rd/protobuf/lib/libprotobuf.a ${OUTPUT_DIR}/
	fi
fi

