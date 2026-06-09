# @author EricYonng<120453674@qq.com>
# @brief hotfix, hotfix.sh testsuit debug TestPlugin libTestPlugin2.so
#!/usr/bin/env bash

SCRIPT_PATH="$(cd $(dirname $0); pwd)"
RUN_PATH=${SCRIPT_PATH}/../../output/gmake/build

PROG_PID=$1

sudo touch ${RUN_PATH}/hotfix_${PROG_PID}.cmd
chmod 644 hotfix_${PROG_PID}.cmd

echo "create hotfix_${PROG_PID}.cmd"
