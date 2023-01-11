# @author EricYonng<120453674@qq.com>
# @brief run svr...
#!/usr/bin/env bash

SCRIPT_PATH="$(cd $(dirname $0); pwd)"
RUN_PATH=${SCRIPT_PATH}/output/gmake

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
		RUN_NAME="${RUN_NAME}_debug"
	fi
fi

echo "will run process:${RUN_NAME}"

nohup ${RUN_PATH}/${RUN_NAME} > ${RUN_PATH}/${RUN_NAME}.nohup 2>&1 &

echo "run process success."

