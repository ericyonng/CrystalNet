# @file stop_perf.sh
# @author EricYonng<120453674@qq.com>
# @brief stop perf...
#!/usr/bin/env bash

# 脚本路径
SCRIPT_PATH="$(cd $(dirname $0); pwd)"

TARGET_PIDS=$(ps -e | grep "perf record" | awk '{print $1}')

# TARGET_PIDS="$(ps -aux |grep "perf record" | sed '/grep/d' | sed '/start_perf/d' | sed '/stop_perf/d' | sed 's/^[^ ]* //' | sed 's/^ *//' | sed 's/ .*$//')"

for pid in $TARGET_PIDS
do
    EXE_PATH=$(ls -l /proc/${pid}/exe)
    echo "kill perf record process:${pid} path:${EXE_PATH}"
    kill $pid

    while [ -n "$(ps -p $pid | sed '1d')" ]
    do
        echo "wait pid:${pid} close..."
        sleep 1
    done

    # 必须等到perf record退出才可以继续后面流程，避免后面流程失败
    echo "pid:${pid}, has closed."
done


# 生成svg
PERF_PATH=${SCRIPT_PATH}/perf_dir/

IS_EXISTS_PATH=$(ls ${SCRIPT_PATH} | grep 'perf_dir')
if [ -z "${IS_EXISTS_PATH}" ]
then
    echo "have no perf dir will stop gen svg"
    exit 1
else
    echo "exist perf dir:${PERF_PATH}"
fi

if [ -z $(ls ${PERF_PATH} | grep "^perf_record_.*") ]
then
    echo "have no any perf record before."
    exit 1
fi

# FLAME_GRAPH_PATH
FLAME_GRAPH_PATH=~/FlameGraphsClone/FlameGraph
if [ -z "$(ls ~ |grep 'FlameGraphsClone')" ]
then
    echo "FlameGraphsClone not exist in ~"
    exit 1
fi
if [ -z "$(ls ~/FlameGraphsClone/ |grep 'FlameGraph')" ]
then
    echo "FlameGraph not exist in ~/FlameGraphsClone"
    exit 1
fi

PERF_RECORD_SUB_DIR_LIST=$(ls ${PERF_PATH} |grep "^perf_record_.*")

for sub_dir in $PERF_RECORD_SUB_DIR_LIST
do
    TOTAL_DIR=${PERF_PATH}/${sub_dir}
    echo "TOTAL_DIR:${TOTAL_DIR}, sub_dir:${sub_dir}"
    echo "perf script -i ${TOTAL_DIR}/perf.data &> ${TOTAL_DIR}/${sub_dir}.unfold"

    # 对perf.data进行解析
    perf script -i ${TOTAL_DIR}/perf.data &> ${TOTAL_DIR}/${sub_dir}.unfold

    # 折叠
    echo "perl ${FLAME_GRAPH_PATH}/stackcollapse-perf.pl ${TOTAL_DIR}/${sub_dir}.unfold &> ${TOTAL_DIR}/${sub_dir}.folded"

    perl ${FLAME_GRAPH_PATH}/stackcollapse-perf.pl ${TOTAL_DIR}/${sub_dir}.unfold &> ${TOTAL_DIR}/${sub_dir}.folded

    # 生成svg图
    echo "perl ${FLAME_GRAPH_PATH}/flamegraph.pl ${TOTAL_DIR}/${sub_dir}.folded > ${TOTAL_DIR}/${sub_dir}.svg"
    perl ${FLAME_GRAPH_PATH}/flamegraph.pl ${TOTAL_DIR}/${sub_dir}.folded > ${TOTAL_DIR}/${sub_dir}.svg

    # 转移到根目录
    mv -f ${TOTAL_DIR}/${sub_dir}.svg ${PERF_PATH}/
done

# 展示svg路径
ls ${PERF_PATH} | grep svg

