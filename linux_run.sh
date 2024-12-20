# @author EricYonng<120453674@qq.com>
# @brief run svr...
#!/usr/bin/env bash

SCRIPT_PATH="$(cd $(dirname $0); pwd)"
RUN_PATH=${SCRIPT_PATH}/output/gmake

# 设置环境变量LD_LIBRARY_PATH以便启动时能够识别当前目录下的so
export LD_LIBRARY_PATH=${RUN_PATH}:${LD_LIBRARY_PATH}
echo "LD_LIBRARY_PATH:${LD_LIBRARY_PATH}"

if [ $# -lt 1 ]
then
	echo "lack of exe name."
	exit 1
fi

RUN_NAME="testsuit"
if [ -n "$1" ]
then
	RUN_NAME=$1
fi

if [ -n "$2" ]
then
VER="$2"
	if [ $VER = "debug" ]
	then
		RUN_NAME="${RUN_NAME}_debug"
	fi
fi

echo "will run process:${RUN_NAME}"

nohup ${RUN_PATH}/${RUN_NAME} > ${RUN_PATH}/${RUN_NAME}.nohup 2>&1 &

GREP_FLAG="${RUN_NAME}"

IS_START=0

echo "wait ${GREP_FLAG} start"

# 等待5秒
sleep 5

while [ $IS_START -eq 0 ]
do
  PID_LIST="$(ps -aux |grep ${GREP_FLAG} | sed '/grep/d' | sed '/linux_run/d' | sed '/start/d' | sed '/stop/d' | sed 's/^[^ ]* //' | sed 's/^ *//' | sed 's/ .*$//')"

  # PID_LIST=$(ps -e | grep ${GREP_FLAG} | awk '{print $1}')
  if [ -n "${PID_LIST}" ]
  then
      IS_START=1
      echo "${GREP_FLAG} started pids:${PID_LIST}."
  else
     sleep 1
     echo "wait ${GREP_FLAG} start"
  fi
done

echo "run process success."

