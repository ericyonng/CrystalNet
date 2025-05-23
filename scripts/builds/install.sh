# @file install.sh
# @author EricYonng<120453674@qq.com>
# @brief install so...
#!/usr/bin/env bash

# 变量与等号之间不可以有空格，否则会被当成命令
SCRIPT_PATH="$(cd $(dirname $0); pwd)"
DEBUG_LIBS=("libCrystalKernel_debug.so")
RELEASE_LIBS=("libCrystalKernel.so")
OPEN_COREDUMP="opencoredump"
COREDUMPFLAG="$2"
ROOT_PATH=${SCRIPT_PATH}/../..
OUTPUT_DIR=${ROOT_PATH}/output/gmake/

# 连接配置与ini
sudo rm -rf ${OUTPUT_DIR}/Cfgs
sudo rm -rf ${OUTPUT_DIR}/ini
sudo ln -sv ${ROOT_PATH}/service/TestService/config/data/cpp ${OUTPUT_DIR}/Cfgs
sudo ln -sv ${ROOT_PATH}/doc/ini ${OUTPUT_DIR}/ini

# 拷贝mysqlclient.so到运行目录
sudo cp -rf  ${ROOT_PATH}/3rd/mysql/linux/lib/libmysqlclient.so ${OUTPUT_DIR}/
rm -f ${OUTPUT_DIR}/libmysqlclient.so.21
ln -sv ${OUTPUT_DIR}/libmysqlclient.so ${OUTPUT_DIR}/libmysqlclient.so.21

# sasl2库
sudo cp -rf  ${SCRIPT_PATH}/../../3rd/sasl2/libs/libsasl2.so ${OUTPUT_DIR}/

# 动态库连接
if [ -n "$1" ]
then
VER="$1"
	# if [ $VER = "debug" ]
	# then
	# 	# 创建debug版本的so连接符号
	# 	for libName in $DEBUG_LIBS
	# 	do
	# 		sudo rm -f /usr/lib64/$libName
	# 		sudo ln -sv ${OUTPUT_DIR}/$libName /usr/lib64/$libName
	# 		sudo rm -f /usr/lib/$libName
	# 		sudo ln -sv ${OUTPUT_DIR}/$libName /usr/lib/$libName
	# 	done
	# else
	# 	# 创建release版本的so连接符号
	# 	for libName in $RELEASE_LIBS
	# 	do
	# 		sudo rm -f /usr/lib64/$libName
	# 		sudo ln -sv ${OUTPUT_DIR}/$libName /usr/lib64/$libName
	# 		sudo rm -f /usr/lib/$libName
	# 		sudo ln -sv ${OUTPUT_DIR}/$libName /usr/lib/$libName
	# 	done
	# fi
	
	# 开启coredump
#	echo "set tmp unlimited"
#    ulimit -c unlimited
#	SET_PATTEN="${OUTPUT_DIR}/core_%e_time%t_pid%p_sinal%s"
#	CORE_PATTERN_PATH="/proc/sys/kernel/core_pattern"
#	echo "set tmp coredump format"
#	if grep -qE ".*${SET_PATTEN}.*" ${CORE_PATTERN_PATH}
#	then
#	    echo "core dump name format is already set"
#	else
#	    echo "will set core dump name format"
#		echo "${SET_PATTEN}" > ${CORE_PATTERN_PATH}
#	fi
	
	# 参数设置
	LIMITS_CONF_PATH="/etc/security/limits.conf"
	# SET_PATTEN="${OUTPUT_DIR}core_%e_timestamp_%t_pid_%p_signal_%s" 
	SET_PATTEN="./core_%e_timestamp_%t_pid_%p_signal_%s"  
	SYSCTL_CONF_PATH="/etc/sysctl.conf"
	
	# coredump设置永久生效
	if [ -n "$2" ]
	then
		if [ $COREDUMPFLAG = $OPEN_COREDUMP ]
		then
			# 设置无限制
			echo "set forever unlimited"
			if grep -qE ".*@root soft core unlimited.*" ${LIMITS_CONF_PATH}
			then
				echo "it is already setted unlimited forever"
			else
				echo "will set unlimited forever"
				sudo echo -e "@root soft core unlimited \n@root hard core unlimited\n" >> ${LIMITS_CONF_PATH}
			fi
			
			# 设置cordump 系统级别	 
			echo "set forever coredump format"
			if grep -qE ".*${SET_PATTEN}.*" ${SYSCTL_CONF_PATH}
			then
				echo "core dump format is already setted forever"
			else
				echo "will set core dump format forever"
				sudo echo -e "kernel.core_pattern=${SET_PATTEN}\nkernel.core_uses_pid=0\n" >> ${SYSCTL_CONF_PATH}
				# sysctl -p
			fi
		fi
	fi	
	
	# 修改当前系统最大文件描述符打开数量
	echo "modify file describe quantity"
	sudo sed -i "/hard nofile/d" ${LIMITS_CONF_PATH}
	sudo sed -i "/soft nofile/d" ${LIMITS_CONF_PATH}
	sudo echo -e "* hard nofile 1024000 \n* soft nofile 1024000 \n" >> ${LIMITS_CONF_PATH}
	
	# 修改系统级的最大文件描述符数量
	echo "modify file describe quantity at system level"	
	sudo sed -i "/fs.file-max/d" ${SYSCTL_CONF_PATH}
	sudo echo -e "fs.file-max=1024000 \n" >> ${SYSCTL_CONF_PATH}	

	# 扩展随机端口范围
	sudo echo -e "net.ipv4.ip_local_port_range = 1024 65535\n" >> ${SYSCTL_CONF_PATH}
	# 生效
	sudo sysctl -p
else
    echo "install with no params please check!"
fi






