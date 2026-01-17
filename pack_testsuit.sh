# @brief pack testsuit...
#!/usr/bin/env bash

SCRIPT_PATH="$(cd $(dirname $0); pwd)"

RUN_NAME=$2
if [ -z ${RUN_NAME} ]
then
    RUN_NAME=testsuit
fi

TAR_NAME=$1
if [ -z ${TAR_NAME} ]
then
    TAR_NAME=testsuit
fi

sh ${SCRIPT_PATH}/runfirstly_scripts.sh
sh ${SCRIPT_PATH}/linuxmakefile_build.sh
cd ${SCRIPT_PATH}/build/gmake/ && make testsuit config=release
sh ${SCRIPT_PATH}/install.sh release "" build

cp -Rf ${SCRIPT_PATH}/tools ${SCRIPT_PATH}/output/gmake/build/
cp -Rf ${SCRIPT_PATH}/start.sh ${SCRIPT_PATH}/output/gmake/build/
cp -Rf ${SCRIPT_PATH}/stop.sh ${SCRIPT_PATH}/output/gmake/build/
cp -Rf ${SCRIPT_PATH}/linux_run.sh ${SCRIPT_PATH}/output/gmake/build/

sudo sed -i 's/^RUN_PATH=.*/RUN_PATH=${SCRIPT_PATH}/' ${SCRIPT_PATH}/output/gmake/build/linux_run.sh
sudo sed -i "s/^RUN_NAME=\".*/RUN_NAME=\"${RUN_NAME}\"/" ${SCRIPT_PATH}/output/gmake/build/linux_run.sh
sudo sed -i "s/linux_run\.sh.*/linux_run\.sh ${RUN_NAME} \$1/" ${SCRIPT_PATH}/output/gmake/build/start.sh
sudo sed -i "s/tools\/CloseProcess.*is_waiting_close.*tools\/CloseProcess\/CloseProcess ${RUN_NAME} is_waiting_close=1/" ${SCRIPT_PATH}/output/gmake/build/stop.sh

cd ${SCRIPT_PATH}
rm -f ${SCRIPT_PATH}/output/gmake/build/${TAR_NAME}.tgz
cd ${SCRIPT_PATH}/output/gmake/build/ && tar -zcvf ${TAR_NAME}.tgz ./testsuit ./Cfgs/* ./ini/* ./libmysqlclient.so ./linux_run.sh ./start.sh ./stop.sh ./tools

