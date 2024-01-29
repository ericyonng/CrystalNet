# @brief pack testsuit...
#!/usr/bin/env bash

SCRIPT_PATH="$(cd $(dirname $0); pwd)"

TAR_NAME=$1
if [ -z ${TAR_NAME} ]
then
    TAR_NAME=client
fi

sh ${SCRIPT_PATH}/runfirstly_scripts.sh
sh ${SCRIPT_PATH}/linuxmakefile_build.sh
cd ${SCRIPT_PATH}/build/gmake/ && make client config=release
sh ${SCRIPT_PATH}/install.sh release

cp -Rf ${SCRIPT_PATH}/tools ${SCRIPT_PATH}/output/gmake/
cp -Rf ${SCRIPT_PATH}/start.sh ${SCRIPT_PATH}/output/gmake/
cp -Rf ${SCRIPT_PATH}/stop.sh ${SCRIPT_PATH}/output/gmake/
cp -Rf ${SCRIPT_PATH}/linux_run.sh ${SCRIPT_PATH}/output/gmake/

cd ${SCRIPT_PATH}
rm -f ${SCRIPT_PATH}/output/gmake/${TAR_NAME}.tgz
cd ${SCRIPT_PATH}/output/gmake && tar -zcvf ${TAR_NAME}.tgz ./client ./Cfgs/* ./ini/* ./linux_run.sh ./start.sh ./stop.sh ./tools

