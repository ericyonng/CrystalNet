
# @author EricYonng<120453674@qq.com>
# @brief 3rd scripts
#!/usr/bin/env bash

# 变量与等号之间不可以有空格，否则会被当成命令
SCRIPT_PATH="$(cd $(dirname $0); pwd)"
# DEBUG_LIBS=("libKernel_debug.so" )
# RELEASE_LIBS=("libKernel.so" )
# OPEN_COREDUMP="opencoredump"
# COREDUMPFLAG="$2"

# 配置环境变量
# sudo export PATH=$PATH:${SCRIPT_PATH}/output/gmake/
ROOT_PATH=${SCRIPT_PATH}/../..
OUTPUT_DIR=${ROOT_PATH}/output/gmake/build_tools
# sudo ln -sv $SCRIPT_PATH/Service/Cfgs ${OUTPUT_DIR}/Cfgs

if [ -n "$1" ]
then
VER="$1"
# sudo export PATH=$PATH:${SCRIPT_PATH}/output/gmake/

	if [ $VER = "debug" ]
	then
		sudo mkdir ${OUTPUT_DIR}../../3rd/kernel
	    sudo rm -f ${OUTPUT_DIR}../../3rd/kernel/libCrystalKernel_debug.a
		sudo cp -rf ${OUTPUT_DIR}libCrystalKernel_debug.a ${OUTPUT_DIR}../../3rd/kernel/libCrystalKernel_debug.a
		sudo mkdir ${OUTPUT_DIR}../../3rd/kernel
	    sudo rm -f ${OUTPUT_DIR}../../3rd/kernel/libCrystalKernel_debug.so
		sudo cp -rf ${OUTPUT_DIR}libCrystalKernel_debug.so ${OUTPUT_DIR}../../3rd/kernel/libCrystalKernel_debug.so

		# 拷贝mysqlclient.so到运行目录
		sudo cp -rf  ${ROOT_PATH}/3rd/mysql/linux/lib/libmysqlclient.so ${OUTPUT_DIR}/
		rm -f ${OUTPUT_DIR}/libmysqlclient.so.21
		ln -sv ${OUTPUT_DIR}/libmysqlclient.so ${OUTPUT_DIR}/libmysqlclient.so.21

		sudo cp -rf  ${ROOT_PATH}/3rd/mysql/linux/lib/libmysqlclient.so ${OUTPUT_DIR}/
		sudo cp -rf  ${ROOT_PATH}/3rd/miniz/libs/debug/libminiz.a ${OUTPUT_DIR}/
		sudo cp -rf  ${ROOT_PATH}/3rd/protobuf/lib/libprotobufd.a ${OUTPUT_DIR}/
		# sudo cp -rf  ${SCRIPT_PATH}/3rd/protobuf/lib/libprotobuf-lited.a ${OUTPUT_DIR}/
		# sudo cp -rf  ${SCRIPT_PATH}/3rd/protobuf/lib/libprotocd.a ${OUTPUT_DIR}/

		sh ${ROOT_PATH}/partition_files.sh

		# sudo ln -sv ${OUTPUT_DIR}libCrystalKernel_debug.so /usr/lib/libCrystalKernel_debug.so
		# 创建debug版本的so连接符号
		# for libName in $DEBUG_LIBS
		# do
		#	sudo rm -f /usr/lib64/$libName
		#	sudo ln -sv ${OUTPUT_DIR}/$libName /usr/lib64/$libName
		# done
	else
		sudo mkdir ${OUTPUT_DIR}../../3rd/kernel
	    sudo rm -f ${OUTPUT_DIR}../../3rd/kernel/libCrystalKernel.a
		sudo cp -rf ${OUTPUT_DIR}libCrystalKernel.a ${OUTPUT_DIR}../../3rd/kernel/libCrystalKernel.a
		sudo mkdir ${OUTPUT_DIR}../../3rd/kernel
	    sudo rm -f ${OUTPUT_DIR}../../3rd/kernel/libCrystalKernel.so
		sudo cp -rf ${OUTPUT_DIR}libCrystalKernel.so ${OUTPUT_DIR}../../3rd/kernel/libCrystalKernel.so

		# 拷贝mysqlclient.so到运行目录
		sudo cp -rf  ${ROOT_PATH}/3rd/mysql/linux/lib/libmysqlclient.so ${OUTPUT_DIR}/
		rm -f ${ROOT_PATH}/libmysqlclient.so.21
		ln -sv ${ROOT_PATH}/libmysqlclient.so ${OUTPUT_DIR}/libmysqlclient.so.21

		sudo cp -rf  ${ROOT_PATH}/3rd/miniz/libs/release/libminiz.a ${OUTPUT_DIR}/
		sudo cp -rf  ${ROOT_PATH}/3rd/protobuf/lib/libprotobuf.a ${OUTPUT_DIR}/
		# sudo cp -rf  ${SCRIPT_PATH}/3rd/protobuf/lib/libprotobuf-lite.a ${OUTPUT_DIR}/
		# sudo cp -rf  ${SCRIPT_PATH}/3rd/protobuf/lib/libprotoc.a ${OUTPUT_DIR}/

		sh ${ROOT_PATH}/partition_files.sh

		# sudo ln -sv ${OUTPUT_DIR}libCrystalKernel.so /usr/lib/libCrystalKernel.so
		# 创建release版本的so连接符号
		# for libName in $RELEASE_LIBS
		# do
		#	sudo rm -f /usr/lib64/$libName
		#	sudo ln -sv ${OUTPUT_DIR}/$libName /usr/lib64/$libName
		# done
	fi
fi

