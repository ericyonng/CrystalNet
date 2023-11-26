# @author EricYonng<120453674@qq.com>
# @brief 3rd scripts
#!/usr/bin/env bash

# 路径
SCRIPT_PATH="$(cd $(dirname $0); pwd)"

# GREP_FLAG="CrystalNet"
# if [ $1 ]
# then
#     echo "param 1:$1"
#     GREP_FLAG=$1
# fi

# echo "will stop process with GREP_FLAG:${GREP_FLAG}"

# PID_LIST="$(ps -aux |grep ${GREP_FLAG} | sed '/grep/d' | sed '/stop/d' | sed 's/^[^ ]* //' | sed 's/^ *//' | sed 's/ .*$//')"

# echo "will stop PID_LIST:${PID_LIST}"

# for pid in $PID_LIST
# do
#     echo "stop process $pid"
#     kill 2 $pid
# done

# for pid in $PID_LIST
# do
#     echo "wait pid:${pid} close..."
#     while [ -n "$(ps -p $pid | sed '1d')" ]
#     do
#         echo "wait pid:${pid} close..."
#         sleep 1
#     done

#     echo "pid:${pid}, has closed."
# done

# echo "all process has closed."

sudo chmod a+x $SCRIPT_PATH/tools/CloseProcess/CloseProcess

${SCRIPT_PATH}/tools/CloseProcess/CloseProcess testsuit is_waiting_close=1

