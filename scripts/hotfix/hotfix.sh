# @author EricYonng<120453674@qq.com>
# @brief hotfix, hotfix.sh testsuit debug TestPlugin libTestPlugin2.so
#!/usr/bin/env bash

SCRIPT_PATH="$(cd $(dirname $0); pwd)"
RUN_PATH=${SCRIPT_PATH}/../../output/gmake/build

if [ $# -lt 3 ]
then
	echo "lack of hotfix module name."
	exit 1
fi

# 进程名
RUN_NAME="testsuit"
if [ -n "$1" ]
then
	RUN_NAME=$1
fi

# 版本
if [ -n "$2" ]
then
VER="$2"
	if [ $VER = "debug" ]
	then
		RUN_NAME="${RUN_NAME}_debug"
	fi
fi

# key
HOTFIX_KEY=$3
# 模块名
HOTFIX_MODULE_NAME=$4

echo "RUN_NAME process:${RUN_NAME}"

GREP_FLAG="${RUN_NAME}"

IS_START=0

echo "search ${GREP_FLAG} process ..."

while [ $IS_START -eq 0 ]
do
  PID_LIST="$(ps -aux |grep ${GREP_FLAG} | sed '/grep/d' | sed '/hotfix/d' | sed '/linux_run/d' | sed '/start/d' | sed '/stop/d' | sed 's/^[^ ]* //' | sed 's/^ *//' | sed 's/ .*$//')"

  # PID_LIST=$(ps -e | grep ${GREP_FLAG} | awk '{print $1}')
  if [ -n "${PID_LIST}" ]
  then
    # 使用 for 循环遍历 PID_LIST
    for pid in $PID_LIST; do
        echo "FilePath:${RUN_PATH}/${HOTFIX_MODULE_NAME}" > ${RUN_PATH}/${RUN_NAME}.hotfix_${pid}.tmp
        echo "HotfixKey:${HOTFIX_KEY}" >> ${RUN_PATH}/${RUN_NAME}.hotfix_${pid}.tmp
        mv -f ${RUN_PATH}/${RUN_NAME}.hotfix_${pid}.tmp ${RUN_PATH}/${RUN_NAME}.hotfix_${pid}
        echo "hotfix finish run name:${RUN_NAME}, pid:${pid}, HOTFIX_KEY:${HOTFIX_KEY}, HOTFIX_MODULE_NAME:${HOTFIX_MODULE_NAME}"
    done

    IS_START=1
    exit 0
  else
     echo "${GREP_FLAG} process not exists"
     exit 1
  fi
done