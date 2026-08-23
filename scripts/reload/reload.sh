# @author EricYonng<120453674@qq.com>
# @brief hotfix, 热更配置 hotfix.sh pid
#!/usr/bin/env bash

SCRIPT_PATH="$(cd $(dirname $0); pwd)"
RUN_PATH=${SCRIPT_PATH}/../../output/gmake/build

PROG_PID=$1

sudo touch ${RUN_PATH}/reload_${PROG_PID}.cmd
chmod 644 ${RUN_PATH}/reload_${PROG_PID}.cmd

echo "create reload_${PROG_PID}.cmd"
